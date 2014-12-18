#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>

#include "mh_sampler.h"
#include "infer.h"
#include "../debug.h"
#include "../config.h"
#include "rejection.h"
#include "execute.h"

#include "../common/mem_profile.h"

unsigned g_mh_sampler_burn_in_iterations = 200;
unsigned g_mh_sampler_lag = 20;
unsigned g_mh_sampler_maximum_initial_round = 200;

void set_mh_burn_in(int burn_in) {	
	g_mh_sampler_burn_in_iterations = burn_in;
}

void set_mh_lag(int lag) {
	g_mh_sampler_lag = lag;
}

void set_mh_max_initial_round(int initial_round) {
	g_mh_sampler_maximum_initial_round = initial_round;
}

#define STACK_DEFAULT_VALUE 0
#define STACK_DESTROY_VALUE(value)
DEFINE_STACK(loop_index, unsigned)
#undef STACK_DEFAULT_VALUE
#undef STACK_DESTROY_VALUE

static PPP_INLINE unsigned bkdr_hash(const char* str) {
	unsigned hash = 0;
	for ( ; *str != '\0'; ++str) {
		hash = (hash * 131 + *str);
	}
	return hash;
}
#define HASH_TABLE_PREFIX mh_sampling_erp_hash_table
#define HASH_TABLE_KEY_TYPE char*
#define HASH_TABLE_VALUE_TYPE mh_sampling_sample_t*
#define HASH_TABLE_DEFINE_STRUCT 1
#define HASH_TABLE_VALUE_DEFAULT_VALUE 0
#define HASH_TABLE_HASH_FUNCTION(key) (bkdr_hash(key))
#define HASH_TABLE_KEY_COMPARATOR(key1, key2) (!strcmp(key1, key2))
#define HASH_TABLE_KEY_CLONE(var, key) var = strdup(key)
#define HASH_TABLE_VALUE_CLONE(var, value) var = mh_sampling_sample_clone(value)
#define HASH_TABLE_KEY_DESTRUCTOR(key) free(key)
#define HASH_TABLE_VALUE_DESTRUCTOR(value) mh_sampling_sample_destroy(value)
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) \
	dump_draw_stmt_impl(buffer, buf_size, (DrawStmtNode*) mh_sampling_name_to_node(key))
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, val)	\
	pp_variable_dump(buffer, buf_size, (val)->value)
#ifdef ENABLE_MEM_PROFILE
#define HASH_TABLE_ALLOC(type, count) PROFILE_MEM_ALLOC(type, count)
#define HASH_TABLE_DEALLOC(type, ptr, count) PROFILE_MEM_FREE(type, ptr, count)
#endif
#include "../common/hash_table.h"

int mh_sampling(
		struct pp_state_t* state, 
		const char* model_name, 
		pp_variable_t* param[], 
		pp_query_t* query, 
		void** internal_data_ptr, 
		pp_trace_t** trace_ptr,
		sample_acceptor sa,
		void *sa_data
		)
{

	mh_sampler_t* mh_sampler = (mh_sampler_t*)(*internal_data_ptr);
	//ERR_OUTPUT("entering mh_sampling\n");

	/* clean up internal data */
	if (!state) {
		//ERR_OUTPUT("clearing internal data\n");
		mh_sampler_destroy(mh_sampler);
		*internal_data_ptr = 0;
		return PP_SAMPLE_FUNCTION_NORMAL;
	}

	/* first invocation or invocation on other model */
	if (!mh_sampler || !mh_sampler_has_same_model(mh_sampler, state, model_name, param, query)) {
		//ERR_OUTPUT("creating internal data\n");
		mh_sampler = new_mh_sampler(state, model_name, param, query);
		*internal_data_ptr = mh_sampler;
		//FIXME what if !mh_sampler_has_same_model ?? possible memory leak!!


		ERR_OUTPUT("sampling initial trace\n");
		int status = mh_sampler_init_trace(mh_sampler);
		if (status != 0) {
			pp_sample_error_return(status, "");	
		}
	
		/*
		 *printf("\nafter init:\n");
		 *static char buffer[8000];
		 *pp_trace_dump(buffer, 8000, (pp_trace_t*) mh_sampler->current_trace);
		 *printf("%s\n", buffer);
		 */

		for (unsigned i = 0; i != g_mh_sampler_burn_in_iterations; ++i) {
			ERR_OUTPUT("burn-in step %d\n", i);
			int status = mh_sampler_step(mh_sampler);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}
		}
	}

	/* run mcmc */
	for (unsigned i = 0; i != g_mh_sampler_lag; ++i) {
		ERR_OUTPUT("step, %d\n", i);
		int status = mh_sampler_step(mh_sampler);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	/*static char buffer[8000];
	pp_trace_dump(mh_sampler->current_trace, buffer, 8000);
	printf("%s\n"); */
	sa(sa_data, (pp_trace_t*) mh_sampler->current_trace);

	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}


