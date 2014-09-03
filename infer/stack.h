#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>

#define DECLARE_STACK_T(prefix, value_type) \
	typedef struct prefix##_stack_t prefix##_stack_t;		\
	struct prefix##_stack_t {				\
		size_t size;		\
		size_t capacity;		\
		value_type* data;		\
	}	

#define DECLARE_STACK_NEW(prefix, value_type) \
	prefix##_stack_t* new_##prefix##_stack(size_t capacity)

#define DECLARE_STACK_TOP(prefix, value_type)	\
	value_type prefix##_stack_top(prefix##_stack_t* stack)

#define DECLARE_STACK_POP(prefix, value_type)	\
	int prefix##_stack_pop(prefix##_stack_t* stack)

#define DECLARE_STACK_PUSH(prefix, value_type)	\
	void prefix##_stack_push(prefix##_stack_t* stack, value_type value)

#define DECLARE_STACK_SIZE(prefix, value_type)	\
	size_t prefix##_stack_size(prefix##_stack_t* stack)

#define DECLARE_STACK_CAPACITY(prefix, value_type)	\
	size_t prefix##_stack_capacity(prefix##_stack_t* stack)

#define DECLARE_STACK_EMPTY(prefix, value_type)	\
	int prefix##_stack_empty(prefix##_stack_t* stack)

#define DECLARE_STACK_CLEAR(prefix, value_type)	\
	void prefix##_stack_clear(prefix##_stack_t* stack)

#define DECLARE_STACK_DESTROY(prefix, value_type)	\
	void prefix##_stack_destroy(prefix##_stack_t* stack)

#define DECLARE_STACK(prefix, value_type)	\
	DECLARE_STACK_T(prefix, value_type);	\
	DECLARE_STACK_NEW(prefix, value_type); \
	DECLARE_STACK_TOP(prefix, value_type);	\
	DECLARE_STACK_POP(prefix, value_type);	\
	DECLARE_STACK_PUSH(prefix, value_type);	\
	DECLARE_STACK_SIZE(prefix, value_type);	\
	DECLARE_STACK_CAPACITY(prefix, value_type);	\
	DECLARE_STACK_EMPTY(prefix, value_type);	\
	DECLARE_STACK_CLEAR(prefix, value_type);	\
	DECLARE_STACK_DESTROY(prefix, value_type);

#define DEFINE_STACK(prefix, value_type)	\
	DECLARE_STACK_NEW(prefix, value_type) {			\
		prefix##_stack_t* stack = malloc(sizeof(prefix##_stack_t));			\
		stack->size = 0;			\
		stack->capacity = capacity;			\
		stack->data = malloc(sizeof(value_type) * capacity);			\
		return stack;			\
	}		\
		\
	DECLARE_STACK_TOP(prefix, value_type) {		\
		if (stack->size) {		\
			return stack->data[stack->size-1];		\
		}		\
		else {		\
			return STACK_DEFAULT_VALUE;		\
		}		\
	}		\
		\
	DECLARE_STACK_POP(prefix, value_type) {		\
		if (!stack->size) {		\
			return 0;		\
		}		\
		\
		STACK_DESTROY_VALUE(stack->data[stack->size-1]);		\
		--stack->size;		\
		return 1;		\
	}		\
		\
	DECLARE_STACK_PUSH(prefix, value_type) {		\
		if (stack->size == stack->capacity) {		\
			value_type* data = malloc(sizeof(value_type) * (stack->capacity * 2));		\
			memcpy(data, stack->data, sizeof(value_type) * stack->capacity);		\
			free(stack->data);		\
			stack->data = data;		\
			stack->capacity *= 2;		\
		}		\
		\
		stack->data[stack->size++] = value;		\
	}		\
		\
	DECLARE_STACK_SIZE(prefix, value_type) {		\
		return stack->size;		\
	}		\
		\
	DECLARE_STACK_CAPACITY(prefix, value_type) {		\
		return stack->capacity;		\
	}		\
		\
	DECLARE_STACK_EMPTY(prefix, value_type) {		\
		return !! stack->size;		\
	}		\
		\
	DECLARE_STACK_CLEAR(prefix, value_type) {		\
		/* the loop can be eliminated if STACK_DESTROY_VALUE is empty and -O2 is enabled */		\
		for (size_t i = 0; i < stack->size; ++i) {		\
			STACK_DESTROY_VALUE(stack->data[i]);		\
		}		\
		stack->size = 0;		\
	}		\
		\
	DECLARE_STACK_DESTROY(prefix, value_type) {		\
		prefix##_stack_clear(stack);		\
		free(stack->data);		\
		free(stack);		\
	}

#endif
