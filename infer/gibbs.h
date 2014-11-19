#ifndef GIBBS_SAMPLER_H
#define GIBBS_SAMPLER_H

#include "infer.h"
#include "../common/trace.h"
#include "../common/variables.h"
#include "../common/stack.h"

extern void (*gibbs_conditional_probability) (char*, int);
void gibbs_markov_field(char*);
void gibbs_clear_conditional_probability();
void gibbs_add_conditional_probability(float val, float prob);
void gibbs_default_conditional_probability(char* name, int index);
int gibbs_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr);

#endif