mh_sampler_t* new_mh_sampler(pp_state_t* state, const char* name, pp_variable_t* param[], pp_query_t* query) {
	mh_sampler_t* mh_sampler = malloc(sizeof(mh_sampler_t));

	mh_sampler->state = state;
	mh_sampler->model_name = strdup(name);
	mh_sampler->model = 0;
	mh_sampler->param = param;	// FIXME param may need deep copy
	mh_sampler->query = query;

	mh_sampler->loop_index = new_loop_index_stack(0x10);
	mh_sampler->current_trace = 0;

	return mh_sampler;
}

int mh_sampler_has_same_model(mh_sampler_t* mh_sampler, pp_state_t* state, const char* name, pp_variable_t* param[], pp_query_t* query) {
	// FIXME check whether the parameters and the query both match
	return (state == mh_sampler->state) && (!strcmp(name, mh_sampler->model_name));
}

int mh_sampler_init_trace(mh_sampler_t* mh_sampler) {
	/* find the model */
	ModelNode* model = model_map_find(mh_sampler->state->model_map, mh_sampler->state->symbol_table, mh_sampler->model_name);
	if (!model) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND, ": %s", mh_sampler->model_name);
	}
	mh_sampler->model = model;

	unsigned round_ = 0;
	unsigned max_initial_round = g_mh_sampler_maximum_initial_round;
	/* get initial trace */
	{
		
		/* initialize trace */
		mh_sampling_trace_t* trace = new_mh_sampling_trace(node_symbol_table(model));

		/* set up parameters */
		ModelParamsNode* param_node = model->params;
		pp_variable_t** param = mh_sampler->param;
		while (param_node) {
			pp_trace_set_variable_s((pp_trace_t*) trace, param_node->name, *(param++));
			param_node = param_node->model_params;
		}

		/* ignore the declarations */

		/* execute statements */
		StmtsNode* stmts = model->stmts;
		while (stmts) {
			StmtNode* stmt = stmts->stmt;

			int status = mh_sampler_execute_stmt(mh_sampler, stmt, (pp_trace_t*) trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				mh_sampling_trace_destroy(trace);
				pp_sample_error_return(status, "");
			}
			stmts = stmts->stmts;
		}

		mh_sampler->current_trace = trace;
	}

	++round_;
	{
		/* check conditions */
		int acc_result = mh_sampler->query->accept(mh_sampler->query, (pp_trace_t*) mh_sampler->current_trace);
		if ( acc_result == 1){
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
		else if (acc_result == 0) {
			/* do nothing */
		}
		else {
			mh_sampling_trace_destroy(mh_sampler->current_trace);
			mh_sampler->current_trace = 0;
			pp_sample_error_return(PP_SAMPLE_FUNCTION_QUERY_ERROR, "");
		}
	}

	/* temporarily disable the mh_sampler's query object */
	pp_query_acceptor_t accept = mh_sampler->query->accept;
	mh_sampler->query->accept = pp_query_always_accept;

	/* run the following rounds with unconditional mh updates */
	for (; round_ < max_initial_round; ++round_) {
		int status = mh_sampler_step(mh_sampler);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			mh_sampler->query->accept = accept;
			pp_sample_error_return(status, "");
		}


		int acc_result = accept(mh_sampler->query, (pp_trace_t*) mh_sampler->current_trace);
		if ( acc_result == 1){
			mh_sampler->query->accept = accept;
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
		else if (acc_result == 0) {
			/* do nothing */
		}
		else {
			mh_sampling_trace_destroy(mh_sampler->current_trace);
			mh_sampler->current_trace = 0;
			mh_sampler->query->accept = accept;
			pp_sample_error_return(PP_SAMPLE_FUNCTION_QUERY_ERROR, "");
		}
	}
	
	mh_sampler->query->accept = accept;
	pp_sample_error_return(PP_SAMPLE_FUNCTION_MH_FAIL_TO_INITIALIZE, ": maximum initial round %d is reached", max_initial_round);
}

