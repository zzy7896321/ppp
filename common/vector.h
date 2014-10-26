#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macro_util.h"
#include "../debug.h"

#if defined(VECTOR_PREFIX) && defined(VECTOR_VALUE_TYPE)

/* DECLARATION OF VECTOR */
#ifdef __VEC
#undef __VEC
#endif
#define __VEC VECTOR_PREFIX

#ifdef __VEC_T
#undef __VEC_T
#endif
#define __VEC_T CONCAT(__VEC, _t)

#ifdef __V_T
#undef __V_T
#endif
#define __V_T VECTOR_VALUE_TYPE 

/*
Macros:

The following must be present:
VECTOR_PREFIX: the prefix of the vector type (e.g. with prefix vec, the type name is vec_t, 
	methods are vec_method_name )

VECTOR_VALUE_TYPE: type of values

The following are optional:
VECTOR_DECLARE_ONLY: non-zero value indicates only declaration is desired

VECTOR_DECLARE_DUMP: non-zero value indiactes the dump function will be declared
	automatically set to 1 if VECTOR_VALUE_DUMP is set

The following must be present if VECTOR_DELARE_ONLY is not defined or is set to 0
VECTOR_VALUE_DEFAULT_VALUE:	the default value when get methods find an out-of-bound index
	and resize method initializes new values

The following are optional and only used only when VECTOR_DECLARE_ONLY == 0 or is set to 0
VECCTOR_NO_RANGE_CHECK: non-zero value indicates no range check shoule be performed in get/set methods

VECTOR_ALLOC(type, count): the allocator for the vector instead of malloc if provided

VECTOR_DEALLOC(type, ptr, count): the deallocator for the vector instead of free
	VECTOR_DEALLOC must be set when VECTOR_ALLOC is set
	VECTOR_DEALLOC is ignored if VECTOR_ALLOC is not set

VECTOR_VALUE_CLONE(var, value): the clone function of the type
	assigment(=) is used if VECTOR_VALUE_CLONE is not provided

VECTOR_VALUE_DESTRUCTOR(var): the destructor of the type
	no cleanup is performed when destroying the vector if VECTOR_VALUE_DESTRUCTOR is not provided

VECTOR_VALUE_DUMP: the dump function of the value type
	implies VECTOR_DECLARE_DUMP
*/


#ifdef VEC_METHOD
#undef VEC_METHOD
#endif
#define VEC_METHOD(method_name) CONCAT3(__VEC, _, method_name)

typedef struct __VEC_T __VEC_T;
struct __VEC_T {
	size_t capacity;
	size_t size;
	__V_T* data;
};

/** vec_t* new_vec(int capacity); */
__VEC_T* CONCAT(new_, __VEC)(int capacity);

int VEC_METHOD(push_back)(__VEC_T* vector, __V_T value);

__V_T VEC_METHOD(pop_back)(__VEC_T* vector);

int VEC_METHOD(set)(__VEC_T* vector, size_t index, __V_T value);

int VEC_METHOD(set_no_clone)(__VEC_T* vector, size_t index, __V_T value);

__V_T VEC_METHOD(at)(__VEC_T* vector, size_t index);

__V_T* VEC_METHOD(data)(__VEC_T* vector);

size_t VEC_METHOD(size)(__VEC_T* vector);

size_t VEC_METHOD(capacity)(__VEC_T* vector);

int VEC_METHOD(empty)(__VEC_T* vector);

int VEC_METHOD(resize)(__VEC_T* vector, size_t new_size, __V_T default_value);

int VEC_METHOD(resize_default)(__VEC_T* vector, size_t new_size);

#if (defined(VECTOR_DECLARE_DUMP) && VECTOR_DECLARE_DUMP != 0) || defined(VECTOR_VALUE_DUMP)
int VEC_METHOD(dump)(char* buffer, int buf_size, __VEC_T* vector);
#endif

int VEC_METHOD(reserve)(__VEC_T* vector, size_t new_capacity);

int VEC_METHOD(shrink)(__VEC_T* vector, size_t new_capacity);

int VEC_METHOD(shrink_to_fit)(__VEC_T* vector);

int VEC_METHOD(clear)(__VEC_T* vector);

/** void vec_destroy(vec_t* vector); */
void VEC_METHOD(destroy)(__VEC_T* vector);


#if !defined(VECTOR_DECLARE_ONLY) || VECTOR_DECLARE_ONLY == 0

/* DEFININITION OF VECTOR */
#ifndef VECTOR_VALUE_DEFAULT_VALUE
#error "VECTOR_VALUE_DEFAULT_VALUE is not defined"
#endif

