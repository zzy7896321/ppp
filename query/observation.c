#include "observation.h"
#include "../common/variables.h"
#include "../common/trace.h"

#include <stdlib.h>
#include <string.h>

typedef struct pp_query_observation_t pp_query_observation_t;

struct pp_query_observation_t {
	pp_query_t super;

	char* varname;
	pp_variable_t* variable;
};

int pp_query_observation_full_accept(pp_query_t* query, pp_trace_t* trace) {
	pp_query_observation_t* observation = (pp_query_observation_t*) query;

	pp_variable_t* var = pp_trace_find_variable(trace, observation->varname);
	if (!var) return PP_QUERY_REJECTED;

	int compare_result = pp_variable_equal(var, observation->variable);
	if (compare_result) return PP_QUERY_ACCEPTED;
	return PP_QUERY_REJECTED;
}

void pp_query_observation_destroy(pp_query_observation_t* observation);

static void __pp_query_observation_destroy(void* query) {
	pp_query_observation_destroy((pp_query_observation_t*) query);
}

pp_query_observation_t* new_pp_query_observation(const char* varname, pp_variable_t* variable) {
	pp_query_observation_t* query = malloc(sizeof(pp_query_observation_t));

	query->super.observe = pp_query_observe_value;
	query->super.accept = pp_query_always_accept;
	query->super.full_accept = pp_query_observation_full_accept;
	query->super.destroy = __pp_query_observation_destroy;

	query->varname = strdup(varname);
	query->variable = variable;

	return query;
}

void pp_query_observation_destroy(pp_query_observation_t* observation) {
	free(observation->varname);
	pp_variable_destroy(observation->variable);

	free(observation);
}

pp_query_t* pp_query_observe_int(pp_state_t* state, const char* varname, int value) {
	pp_variable_t* var = new_pp_int(value);
	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_query_t* pp_query_observe_float(pp_state_t* state, const char* varname, float value) {
	pp_variable_t* var = new_pp_float(value);
	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_query_t* pp_query_observe_int_array(pp_state_t* state, const char* varname, int array[], int dim1) {
	pp_variable_t* var = pp_variable_int_array_to_vector(array, dim1);
	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_query_t* pp_query_observe_float_array(pp_state_t* state, const char* varname, float array[], int dim1) {
	pp_variable_t* var = pp_variable_float_array_to_vector(array, dim1);
	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_query_t* pp_query_observe_int_array_2D(pp_state_t* state, const char* varname, int array[], int dim1, int dim2) {
	pp_variable_t* var = new_pp_vector(dim1);

	for (int i = 0; i < dim1; ++i) {
		PP_VARIABLE_VECTOR_VALUE(var)[i] = pp_variable_int_array_to_vector(array + i * dim2, dim2);
		++PP_VARIABLE_VECTOR_LENGTH(var);
	}

	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_query_t* pp_query_observe_float_array_2D(pp_state_t* state, const char* varname, float array[], int dim1, int dim2) {
	pp_variable_t* var = new_pp_vector(dim1);

	for (int i = 0; i < dim1; ++i) {
		PP_VARIABLE_VECTOR_VALUE(var)[i] = pp_variable_float_array_to_vector(array + i * dim2, dim2);
		++PP_VARIABLE_VECTOR_LENGTH(var);
	}

	pp_query_t* query = (pp_query_t*) new_pp_query_observation(varname, var);
	return query;
}

pp_variable_t* pp_query_observe_value(pp_query_t* query, const char* varname) {
	if (!query) return 0;
	if (!varname) return 0;

	pp_query_observation_t* observation = (pp_query_observation_t*) query;

	pp_variable_t* var = observation->variable;

	/* find main part */
	const char* start = varname;
	const char* delim = strchr(start, '[');

	if (!delim) {
		if (strcmp(observation->varname, start)) return 0;
	}
	else {
		if (strncmp(observation->varname, start, delim - start)) return 0;
	}
	start = delim;	/* *start == '[' */

	/* index part */
	while (start) {
		if (var->type != PP_VARIABLE_VECTOR) return 0;

		delim = strchr(start, ']');
		if (!delim) return 0;

		int index = atoi(start + 1);
		if (index < 0 || index >= PP_VARIABLE_VECTOR_LENGTH(var)) return 0;
		var = PP_VARIABLE_VECTOR_VALUE(var)[index];

		start = delim + 1;
		if (*start == '\0') start = 0;
		else if (*start != '[') return 0;
	}

	return pp_variable_clone(var);
}


