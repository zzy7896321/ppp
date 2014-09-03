#include "variables.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

pp_variable_t* new_pp_int(int value) {
	pp_int_t* var = malloc(sizeof(pp_int_t));
	var->super.type = INT;
	var->value = value;
	return (pp_variable_t*) var;
}

pp_variable_t* new_pp_float(float value) {
	pp_float_t* var = malloc(sizeof(pp_float_t));
	var->super.type = FLOAT;
	var->value = value;
	return (pp_variable_t*) var;
}

pp_variable_t* new_pp_vector(size_t capacity) {
	pp_vector_t* var = malloc(sizeof(pp_vector_t));
	var->super.type = VECTOR;
	var->length = 0;
	var->capacity = capacity;
	var->value = calloc(capacity, sizeof(pp_variable_t*));
	return (pp_variable_t*) var;
}

pp_variable_t* pp_variable_clone(pp_variable_t* variable) {
	if (!variable) return 0;

	switch (variable->type) {
	case INT:
		{
			return new_pp_int(PP_VARIABLE_INT_VALUE(variable));
		}
	case FLOAT:
		{
			return new_pp_float(PP_VARIABLE_FLOAT_VALUE(variable));
		}
	case VECTOR:
		{
			pp_vector_t* vec = (pp_vector_t*) new_pp_vector(PP_VARIABLE_VECTOR_CAPACITY(variable));
			size_t length = PP_VARIABLE_VECTOR_LENGTH(variable);
			for (size_t i = 0; i != length; ++i) {
				vec->value[vec->length++] = pp_variable_clone(PP_VARIABLE_VECTOR_VALUE(variable)[i]);
			} 
			return (pp_variable_t*) vec;
		}
	}

	return 0;
}

int pp_variable_dump(pp_variable_t* variable, char* buffer, int buf_size) {
	if (!buf_size) return -1;

	buffer[0] = '\0';

	if (!variable) {
		return snprintf(buffer, buf_size, "NULL");
	}

	switch (variable->type) {
	case INT:
		return snprintf(buffer, buf_size, "%d", PP_VARIABLE_INT_VALUE(variable));
	case FLOAT:
		return snprintf(buffer, buf_size, "%f", PP_VARIABLE_FLOAT_VALUE(variable));
	case VECTOR:
		{
			int num_written = 0;
			size_t length = PP_VARIABLE_VECTOR_LENGTH(variable);
			for (size_t i = 0; i != length; ++i) {
				num_written += pp_variable_dump(PP_VARIABLE_VECTOR_VALUE(variable)[i], buffer + num_written, buf_size - num_written);
			} 

			return num_written;
		}
	}

	return 0;
}

int pp_variable_to_int(pp_variable_t* variable) {
	switch (variable->type) {
	case INT:
		return PP_VARIABLE_INT_VALUE(variable);
	case FLOAT:
		return (int) PP_VARIABLE_FLOAT_VALUE(variable);
	case VECTOR:
		return 0;
	}
	return 0;
}

float pp_variable_to_float(pp_variable_t* variable) {
	switch (variable->type) {
	case INT:
		return (float) PP_VARIABLE_INT_VALUE(variable);
	case FLOAT:
		return PP_VARIABLE_FLOAT_VALUE(variable);
	case VECTOR:
		return 0;
	}
	return 0;
}

float* pp_variable_to_float_vector(pp_variable_t* variable) {
	switch (variable->type) {
	case INT:
		return 0;
	case FLOAT:
		return 0; 
	case VECTOR:
		{
			size_t length = PP_VARIABLE_VECTOR_LENGTH(variable);
			float* vec = malloc(length * sizeof(float));
			for (size_t i = 0; i != length; ++i) {
				vec[i] = pp_variable_to_float(PP_VARIABLE_VECTOR_VALUE(variable)[i]);
			}

			return vec;
		}
	}
	return 0;
}
int* pp_variable_to_int_vector(pp_variable_t* variable) {
	switch (variable->type) {
	case INT:
		return 0;
	case FLOAT:
		return 0; 
	case VECTOR:
		{
			size_t length = PP_VARIABLE_VECTOR_LENGTH(variable);
			int* vec = malloc(length * sizeof(int));
			for (size_t i = 0; i != length; ++i) {
				vec[i] = pp_variable_to_int(PP_VARIABLE_VECTOR_VALUE(variable)[i]);
			}

			return vec;
		}
	}
	return 0;
}

int pp_variable_equal(pp_variable_t* lhs, pp_variable_t* rhs) {
	if (!lhs) {
		return !rhs;
	}

	switch (lhs->type) {
	case INT:
		switch (rhs->type) {
		case INT:
			return PP_VARIABLE_INT_VALUE(lhs) == PP_VARIABLE_INT_VALUE(rhs);
		case FLOAT:
			return PP_VARIABLE_INT_VALUE(lhs) == PP_VARIABLE_FLOAT_VALUE(rhs);
		case VECTOR:
			return 0;	
		}
		break;
	case FLOAT:
		switch (rhs->type) {
		case INT:
			return PP_VARIABLE_FLOAT_VALUE(lhs) == PP_VARIABLE_INT_VALUE(rhs);
		case FLOAT:
			return PP_VARIABLE_FLOAT_VALUE(lhs) == PP_VARIABLE_FLOAT_VALUE(rhs);
		case VECTOR:
			return 0;	
		}
		break;
	case VECTOR:
		switch (rhs->type) {
		case INT:
			return 0;
		case FLOAT:
			return 0;
		case VECTOR:
			{
				size_t length = PP_VARIABLE_VECTOR_LENGTH(lhs);
				if (length != PP_VARIABLE_VECTOR_LENGTH(rhs)) {
					return 0;
				}
				size_t i;
				for (i = 0; i != length; ++i) {
					if (!pp_variable_equal(PP_VARIABLE_VECTOR_VALUE(lhs)[i], PP_VARIABLE_VECTOR_VALUE(rhs)[i])) {
						break;
					}
				}
				return i == length;
			}
		}
		break;
	}

	return 0;
}

void pp_variable_destroy(pp_variable_t* variable) {
	if (!variable) return ;

	switch (variable->type) {
	case INT:
		free(variable);
		break;
	case FLOAT:
		free(variable);
		break;
	case VECTOR:
		{
			pp_vector_t* var = (pp_vector_t*) variable;
			for (unsigned i = 0; i != var->length; ++i) {
				pp_variable_destroy(var->value[i]);
			}
		}
		free(variable);
		break;
	}
}
