#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ppp.h"
#include "../query/observation.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"
#include "../common/trace.h"

#define ALPHA ((float)1)
#define BETA ((float)1)
#define K 2
#define N 10
#define NWORDS 20
#define V 30

int main() {
	set_sample_method("Rejection");
	set_sample_iterations(1);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;

	state = pp_new_state();

	pp_load_file(state, "parse/models/naive.model");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "naive_bayes");
	if (!model) {
		printf("error: model not found\n");
		return 1;
	}
	printf(dump_model(model));

	pp_variable_t* param[6] = {
		new_pp_int(K),	/* K : #classes */
		new_pp_int(N),	/* N : #docs	*/
		new_pp_int(NWORDS), /* nwords: #words per doc */
		new_pp_int(V), /* V: vocabulary size */
		new_pp_float(ALPHA), /* alpha */
		new_pp_float(BETA),	/* beta*/
	};

	query = pp_query_no_condition();
	
	traces = pp_sample(state, "naive_bayes", param, query);
	if (!traces) {
		return 1;
	}

	FILE* out = fopen("test/naive_param.txt", "w");
	fprintf(out, "%d %d %d %d %f %f\n\n", K, N, NWORDS, V, ALPHA, BETA);
	
	pp_trace_t* trace = traces->trace[0];
	pp_variable_t* p_theta = pp_trace_find_variable(trace, "theta");
	for (int i = 0; i < K; ++i) {
		float val = PP_VARIABLE_FLOAT_VALUE(PP_VARIABLE_VECTOR_VALUE(p_theta)[i]);
		if (0 == i) {
			fprintf(out, "%f", val);	
		}
		else {
			fprintf(out, " %f", val);
		}
	}
	fprintf(out, "\n\n");

	pp_variable_t* p_phi = pp_trace_find_variable(trace, "phi");
	for (int i = 0; i < K; ++i) {
		pp_variable_t* p_phi_i = PP_VARIABLE_VECTOR_VALUE(p_phi)[i];
		for (int j = 0; j < V; ++j) {
			float val = PP_VARIABLE_FLOAT_VALUE(PP_VARIABLE_VECTOR_VALUE(p_phi_i)[j]);
			if (0 == j) {
				fprintf(out, "%f", val);
			}
			else {
				fprintf(out, " %f", val);
			}
		}
		fprintf(out, "\n");
	}

	fclose(out);

	out = fopen("test/naive_class.txt", "w");
	pp_variable_t* p_c = pp_trace_find_variable(trace, "c");
	for (int i = 0; i < N; ++i) {
		int val = PP_VARIABLE_INT_VALUE(PP_VARIABLE_VECTOR_VALUE(p_c)[i]);
		fprintf(out, "%d\n", val);
	}

	fclose(out);

	out = fopen("test/naive_data.txt", "w");
	pp_variable_t* p_X = pp_trace_find_variable(trace, "X");
	for (int i = 0; i < N; ++i) {
		pp_variable_t* p_X_i = PP_VARIABLE_VECTOR_VALUE(p_X)[i];
		for (int j = 0; j < NWORDS; ++j) {
			int val = PP_VARIABLE_INT_VALUE(PP_VARIABLE_VECTOR_VALUE(p_X_i)[j]);
			if (0 == j) {
				fprintf(out, "%d", val);
			}
			else {
				fprintf(out, " %d", val);
			}
		}
		fprintf(out, "\n");
	}

	fclose(out);

	pp_variable_destroy_all(param, 6);
	pp_query_destroy(query);
	pp_free(state);

	return 0;
}

