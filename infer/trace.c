#include "trace.h"
#include "variables.h"
#include "hash_table.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

pp_trace_t* new_pp_trace() {
	pp_trace_t* trace = malloc(sizeof(pp_trace_t));
	pp_trace_init(trace, sizeof(pp_trace_t));

	return trace;
};

void pp_trace_init(pp_trace_t* trace, size_t struct_size) {
	trace->struct_size = struct_size;
	trace->variable_map = new_variable_hash_table(0x10000);
	trace->logprob = 0.0;
}

int pp_trace_dump(pp_trace_t* trace, char* buffer, int buf_size) {
	int num_written = 0;
	buffer[0] = '\0';
	num_written += snprintf(buffer + num_written, buf_size - num_written, "logprob = %f\n", trace->logprob);
	num_written += variable_hash_table_dump(trace->variable_map, buffer + num_written, buf_size - num_written);
	return num_written;
}

pp_trace_t* pp_trace_clone(pp_trace_t* trace) {
	/* only clone the pp_trace_t part */
	pp_trace_t* new_trace = malloc(sizeof(pp_trace_t));
	new_trace->struct_size = sizeof(pp_trace_t);
	new_trace->variable_map = variable_hash_table_clone(trace->variable_map);
	new_trace->logprob = trace->logprob;

	return new_trace;
}

void pp_trace_destroy(pp_trace_t* trace) {
	if (!trace) return ;
	variable_hash_table_destroy(trace->variable_map);
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
