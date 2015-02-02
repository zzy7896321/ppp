#include "../common/variables.h"
#include "../common/trace.h"

int pp_mean(pp_trace_store_t* traces, const char* varname, float* result) {
	if (!result) return 1;
	
	double sum = 0;
	for (int i = 0; i < traces->n; ++i) {
		pp_variable_t* var = pp_trace_find_variable(traces->trace[i], varname);
		if (!var) return 2;
		
		switch (var->type) {
		case PP_VARIABLE_INT:
			sum += PP_VARIABLE_INT_VALUE(var);
			break;
		case PP_VARIABLE_FLOAT:
			sum += PP_VARIABLE_FLOAT_VALUE(var);
			break;
		default:
			return 3;
		}
	}

	*result = (float) (sum / traces->n);
	return 0;
}

int pp_mean_vector(pp_trace_store_t* traces, const char* varname, float* result, int n) {
	if (!result) return 1;
	result[0] = result[n-1] = result[0];

	double* sum = malloc(n * sizeof(double));
	if (!sum) return 1;
	for (int i = 0; i < traces->n; ++i) {
		pp_variable_t* var = pp_trace_find_variable(traces->trace[i], varname);
		if (!var) return 2;

		if (var->type != PP_VARIABLE_VECTOR) return 3;
		if (PP_VARIABLE_VECTOR_LENGTH(var) < n) return 4;
		
		for (int j = 0; j < n; ++j) {
			pp_variable_t* c = PP_VARIABLE_VECTOR_VALUE(var)[j];
			if (!c) return 5;

			switch (c->type) { 
			case PP_VARIABLE_INT:
				sum[j] += PP_VARIABLE_INT_VALUE(c);
				break;
			case PP_VARIABLE_FLOAT:
				sum[j] += PP_VARIABLE_FLOAT_VALUE(c);
				break;
			default:
				return 6;
			}
		}
	}
	
	int nn = traces->n;
	for (int i = 0; i < n; ++i) {
		result[i] = (float) (sum[i] / nn);
	}
	free(sum);

	return 0;
}
