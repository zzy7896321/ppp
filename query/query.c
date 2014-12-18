#include "query.h"

int pp_query_always_accept(pp_query_t* query, pp_trace_t* trace) {
	return PP_QUERY_ACCEPTED;
}

pp_variable_t* pp_query_observe_nothing(pp_query_t* query, const char* varname) {
	return 0;
}

void __pp_query_no_condition_destroy(void* query) {}

pp_query_t no_condition = {
	pp_query_observe_nothing, pp_query_always_accept, pp_query_always_accept, __pp_query_no_condition_destroy,
};

pp_query_t* pp_query_no_condition() {
	return &no_condition;
}

void pp_query_destroy(pp_query_t* query) {
	query->destroy(query);
}