int mh_sampler_execute_draw_stmt(mh_sampler_t* sampler, DrawStmtNode* stmt, pp_trace_t* p_trace) {
	assert(p_trace->struct_size == sizeof(mh_sampling_trace_t));
	mh_sampling_trace_t* trace = (mh_sampling_trace_t*) p_trace;

	/* step run */
	if (sampler->current_trace) {
		const mh_sampling_name_t stmt_name = mh_sampling_get_name(stmt, sampler->loop_index);
		mh_sampling_erp_hash_table_node_t* old_erp_node = mh_sampling_erp_hash_table_find_node(sampler->current_trace->erp_hash_table, stmt_name);

		pp_variable_t* observed_value = 0;
		int status = mh_sampler_observe_value(sampler, stmt->variable, p_trace, &observed_value);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}

		if (observed_value) {
			int status = mh_sampling_use_observed_value(stmt, trace, observed_value);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}

			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}

		mh_sampling_erp_hash_table_node_t* new_erp_node = mh_sampling_erp_hash_table_get_node(trace->erp_hash_table, stmt_name);

		/* no previous sample found */
		if (!old_erp_node) {
			/* get new sample */
			int status = mh_sampling_get_new_sample(stmt, p_trace, &(new_erp_node->value));
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}

			sampler->ll_fresh += new_erp_node->value->logprob;
		}

		else {
			int status = mh_sampling_reuse_old_sample(stmt, trace, old_erp_node->value, &(new_erp_node->value));
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}
			/* rescoring has been done in mh_sampling_reuse_old_sample */
			sampler->ll_stale -= old_erp_node->value->logprob;
		}

		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}

	/* initialization run */
	else {
		pp_variable_t* observed_value = 0;
		int status = mh_sampler_observe_value(sampler, stmt->variable, p_trace, &observed_value);
		if (status != PP_SAMPLE_FUNCTION_NORMAL){
			pp_sample_error_return(status, "");
		}

		if (observed_value) {
			/* use the observed value and don't put it into the sample map */
			int status = mh_sampling_use_observed_value(stmt, trace, observed_value);	
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}

		const mh_sampling_name_t stmt_name = mh_sampling_get_name(stmt, sampler->loop_index);
		mh_sampling_sample_t** sample_ptr = &(mh_sampling_erp_hash_table_get_node(trace->erp_hash_table, stmt_name)->value);

		status = mh_sampling_get_new_sample(stmt, p_trace, sample_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}

		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}
}

int mh_sampler_observe_value(mh_sampler_t* sampler, VariableNode* variable, pp_trace_t* trace, pp_variable_t** result_ptr) {
	pp_query_t* query = sampler->query;
	symbol_table_t* symbol_table = node_symbol_table(variable);

	switch (variable->type) {
	case NAME_VAR: 
		{
			NameVarNode* name_var = (NameVarNode*) variable;
			const char* varname = symbol_table_get_name(symbol_table, name_var->name);
			
			*result_ptr = query->observe(query, varname);
		}
		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);

	case FIELD_VAR:
		pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		//break;
	case INDEX_VAR:
		{
			IndexVarNode* index_var = (IndexVarNode*) variable;
#define MH_SAMPLER_VARNAME_BUFFER_SIZE 1024
			char varname[MH_SAMPLER_VARNAME_BUFFER_SIZE];

			DUMP_START();
	
			/* array name */
			const char* arrname = symbol_table_get_name(symbol_table, index_var->name);
			DUMP(varname, MH_SAMPLER_VARNAME_BUFFER_SIZE, "%s", arrname);

			/* indecies */
			ExprSeqNode* expr_seq = ((IndexVarNode*) variable)->expr_seq;
			if (!expr_seq) {
				pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_EXPRESSION, "");
			}
			while (expr_seq) {

				ExprNode* expr = expr_seq->expr;
				pp_variable_t* sub = 0;
				int status = execute_expr(expr, trace, &sub);
				if (status != PP_SAMPLE_FUNCTION_NORMAL) {
					pp_sample_error_return(status, "");
				}
				if (sub->type != PP_VARIABLE_INT) {
					pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION, "");
				}
				int index = PP_VARIABLE_INT_VALUE(sub);
				pp_variable_destroy(sub);
				
				DUMP(varname, MH_SAMPLER_VARNAME_BUFFER_SIZE, "[%d]", index);

				expr_seq = expr_seq->expr_seq;
			}

			*result_ptr = query->observe(query, varname);
			
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int mh_sampler_execute_for_stmt(mh_sampler_t* mh_sampler, ForStmtNode* stmt, pp_trace_t* p_trace) {
	assert(p_trace->struct_size == sizeof(mh_sampling_trace_t));
	mh_sampling_trace_t* trace = (mh_sampling_trace_t*) p_trace;

	pp_variable_t** loop_var_ptr = pp_trace_get_variable_ptr_s((pp_trace_t*) trace, stmt->name);

	pp_variable_t* loop_var = 0;
	int status = execute_expr(stmt->start_expr, (pp_trace_t*) trace, &loop_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		pp_sample_error_return(status, "");
	}
	if (*loop_var_ptr) {
		pp_variable_destroy(*loop_var_ptr);
	}
	*loop_var_ptr = loop_var;
	if (loop_var->type != PP_VARIABLE_INT) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE, "");
	}

	pp_variable_t* end_var = 0;
	status = execute_expr(stmt->end_expr, (pp_trace_t*) trace, &end_var);
	if (status != PP_SAMPLE_FUNCTION_NORMAL) {
		pp_sample_error_return(status, "");
	}
	if (end_var->type != PP_VARIABLE_INT) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE, "");
	}

	for (; PP_VARIABLE_INT_VALUE(*loop_var_ptr) <= PP_VARIABLE_INT_VALUE(end_var); ++PP_VARIABLE_INT_VALUE(*loop_var_ptr)) {
		loop_index_stack_push(mh_sampler->loop_index, PP_VARIABLE_INT_VALUE(*loop_var_ptr));
		StmtsNode* stmts = stmt->stmts;
		while (stmts) {
			status = mh_sampler_execute_stmt(mh_sampler, stmts->stmt, (pp_trace_t*) trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_variable_destroy(end_var);
				loop_index_stack_pop(mh_sampler->loop_index);
				pp_sample_error_return(status, "");
			}
			stmts = stmts->stmts;
		}
		loop_index_stack_pop(mh_sampler->loop_index);
	}

	pp_variable_destroy(end_var);
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);	
}

