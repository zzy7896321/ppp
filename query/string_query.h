#ifndef QUERY_STRING_QUERY_H
#define QUERY_STRING_QUERY_H

#include "../common/variables.h"
#include "../common/trace.h"
#include "../common/ilist.h"

#include "query.h"

typedef enum pp_string_query_compare_t {
		PP_QUERY_EQ, 
		PP_QUERY_NE, 
		PP_QUERY_LT, 
		PP_QUERY_GT, 
		PP_QUERY_GE, 
		PP_QUERY_LE} pp_string_query_compare_t;
extern const char* pp_string_query_compare_string[]; 

typedef struct pp_string_query_t {
	pp_query_t super;
	char* varname;
	ilist_entry_t* index;
	pp_string_query_compare_t compare;
	pp_variable_t* threshold;
	struct pp_string_query_t* next;	
} pp_string_query_t;

pp_string_query_t* new_pp_string_query(const char* varname, ilist_entry_t* index,
		pp_string_query_compare_t compare, pp_variable_t* threshold, pp_string_query_t* next);
int pp_string_query_dump(pp_string_query_t* query, char* buffer, int buf_size);
void pp_string_query_destroy(pp_string_query_t* query);

pp_query_t* pp_compile_string_query(const char* query_string);
void pp_compiled_query_destroy(pp_query_t* query);

#endif
