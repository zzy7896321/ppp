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