#ifndef VECTOR_ALLOC
#define VECTOR_ALLOC(type, count) (type*) malloc(sizeof(type) * count)

#ifdef VECTOR_DEALLOC
#undef VECTOR_DEALLOC
#endif

#define VECTOR_DEALLOC(type, ptr, count) free(ptr)

#else

#ifndef VECTOR_DEALLOC
#error "VECTOR_DEALLOC is not defined while VECTOR_ALLOC is defined"
#endif

#endif

#if defined(VECTOR_DECLARE_DUMP) && VECTOR_DECLARE_DUMP != 0 && !defined(VECTOR_VALUE_DUMP)
#warning "dump function is declared but not defined"
#endif


__VEC_T* CONCAT(new_, __VEC)(int capacity) {
	__VEC_T* vec = VECTOR_ALLOC(__VEC_T, 1);	
	if (!vec) {
		return 0;
	}

	__V_T* data = VECTOR_ALLOC(__V_T, capacity);
	if (!data) {
		VECTOR_DEALLOC(__VEC_T, vec, 1);
		return 0;
	}

	vec->capacity = capacity;
	vec->size = 0;
	vec->data = data;

	return vec;
}

int VEC_METHOD(increase_capacity)(__VEC_T* vector, size_t at_least) {
	int new_capacity = vector->capacity;
	while (new_capacity > 0 && new_capacity < at_least) new_capacity <<= 1;
	if (new_capacity <= 0) return 0;

	__V_T* data = VECTOR_ALLOC(__V_T, new_capacity);
	if (!data) {
		return 0;
	}

	memcpy(data, vector->data, sizeof(__V_T) * vector->size);
	VECTOR_DEALLOC(__V_T, vector->data, vector->capacity);

	vector->capacity = new_capacity;
	vector->data = data;
	return 1;
}

int VEC_METHOD(push_back)(__VEC_T* vector, __V_T value) {
	if (vector->size == vector->capacity) {
		if (!VEC_METHOD(increase_capacity)(vector, vector->size + 1)) {
			return 0;
		}
	}

	#ifdef VECTOR_VALUE_CLONE
		VECTOR_VALUE_CLONE(vector->data[vector->size], value);
	#else
		vector->data[vector->size] = value;
	#endif
	vector->size++;
	return 1;
}

__V_T VEC_METHOD(pop_back)(__VEC_T* vector) {
	#if !defined(VECTOR_NO_RANGE_CHECK) || VECTOR_NO_RANGE_CHECK == 0
	if (vector->size == 0) {
		return VECTOR_VALUE_DEFAULT_VALUE;
	}	
	#endif

	return vector->data[--vector->size];
}

int VEC_METHOD(set)(__VEC_T* vector, size_t index, __V_T value) {
	#if !defined(VECTOR_NO_RANGE_CHECK) || VECTOR_NO_RANGE_CHECK == 0
	if (index >= vector->size) return 0;
	#endif

	#ifdef VECTOR_VALUE_DESTRUCTOR
		VECTOR_VALUE_DESTRUCTOR(vector->data[index]);
	#endif

	#ifdef VECTOR_VALUE_CLONE
		VECTOR_VALUE_CLONE(vector->data[index], value);
	#else
		vector->data[index] = value;
	#endif

	return 1;
}

int VEC_METHOD(set_no_clone)(__VEC_T* vector, size_t index, __V_T value) {
	#if !defined(VECTOR_NO_RANGE_CHECK) || VECTOR_NO_RANGE_CHECK == 0
	if (index >= vector->size) return 0;
	#endif

	#ifdef VECTOR_VALUE_DESTRUCTOR
		VECTOR_VALUE_DESTRUCTOR(vector->data[index]);
	#endif

	vector->data[index] = value;

	return 1;
}

__V_T VEC_METHOD(at)(__VEC_T* vector, size_t index) {
	#if !defined(VECTOR_NO_RANGE_CHECK) || VECTOR_NO_RANGE_CHECK == 0
	if (index >= vector->size) return VECTOR_VALUE_DEFAULT_VALUE;
	#endif

	return vector->data[index];
}

__V_T* VEC_METHOD(data)(__VEC_T* vector) {
	return vector->data;
}

size_t VEC_METHOD(size)(__VEC_T* vector) {
	return vector->size;	
}

size_t VEC_METHOD(capacity)(__VEC_T* vector) {
	return vector->capacity;
}

int VEC_METHOD(empty)(__VEC_T* vector) {
	return !vector->size;
}