int mh_sampler_execute_stmt(mh_sampler_t* mh_sampler, StmtNode* stmt, pp_trace_t* trace) {
	assert(trace->struct_size == sizeof(mh_sampling_trace_t));

	if (!stmt) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_INVALID_STATEMENT, "");
	}

	//ERR_OUTPUT("executing stmt:\n%s", dump_stmt(stmt));
	switch (stmt->type) {
	case DRAW_STMT:
		pp_sample_return(mh_sampler_execute_draw_stmt(mh_sampler, (DrawStmtNode*) stmt, trace));
	case LET_STMT:
		pp_sample_return(execute_let_stmt((LetStmtNode*) stmt, trace));
	case FOR_STMT:
		pp_sample_return(mh_sampler_execute_for_stmt(mh_sampler, (ForStmtNode*) stmt, trace));
	}

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int mh_sampler_step(mh_sampler_t* mh_sampler) {
	if (!mh_sampling_erp_hash_table_size(mh_sampler->current_trace->erp_hash_table)) {
		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);	// no randomness
	}

	mh_sampling_erp_hash_table_node_t* erp_entry = mh_sampling_trace_randomly_pick_one_erp(mh_sampler->current_trace);
	DrawStmtNode* erp_node = (DrawStmtNode*) mh_sampling_name_to_node(erp_entry->key);

	mh_sampling_kernel_t kernel = mh_sampling_random_walk;
	mh_sampling_sample_t* new_sample = 0;
	float F = 0.0;
	float R = 0.0; 
	{
		int status = kernel(erp_node, erp_entry->value, &new_sample, &F, &R);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	mh_sampling_sample_t* old_sample = erp_entry->value;
	erp_entry->value = new_sample;	

	ModelNode* model = mh_sampler->model;
	pp_variable_t** param = mh_sampler->param;
	pp_query_t* query = mh_sampler->query;

	/* initialize new trace */
	mh_sampling_trace_t* new_trace = new_mh_sampling_trace(((pp_trace_t*) mh_sampler->current_trace)->symbol_table);
	mh_sampler->ll_stale = ((pp_trace_t*) mh_sampler->current_trace)->logprob
			- mh_sampler->current_trace->observed_logprob
			- old_sample->logprob + new_sample->logprob;
	mh_sampler->ll_fresh = 0.0;

	/* set up parameters */
	ModelParamsNode* param_node = model->params;
	while (param_node) {
		pp_trace_set_variable_s((pp_trace_t*) new_trace, param_node->name, *(param++));
		param_node = param_node->model_params;
	}

	/* ignore the declarations */

	/* execute statements */
	StmtsNode* stmts = model->stmts;
	while (stmts) {
		StmtNode* stmt = stmts->stmt;

		int status = mh_sampler_execute_stmt(mh_sampler, stmt, (pp_trace_t*) new_trace);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			erp_entry->value = old_sample;
			pp_variable_destroy(new_sample->value);
			mh_sampling_sample_destroy(new_sample);
			mh_sampling_trace_destroy(new_trace);
			pp_sample_error_return(status, "");
		}
		stmts = stmts->stmts;
	}

	/* recover old trace */
	erp_entry->value = old_sample;
	mh_sampling_sample_destroy(new_sample);

	/*	printf("\nnew trace:\n");
		static char buffer[8000];
		pp_trace_dump(buffer, 8000, (pp_trace_t*) new_trace);
		printf("%s\n", buffer); */

	/* reject invaid runs */
	int acc_result = query->accept(query, (pp_trace_t*) new_trace);
	if (acc_result == 0) {
		((pp_trace_t*) new_trace)->logprob = -INFINITY;
	}
	else if (acc_result == -1) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_QUERY_ERROR, "");
	}
	
	/*printf("\n\t\tlogprob\t\tsize\t\t\n");
	printf("new\t%f\t\t%d\t\t\n", ((pp_trace_t*) new_trace)->logprob, mh_sampling_erp_hash_table_size(new_trace->erp_hash_table));
	printf("old\t%f\t\t%d\t\t\n", ((pp_trace_t*) mh_sampler->current_trace) -> logprob, 
			mh_sampling_erp_hash_table_size(mh_sampler->current_trace->erp_hash_table));
	printf("R = %f, F = %f\n", R, F);
	printf("ll_stale = %f, ll_fresh = %f\n\n", mh_sampler->ll_stale, mh_sampler->ll_fresh); */

	float acceptance_rate = ((pp_trace_t*) new_trace)->logprob - ((pp_trace_t*) mh_sampler->current_trace)->logprob
							+ R - F + log(mh_sampling_erp_hash_table_size((mh_sampler->current_trace)->erp_hash_table))
							- log(mh_sampling_erp_hash_table_size(new_trace->erp_hash_table)) + mh_sampler->ll_stale
							- mh_sampler->ll_fresh;
	float randomness = log(randomR());
	if (randomness < acceptance_rate) {
		/* accept */
		mh_sampling_trace_destroy(mh_sampler->current_trace);
		mh_sampler->current_trace = new_trace;
	}
	else {
		/* reject */
		mh_sampling_trace_destroy(new_trace);
	}

	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}

