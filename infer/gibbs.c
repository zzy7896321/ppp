#include "gibbs.h"

void (*gibbs_conditional_probability) (char*, int);

char* markov_field;
void gibbs_markov_field(char* var){
	markov_field=strdup(var);
	//TODO solve several vars
	return;
}
struct val_prob_t{
	float val;
	float prob;
	struct val_prob_t* next;
};
struct val_prob_t *tmp_conditional_probability;
void gibbs_clear_conditional_probability(){
	struct val_prob_t *tmp;
	while(tmp_conditional_probability!=0){
		tmp=tmp_conditional_probability;
		tmp_conditional_probability=tmp_conditional_probability->next;
		free(tmp);
	}
}
void gibbs_add_conditional_probability(float val, float prob){
	struct val_prob_t *tmp;
	tmp=calloc(sizeof(struct val_prob_t));
	tmp->val=val;
	tmp->prob=prob;
	tmp->next=tmp_conditional_probability;
	tmp_conditional_probability=tmp;
}
void gibbs_default_conditional_probability(char* name, int index){
	//TODO
	gibbs_clear_conditional_probability();
	gibbs_add_conditional_probability(1,1);
}
pp_trace_t * last_trace=0;
int gibbs_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr) {
	/* nothing to do with internal_data_ptr */
	if (!state) {
		//ERR_OUTPUT("clean up internal data\n");
		pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
	}
	/* find model */
	ModelNode* model = model_map_find(state->model_map, state->symbol_table, model_name);
	if (!model) {
		pp_sample_error_return(PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND, ": %s", model_name);
	}
	pp_trace_t* trace = 0;
	if(last_trace==0){
		trace = new_pp_trace();
		last_trace=trace;
		while(0){
		//TODO find the markov field and its dimension
		//TODO initial the trace value with all 0
		};
	}
	else{
		trace = pp_trace_clone((pp_trace_t*) last_trace);
		int d=2;//TODO find the dimensions of the markov field
		int id=(int)(rand()*d);
		while(0){
			gibbs_conditional_probability("topic",id);//TODO solve several vars
		}
		float r=rand();

		struct val_prob_t *tmp=tmp_conditional_probability;
		while(tmp!=0){
			if(r<=tmp->prob){
				r=r-tmp->prob;
				tmp=tmp->next;
			}
			else{
				//set trace with value tmp->val
				break;
			}
		}
	}
	*trace_ptr = trace;
	pp_sample_normal_return(PP_SAMPLE_FUNCTION_NORMAL);
}


