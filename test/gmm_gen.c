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
#define alpha 1.0
#define beta 1.0
#define b 1.0

int main() {
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(1);
	set_mh_lag(1);
	set_mh_burn_in(0);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;

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
	
	float phi[2] = {0.5, 0.5};

	query = pp_query_observe_float_array(state, "phi", phi, 2);

	traces = pp_sample(state, "gaussian_mixture_model", param, query);

	pp_variable_destroy_all(param, sizeof(param) / sizeof(*param));

	if (!traces) return 1;
	printf("sample done\n");
	
	pp_trace_t* trace = traces->trace[0];
	FILE* fout = fopen("test/gmm_output.txt", "w");
	
#define PRINT(...)	\
	do {	\
		printf(__VA_ARGS__);	\
		fprintf(fout, __VA_ARGS__);	\
	} while (0)

	PRINT("K = %d\n", K);
	PRINT("N = %d\n", N);
	PRINT("mu0 = %f\n", mu0);
	PRINT("lambda = %f\n", lambda);
	PRINT("alpha = %f\n", alpha);
	PRINT("beta = %f\n", beta);
	PRINT("b = %f\n", b);

	PRINT("\nphi:\n");
	pp_variable_t* pp_phi = pp_trace_find_variable(trace, "phi");
	for (int i = 0; i < K; ++i) {
		pp_variable_t* pp_phi_i = PP_VARIABLE_VECTOR_VALUE(pp_phi)[i];
		PRINT("\t%f", PP_VARIABLE_FLOAT_VALUE(pp_phi_i));
	}
	
	PRINT("\n\ncomponents:\n\tno.\tmu\tstddev\n");
	pp_variable_t* pp_mu = pp_trace_find_variable(trace, "mu");
	pp_variable_t* pp_stddev = pp_trace_find_variable(trace, "stddev");
	for (int i = 0; i < K; ++i) {
		pp_variable_t* pp_mu_i = PP_VARIABLE_VECTOR_VALUE(pp_mu)[i];
		pp_variable_t* pp_stddev_i = PP_VARIABLE_VECTOR_VALUE(pp_stddev)[i];
		PRINT("\t%d\t%f\t%f\n", i, PP_VARIABLE_FLOAT_VALUE(pp_mu_i),
				PP_VARIABLE_FLOAT_VALUE(pp_stddev_i));
	}

#define NPERLINE 5
	PRINT("\nz:\n");
	pp_variable_t* pp_z = pp_trace_find_variable(trace, "z");
	int nprinted = 0;
	for (int i = 0; i < N; ++i) {
		pp_variable_t* pp_z_i = PP_VARIABLE_VECTOR_VALUE(pp_z)[i];
		PRINT("\t%d", PP_VARIABLE_INT_VALUE(pp_z_i));
		if (++nprinted % NPERLINE == 0) PRINT("\n");
	}

	PRINT("\n\nx:\n");
	pp_variable_t* pp_x = pp_trace_find_variable(trace, "x");
	nprinted = 0;
	for (int i = 0; i < N; ++i ) {
		pp_variable_t* pp_x_i = PP_VARIABLE_VECTOR_VALUE(pp_x)[i];
		PRINT("\t%f", PP_VARIABLE_FLOAT_VALUE(pp_x_i));
		if (++nprinted % NPERLINE == 0) PRINT("\n");
	}
	PRINT("\n");

#undef NPERLINE
#undef PRINT

	fclose(fout);

	return 0;
}