void mh_sampler_destroy(mh_sampler_t* mh_sampler) {
	free(mh_sampler->model_name);

	loop_index_stack_destroy(mh_sampler->loop_index);
	if (mh_sampler->current_trace) {
		mh_sampling_trace_destroy(mh_sampler->current_trace);
	}
	free(mh_sampler);
}

const mh_sampling_name_t mh_sampling_get_name(void* node, loop_index_stack_t* loop_index) {
	#define MH_SAMPLING_GET_NAME_BUFFER_SIZE 100
	#define MH_SAMPLING_GET_NAME_PRINT(buffer, buf_size, ...)	\
		num_written += \
		snprintf(buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0, __VA_ARGS__)

	static char buffer[MH_SAMPLING_GET_NAME_BUFFER_SIZE];
	static char* buffer_ptr = 0;
	
	if (buffer_ptr) {
		free(buffer_ptr);
		buffer_ptr = 0;
	}

	size_t num_written = 0;
	MH_SAMPLING_GET_NAME_PRINT(buffer, MH_SAMPLING_GET_NAME_BUFFER_SIZE, "%016"PRIxPTR, (uintptr_t) node);
	for (size_t i = 0, end = loop_index_stack_size(loop_index); i != end; ++i) {
		MH_SAMPLING_GET_NAME_PRINT(buffer, MH_SAMPLING_GET_NAME_BUFFER_SIZE, " %d", loop_index->data[i]);
	}

	if (num_written + 1 > MH_SAMPLING_GET_NAME_BUFFER_SIZE) {
		/* failed to print the complete name due to buffer size limitation */
		size_t limit = num_written + 1;
		num_written = 0;
		buffer_ptr = malloc(limit * sizeof(char));

		MH_SAMPLING_GET_NAME_PRINT(buffer_ptr, MH_SAMPLING_GET_NAME_BUFFER_SIZE, "%016"PRIxPTR, (uintptr_t) node);
		for (size_t i = 0, end = loop_index_stack_size(loop_index); i != end; ++i) {
			MH_SAMPLING_GET_NAME_PRINT(buffer_ptr, MH_SAMPLING_GET_NAME_BUFFER_SIZE, " %d", loop_index->data[i]);
		}

		return buffer_ptr;
	}

	return buffer;

	#undef MH_SAMPLING_GET_NAME_BUFFER_SIZE
	#undef MH_SAMPLING_GET_NAME_PRINT
}

void* mh_sampling_name_to_node(const char* name) {
	void* ret = 0;
	sscanf(name, "%"PRIxPTR, &ret);
	return ret;
}

mh_sampling_sample_t* new_mh_sampling_sample(pp_variable_t* value, float logprob, size_t num_param, ...) {
	mh_sampling_sample_t* sample = malloc(sizeof(mh_sampling_sample_t) + sizeof(pp_variable_t*) * num_param);
	sample->value = pp_variable_clone(value);
	sample->logprob = logprob;
	sample->num_param = num_param;

	va_list args;
	va_start(args, num_param);
	for (size_t i = 0; i != num_param; ++i) {
		sample->param[i] = va_arg(args, pp_variable_t*);
	}
	va_end(args);

	return sample;
}

