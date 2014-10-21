#ifndef INFER_H_
#define INFER_H_

//#include "../defs.h"

#include "../defs.h"
#include "../ppp.h"
#include "../parse/parse.h"
#include "../parse/interface.h"
#include "../parse/list.h"
#include "../parse/ilist.h"
#include "../parse/symbol_table.h"
#include "variables.h"
#include "trace.h"
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

	PP_SAMPLE_FUNCTION_ERROR_NUM
};

extern const char* pp_sample_error_string[PP_SAMPLE_FUNCTION_ERROR_NUM];
#define pp_sample_get_error_string(status) (pp_sample_error_string[status])

#define pp_sample_error_return(status, fmt, ...) \
	do {	\
		ERR_OUTPUT("%s" fmt "\n", pp_sample_get_error_string(status), ##__VA_ARGS__);	\
		return status;	\
	} while(0)

#define pp_sample_return(status)	\
	do {	\
		if (status != PP_SAMPLE_FUNCTION_NORMAL) {	\
			pp_sample_error_return(status, "");	\
		}	\
		else {	\
			pp_sample_normal_return(status);	\
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

extern unsigned g_sample_iterations;
extern char* g_sample_method;
extern sample_function_t g_sample_function;

/*struct pp_trace_store_t {
	struct pp_instance_t* instance;
    int num_iters;
    int num_verts;
    int num_accepts;
    float t[1];
};*/

/*int rejection_sampling(struct pp_instance_t* instance, acceptor_t accept,
                       name_to_value_t F, void* raw_data, 
                       vertices_handler_t add_trace, void* add_trace_data);

int trace_store_insert(struct pp_instance_t* instance, void* raw_data);

float getsample(struct BNVertexDraw* vertexDraw);
float getcomp(struct BNVertexCompute* vertexComp);
*/
/************************************** Sample Functions ******************************************/

float randomC();
float randomR();
float randomL();

int flip(float p);
float flip_logprob(int value, float p);

float flipD();
float flipD_logprob(float value);

float log_flip(float p);
float log_flip_logprob(float value, float p);

int multinomial(float theta[], int n);
float multinomial_logprob(int m, float theta[], int n);

float uniform(float low, float high);
float uniformDiscrete(int low, int high);

float gaussian(float mu, float sigma);
float gaussian_logprob(float x, float mu, float sigma);

float gamma1(float a, float b); // avoid name conflict with gcc built-in function gamma
float gamma_logprob(float x, float a, float b);

float beta(float a, float b);
float beta_logprob(float x, float a, float b);

float binomial(float p, int n);
float binomial_lgoprob(float s, float p, int n);

float poisson(int mu);
float poisson_logprob(float k, int mu);

float* dirichlet(float alpha[], int n);
float dirichlet_logprob(float theta[], float alpha[], int n);

#endif
