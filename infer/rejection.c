#include "rejection.h"
#include "execute.h"
#include "../debug.h"


int rejection_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr) {
	/* nothing to do with internal_data_ptr */
	if (!state) {
		//ERR_OUTPUT("clean up internal data\n");
		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}

	/* find model */
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, model_name);
	if (!model) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND, ": %s", model_name);
	}

	pp_trace_t* trace = 0;
	while (1) {
		/* initialize trace */
		trace = new_pp_trace();

		/* set up parameters */
		ModelParamsNode* param_node = model->params;
		while (param_node) {
			const char* varname = symbol_to_string(node_symbol_table(param_node), param_node->name);
			pp_trace_set_variable(trace, varname, *(param++));	
			param_node = param_node->model_params;
		}

		/* ignore the declarations */

		/* execute statements */
		StmtsNode* stmts = model->stmts;
		while (stmts) {
			StmtNode* stmt = stmts->stmt;
			//ERR_OUTPUT("executing statement: %s", dump_stmt(stmt));

			int status = execute_stmt(stmt, trace);
			if (status != PP_SAMPLE_FUNCTION_NORMAL) {
				pp_trace_destroy(trace);
				pp_sample_error_return(status, "");
			}
			stmts = stmts->stmts;
		}

		/* check condition */
		if (pp_query_acceptor(trace, query)) {
			break;
		}
		else {
			pp_trace_destroy(trace);
		}
	}

	*trace_ptr = trace;
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}

