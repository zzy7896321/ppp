#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include "../query/string_query.h"
#include "../query/observation.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"

#include "../common/mem_profile.h"

#define K 5
#define N 100
#define NWORDS 100
#define NTRAINWORDS 80
#define NTESTWORDS 20
#define VOCAB_SIZE 1000
#define ALPHA 1
#define BETA 1

#define NSAMPLES 1000

int num_docs[N];
int X[N][NTRAINWORDS];
int X_t[N][NTESTWORDS];
int nd[N][K];
int nw[K][VOCAB_SIZE];
int snw[K];

float theta[N][K];
float phi[K][VOCAB_SIZE];

void stat_sample(void* data, struct pp_trace_t* trace) {
	pp_variable_t* topic = pp_trace_find_variable(trace, "topic");

	for (int i = 0; i < N; ++i) {
		pp_variable_t* topic_i = PP_VARIABLE_VECTOR_VALUE(topic)[i];
		for (int j = 0; j < NTRAINWORDS; ++j) {
			pp_variable_t* topic_i_j = PP_VARIABLE_VECTOR_VALUE(topic_i)[j];
			int t = PP_VARIABLE_INT_VALUE(topic_i_j);

			nd[i][t]++;
			nw[t][X[i][j]]++;
			snw[t]++;
		}
	}
}

void estimate() {	
	FILE* out = fopen("test/lda_estimated_param.txt", "w");
	fprintf(out, "theta:\n");

	for (int i = 0; i < N; ++i) {
			
		for (int j = 0; j < K; ++j) {
			theta[i][j] = (nd[i][j] + ALPHA) / (float)(NTRAINWORDS * NSAMPLES + K * ALPHA); 
			fprintf(out, "%f ", theta[i][j]);		
		}	
		fprintf(out, "\n");
	}
		
	fprintf(out, "\nphi:\n");

	for (int i = 0; i < K; ++i) {
		for (int j = 0; j < VOCAB_SIZE; ++j) {
			phi[i][j] = (nw[i][j] + BETA) / (float)(snw[i] + VOCAB_SIZE * BETA);
			fprintf(out, "%f ", phi[i][j]);
		}
		fprintf(out, "\n");
	}

	fclose(out);
}

float get_word_logprob(int doc, int word) {

	float s = 0;
	for (int i = 0; i < K; ++i) {
		s += theta[doc][i] * phi[i][word];
	}
	return log(s);
}

void calculate_perplexity() {
	float s = 0;
	for (int i = 0; i < N; ++i) {
		for (int j = NTRAINWORDS; j < NWORDS; ++j) {
			s += get_word_logprob(i, X_t[i][j - NTRAINWORDS]);
		}
	}
	s /= N * NTESTWORDS;
	s = exp(-s);	

	printf("perplexity = %f\n", s);
}

int main()
{
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(NSAMPLES);
	set_mh_burn_in(20);
	set_mh_lag(5);
	set_mh_max_initial_round(2000);

    /* use pointers to structs because the client doesn't need to know the struct sizes */
    struct pp_state_t* state;
    struct pp_instance_t* instance;
    struct pp_query_t* query;
    struct pp_trace_store_t* traces;

    state = pp_new_state();
    printf("> state created\n");

    pp_load_file(state, "parse/models/lda.model");
    printf("> file loaded\n");
	
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "latent_dirichlet_allocation");
	printf(dump_model(model));

	for (int i = 0; i < N; ++i) num_docs[i] = NTRAINWORDS;

	pp_variable_t* param[4] = {
		new_pp_int(K),
		new_pp_int(N),
		pp_variable_int_array_to_vector(num_docs, N),
		new_pp_int(VOCAB_SIZE),
	};
	
	FILE* in = fopen("test/lda_words.txt", "r");
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < NTRAINWORDS; ++j) {
			fscanf(in, "%d", &X[i][j]);
		}
		for (int j = NTRAINWORDS; j < NWORDS; ++j) {
			fscanf(in, "%d", &X_t[i][j - NTRAINWORDS]);
		}
	}
	fclose(in);

	query = pp_query_observe_int_array_2D(state, "X", (int*)X, N, NTRAINWORDS);
	
	int result = pp_sample_f(state, "latent_dirichlet_allocation", param, query, stat_sample, 0);	
	if (result) {
		printf("error\n");
		return 1;
	}

	free_pp_query_observation(query);
	
	printf("estimate\n");
	estimate();
	printf("perplexity\n");
	calculate_perplexity();
	
	for (int i = 0; i < 4; ++i) pp_variable_destroy(param[i]);
	pp_free(state);
	
    return 0;
}
