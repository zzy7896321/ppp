#ifndef TRACE_H
#define TRACE_H

#include "variables.h"

#include <string.h>
#include <stddef.h>

#include "symbol_table.h"

typedef struct trace_var_map_t trace_var_map_t;

typedef struct pp_trace_t pp_trace_t;

struct pp_trace_t {
	size_t struct_size;	// to distinguish structs inherited from pp_trace_t
	symbol_table_t* symbol_table;
	trace_var_map_t* var_map;
	float logprob;
};

pp_trace_t* new_pp_trace(symbol_table_t* symbol_table);

void __pp_trace_init(pp_trace_t* trace, symbol_table_t* symbol_table, size_t struct_size);

bool pp_trace_set_variable_s(pp_trace_t* trace, symbol_t symbol, pp_variable_t* variable);

pp_variable_t** pp_trace_get_variable_ptr_s(pp_trace_t* trace, symbol_t symbol);

pp_variable_t* pp_trace_find_variable_s(pp_trace_t* trace, symbol_t symbol);

bool pp_trace_set_variable(pp_trace_t* trace, const char* varname, pp_variable_t* variable);

pp_variable_t** pp_trace_get_variable_ptr(pp_trace_t* trace, const char* varname);

pp_variable_t* pp_trace_find_variable(pp_trace_t* trace, const char* varname);

int pp_trace_dump(char* buffer, int buf_size, pp_trace_t* trace);

pp_trace_t* pp_trace_clone(pp_trace_t* trace);

pp_trace_t* pp_trace_output(pp_trace_t* trace, int num_output_vars, symbol_t output_vars[]);

size_t pp_trace_size(pp_trace_t* trace);

size_t pp_trace_get_struct_size(pp_trace_t* trace);

void pp_trace_clear(pp_trace_t* trace);

void pp_trace_destroy(pp_trace_t* trace);

typedef struct pp_trace_store_t {
	unsigned n;
	pp_trace_t* trace[];
} pp_trace_store_t;

pp_trace_store_t* new_pp_trace_store(unsigned n);

void pp_trace_store_destroy(pp_trace_store_t* traces);

#endif
