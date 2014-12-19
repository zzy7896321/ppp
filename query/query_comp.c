#include "query_comp.h"

#include <stdlib.h>

typedef struct pp_query_comp_t {
	pp_query_t super;
	pp_query_t* first;
	pp_query_t* second;
} pp_query_comp_t;

static pp_variable_t* __pp_query_comp_observe(pp_query_t* query, const char* varname) {
	pp_query_comp_t* c = (pp_query_comp_t*) query;

	pp_query_t* now = c->first;
	pp_variable_t* var = now->observe(now, varname);
	if (var) return var;

	now = c->second;
	return now->observe(now, varname);
}

static int __pp_query_comp_accept(pp_query_t* query, pp_trace_t* trace) {
	pp_query_comp_t* c = (pp_query_comp_t*) query;
	
	int r1 = c->first->accept(c->first, trace);
	if (r1 == PP_QUERY_ERROR) return PP_QUERY_ERROR;
	if (r1 == PP_QUERY_REJECTED) return PP_QUERY_REJECTED;

	return c->second->accept(c->second, trace);
}

static int __pp_query_comp_full_accept(pp_query_t* query, pp_trace_t* trace) {
	pp_query_comp_t* c = (pp_query_comp_t*) query;
	
	int r1 = c->first->full_accept(c->first, trace);
	if (r1 == PP_QUERY_ERROR) return PP_QUERY_ERROR;
	if (r1 == PP_QUERY_REJECTED) return PP_QUERY_REJECTED;

	return c->second->full_accept(c->second, trace);
}

void pp_query_comp_destroy(pp_query_comp_t* query);

static void __pp_query_comp_destroy(void* query) {
	pp_query_comp_destroy((pp_query_comp_t*) query);
}

pp_query_comp_t* new_pp_query_comp(pp_query_t* first, pp_query_t* second) {
	pp_query_comp_t* query = malloc(sizeof(pp_query_comp_t));

	query->super.observe = __pp_query_comp_observe;
	query->super.accept = __pp_query_comp_accept;
	query->super.full_accept = __pp_query_comp_full_accept;
	query->super.destroy = __pp_query_comp_destroy;

	query->first = first;
	query->second = second;
	return query;
}

void pp_query_comp_destroy(pp_query_comp_t* query) {
	query->first->destroy(query->first);
	query->second->destroy(query->second);

	free(query);
}

pp_query_t* pp_query_composite(pp_query_t* first, pp_query_t* second) {
	if (!first) return second;
	if (!second) return first;

	return (pp_query_t*) new_pp_query_comp(first, second);
}

