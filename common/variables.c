#include "variables.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

pp_variable_t* new_pp_int(int value) {
	pp_int_t* var = malloc(sizeof(pp_int_t));
	var->super.type = PP_VARIABLE_INT;
	var->value = value;
	return (pp_variable_t*) var;
}

pp_variable_t* new_pp_float(float value) {
	pp_float_t* var = malloc(sizeof(pp_float_t));
	var->super.type = PP_VARIABLE_FLOAT;
	var->value = value;
	return (pp_variable_t*) var;
}

pp_variable_t* new_pp_vector(size_t capacity) {
	pp_vector_t* var = malloc(sizeof(pp_vector_t));
	var->super.type = PP_VARIABLE_VECTOR;
	var->length = 0;
	var->capacity = capacity;
	var->value = calloc(capacity, sizeof(pp_variable_t*));
	return (pp_variable_t*) var;
}

int pp_variable_vector_resize(pp_vector_t* vector, int new_size) {
	if (!vector) return 0;

	if (new_size < 0) new_size = 0;

	int length = PP_VARIABLE_VECTOR_LENGTH(vector);
	if (new_size < length) {
		for (int i = new_size; i != length; ++i) {
			pp_variable_destroy(PP_VARIABLE_VECTOR_VALUE(vector)[i]);	
			PP_VARIABLE_VECTOR_VALUE(vector)[i] = 0;
		}
		PP_VARIABLE_VECTOR_LENGTH(vector) = new_size; 
	}

	else if (new_size > length) {
		if (new_size > PP_VARIABLE_VECTOR_CAPACITY(vector)) {
			if (!pp_variable_vector_increase_capacity(vector, new_size)) {
				return 0;
			}
		}
		PP_VARIABLE_VECTOR_LENGTH(vector) = new_size;
	}

	return 1;
}

int pp_variable_vector_increase_capacity(pp_vector_t* vector, int size_to_fit) {
	int new_capacity = PP_VARIABLE_VECTOR_CAPACITY(vector);
	while (size_to_fit > new_capacity && new_capacity > 0) {
		new_capacity *= 2;
	}
	
	if (new_capacity < size_to_fit) {
		return 0;
	}

	if (new_capacity > PP_VARIABLE_VECTOR_CAPACITY(vector)) {
		pp_variable_t** new_arr = calloc(new_capacity, sizeof(pp_variable_t*));
		if (!new_arr) {
			return 0;
		}
		memcpy(new_arr, vector->value, sizeof(pp_variable_t*) * vector->length);
		free(vector->value);
		vector->value = new_arr;
		vector->capacity = new_capacity;
	}
	return 1;
}

pp_variable_t* pp_variable_clone(pp_variable_t* variable) {
	if (!variable) return 0;

	switch (variable->type) {
	case PP_VARIABLE_INT:
		{
			return new_pp_int(PP_VARIABLE_INT_VALUE(variable));
		}
	case PP_VARIABLE_FLOAT:
		{
			return new_pp_float(PP_VARIABLE_FLOAT_VALUE(variable));
		}
	case PP_VARIABLE_VECTOR:
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
	case PP_VARIABLE_INT:
		return snprintf(buffer, buf_size, "%d", PP_VARIABLE_INT_VALUE(variable));
	case PP_VARIABLE_FLOAT:
		return snprintf(buffer, buf_size, "%f", PP_VARIABLE_FLOAT_VALUE(variable));
	case PP_VARIABLE_VECTOR:
		{
			int num_written = 0;
			num_written += snprintf(buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0, "[");
			size_t length = PP_VARIABLE_VECTOR_LENGTH(variable);
			for (size_t i = 0; i != length; ++i) {
				num_written += pp_variable_dump(PP_VARIABLE_VECTOR_VALUE(variable)[i], buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0);;
				num_written += snprintf(buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0, " ");
			} 
			num_written += snprintf(buffer + num_written, (buf_size >= num_written) ? (buf_size - num_written) : 0, "]");

			return num_written;
		}
	}

	return 0;
}

int pp_variable_to_int(pp_variable_t* variable) {
	switch (variable->type) {
	case PP_VARIABLE_INT:
		return PP_VARIABLE_INT_VALUE(variable);
	case PP_VARIABLE_FLOAT:
		return (int) PP_VARIABLE_FLOAT_VALUE(variable);
	case PP_VARIABLE_VECTOR:
		return 0;
	}
	return 0;
}

float pp_variable_to_float(pp_variable_t* variable) {
	switch (variable->type) {
	case PP_VARIABLE_INT:
		return (float) PP_VARIABLE_INT_VALUE(variable);
	case PP_VARIABLE_FLOAT:
		return PP_VARIABLE_FLOAT_VALUE(variable);
	case PP_VARIABLE_VECTOR:
		return 0;
	}
	return 0;
}

float* pp_variable_to_float_vector(pp_variable_t* variable) {
	switch (variable->type) {
	case PP_VARIABLE_INT:
		return 0;
	case PP_VARIABLE_FLOAT:
		return 0; 
	case PP_VARIABLE_VECTOR:
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
	case PP_VARIABLE_INT:
		return 0;
	case PP_VARIABLE_FLOAT:
		return 0; 
	case PP_VARIABLE_VECTOR:
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
	case PP_VARIABLE_INT:
		switch (rhs->type) {
		case PP_VARIABLE_INT:
			return PP_VARIABLE_INT_VALUE(lhs) == PP_VARIABLE_INT_VALUE(rhs);
		case PP_VARIABLE_FLOAT:
			return PP_VARIABLE_INT_VALUE(lhs) == PP_VARIABLE_FLOAT_VALUE(rhs);
		case PP_VARIABLE_VECTOR:
			return 0;	
		}
		break;
	case PP_VARIABLE_FLOAT:
		switch (rhs->type) {
		case PP_VARIABLE_INT:
			return PP_VARIABLE_FLOAT_VALUE(lhs) == PP_VARIABLE_INT_VALUE(rhs);
		case PP_VARIABLE_FLOAT:
			return PP_VARIABLE_FLOAT_VALUE(lhs) == PP_VARIABLE_FLOAT_VALUE(rhs);
		case PP_VARIABLE_VECTOR:
			return 0;	
		}
		break;
	case PP_VARIABLE_VECTOR:
		switch (rhs->type) {
		case PP_VARIABLE_INT:
			return 0;
		case PP_VARIABLE_FLOAT:
			return 0;
		case PP_VARIABLE_VECTOR:
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

pp_variable_t* pp_variable_float_array_to_vector(float arr[], int n) {
	pp_variable_t* vec = new_pp_vector((size_t) n);
	for (int i = 0; i != n; ++i) {
		PP_VARIABLE_VECTOR_VALUE(vec)[i] = new_pp_float(arr[i]);
	}
	PP_VARIABLE_VECTOR_LENGTH(vec) = n;
	return vec;
}

pp_variable_t* pp_variable_int_array_to_vector(int arr[], int n) {
	pp_variable_t* vec = new_pp_vector((size_t) n);
	for (int i = 0; i != n; ++i) {
		PP_VARIABLE_VECTOR_VALUE(vec)[i] = new_pp_int(arr[i]);
	}
	PP_VARIABLE_VECTOR_LENGTH(vec) = n;
	return vec;
}

void pp_variable_destroy(pp_variable_t* variable) {
	if (!variable) return ;

	switch (variable->type) {
	case PP_VARIABLE_INT:
		free(variable);
		break;
	case PP_VARIABLE_FLOAT:
		free(variable);
		break;
	case PP_VARIABLE_VECTOR:
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
