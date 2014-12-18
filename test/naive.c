#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "ppp.h"
#include "../query/observation.h"
#include "../query/query_comp.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/variables.h"
#include "../common/trace.h"

#define ALPHA ((float)1)
#define BETA ((float)1)
#define K 2
#define N 1000
#define NWORDS 100
#define V 30000

#define PORTION_TRAIN 0.7
#define NTRAIN ((int) N * PORTION_TRAIN)
#define NTEST (N - NTRAIN)

#define BURN_IN 200
#define NSAMPLES 1000
#define LAG 100

int c[N];
int X[N][NWORDS];

int num[N][K];

void stat_sample(void* data, struct pp_trace_t* trace) {
	pp_variable_t* p_c = pp_trace_find_variable(trace, "c");

	for (int i = 0; i < N; ++i) {
		int cat = PP_VARIABLE_INT_VALUE(p_c);
		++num[i][cat];
	}
}

int main() {
	set_sample_method("metropolis-hastings");
	set_sample_iterations(NSAMPLES);
	set_mh_burn_in(BURN_IN);
	set_mh_lag(LAG);
	set_prompt_per_round(1);

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
	
	FILE* fin = fopen("test/naive_data.txt", "r");
	if (!fin) {
		printf("data not found\n");
		return 1;
	}

	for (int i = 0; i < N; ++i) {
		for (int j = 0; j < NWORDS; ++j) {
			fscanf(fin, "%d", &X[i][j]);
		}
	}
	fclose(fin);

	fin = fopen("test/naive_class.txt", "r");
	if (!fin) {
		printf("data not found\n");
		return 1;
	}
	for (int i = 0; i < N; ++i) {
		fscanf(fin, "%d", &c[i]);	
	}
	fclose(fin);
	
	query = pp_query_composite(
		pp_query_observe_int_array(state, "c", c, NTRAIN),
		pp_query_observe_int_array_2D(state, "X", &X[0][0], N, NWORDS)
		);

	int result = pp_sample_f(state, "naive_bayes", param, query, stat_sample, 0);

	

	pp_variable_destroy_all(param, 6);
	pp_query_destroy(query);
	pp_free(state);
	
	FILE* fout = fopen("test/naive_inferred.txt", "w");
	
	fprintf(fout, "%-8s%10s%10s%4s%4s\n", "no.", "p(0)", "p(1)", "tru", "inf");
	
	int cnt[K][K];
	for (int i = 0; i < N; ++i) {
		fprintf(fout, "%-8d", i);
		int cat = 0;
		for (int j = 0; j < K; ++j) {
			if (num[i][j] > num[i][cat]) cat = j;
			fprintf(fout, "%10f", num[i][j] / (float) NSAMPLES);
		}

		fprintf(fout, "%4d%4d\n", c[i], cat);

		if (i >= NTRAIN) {
			cnt[cat][c[i]]++;
		}
	}

	float prec = ((float) cnt[1][1]) / (cnt[1][1] + cnt[1][0]);
	float recall = ((float) cnt[1][1]) / (cnt[1][1] + cnt[0][1]);

	float f1_score = 2 * (prec * recall) / (prec + recall);

	fprintf(fout, "\nf1 score = %f\n", f1_score);
	printf("f1 score = %f", f1_score);

	fclose(fout);

	return 0;
}

