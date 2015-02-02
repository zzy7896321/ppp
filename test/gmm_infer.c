#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/observation.h"
#include "../query/string_query.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"
#include "../common/trace.h"

#define K 2
#define N 20
#define mu0 0
#define lambda 0.1
#define alpha 1
#define beta 1
#define b 1

float x[N];
float mu[K];

int main() {
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(5000);
	set_mh_burn_in(500000);
	set_mh_lag(100);
	set_mh_max_initial_round(2000);
	set_prompt_per_round(100);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;
	
	FILE* fin = fopen("test/gmm_input.txt", "r");
	for (int i = 0; i < N; ++i) fscanf(fin, "%f", x + i);
	fclose(fin);

	state = pp_new_state();

	pp_load_file(state, "parse/models/gmm.model");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "gaussian_mixture_model");
	printf(dump_model(model));
	
	pp_variable_t* param[] = {
		new_pp_int(K),
		new_pp_int(N),
		new_pp_float(mu0),
		new_pp_float(lambda),
		new_pp_float(alpha),
		new_pp_float(beta),
		new_pp_float(b),
	};
	
	query = pp_query_observe_float_array(state, "x", x, N);

	traces = pp_sample_v(state, "gaussian_mixture_model", param, query, 1, "mu");

	pp_variable_destroy_all(param, sizeof(param) / sizeof(*param));
	pp_query_destroy(query);

	if (!traces) return 1;
	printf("sample done\n");
	
	int result = pp_mean_vector(traces, "mu", mu, K);
	if (result) return result;

	printf("\nmu:\n");
	for (int i = 0; i < K; ++i) {
		printf("\t%f", mu[i]);
	}
	printf("\n");
	
	char buffer[8096];
    FILE* trace_dump_file = fopen("trace_dump.txt", "w");
    for (size_t i = 0; i != traces->n; ++i) {
        pp_trace_dump(buffer, 8096, traces->trace[i]);
        fprintf(trace_dump_file, "[trace %u]\n", i);
        fprintf(trace_dump_file, buffer);
    }
    fclose(trace_dump_file);

	return 0;
}

