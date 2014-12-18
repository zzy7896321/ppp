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

#define K 5
#define N 100
#define NWORDS 100
#define VOCAB_SIZE 1000

int num_docs[N];

int main()
{
	set_sample_method("Metropolis-hastings");
	set_sample_iterations(1);
	set_mh_burn_in(0);
	set_mh_lag(1);
	set_mh_max_initial_round(2000);

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

	for (int i = 0; i < N; ++i) num_docs[i] = NWORDS;

	pp_variable_t* param[4] = {
		new_pp_int(K),
		new_pp_int(N),
		pp_variable_int_array_to_vector(num_docs, N),
		new_pp_int(VOCAB_SIZE),
	};

	query = pp_query_no_condition();

    traces = pp_sample(state, "latent_dirichlet_allocation", param, query);
    printf("> traces sampled\n");

    if (!traces) {
        printf("ERROR encountered!!\n");
        return 1;
    }

	pp_trace_t* trace = traces->trace[0];

	int error = 0;
	FILE* out = fopen("test/lda_param.txt", "w");
	fprintf(out, "%d %d %d %d\n\n", K, N, NWORDS, VOCAB_SIZE);

	/* phi */
	pp_variable_t* topics = pp_trace_find_variable(trace, "topics");
	if (topics)
	for (int i = 0; i < K; ++i) {
		float phi;

		for (int j = 0; j < VOCAB_SIZE - 1; ++j) {
			error |= pp_variable_access_float(topics, &phi, 2, i, j);
			fprintf(out, "%f ", phi);
		}
		error |= pp_variable_access_float(topics, &phi, 2, i, VOCAB_SIZE - 1);
		fprintf(out, "%f\n", phi);
	}
	else error = 1;
	fprintf(out, "\n");

	/* theta */
	pp_variable_t* topic_dist = pp_trace_find_variable(trace, "topic_dist");
	if (topic_dist)
	for (int i = 0; i < N; ++i) {
		float theta;

		for (int j = 0; j < K; ++j) {
			error |= pp_variable_access_float(topic_dist, &theta, 2, i, j);
			fprintf(out, "%f ", theta);
		}
		error |= pp_variable_access_float(topic_dist, &theta, 2, i, K - 1);
		fprintf(out, "%f\n", theta);
	}
	else error = 1;

	fclose(out);

	out = fopen("test/lda_topic.txt", "w");
	FILE* out2 = fopen("test/lda_words.txt", "w");

	/* topic */
	pp_variable_t* topic = pp_trace_find_variable(trace, "topic");
	pp_variable_t* X = pp_trace_find_variable(trace, "X");
	if (topic && X)
	for (int i = 0; i < N; ++i) {
		int t, x;

		for (int j = 0; j < NWORDS - 1; ++j) {
			error |= pp_variable_access_int(topic, &t, 2, i, j);
			error |= pp_variable_access_int(X, &x, 2, i, j);

			fprintf(out, "%d ", t);
			fprintf(out2, "%d ", x);
		}

		error |= pp_variable_access_int(topic, &t, 2, i, NWORDS - 1);
		error |= pp_variable_access_int(X, &x, 2, i, NWORDS - 1);

		fprintf(out, "%d\n", t);
		fprintf(out2, "%d\n", x);
	}
	else error = 1;

	fclose(out);
	fclose(out2);

	if (!error) {
		printf("success\n");
	}
	else {
		printf("error\n");
	}

	
	// pp_free is broken
    pp_free(state);  /* free memory, associated models, instances, queries, and trace stores are deallocated */

    pp_trace_store_destroy(traces);

	pp_query_destroy(query);

    for (int i = 0; i < 4; ++i)
        pp_variable_destroy(param[i]);

    return 0;
}
