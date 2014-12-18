#include "rejection.h"
#include "execute.h"
#include "../debug.h"


int rejection_sampling(
		struct pp_state_t* state, 
		const char* model_name, 
		pp_variable_t* param[], 
		pp_query_t* query, 
		void** internal_data_ptr, 
		pp_trace_t** trace_ptr, 
		sample_acceptor sa,
		void *sa_data)
{
	pp_trace_t* trace = (pp_trace_t*) *internal_data_ptr;

	/* nothing to do with internal_data_ptr */
	if (!state) {
		//ERR_OUTPUT("clean up internal data\n");
		if (trace)
			pp_trace_destroy(trace);
		*internal_data_ptr = 0;

		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}

	if (!trace) {
		trace = new_pp_trace(state->symbol_table);
		*internal_data_ptr = (void*) trace;
	}

	/* find model */
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, model_name);
	if (!model) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND, ": %s", model_name);
	}

	while (1) {
		pp_trace_clear(trace);

		/* set up parameters */
		pp_variable_t** realparam = param;
		ModelParamsNode* param_node = model->params;
		while (param_node) {
			pp_trace_set_variable_s(trace, param_node->name, *(realparam++));
			param_node = param_node->model_params;
		}

		/* ignore the declarations */

		/* execute statements */
		StmtsNode* stmts = model->stmts;
		while (stmts) {
			StmtNode* stmt = stmts->stmt;

			int status = execute_stmt(stmt, trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_sample_error_return(status, "");
			}
			stmts = stmts->stmts;
		}

		/* check condition */
		int acc_result = query->full_accept(query, trace);
		if (acc_result == PP_QUERY_ACCEPTED) {
			// ERR_OUTPUT("accept\n");
			break;
		}
		else if (acc_result == PP_QUERY_REJECTED){
			// ERR_OUTPUT("reject\n");
		}
		else {
			pp_sample_error_return(PP_SAMPLE_FUNCTION_QUERY_ERROR, "");
		}
	}

	sa(sa_data, trace);

	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}

