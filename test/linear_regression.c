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

int main() {
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(1000);
	set_mh_burn_in(200);
	set_mh_lag(10);
	set_mh_max_initial_round(2000);
	
	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;

	state = pp_new_state();

	pp_load_file(state, "parse/models/linear_regression.model");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "linear_regression");
	printf(dump_model(model));
	
	FILE* df = fopen("test/linear_regression.txt", "r");
	int N;
	fscanf(df, "%d", &N);

	float *X = malloc(sizeof(float) * N), *Y = malloc(sizeof(float) * N);
	for (int i = 0; i < N; ++i) fscanf(df, "%f", X + i);
	for (int i = 0; i < N; ++i) fscanf(df, "%f", Y + i);
	fclose(df);

	
	pp_variable_t* param[2] = {
		new_pp_int(N),
	 	pp_variable_float_array_to_vector(X, N)
	};

	query = pp_query_observe_float_array(state, "y", Y, N);

	traces = pp_sample_v(state, "linear_regression", param, query, 2, "a", "b");
	if (!traces) {
		return 1;
	}

	printf("last sample:\n");
	char buffer[8096];
	pp_trace_dump(buffer, 8096, traces->trace[traces->n-1]);
	printf(buffer); 

    FILE* trace_dump_file = fopen("trace_dump.txt", "w");
    for (size_t i = 0; i != traces->n; ++i) {
        pp_trace_dump(buffer, 8096, traces->trace[i]);
        fprintf(trace_dump_file, "[trace %u]\n", i);
        fprintf(trace_dump_file, buffer);
    }
    fclose(trace_dump_file);

	int n_samples = traces->n;
	float sa = 0.0, sb = 0.0, sqa = 0.0, sqb = 0.0;
	float ma, mb, va, vb;

	for (size_t i = 0; i != n_samples; ++i) {
		pp_variable_t* var;
		pp_trace_t* trace = traces->trace[i];

		var = pp_trace_find_variable(trace, "a");
		float a = PP_VARIABLE_FLOAT_VALUE(var);
		sa += a;
		sqa += a * a;

		var = pp_trace_find_variable(trace, "b");
		float b = PP_VARIABLE_FLOAT_VALUE(var);
		sb += b;
		sqb += b * b;
	}

	ma = sa / n_samples;
	mb = sb / n_samples;
	va = sqa / n_samples - ma * ma;
	vb = sqb / n_samples - mb * mb;
	
	printf("\nE[a] = %f, var(a) = %f\n", ma, va);
	printf("E[b] = %f, var(b) = %f\n", mb, vb);

	pp_trace_store_destroy(traces);
	pp_query_destroy(query);

	free(X); free(Y);
	
	return 0;
}