int mh_sampling_get_new_sample(DrawStmtNode* stmt, pp_trace_t* trace, mh_sampling_sample_t** sample_ptr) {
	pp_variable_t** result_ptr = 0;
	{
		int status = get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}

	#define EXECUTE_DRAW_STMT_GET_PARAM(n)	\
		pp_variable_t** param = 0;	\
	do {	\
		int status = get_parameters(stmt->expr_seq, trace, n, &param);	\
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			pp_sample_error_return(status, "");	\
		}	\
	} while(0) 

	#define EXECUTE_DRAW_STMT_CONVERT_PARAM(i, type, name, pp_type)	\
		type name = pp_variable_to_##pp_type (param[ i ])

	#define EXECUTE_DRAW_STMT_CLEAR(n)	\
	do {	\
		for (size_t i = 0; i < n; ++i) {	\
			pp_variable_destroy(param[i]);	\
		}	\
		free(param);	\
	} while(0)

	switch (stmt->dist_type) {
	case ERP_FLIP:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, p, float);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = flip(p);
			float logprob = flip_logprob(sample, p);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_int(sample);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 1, new_pp_float(p));
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}
	case ERP_MULTINOMIAL:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float*, theta, float_vector);
			int n = PP_VARIABLE_VECTOR_LENGTH(param[0]);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = multinomial(theta, n);
			float logprob = multinomial_logprob(sample, theta, n);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_int(sample);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 1, pp_variable_float_array_to_vector(theta, n));
			free(theta);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_UNIFORM:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_GAUSSIAN:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, mu, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, sigma, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = gaussian(mu, sigma);
			float logprob = gaussian_logprob(sample, mu, sigma);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 2, new_pp_float(mu), new_pp_float(sigma));
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_GAMMA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = gamma1(a, b);
			float logprob = gamma_logprob(sample, a, b);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 2, new_pp_float(a), new_pp_float(b));
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
		break;
	case ERP_BETA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = beta(a, b);
			float logprob = beta_logprob(sample, a, b);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = new_pp_float(sample);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 2, new_pp_float(a), new_pp_float(b));
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_BINOMIAL:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_POISSON:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_DIRICHLET:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, alpha, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, int, n, int);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float* alphas = malloc(sizeof(float) * n);
			for (int i = 0; i != n; ++i) {
				alphas[i] = alpha;
			}
			float* sample = dirichlet(alphas, n);
			float logprob = dirichlet_logprob(sample, alphas, n);
			if (*result_ptr) {
				pp_variable_destroy(*result_ptr);
			}
			*result_ptr = pp_variable_float_array_to_vector(sample, n);
			trace->logprob += logprob;
			*sample_ptr = new_mh_sampling_sample(*result_ptr, logprob, 2, new_pp_float(alpha), new_pp_int(n));
			free(sample);
			free(alphas);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	}

	#undef EXECUTE_DRAW_STMT_GET_PARAM
	#undef EXECUTE_DRAW_STMT_CONVERT_PARAM
	#undef EXECUTE_DRAW_STMT_CLEAR

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int mh_sampling_reuse_old_sample(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, mh_sampling_sample_t* old_sample, mh_sampling_sample_t** sample_ptr) {
	pp_trace_t* trace = (pp_trace_t*) m_trace;
	pp_variable_t** result_ptr = 0;
	{
		int status = get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}
	if (*result_ptr) {
		pp_variable_destroy(*result_ptr);
	}
	*result_ptr = pp_variable_clone(old_sample->value);

	int status = mh_sampling_rescore(stmt, m_trace, *result_ptr, sample_ptr);
	return status;
}

int mh_sampling_use_observed_value(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, pp_variable_t* variable) {
	pp_trace_t* trace = (pp_trace_t*) m_trace;
	pp_variable_t** result_ptr = 0;
	{
		int status = get_variable_ptr(stmt->variable, trace, &result_ptr);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			pp_sample_error_return(status, "");
		}
	}
	if (*result_ptr) {
		pp_variable_destroy(*result_ptr);
	}
	*result_ptr = variable;

	int status = mh_sampling_rescore(stmt, m_trace, variable, 0);
	return status;
}

