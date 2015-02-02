#ifndef QUERY_H
#define QUERY_H

#include "../common/variables.h"
#include "../common/trace.h"
#include "../common/symbol_table.h"

#include "../config.h"

typedef struct pp_query_t pp_query_t;

typedef pp_variable_t* (*pp_observed_name_to_value_t)(pp_query_t* query, const char* varname);

typedef int (*pp_query_acceptor_t)(pp_query_t* query, pp_trace_t* trace);

typedef void (*pp_query_destroy_t)(void* query);

typedef enum pp_query_type_t {
	PP_STRING_QUERY,
	PP_OBSERVATION,
	PP_COMPOSITION
} pp_query_type_t;

struct pp_query_t {
	pp_observed_name_to_value_t observe;
	pp_query_acceptor_t accept;
	pp_query_acceptor_t full_accept;
	pp_query_destroy_t destroy;
};

enum pp_query_accept_result {
	PP_QUERY_ERROR = -1,
	PP_QUERY_REJECTED = 0,
	PP_QUERY_ACCEPTED = 1,
};

int pp_query_always_accept(pp_query_t* query, pp_trace_t* trace);

pp_variable_t* pp_query_observe_nothing(pp_query_t* query, const char* varname);

pp_query_t* pp_query_no_condition();

void pp_query_destroy(pp_query_t* query);

int pp_mean(pp_trace_store_t* traces, const char* varname, float* result);

int pp_mean_vector(pp_trace_store_t* traces, const char* varname, float* result, int n);

#endif /* QUERY_H */

