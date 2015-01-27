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

int Y[T];

int cnt_phi[N][N];
int cnt_theta[N][M];
int sum_phi[N];
int sum_theta[N];

float phi[N][N];
float theta[N][M];

void acceptor(void* data, struct pp_trace_t* trace) {
	pp_variable_t* pp_X = pp_trace_find_variable(trace, "X");
	pp_variable_t* pp_X_i = PP_VARIABLE_VECTOR_VALUE(pp_X)[0];

	int prev = PP_VARIABLE_INT_VALUE(pp_X_i);
	cnt_theta[prev][Y[0]]++;
	sum_theta[prev]++;

	for (int i = 1; i < T; ++i) {
		pp_X_i = PP_VARIABLE_VECTOR_VALUE(pp_X)[i];
		int now = PP_VARIABLE_INT_VALUE(pp_X_i);

		cnt_theta[now][Y[i]]++;
		sum_theta[now]++;
		cnt_phi[prev][now]++;
		sum_phi[prev]++;
		prev = now;
	}
}

int main() {
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(5000);
	set_mh_burn_in(1000);
	set_mh_lag(100);
	set_mh_max_initial_round(2000);
	set_prompt_per_round(1000);

	pp_state_t* state;
	struct pp_instance_t* instance;
	pp_query_t* query;
	pp_trace_store_t* traces;
	
	/* load data */
	FILE* fin = fopen("test/hmm_input.txt", "r");
	for (int i = 0; i < T; ++i) fscanf(fin, "%d", Y + i);	
	fclose(fin);

	/* load model */
	state = pp_new_state();
	pp_load_file(state, "parse/models/hmm.model");
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, "hidden_markov_model");
	printf(dump_model(model));
	
	/* set up parameters and queries */
	pp_variable_t* param[] = {
		new_pp_int(T),
		new_pp_int(N),
		new_pp_int(M),
		new_pp_float(ALPHA),
		new_pp_float(BETA),
	};
	query = pp_query_observe_int_array(state, "Y", Y, T);

	memset(cnt_phi, 0, sizeof(cnt_phi));
	memset(cnt_theta, 0, sizeof(cnt_theta));
	memset(sum_phi, 0, sizeof(sum_phi));
	memset(sum_theta, 0, sizeof(sum_theta));

	pp_sample_f(state, "hidden_markov_model", param, query, acceptor, 0);

	pp_variable_destroy_all(param, sizeof(param) / sizeof(*param));

	printf("sample done\n");

	printf("\nphi:\n");
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < N; ++j) {
			phi[i][j] = cnt_phi[i][j] / (float) sum_phi[i];
			printf("\t%f", phi[i][j]);
		}
		printf("\n");	
	}

	printf("\ntheta:\n");
	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < M; ++j) {
			theta[i][j] = cnt_theta[i][j] / (float) sum_theta[i];
			printf("\t%f", theta[i][j]);
		}
		printf("\n");	
	}

	return 0;
}

