#ifndef TRACE_H
#define TRACE_H

#include "variables.h"

#include <string.h>
#include <stddef.h>

#include "hash_table.h.old"

//DECLARE_HASH_TABLE(variable, const char*, pp_variable_t*);
#define HASH_TABLE_PREFIX variable_hash_table
#define HASH_TABLE_KEY_TYPE const char*
#define HASH_TABLE_VALUE_TYPE pp_variable_t*
#define HASH_TABLE_DECLARE_ONLY 1
#define HASH_TABLE_DECLARE_DUMP 1
#define HASH_TABLE_DEFINE_STRUCT 1
#include "hash_table.h"

typedef struct pp_trace_t {
	size_t struct_size;	// to distinguish structs inherited from pp_trace_t
	variable_hash_table_t* variable_map;	
	float logprob;
} pp_trace_t;

pp_trace_t* new_pp_trace();

void pp_trace_init(pp_trace_t* trace, size_t struct_size);

//void pp_trace_set_variable(pp_trace_t* trace, const char* varname, pp_variable_t* variable); 
#define pp_trace_set_variable(trace, varname, variable) \
	(variable_hash_table_put(((pp_trace_t*) trace)->variable_map, varname, variable))

//pp_variable_t* pp_trace_get_variable(pp_trace_t* trace, const char* varname);
#define pp_trace_get_variable(trace, varname) \
	(variable_hash_table_get_node(((pp_trace_t*) trace)->variable_map, varname)->value)

//pp_varaible_t* pp_trace_find_variable(pp_trace_t* trace, const char* varname);
#define pp_trace_find_variable(trace, varname) \
	(variable_hash_table_find(((pp_trace_t*) trace)->variable_map, varname))

int pp_trace_dump(pp_trace_t* trace, char* buffer, int buf_size);

pp_trace_t* pp_trace_clone(pp_trace_t* trace);

//size_t pp_trace_get_struct_size(pp_trace_t* trace);
#define pp_trace_get_struct_size(trace) (trace->struct_size)

void pp_trace_destroy(pp_trace_t* trace);

typedef struct pp_trace_store_t {
	unsigned n;
	pp_trace_t* trace[];
} pp_trace_store_t;

pp_trace_store_t* new_pp_trace_store(unsigned n);

void pp_trace_store_destroy(pp_trace_store_t* traces);

#endif
