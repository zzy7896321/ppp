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

#define T 50
#define N 2
#define M 3
#define ALPHA 1.0
#define BETA 1.0

int main() {
	set_sample_method("Rejection");
	set_sample_iterations(1);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;

	state = pp_new_state();

	pp_load_file(state, "parse/models/hmm.model");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "hidden_markov_model");
	printf(dump_model(model));
	
	pp_variable_t* param[] = {
		new_pp_int(T),
		new_pp_int(N),
		new_pp_int(M),
		new_pp_float(ALPHA),
		new_pp_float(BETA),
	};

	query = pp_query_no_condition();

	traces = pp_sample(state, "hidden_markov_model", param, query);

	pp_variable_destroy_all(param, sizeof(param) / sizeof(*param));

	if (!traces) return 1;

	printf("sample done\n");

	pp_trace_t* trace = traces->trace[0];
	FILE* fout = fopen("test/hmm_output.txt", "w");

	fprintf(fout, "T = %d\n", T);
	fprintf(fout, "N = %d\n", N);
	fprintf(fout, "M = %d\n", M);
	fprintf(fout, "ALPHA = %f\n", ALPHA);
	fprintf(fout, "BETA = %f\n", BETA);

	fputs("\nphi:\n", fout);
	pp_variable_t* pp_phi = pp_trace_find_variable(trace, "phi");
	for (int i = 0; i < N; ++i) {
		pp_variable_t* pp_phi_i = PP_VARIABLE_VECTOR_VALUE(pp_phi)[i];
		for (int j = 0; j < N; ++j) {
			pp_variable_t* pp_phi_i_j = PP_VARIABLE_VECTOR_VALUE(pp_phi_i)[j];
			fprintf(fout, "\t%f", PP_VARIABLE_FLOAT_VALUE(pp_phi_i_j)); 
		}
		fprintf(fout, "\n");
	}

	fputs("\ntheta:\n", fout);
	pp_variable_t* pp_theta = pp_trace_find_variable(trace, "theta");
	for (int i = 0; i < N; ++i) {
		pp_variable_t* pp_theta_i = PP_VARIABLE_VECTOR_VALUE(pp_theta)[i];
		for (int j = 0; j < M; ++j) {
			pp_variable_t* pp_theta_i_j = PP_VARIABLE_VECTOR_VALUE(pp_theta_i)[j];
			fprintf(fout, "\t%f", PP_VARIABLE_FLOAT_VALUE(pp_theta_i_j)); 
		}
		fprintf(fout, "\n");
	}

	fputs("\nstart:\n", fout);
	pp_variable_t* pp_start = pp_trace_find_variable(trace, "start");
	for (int i = 0; i < N; ++i) {
		pp_variable_t* pp_start_i = PP_VARIABLE_VECTOR_VALUE(pp_start)[i];
		fprintf(fout, "\t%f", PP_VARIABLE_FLOAT_VALUE(pp_start_i));
	}
	fputs("\n", fout);
	
	fputs("\nX:\n", fout);
	pp_variable_t* pp_X = pp_trace_find_variable(trace, "X");
	for (int i = 0; i < T; ++i) {
		pp_variable_t* pp_X_i = PP_VARIABLE_VECTOR_VALUE(pp_X)[i];
		fprintf(fout, "\t%d", PP_VARIABLE_INT_VALUE(pp_X_i));
		if (i % 10 == 9) fputs("\n", fout);
	}
	fputs("\n", fout);

	fputs("\nY:\n", fout);
	pp_variable_t* pp_Y = pp_trace_find_variable(trace, "Y");
	for (int i = 0; i < T; ++i) {
		pp_variable_t* pp_Y_i = PP_VARIABLE_VECTOR_VALUE(pp_Y)[i];
		fprintf(fout, "\t%d", PP_VARIABLE_INT_VALUE(pp_Y_i));
		if (i % 10 == 9) fputs("\n", fout);
	}
	fputs("\n", fout);

	fclose(fout);

	return 0;
}