int VEC_METHOD(resize)(__VEC_T* vector, size_t new_size, __V_T default_value) {
	if (new_size == vector->size) return 1;

	if (new_size < vector->size) {
		#ifdef VECTOR_VALUE_DESTRUCTOR
		{
			size_t i;
			for (i = new_size; i != vector->size; ++i) {
				VECTOR_VALUE_DESTRUCTOR(vector->data[i]);
			}
		}
		#endif
	}

	else {
		if (new_size > vector->capacity) {
			if (!VEC_METHOD(increase_capacity)(vector, new_size)) {
				return 0;
			}
		}
		{
			size_t i;
			for (i = vector->size; i != new_size; ++i) {
				vector->data[i] = default_value;
			}
		}
	}

	vector->size = new_size;
	return 1;
}

int VEC_METHOD(resize_default)(__VEC_T* vector, size_t new_size) {
	return VEC_METHOD(resize)(vector, new_size, VECTOR_VALUE_DEFAULT_VALUE);
}

#ifdef VECTOR_VALUE_DUMP
int VEC_METHOD(dump)(char* buffer, int buf_size, __VEC_T* vector) {
	DUMP_START();
	DUMP(buffer, buf_size, "[");
	if (vector->size) {
		DUMP_CALL(VECTOR_VALUE_DUMP, buffer, buf_size, vector->data[0]);
		{
			size_t i;
			for (i = 1; i < vector->size; ++i) {
				DUMP(buffer, buf_size, ", ");
				DUMP_CALL(VECTOR_VALUE_DUMP, buffer, buf_size, vector->data[i]);
			}
		}
	}
	DUMP(buffer, buf_size, "]");
	DUMP_SUCCESS();
}
#endif

int VEC_METHOD(reserve)(__VEC_T* vector, size_t new_capacity) {
	if (new_capacity > vector->capacity) {
		__V_T* data = VECTOR_ALLOC(__V_T, new_capacity);
		if (!data) {
			return 0;
		}

		memcpy(data, vector->data, sizeof(__V_T) * vector->size);
		VECTOR_DEALLOC(__V_T, vector->data, vector->capacity);

		vector->capacity = new_capacity;
		vector->data = data;
	}	
	return 1;
}

int VEC_METHOD(shrink)(__VEC_T* vector, size_t new_capacity) {
	if (new_capacity < vector->size) return 0;
	if (new_capacity >= vector->capacity) return 1;

	__V_T* data = VECTOR_ALLOC(__V_T, new_capacity);
	if (!data) {
		return 0;
	}

	memcpy(data, vector->data, sizeof(__V_T) * vector->size);
	VECTOR_DEALLOC(__V_T, vector->data, vector->capacity);

	vector->capacity = new_capacity;
	vector->data = data;
	return 1;
}

int VEC_METHOD(shrink_to_fit)(__VEC_T* vector) {
	VEC_METHOD(shrink)(vector, (vector->size) ? vector->size : 1);
	return 1;
}

int VEC_METHOD(clear)(__VEC_T* vector) {
	return VEC_METHOD(resize)(vector, 0, VECTOR_VALUE_DEFAULT_VALUE);
}

void VEC_METHOD(destroy)(__VEC_T* vector) {
	#ifdef VECTOR_VALUE_DESTRUCTOR
	{
		size_t i;
		for (i = 0; i != vector->size; ++i) {
			VECTOR_VALUE_DESTRUCTOR(vector->data[i]);
		}
	}
	#endif

	VECTOR_DEALLOC(__V_T, vector->data, vector->capacity);
	VECTOR_DEALLOC(__VEC_T, vector, 1);
}

#undef VECTOR_VALUE_DEFAULT_VALUE

#ifdef VECTOR_VALUE_CLONE 
#undef VECTOR_VALUE_CLONE
#endif

#ifdef VECTOR_VALUE_DESTRUCTOR
#undef VECTOR_VALUE_DESTRUCTOR
#endif

#undef VECTOR_ALLOC

#undef VECTOR_DEALLOC

#ifdef VECTOR_NO_RANGE_CHECK
#undef VECTOR_NO_RANGE_CHECK
#endif

#ifdef VECTOR_VALUE_DUMP
#undef VECTOR_VALUE_DUMP
#endif

#undef VECTOR_DECLARE_ONLY

#endif	/* VECTOR_DECLARE_ONLY */

#undef __VEC
#undef ___VEC_T
#undef ___V_T
#undef VEC_METHOD

#undef VECTOR_PREFIX
#undef VECTOR_VALUE_TYPE

#ifdef VECTOR_DECLARE_DUMP
#undef VECTOR_DECLARE_DUMP
#endif

#else

#error "VECTOR_PREFIX and VECTOR_VALUE_TYPE must be defined before including vector.h"

#endif	/* defined(VECTOR_PREFIX) && defined(VECTOR_VALUE_TYPE) */
