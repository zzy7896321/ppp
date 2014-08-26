#ifndef QUERY_H
#define QUERY_H

#include "../infer/variables.h"
#include "../infer/trace.h"

typedef enum pp_query_compare_t {
		PP_QUERY_EQ, 
		PP_QUERY_NE, 
		PP_QUERY_LT, 
		PP_QUERY_GT, 
		PP_QUERY_GE, 
		PP_QUERY_LE} pp_query_compare_t;
extern const char* pp_query_compare_string[]; 

typedef struct pp_query_t {
	char* varname;
	pp_query_compare_t compare;
	pp_variable_t* threshold;
	struct pp_query_t* next;	
} pp_query_t;

pp_query_t* new_pp_query(const char* varname, pp_query_compare_t compare, pp_variable_t* threshold, pp_query_t* next);
int pp_query_dump(pp_query_t* query, char* buffer, int buf_size);
void pp_query_destroy(pp_query_t* query);

pp_query_t* pp_compile_query(const char* query_string);
int pp_query_acceptor(pp_trace_t* trace, pp_query_t* query);

#endif