int mh_sampling_rescore(DrawStmtNode* stmt, mh_sampling_trace_t* m_trace, pp_variable_t* variable, mh_sampling_sample_t** sample_ptr) {
	pp_trace_t* trace = (pp_trace_t*) m_trace;

	#define EXECUTE_DRAW_STMT_GET_PARAM(n)	\
		pp_variable_t** param = 0;	\
	do {	\
		int status = get_parameters(stmt->expr_seq, trace, n, &param);	\
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			pp_sample_error_return(status, "");	\
		}	\
	} while(0) 

	#define EXECUTE_DRAW_STMT_CONVERT_PARAM(i, type, name, pp_type)	\
		type name = pp_variable_to_##pp_type (param[ i ])

	#define EXECUTE_DRAW_STMT_CLEAR(n)	\
	do {	\
		for (size_t i = 0; i < n; ++i) {	\
			pp_variable_destroy(param[i]);	\
		}	\
		free(param);	\
	} while(0)

	switch (stmt->dist_type) {
	case ERP_FLIP:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, p, float);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = PP_VARIABLE_INT_VALUE(variable);
			float logprob = flip_logprob(sample, p);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 1, new_pp_float(p));
			else
				m_trace->observed_logprob += logprob;

			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_MULTINOMIAL:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(1);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float*, theta, float_vector);
			int n = PP_VARIABLE_VECTOR_LENGTH(param[0]);
			EXECUTE_DRAW_STMT_CLEAR(1);

			int sample = PP_VARIABLE_INT_VALUE(variable);
			float logprob = multinomial_logprob(sample, theta, n);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 1, pp_variable_float_array_to_vector(theta, n));
			else
				m_trace->observed_logprob += logprob;

			free(theta);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_UNIFORM:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_GAUSSIAN:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, mu, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, sigma, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = PP_VARIABLE_FLOAT_VALUE(variable);
			float logprob = gaussian_logprob(sample, mu, sigma);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 2, new_pp_float(mu), new_pp_float(sigma));
			else
				m_trace->observed_logprob += logprob;

			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_GAMMA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = PP_VARIABLE_FLOAT_VALUE(variable); 
			float logprob = gamma_logprob(sample, a, b);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 2, new_pp_float(a), new_pp_float(b));
			else
				m_trace->observed_logprob += logprob;

			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
		break;
	case ERP_BETA:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, a, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, float, b, float);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float sample = PP_VARIABLE_FLOAT_VALUE(variable);
			float logprob = beta_logprob(sample, a, b);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 2, new_pp_float(a), new_pp_float(b));
			else
				m_trace->observed_logprob += logprob;

			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	case ERP_BINOMIAL:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_POISSON:
		{
			pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
		}
	case ERP_DIRICHLET:
		{
			EXECUTE_DRAW_STMT_GET_PARAM(2);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(0, float, alpha, float);
			EXECUTE_DRAW_STMT_CONVERT_PARAM(1, int, n, int);
			EXECUTE_DRAW_STMT_CLEAR(2);

			float* alphas = malloc(sizeof(float) * n);
			for (int i = 0; i != n; ++i) {
				alphas[i] = alpha;
			}
			float* sample = pp_variable_to_float_vector(variable);
			float logprob = dirichlet_logprob(sample, alphas, n);
			trace->logprob += logprob;

			if (sample_ptr)
				*sample_ptr = new_mh_sampling_sample(variable, logprob, 2, new_pp_float(alpha), new_pp_int(n));
			else
				m_trace->observed_logprob += logprob;
			
			free(sample);
			free(alphas);
			pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
	}

	#undef EXECUTE_DRAW_STMT_GET_PARAM
	#undef EXECUTE_DRAW_STMT_CONVERT_PARAM
	#undef EXECUTE_DRAW_STMT_CLEAR

	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, "");
}

int mh_samling_sample_has_same_param(mh_sampling_sample_t* lhs, mh_sampling_sample_t* rhs) {
	size_t num_param = lhs->num_param;
	if (num_param != rhs->num_param) {
		return 0;
	}

	size_t i;
	for ( i = 0; i != num_param; ++i) {
		if (!pp_variable_equal(lhs->param[i], rhs->param[i])) {
			break;
		}
	}
	return i == num_param;
}

mh_sampling_sample_t* mh_sampling_sample_clone(mh_sampling_sample_t* sample) {
	if (!sample) return 0;

	mh_sampling_sample_t* new_sample = malloc(sizeof(mh_sampling_sample_t) + sizeof(pp_variable_t*) * sample->num_param);
	new_sample->value = sample->value;
	new_sample->logprob = sample->logprob;
	new_sample->num_param = sample->num_param;

	for (size_t i = 0; i != sample->num_param; ++i) {
		new_sample->param[i] = pp_variable_clone(sample->param[i]);
	}

	return new_sample;
}

void mh_sampling_sample_destroy(mh_sampling_sample_t* sample) {
	pp_variable_destroy(sample->value);
	for (size_t i = 0; i != sample->num_param; ++i) {
		pp_variable_destroy(sample->param[i]);
	}
	free(sample);
}

