
#include "infer.h"
#include "../debug.h"
#include "mh_sampler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../common/mem_profile.h"

unsigned g_sample_iterations = 200;
char* g_sample_method = "REJECTION";
sample_function_t g_sample_function = rejection_sampling;

void init_sample_iterations()
{
    char* sample_iterations = getenv("SAMPLE_ITERATIONS");
    if (sample_iterations) {
        g_sample_iterations = atoi(sample_iterations);
        fprintf(stderr, "init_sample_iterations (%d): g_sample_iterations=%d\n", __LINE__, g_sample_iterations);
    }

    ERR_OUTPUT("SAMPLE_ITERATIONS = %d\n", g_sample_iterations);
}

void init_sample_method() {
	char* sample_method= getenv("SAMPLE_METHOD");
	if (sample_method) {
		g_sample_method = sample_method;
	}

	if (g_sample_method[0] == 'R') {
		ERR_OUTPUT("SAMPLE_METHOD = Rejection\n");
		g_sample_function = rejection_sampling;
	}

	else {
		ERR_OUTPUT("SAMPLE METHOD = Metropolis-Hastings\n");
		g_sample_function = mh_sampling;

		char* burn_in_round = getenv("MH_BURN_IN");
		if (burn_in_round) {
			g_mh_sampler_burn_in_iterations = atoi(burn_in_round);
		}
		ERR_OUTPUT("MH_BURN_IN = %d\n", g_mh_sampler_burn_in_iterations);

		char* lag = getenv("MH_LAG");
		if (lag) {
			g_mh_sampler_lag = atoi(lag);
		}
		ERR_OUTPUT("MH_LAG = %d\n", g_mh_sampler_lag);

		char* max_initial_round = getenv("MH_MAX_INITIAL_ROUND");
		if (max_initial_round) {
			g_mh_sampler_maximum_initial_round = atoi(max_initial_round);
		}
		ERR_OUTPUT("MH_MAX_INITIAL_ROUND = %d\n", g_mh_sampler_maximum_initial_round);
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

struct pp_trace_store_t* pp_sample(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], struct pp_query_t* query) {
	init_sample_iterations();
	init_sample_method();

	pp_trace_store_t* traces = new_pp_trace_store(g_sample_iterations);
	void* internal_data = 0;
	for (unsigned i = 0; i < g_sample_iterations; ++i) {
		//ERR_OUTPUT("sample round %d\n", i);
		int status = g_sample_function(state, model_name, param, query, &internal_data, &(traces->trace[i]));
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {
			ERR_OUTPUT("error: %s\n", pp_sample_get_error_string(status));
			pp_trace_store_destroy(traces);
			return 0;
		}

		/*ERR_OUTPUT("sample round %d\n", i);
		char buffer[8096];
		pp_trace_dump(traces->trace[i], buffer, 8096);
		printf(buffer); */
	}

	/* clear up */
	g_sample_function(0, 0, 0, 0, &internal_data, 0);

	return traces;
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

