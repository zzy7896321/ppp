#ifndef REJECTION_H
#define REJECTION_H

#include "infer.h"
#include "variables.h"
#include "trace.h"

int rejection_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr);

#endif
