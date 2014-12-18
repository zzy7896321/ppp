#ifndef QUERY_OBSERVATION_H
#define QUERY_OBSERVATION_H

#include "query.h"
#include "../parse/interface.h"

pp_query_t* pp_query_observe_int(pp_state_t* state, const char* varname, int value);
pp_query_t* pp_query_observe_float(pp_state_t* state, const char* varname, float value);
pp_query_t* pp_query_observe_int_array(pp_state_t* state, const char* varname, int array[], int dim1);
pp_query_t* pp_query_observe_float_array(pp_state_t* state, const char* varname, float array[], int dim1);
pp_query_t* pp_query_observe_int_array_2D(pp_state_t* state, const char* varname, int array[], int dim1, int dim2);
pp_query_t* pp_query_observe_float_array_2D(pp_state_t* state, const char* varname, float array[], int dim1, int dim2);

pp_variable_t* pp_query_observe_value(pp_query_t* query, const char* varname);

#endif	/* QUERY_OBSERVATION */

