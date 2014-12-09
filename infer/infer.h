#ifndef INFER_H_
#define INFER_H_

#include "../defs.h"
#include "../ppp.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../common/list.h"
#include "../common/ilist.h"
#include "../common/symbol_table.h"
#include "../common/variables.h"
#include "../common/trace.h"
#include "../query/query.h"
#include "../debug.h"

enum {
	PP_SAMPLE_FUNCTION_NORMAL = 0,
	PP_SAMPLE_FUNCTION_MODEL_NOT_FOUND,
	PP_SAMPLE_FUNCTION_INVALID_STATEMENT,
	PP_SAMPLE_FUNCTION_INVALID_EXPRESSION,
	PP_SAMPLE_FUNCTION_NON_SCALAR_TYPE_AS_CONDITION,
	PP_SAMPLE_FUNCTION_UNHANDLED,
	PP_SAMPLE_FUNCTION_INVALID_OPERAND_TYPE,
	PP_SAMPLE_FUNCTION_VECTOR_LENGTH_MISMATCH,
	PP_SAMPLE_FUNCTION_DIVISION_BY_ZERO,
	PP_SAMPLE_FUNCTION_VARIABLE_NOT_FOUND,
	PP_SAMPLE_FUNCTION_SUBSCRIPTING_TO_NON_VECTOR,
	PP_SAMPLE_FUNCTION_NON_INTEGER_SUBSCRIPTION,
	PP_SAMPLE_FUNCTION_NUMBER_OF_PARAMETER_MISMATCH,
	PP_SAMPLE_FUNCTION_NON_INTEGER_LOOP_VARIABLE,
	PP_SAMPLE_FUNCTION_INDEX_OUT_OF_BOUND,

	PP_SAMPLE_FUNCTION_MH_FAIL_TO_INITIALIZE,
	PP_SAMPLE_FUNCTION_QUERY_ERROR,
	PP_SAMPLE_FUNCTION_UNKNOWN_FUNCTION,

	PP_SAMPLE_FUNCTION_ERROR_NUM
};

extern const char* pp_sample_error_string[PP_SAMPLE_FUNCTION_ERROR_NUM];
#define pp_sample_get_error_string(status) (pp_sample_error_string[status])

#define pp_sample_error_return(status, fmt, ...) \
	do {	\
		/* Bug fixed: cache status first in case it's an expression. */	\
		int __pp_sample_error_return_status = status;	\
		ERR_OUTPUT("%s" fmt "\n", pp_sample_get_error_string(__pp_sample_error_return_status), ##__VA_ARGS__);	\
		return __pp_sample_error_return_status;	\
	} while(0)

#define pp_sample_return(status)	\
	do {	\
		/* Bug fixed: have to cache status since it could be an expression */	\
		int __pp_sample_return_status = status;	\
		if (__pp_sample_return_status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			pp_sample_error_return(__pp_sample_return_status, "");	\
		}	\
		else {	\
			pp_sample_normal_return(__pp_sample_return_status);	\
		}	\
	} while(0)

#define pp_sample_normal_return(status)	return PP_SAMPLE_FUNCTION_NORMAL

typedef int (*sample_function_t)(
	struct pp_state_t* state,
	const char* model_name,
	pp_variable_t* param[],
	pp_query_t* query,
	void** internal_data_ptr, 
	pp_trace_t** trace_ptr);

int rejection_sampling(struct pp_state_t* state, const char* model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr);
int mh_sampling(struct pp_state_t* state, const char*  model_name, pp_variable_t* param[], pp_query_t* query, void** internal_data_ptr, pp_trace_t** trace_ptr);

int pp_sample_full_accept(pp_query_t* query, pp_trace_t* trace);

extern unsigned g_sample_iterations;
extern char* g_sample_method;
extern sample_function_t g_sample_function;

#endif