int mh_sampling_random_walk(DrawStmtNode* node, mh_sampling_sample_t* sample, mh_sampling_sample_t** result_ptr, float* F, float* R) {
	switch (node->dist_type) {
    case ERP_FLIP:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		if (rand() < (RAND_MAX >> 1)) {
    			result->value = pp_variable_clone(sample->value);
    			result->logprob = sample->logprob;
    		}
    		else {
    			int new_value = 1 - PP_VARIABLE_INT_VALUE(sample->value);
    			result->value = new_pp_int(new_value);
    			result->logprob = flip_logprob(new_value, PP_VARIABLE_FLOAT_VALUE(sample->param[0]));
    		}
    		*result_ptr = result;
    		*F = 0.5;
    		*R = 0.5;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
    	}
    	break;
    case ERP_MULTINOMIAL:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		float* theta = pp_variable_to_float_vector(sample->param[0]);
    		int n = PP_VARIABLE_VECTOR_LENGTH(sample->param[0]);

    		int new_value = multinomial(theta, n);
    		result->value = new_pp_int(new_value);
    		result->logprob = multinomial_logprob(new_value, theta, n);
    		free(theta);
    		*F = result->logprob;
    		*R = sample->logprob;
    		*result_ptr = result;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
    	}
    case ERP_UNIFORM:
    	break;
    case ERP_GAUSSIAN:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		float old_value = PP_VARIABLE_FLOAT_VALUE(sample->value);
    		float new_value = uniform(old_value - 1, old_value + 1);
    		*F = *R = 0.5;
    		result->value = new_pp_float(new_value);
    		result->logprob = gaussian_logprob(new_value, PP_VARIABLE_FLOAT_VALUE(sample->param[0]), PP_VARIABLE_FLOAT_VALUE(sample->param[1]));
    		*result_ptr = result;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
    	}
    	break;
    case ERP_GAMMA:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		float old_value = PP_VARIABLE_FLOAT_VALUE(sample->value);
    		float new_value = uniform(old_value - 1, old_value + 1);
    		*F = *R = 0.5;
    		result->value = new_pp_float(new_value);
    		result->logprob = gamma_logprob(new_value, PP_VARIABLE_FLOAT_VALUE(sample->param[0]), PP_VARIABLE_FLOAT_VALUE(sample->param[1]));
    		*result_ptr = result;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
    	}
    	break;
    case ERP_BETA:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		float old_value = PP_VARIABLE_FLOAT_VALUE(sample->value);
    		float new_value = uniform(old_value - 1, old_value + 1);
    		*F = *R = 0.5;
    		result->value = new_pp_float(new_value);
    		result->logprob = beta_logprob(new_value, PP_VARIABLE_FLOAT_VALUE(sample->param[0]), PP_VARIABLE_FLOAT_VALUE(sample->param[1]));
    		*result_ptr = result;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
		}
    case ERP_BINOMIAL:
    	break;
    case ERP_POISSON:
    	break;
    case ERP_DIRICHLET:
    	{
    		mh_sampling_sample_t* result = mh_sampling_sample_clone(sample);
    		float alpha = PP_VARIABLE_FLOAT_VALUE(sample->param[0]);
    		int n = PP_VARIABLE_INT_VALUE(sample->param[1]);

    		float* alphas = malloc(sizeof(float) * n);
    		for (int i = 0; i != n; ++i) {
    			alphas[i] = alpha;
    		}

    		float* new_value = dirichlet(alphas, n);
    		result->value = pp_variable_float_array_to_vector(new_value, n);
    		result->logprob = dirichlet_logprob(new_value, alphas, n);
    		free(new_value);
    		free(alphas);
    		*F = result->logprob;
    		*R = sample->logprob;
    		*result_ptr = result;
    		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
    	}
	}
	pp_sample_error_return(PP_SAMPLE_FUNCTION_UNHANDLED, ": erp %s", erp_name(node->dist_type));
}


mh_sampling_trace_t* new_mh_sampling_trace(symbol_table_t* symbol_table) {
	mh_sampling_trace_t* trace = malloc(sizeof(mh_sampling_trace_t));
	__pp_trace_init((pp_trace_t*) trace, symbol_table, sizeof(mh_sampling_trace_t));
	
	trace->observed_logprob = 0.0;
	trace->erp_hash_table = new_mh_sampling_erp_hash_table(8, 0.8);

	return trace;
}

mh_sampling_erp_hash_table_node_t* mh_sampling_trace_randomly_pick_one_erp(mh_sampling_trace_t* trace) {
	size_t i = ((((unsigned) rand()) << 16) + ((unsigned) rand())) % mh_sampling_erp_hash_table_size(trace->erp_hash_table) + 1;

	HASH_TABLE_FOR_EACH_NODE(mh_sampling_erp_hash_table, trace->erp_hash_table, node, {
		if (!--i) return node;
	});

	ERR_OUTPUT("WARNING: randomly pick one erp failed\n");
	return 0;
}

void mh_sampling_trace_destroy(mh_sampling_trace_t* trace) {
	assert(trace->super.struct_size == sizeof(mh_sampling_trace_t));
	mh_sampling_erp_hash_table_destroy(trace->erp_hash_table);

	trace->super.struct_size = sizeof(pp_trace_t);
	pp_trace_destroy((pp_trace_t*) trace);
}
