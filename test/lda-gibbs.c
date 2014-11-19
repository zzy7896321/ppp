#include <stdio.h>
#include "ppp.h"
#include <stdlib.h>
#include <time.h>

#include "../query/query.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../infer/infer.h"
#include "../infer/gibbs.h"
#include "../common/variables.h"

#include "../common/mem_profile.h"

int num_topic,num_docs,vocab_size;
int *num_words;
int **docs;
float *prior_alpha,*prior_beta;

/* use pointers to structs because the client doesn't need to know the struct sizes */
struct pp_state_t* state;
struct pp_instance_t* instance;
struct pp_query_t* query;
struct pp_trace_store_t* traces;
float result;

void lda_gibbs_conditional_probability(char* name,int index);

int main() {
#ifdef ENABLE_MEM_PROFILE
	mem_profile_init();
#endif

	num_topic=2;
	num_docs=2;
	num_words=malloc(sizeof(int) * num_docs);
	num_words[0]=2;
	num_words[1]=2;
	vocab_size=2;
	docs=malloc(sizeof(int*) * num_docs);
	docs[0]=malloc(sizeof(int*) * num_words[0]);
	docs[1]=malloc(sizeof(int*) * num_words[1]);
	docs[0][0]=0;
	docs[0][1]=0;
	docs[1][0]=1;
	docs[1][1]=1;
	prior_alpha=malloc(sizeof(float) * num_words[num_topic]);
	prior_alpha[0]=1;
	prior_alpha[1]=1;
	prior_beta=malloc(sizeof(float) * num_words[num_topic]);
	prior_beta[0]=1;
	prior_beta[1]=1;


	state = pp_new_state();
	printf("> state created\n");

	pp_load_file(state, "parse/models/lda.model");
	printf("> file loaded\n");

	ModelNode* model = model_map_find(state->model_map, state->symbol_table,
			"latent_dirichlet_allocation");
	printf("%s", dump_model(model));

	//query = pp_compile_query("");
	//printf("> condition compiled\n");

	pp_variable_t** param = malloc(sizeof(pp_variable_t*) * 4);
	param[0] = new_pp_int(num_topic);
	param[1] = new_pp_int(num_docs);
	param[2] = new_pp_vector(num_docs);
	PP_VARIABLE_VECTOR_LENGTH(param[2]) = num_docs;
	PP_VARIABLE_VECTOR_VALUE(param[2])[0] = new_pp_int(num_words[0]);
	PP_VARIABLE_VECTOR_VALUE(param[2])[1] = new_pp_int(num_words[1]);
	param[3] = new_pp_int(vocab_size);

	query = pp_compile_query("X[0,0]==0 X[0,1]==0 X[1,0]==1 X[1,1]==1");
	if (!query)
		return 1;
	{
		char buffer[8096];
		pp_query_dump(query, buffer, 8096);
		printf("%s\n", buffer);
	}

	g_sample_method = strdup("Gibbs");

	gibbs_markov_field("topic");

	gibbs_conditional_probability=lda_gibbs_conditional_probability;

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
	pp_trace_dump(traces->trace[max_index], buffer, 8096);
	printf("%s", buffer);

	printf("\nlast sample:\n");
	pp_trace_dump(traces->trace[traces->n - 1], buffer, 8096);
	printf("%s", buffer);

	FILE* trace_dump_file = fopen("trace_dump.txt", "w");
	for (size_t i = 0; i != traces->n; ++i) {
		pp_trace_dump(traces->trace[i], buffer, 8096);
		fprintf(trace_dump_file, "[trace %u]\n", (unsigned) i);
		fprintf(trace_dump_file, "%s", buffer);
	}
	fclose(trace_dump_file);

	// pp_free is broken
	pp_free(state); /* free memory, associated models, instances, queries, and trace stores are deallocated */

	pp_trace_store_destroy(traces);

	pp_query_destroy(query);

	for (int i = 0; i < 4; ++i)
		pp_variable_destroy(param[i]);
	free(param);

#ifdef ENABLE_MEM_PROFILE
	mem_profile_print();
	mem_profile_destroy();
#endif

	return 0;
}

void lda_gibbs_conditional_probability(char* name,int index){
	float cal1,cal2,cal3,cal4;
	int *word_num_of_topic,**word_id_of_topic;
	int ii,jj;
	int tmp_topic;
	int k=0,t=0,doc_id=0,word_id=0;

	gibbs_clear_conditional_probability();
	for(ii=0;ii<num_docs;ii=ii+1){
		if(index<num_words[ii]){
			word_id=index;
			break;
		}
		doc_id=doc_id+1;
		index=index-num_words[ii];
	}
	t=docs[doc_id][word_id];
	word_num_of_topic=malloc(sizeof(int)*num_topic);
	word_id_of_topic=malloc(sizeof(int*)*num_topic);
	for(ii=0;ii<num_topic;ii=ii+1){
		word_num_of_topic[ii]=0;
		word_id_of_topic[ii]=malloc(sizeof(int)*vocab_size);
		for(jj=0;jj<vocab_size;jj=jj+1)
			word_id_of_topic[ii][jj]=0;
	}
	for(ii=0;ii<num_docs;ii=ii+1){
		for(jj=0;jj<num_words[ii];jj=jj+1){
			int tmp_topic=pp_variable_to_int(pp_array_at(pp_array_at(pp_look_up(instance, "topic"),ii),jj));
			word_num_of_topic[tmp_topic]=word_num_of_topic[tmp_topic]+1;
			word_id_of_topic[tmp_topic][docs[ii][jj]]=word_id_of_topic[tmp_topic][docs[ii][jj]]+1;
		}
	}
	for(k=0;k<num_topic;k=k+1){
		/* reduce the sample */
		word_num_of_topic[k]=word_num_of_topic[k]-1;
		word_id_of_topic[k][t]=word_id_of_topic[k][t]-1;
		/* calc prob */
		cal1=word_num_of_topic[k]+prior_alpha[k];
		cal2=num_topic*prior_alpha[k];
		for(ii=0;ii<num_topic;ii=ii+1){
			cal2=cal2+word_num_of_topic[k];
		}
		cal3=word_id_of_topic[k][t]+prior_beta[t];
		cal4=vocab_size*prior_beta[t];
		for(ii=0;ii<vocab_size;ii=ii+1){
			cal4=cal4+word_id_of_topic[k][t];
		}
		/* restore the sample */
		word_num_of_topic[k]=word_num_of_topic[k]+1;
		word_id_of_topic[k][t]=word_id_of_topic[k][t]+1;
		gibbs_add_conditional_probability(k, (cal1*cal3)/(cal2*cal4));
	}
}
