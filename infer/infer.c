
#include "infer.h"
#include "../debug.h"
#include "mh_sampler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "../common/mem_profile.h"

unsigned g_sample_iterations = 200;
sample_function_t g_sample_function = rejection_sampling;

void set_sample_iterations(int sample_iterations) {
	g_sample_iterations = sample_iterations;
}

void set_sample_method(const char* sample_method) {
	if (sample_method[0] == 'R') {
		g_sample_function = rejection_sampling;
	}

	else {
		g_sample_function = mh_sampling;
	}
}

const char* pp_sample_error_string[PP_SAMPLE_FUNCTION_ERROR_NUM] = {
	"PP_SAMPLE_FUNCTION_NORMAL",
	"PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND",
	"PP_SAMPLE_FUNCTION_INVALID_STATEMENT",
	"PP_SAMPLE_FUNCTION_INVALID_EXPRESSION",
	"PP_SAMPLE_FUNCTION_NON_SCALAR_TYPE_AS_CONDITION",
	"PP_SAMPLE_FUNCTION_UNHANDLED",
	"PP_SAMPLE_FUNCTION_INVALID_OPEARND_TYPE",
	"PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH",
	"PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO",
	"PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND",
	"PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR",
	"PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION",
	"PP_SAMPLE_FUNCTION_NUMBER_OF_PARAMETER_MISMATCH",
	"PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE",
	"PP_SAMPLE_FUNCTION_INDEX_OUT_OF_BOUND",

	"PP_SAMPLE_FUNCTION_MH_FAIL_TO_INITIALIZE",
	"PP_SAMPLE_FUNCTION_QUERY_ERROR",
	"PP_SAMPLE_FUNCTION_UNKNOWN_FUNCTION",
};

struct pp_trace_store_t* pp_sample(
		struct pp_state_t* state, 
		const char* model_name, 
		pp_variable_t* param[], 
		struct pp_query_t* query)
{
	return pp_sample_v(state, model_name, param, query, 0);
}

typedef struct __pp_sample_v_data_t {
	int num_output_vars;
	symbol_t *output_vars;
	int itrace;
	pp_trace_store_t *traces;
} __pp_sample_v_data_t;

void __pp_sample_v_acceptor(void *p_data, pp_trace_t* trace) {
	__pp_sample_v_data_t *data = (__pp_sample_v_data_t*) p_data;

	if (data->num_output_vars) 
		data->traces->trace[data->itrace++] = pp_trace_output(trace, data->num_output_vars, data->output_vars);
	else
		data->traces->trace[data->itrace++] = pp_trace_clone(trace);
}

struct pp_trace_store_t* pp_sample_v(
		struct pp_state_t* state, 
		const char* model_name, 
		pp_variable_t* param[], 
		struct pp_query_t* query, 
		int num_output_vars, 
		...) 
{
	symbol_t* output_vars = malloc(sizeof(symbol_t) * num_output_vars);
	symbol_table_t* symbol_table = state->symbol_table;

	va_list varargs;
	va_start(varargs, num_output_vars);
	for (int i = 0; i < num_output_vars; ++i) {
		const char* varname = va_arg(varargs, const char*);
		symbol_t symbol = symbol_table_lookup(symbol_table, varname);

		output_vars[i] = symbol;
	}
	va_end(varargs);
	
	__pp_sample_v_data_t data = { 
		.num_output_vars = num_output_vars,
		.output_vars = output_vars,
		.itrace = 0,
		.traces = new_pp_trace_store(g_sample_iterations),
	};

	int ret = pp_sample_f(state, model_name, param, query, __pp_sample_v_acceptor, &data);

	free(output_vars);

	if (ret) return 0;
	return data.traces;
}

int pp_sample_f(
		struct pp_state_t *state,
		const char *model_name,
		pp_variable_t *param[],
		struct pp_query_t *query,
		sample_acceptor sa,
		void* sa_data)
{
	if (g_sample_function == rejection_sampling) {
		ERR_OUTPUT("sample_method = rejection\n");
	}
	else if (g_sample_function == mh_sampling) {
		ERR_OUTPUT("sample_method = Metropolis-hastings\n");
	}
	else {
		ERR_OUTPUT("sample_method = unknown\n");
	}
	ERR_OUTPUT("sample_iterations = %d\n", g_sample_iterations);
	if (g_sample_function == mh_sampling) {
		ERR_OUTPUT("MH_BURN_IN = %d\n", g_mh_sampler_burn_in_iterations);
		ERR_OUTPUT("MH_LAG = %d\n", g_mh_sampler_lag);
		ERR_OUTPUT("MH_MAX_INITIAL_ROUND = %d\n", g_mh_sampler_maximum_initial_round);
	}

	
	pp_trace_store_t* traces = new_pp_trace_store(g_sample_iterations);
	void* internal_data = 0;
	for (unsigned i = 0; i < g_sample_iterations; ++i) {
		ERR_OUTPUT("sample round %d\n", i);
		int status = g_sample_function(state, model_name, param, query, &internal_data, &(traces->trace[i]), sa, sa_data);
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("error: %s\n", pp_sample_get_error_string(status));
			pp_trace_store_destroy(traces);
			return 1;
		}

		/*ERR_OUTPUT("sample round %d\n", i);
		char buffer[8096];
		pp_trace_dump(traces->trace[i], buffer, 8096);
		printf(buffer); */
	}

	/* clear up */
	g_sample_function(0, 0, 0, 0, &internal_data, 0, 0, 0);

	return 0;
}


int pp_get_result(pp_trace_store_t* traces, pp_query_t* query, float* result) {
	if (!result) return 1;
	if (!traces->n) {
		*result = 0.0;
		return 0;
	}

	int num_traces = traces->n;
	int accepted_cnt = 0;

	for (unsigned i = 0; i < num_traces; ++i) {
		/*ERR_OUTPUT("trace %d, 0x%08x:\n", i, traces->trace[i]);
		char buffer[8096];
		pp_trace_dump(traces->trace[i], buffer, 8096);
		printf(buffer); */
		int acc_result = query->full_accept(query, traces->trace[i]);
		if (acc_result == 1) {
			++accepted_cnt;
		}
		else if (acc_result == -1) {
			return 1;
		}
	}

	*result = (float)(accepted_cnt) / num_traces;
	return 0;
}

