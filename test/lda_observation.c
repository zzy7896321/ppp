#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/string_query.h"
#include "../query/observation.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"

#include "../common/mem_profile.h"

void estimate_parameters(pp_trace_store_t* traces, float alpha, float beta, int k, int ndocs, int nwords[], int vocab_size) {
	
	int n = traces->n;

	int* nd = (int*) calloc(ndocs * k, sizeof(int));
	int* nw = (int*) calloc(k * vocab_size, sizeof(int));

	for (int s = 0; s < n; ++s) {
		pp_trace_t* trace = traces->trace[s];

		pp_variable_t* topic = pp_trace_find_variable(trace, "topic");
		pp_variable_t* X = pp_trace_find_variable(trace, "X");
		
		for (int i = 0; i < ndocs; ++i) {
			pp_variable_t* topic_i = PP_VARIABLE_VECTOR_VALUE(topic)[i];
			pp_variable_t* X_i = PP_VARIABLE_VECTOR_VALUE(X)[i];

			for (int j = 0; j < nwords[i]; ++j) {
				pp_variable_t* topic_i_j = PP_VARIABLE_VECTOR_VALUE(topic_i)[j];
				pp_variable_t* X_i_j = PP_VARIABLE_VECTOR_VALUE(X_i)[j];
				int topic_i_j_value = PP_VARIABLE_INT_VALUE(topic_i_j);
				int X_i_j_value = PP_VARIABLE_INT_VALUE(X_i_j);

				nd[i * k + topic_i_j_value]++;
				nw[topic_i_j_value * vocab_size + X_i_j_value]++;
			}
		}
	}

	float* theta = (float*) calloc(ndocs * k, sizeof(float));
	float* phi = (float*) calloc(k * vocab_size, sizeof(float));

	printf("theta:\n");

	for (int i = 0; i < ndocs; ++i) {
		for (int j = 0; j < k; ++j) {
			theta[i * k + j] = (nd[i * k + j] + alpha) / (nwords[i] * n + k * alpha);
			printf("%f ", theta[i * k + j]);		
		}	
		printf("\n");
	}
		
	printf("\nphi:\n");

	for (int i = 0; i < k; ++i) {
		int s = 0;
		for (int j = 0; j < vocab_size; ++j) {
			s += nw[i * vocab_size + j];
		}

		for (int j = 0; j < vocab_size; ++j) {
			phi[i * vocab_size + j] = (nw[i * vocab_size + j] + beta) / (s + vocab_size * beta);
			printf("%f ", phi[i * vocab_size + j]);
		}
		printf("\n");
	}

	free(theta);
	free(phi);
	free(nw);
	free(nd);
}

int main()
{
#ifdef ENABLE_MEM_PROFILE
    mem_profile_init();
#endif

    /* use pointers to structs because the client doesn't need to know the struct sizes */
    struct pp_state_t* state;
    struct pp_instance_t* instance;
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;
    float result;

    state = pp_new_state();
    printf("> state created\n");

    pp_load_file(state, "parse/models/lda.model");
    printf("> file loaded\n");
	
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "latent_dirichlet_allocation");
	printf(dump_model(model));

    //query = pp_compile_string_query("");
    //printf("> condition compiled\n");

	pp_variable_t** param = malloc(sizeof(pp_variable_t*) * 4);
	param[0] = new_pp_int(2);
	param[1] = new_pp_int(2);
	param[2] = new_pp_vector(2);
	PP_VARIABLE_VECTOR_LENGTH(param[2]) = 2;
	for (int i = 0; i < 2; ++i) {
		PP_VARIABLE_VECTOR_VALUE(param[2])[i] = new_pp_int(2);
	}
	param[3] = new_pp_int(3);	

	int X[2][2] = {
		{0, 0},
		{1, 1},
	};
	query = pp_query_observe_int_array_2D(state, "X", &X[0][0], 2, 2);
	if (!query) return 1;

    traces = pp_sample(state, "latent_dirichlet_allocation", param, query);
    printf("> traces sampled\n");

    if (!traces) {
        printf("ERROR encountered!!\n");
        return 1;
    }

	char buffer[8096];

    size_t max_index = 0;
    for (size_t i = 1; i < traces->n; ++i) {
    	if (traces->trace[i]->logprob > traces->trace[max_index]->logprob) {
    		max_index = i;
    	}
    }
    printf("\nsample with max logprob:\n");
    pp_trace_dump(buffer, 8096, traces->trace[max_index]);
    printf(buffer);

	printf("\nlast sample:\n");
	pp_trace_dump(buffer, 8096, traces->trace[traces->n - 1]);
	printf(buffer); 

    FILE* trace_dump_file = fopen("trace_dump.txt", "w");
    for (size_t i = 0; i != traces->n; ++i) {
        pp_trace_dump(buffer, 8096, traces->trace[i]);
        fprintf(trace_dump_file, "[trace %u]\n", i);
        fprintf(trace_dump_file, buffer);
    }
    fclose(trace_dump_file);

	int nwords[] = {2, 2};

	/* parameter estimation */
	estimate_parameters(traces, 1.0, 1.0, 2, 2, nwords, 3);

	// pp_free is broken
    pp_free(state);  /* free memory, associated models, instances, queries, and trace stores are deallocated */

    pp_trace_store_destroy(traces);

    free_pp_query_observation(query);

    for (int i = 0; i < 4; ++i)
        pp_variable_destroy(param[i]);
	free(param);

#ifdef ENABLE_MEM_PROFILE
    mem_profile_print();
    mem_profile_destroy();
#endif

    return 0;
}
