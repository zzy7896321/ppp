#include "trace.h"
#include "variables.h"
#include "symbol_table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mem_profile.h"

#include "../debug.h"
#include "../config.h"

#define HASH_TABLE_PREFIX trace_var_map
#define HASH_TABLE_KEY_TYPE symbol_t
#define HASH_TABLE_VALUE_TYPE pp_variable_t*
#define HASH_TABLE_DEFINE_STRUCT 1
#define HASH_TABLE_VALUE_DEFAULT_VALUE 0
#define HASH_TABLE_HASH_FUNCTION(key) ((unsigned) (key))
#define HASH_TABLE_KEY_CLONE(var, key) (var = key)
#define HASH_TABLE_VALUE_CLONE(var, value) (var = pp_variable_clone(value))
#define HASH_TABLE_KEY_DESTRUCTOR(key) ((void) 0)
#define HASH_TABLE_VALUE_DESTRUCTOR(value) pp_variable_destroy(value)
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) snprintf(buffer, buf_size, "%d", key)
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) pp_variable_dump(buffer, buf_size, value)
#ifdef ENABLE_MEM_PROFILE
#define HASH_TABLE_ALLOC(type, count) PROFILE_MEM_ALLOC(type, count)
#define HASH_TABLE_DEALLOC(type, ptr, count) PROFILE_MEM_FREE(type, ptr, count)
#endif
#include "hash_table.h"


pp_trace_t* new_pp_trace(symbol_table_t* symbol_table) {
	pp_trace_t* trace = malloc(sizeof(pp_trace_t));
	__pp_trace_init(trace, symbol_table, sizeof(pp_trace_t));

	return trace;
};

void __pp_trace_init(pp_trace_t* trace, symbol_table_t* symbol_table, size_t struct_size) {
	trace->struct_size = struct_size;
	trace->symbol_table = symbol_table;
	trace->logprob = 0.0;

	trace->var_map = new_trace_var_map(8, 0.8);
}

bool pp_trace_set_variable_s(pp_trace_t* trace, symbol_t symbol, pp_variable_t* variable) {
	return trace_var_map_put(trace->var_map, symbol, variable);
}

pp_variable_t** pp_trace_get_variable_ptr_s(pp_trace_t* trace, symbol_t symbol) {
	trace_var_map_node_t* node = trace_var_map_get_node(trace->var_map, symbol);
	return &(node->value);
}

pp_variable_t* pp_trace_find_variable_s(pp_trace_t* trace, symbol_t symbol) {
	return trace_var_map_find(trace->var_map, symbol);
}

bool pp_trace_set_variable(pp_trace_t* trace, const char* varname, pp_variable_t* variable) {
	symbol_t symbol = symbol_table_lookup(trace->symbol_table, varname);
	if (symbol == SYMBOL_NULL) return false;
	return trace_var_map_put(trace->var_map, symbol, variable);
}

pp_variable_t** pp_trace_get_variable_ptr(pp_trace_t* trace, const char* varname) {
	symbol_t symbol = symbol_table_lookup(trace->symbol_table, varname);
	if (symbol == SYMBOL_NULL) return 0;
	trace_var_map_node_t* node = trace_var_map_get_node(trace->var_map, symbol);
	return &(node->value);
}

pp_variable_t* pp_trace_find_variable(pp_trace_t* trace, const char* varname) {
	symbol_t symbol = symbol_table_lookup(trace->symbol_table, varname);
	if (symbol == SYMBOL_NULL) return 0;
	return trace_var_map_find(trace->var_map, symbol);
}

int pp_trace_dump(char* buffer, int buf_size, pp_trace_t* trace) {
	DUMP_START();
	DUMP(buffer, buf_size, "logprob = %f\n", trace->logprob);

	HASH_TABLE_FOR_EACH(trace_var_map, trace->var_map, symbol_t, key, pp_variable_t*, value, {
		const char* name = symbol_table_get_name(trace->symbol_table, key);
		if (!name) name = "(NULL)";
		DUMP(buffer, buf_size, "[%d] %s = ", key, name);
		DUMP_CALL(pp_variable_dump, buffer, buf_size, value);
		DUMP(buffer, buf_size, "\n");
	})

	DUMP_SUCCESS();
}

pp_trace_t* pp_trace_clone(pp_trace_t* trace) {
	/* only clone the pp_trace_t part */
	pp_trace_t* new_trace = malloc(sizeof(pp_trace_t));
	new_trace->struct_size = sizeof(pp_trace_t);
	new_trace->symbol_table = trace->symbol_table;
	new_trace->logprob = trace->logprob;
	
	new_trace->var_map = trace_var_map_clone(trace->var_map);

	return new_trace;
}

pp_trace_t* pp_trace_output(pp_trace_t* trace, int num_output_vars, symbol_t output_vars[]) {
	pp_trace_t* new_trace = malloc(sizeof(pp_trace_t));
	new_trace->struct_size = sizeof(pp_trace_t);
	new_trace->symbol_table = trace->symbol_table;
	new_trace->logprob = trace->logprob;

	new_trace->var_map = new_trace_var_map(num_output_vars * 5 / 4, 1);

	for (int i = 0; i < num_output_vars; ++i) {
		pp_variable_t* var = trace_var_map_find(trace->var_map, output_vars[i]);
		trace_var_map_put(new_trace->var_map, output_vars[i], var);
	}
	
	return new_trace;
}

size_t pp_trace_size(pp_trace_t* trace) {
	return trace_var_map_size(trace->var_map);
}

size_t pp_trace_get_struct_size(pp_trace_t* trace) {
	return trace->struct_size;
}

void pp_trace_destroy(pp_trace_t* trace) {
	if (!trace) return ;
	
	trace_var_map_destroy(trace->var_map);

	free(trace);
}

pp_trace_store_t* new_pp_trace_store(unsigned n) {
	pp_trace_store_t* traces = malloc(sizeof(pp_trace_store_t) + sizeof(pp_trace_t*) * n);
	traces->n = n;
	memset(traces->trace, 0, sizeof(pp_trace_t*) * n);
	return traces;
}

void pp_trace_store_destroy(pp_trace_store_t* traces) {
	for (unsigned i = 0; i < traces->n; ++i)
		pp_trace_destroy(traces->trace[i]);
	free(traces);
}

