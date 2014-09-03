#ifndef VARIABLES_H
#define VARIABLES_H

#include <stddef.h>

typedef struct pp_variable_t {
	enum {INT, FLOAT, VECTOR} type;
	float logprob;
} pp_variable_t;

typedef struct pp_int_t {
	pp_variable_t super;
	int value;
} pp_int_t;

typedef struct pp_float_t {
	pp_variable_t super;
	float value;
} pp_float_t;

typedef struct pp_vector_t {
	pp_variable_t super;
	size_t length;
	size_t capacity;
	pp_variable_t** value;
} pp_vector_t;

pp_variable_t* new_pp_int(int value);
pp_variable_t* new_pp_float(float value);
pp_variable_t* new_pp_vector(size_t capacity);

#define PP_VARIABLE_LOGPROB(pvar) (((pp_variable_t*) pvar)->logprob)

#define PP_VARIABLE_INT_VALUE(pvar) (((pp_int_t*) pvar)->value)
#define PP_VARIABLE_FLOAT_VALUE(pvar) (((pp_float_t*) pvar)->value)

#define PP_VARIABLE_VECTOR_LENGTH(pvar) (((pp_vector_t*) pvar)->length)
#define PP_VARIABLE_VECTOR_CAPACITY(pvar) (((pp_vector_t*) pvar)->capacity)
#define PP_VARIABLE_VECTOR_VALUE(pvar) (((pp_vector_t*) pvar)->value)

pp_variable_t* pp_variable_clone(pp_variable_t* variable);

int pp_variable_dump(pp_variable_t* variable, char* buffer, int buf_size);

int pp_variable_to_int(pp_variable_t* variable);
float pp_variable_to_float(pp_variable_t* variable);
float* pp_variable_to_float_vector(pp_variable_t* variable);
int* pp_variable_to_int_vector(pp_variable_t* variable);

int pp_variable_equal(pp_variable_t* lhs, pp_variable_t* rhs);

void pp_variable_destroy(pp_variable_t* variable);

#endif
