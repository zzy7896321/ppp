#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "macro_util.h"
#include "../debug.h"

/*
 * Macros.
 * The following macros must be present before include hash_table.h:
 * HASH_TABLE_PREFIX: the prefix
 *
 * HASH_TABLE_KEY_TYPE: the type of keys
 *
 * HASH_TABLE_VALUE_TYPE: the type of values
 *
 * The following are optional:
 * HASH_TABLE_DECLARE_ONLY: non-zero value indicates only declaration is desired
 *
 * HASH_TABLE_DECLARE_DUMP: non-zero value indicates the dump function should be declared
 *
 * HASH_TABLE_DEFINE_STRUCT: non-zero value indicates struct should be defined
 * 	when HASH_TABLE_DECLARE_ONLY != 0, it defaults to 1. O.w, 0.
 *
 * The following must be present if VECTOR_DECLARE_ONLY is not defined or is set to 0:
 * HASH_TABLE_VALUE_DEFAULT_VALUE: the default value returned when a node is not found in get/find functions
 *
 * HASH_TABLE_HASH_FUNCTION(key): the hash function of keys
 *
 * The following are optional when defining the hash table:
 * HASH_TABLE_ALLOC(type, count): the allocator. Default value is (type*) malloc(sizeof(type) * count).
 *
 * HASH_TABLE_DEALLOC(type, ptr, count): the deallocator. Default value is free(ptr).
 * It must be provided if HASH_TABLE_ALLOC is provided.
 *
 * HASH_TABLE_KEY_DUMP(buffer, buf_size, key): the dump function of keys
 *
 * HASH_TABLE_VALUE_DUMP(buffer, buf_size, value): the value function of values
 * 	if both of the dump functions of keys and values are defined, HASH_TABLE_DECLARE_DUMP is set to 1 automatically.
 *
 * HASH_TABLE_KEY_CLONE(var, key): the clone function of keys.
 * If it is not set, assignment(=) is used.
 *
 * HASH_TABLE_VALUE_CLONE(var, value): the clone function of values.
 * If it is not set, assigment(=) is used.
 *
 * HASH_TABLE_KEY_COMPARATOR(key1, key2): the comaprator of keys, returns non-zero when the keys are equal.
 * If not set, == is used.
 *
 * HASH_TABLE_KEY_DESTRUCTOR: the destructor of keys
 *
 * HASH_TABLE_VALUE_DESTRUCTOR: the destructor of values
 *
 */

#if defined(HASH_TABLE_PREFIX) && defined(HASH_TABLE_KEY_TYPE)	\
	&& defined(HASH_TABLE_VALUE_TYPE)


#ifdef __HTAB
#undef __HTAB
#endif
#define __HTAB HASH_TABLE_PREFIX

#ifdef __HTAB_T
#undef __HTAB_T
#endif
#define __HTAB_T CONCAT(__HTAB, _t)

#ifdef __HTAB_K_T
#undef __HTAB_K_T
#endif
#define __HTAB_K_T HASH_TABLE_KEY_TYPE

#ifdef __HTAB_V_T
#undef __HTAB_V_T
#endif
#define __HTAB_V_T HASH_TABLE_VALUE_TYPE


#ifdef __HTAB_NODE_T
#undef __HTAB_NODE_T
#endif
#define __HTAB_NODE_T CONCAT(__HTAB, _node_t)

#ifdef HTAB_METHOD
#undef HTAB_METHOD
#endif
#define HTAB_METHOD(method_name) CONCAT3(__HTAB, _, method_name)

#ifdef HTAB_NODE_METHOD
#undef HTAB_NODE_METHOD
#endif
#define HTAB_NODE_METHOD(method_name) CONCAT3(__HTAB, _node_, method_name)

#if defined(HASH_TABLE_KEY_DUMP) && defined(HASH_TABLE_VALUE_DUMP)
#if !defined(HASH_TABLE_DECLARE_DUMP)
#	define HASH_TABLE_DECLARE_DUMP 1
#else
#if HASH_TABLE_DECLARE_DUMP == 0
#	undef HASH_TABLE_DECLARE_DUMP
#	define HASH_TABLE_DECLARE_DUMP 1
#endif	/* HASH_TABLE_DECLARE_DUMP == 0*/
#endif	/* !defined(HASH_TABLE_DECLARE_DUMP) */
#endif

#if !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0

#ifndef HASH_TABLE_DEFINE_STRUCT
#	define HASH_TABLE_DEFINE_STRUCT 1
#endif

#ifndef HASH_TABLE_ALLOC
#	define HASH_TABLE_ALLOC(type, count) (type*) malloc(sizeof(type) * count)

#ifdef HASH_TABLE_DEALLOC
#	undef HASH_TABLE_DEALLOC
#endif

#	define HASH_TABLE_DEALLOC(type, ptr, count) free(ptr)

#endif	/* HASH_TABLE_ALLOC */

#ifndef HASH_TABLE_VALUE_DEFAULT_VALUE
#error "HASH_TABLE_VALUE_DEFAULT_VALUE is not defined"
#endif

#if defined(HASH_TABLE_DECLARE_DUMP) && HASH_TABLE_DECLARE_DUMP != 0 &&	\
	(!defined(HASH_TABLE_KEY_DUMP) || !defined(HASH_TABLE_VALUE_DUMP))
#warning "hashtable: dump function is declared but not defined"
#endif

#ifndef HASH_TABLE_HASH_FUNCTION
#error "HASH_TABLE_HASH_FUNCTION is not defined"
#endif

#ifdef __HTAB_H
#undef __HTAB_H
#endif
#define __HTAB_H HASH_TABLE_HASH_FUNCTION

#ifdef HASH_TABLE_KEY_COMPARATOR
#	define __HTAB_COMPARE(key1, key2) HASH_TABLE_KEY_COMPARATOR(key1, key2)
#else
#	define __HTAB_COMPARE(key1, key2) (key1 == key2)
#endif

#else

#ifndef HASH_TABLE_DEFINE_STRUCT
#	define HASH_TABLE_DEFINE_STRUCT 0
#endif

#endif /* !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0 */

/* DECLARATION OF HASH_TABLE node type */
typedef struct __HTAB_NODE_T __HTAB_NODE_T;
#if HASH_TABLE_DEFINE_STRUCT != 0
struct __HTAB_NODE_T {
	__HTAB_K_T key;
	__HTAB_V_T value;
	__HTAB_NODE_T* next;
};
#endif

__HTAB_NODE_T* CONCAT3(new_, __HTAB, _node)(__HTAB_K_T key, __HTAB_V_T value, __HTAB_NODE_T* next);

__HTAB_NODE_T* HTAB_NODE_METHOD(destroy)(__HTAB_NODE_T* node);

#if defined(HASH_TABLE_DECLARE_DUMP) && HASH_TABLE_DECLARE_DUMP != 0
int HTAB_NODE_METHOD(dump)(char* buffer, int buf_size, __HTAB_NODE_T* node);
#endif

/* DEFINICTION OF HASH_TABLE node type functions*/
#if !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0

__HTAB_NODE_T* CONCAT3(new_, __HTAB, _node)(__HTAB_K_T key, __HTAB_V_T value, __HTAB_NODE_T* next) {
	__HTAB_NODE_T* node = HASH_TABLE_ALLOC(__HTAB_NODE_T, 1);
	if (!node) return next;

#ifdef HASH_TABLE_KEY_CLONE
	HASH_TABLE_KEY_CLONE(node->key, key);
#else
	node->key = key;
#endif
#ifdef HASH_TABLE_VALUE_CLONE
	HASH_TABLE_VALUE_CLONE(node->value, value);
#else
	node->value = value;
#endif
	node->next = next;
	return node;
}

__HTAB_NODE_T* HTAB_NODE_METHOD(destroy)(__HTAB_NODE_T* node) {
	if (node) {
		__HTAB_NODE_T* to_free = node;
		node = node->next;

		#ifdef HASH_TABLE_KEY_DESTRUCTOR
			HASH_TABLE_KEY_DESTRUCTOR(to_free->key);
		#endif
		#ifdef HASH_TABLE_VALUE_DESTRUCTOR
			HASH_TABLE_VALUE_DESTRUCTOR(to_free->value);
		#endif
		HASH_TABLE_DEALLOC(__HTAB_NODE_T, to_free, 1);
		return node;
	}
	return 0;
}

#if defined(HASH_TABLE_DECLARE_DUMP) && HASH_TABLE_DECLARE_DUMP != 0
int HTAB_NODE_METHOD(dump)(char* buffer, int buf_size, __HTAB_NODE_T* node) {
	DUMP_START();
	DUMP_CALL(HASH_TABLE_KEY_DUMP, buffer, buf_size, node->key);
	DUMP(buffer, buf_size, " = ");
	DUMP_CALL(HASH_TABLE_VALUE_DUMP, buffer, buf_size, node->value);
	DUMP_SUCCESS();
}
#endif

#endif /* !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0 */

/* DECLARATION OF HASH TABLE */
typedef struct __HTAB_T __HTAB_T;
#if HASH_TABLE_DEFINE_STRUCT != 0
struct __HTAB_T {
	float load_factor;
	size_t rehash_threshold;
	size_t capacity;
	size_t size;
	__HTAB_NODE_T** entries;
};
#endif

__HTAB_T* CONCAT(new_, __HTAB)(int capacity, float load_factor);

int HTAB_METHOD(rehash)(__HTAB_T* hash_table, size_t new_capacity);

int HTAB_METHOD(put)(__HTAB_T* hash_table, __HTAB_K_T key, __HTAB_V_T value);

__HTAB_V_T HTAB_METHOD(get)(__HTAB_T* hash_table, __HTAB_K_T key);

__HTAB_NODE_T* HTAB_METHOD(get_node)(__HTAB_T* hash_table, __HTAB_K_T key);

__HTAB_V_T HTAB_METHOD(find)(__HTAB_T* hash_table, __HTAB_K_T key);

__HTAB_NODE_T* HTAB_METHOD(find_node)(__HTAB_T* hash_table, __HTAB_K_T key);

int HTAB_METHOD(remove)(__HTAB_T* hash_table, __HTAB_K_T key);

size_t HTAB_METHOD(size)(__HTAB_T* hash_table);

size_t HTAB_METHOD(capacity)(__HTAB_T* hash_table);

float HTAB_METHOD(load_factor)(__HTAB_T* hash_table);

void HTAB_METHOD(clear)(__HTAB_T* hash_table);

int HTAB_METHOD(empty)(__HTAB_T* hash_table);

__HTAB_T* HTAB_METHOD(clone)(__HTAB_T* hash_table);

__HTAB_NODE_T** HTAB_METHOD(data)(__HTAB_T* hash_table);

#if defined(HASH_TABLE_DECLARE_DUMP) && HASH_TABLE_DECLARE_DUMP != 0
int HTAB_METHOD(dump)(char* buffer, int buf_size, __HTAB_T* hash_table);
#endif

void HTAB_METHOD(destroy)(__HTAB_T* hash_table);

/* DEFINITION OF HASH TABLE */
#if !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0


__HTAB_T* CONCAT(new_, __HTAB)(int capacity, float load_factor) {
	__HTAB_T* hash_table = HASH_TABLE_ALLOC(__HTAB_T, 1);
	if (!hash_table) return 0;

	hash_table->load_factor = load_factor;
	hash_table->rehash_threshold = (size_t) (load_factor * capacity);
	hash_table->size = 0;
	hash_table->capacity = capacity;
	hash_table->entries = HASH_TABLE_ALLOC(__HTAB_NODE_T*, capacity);
	if (!hash_table->entries) {
		HASH_TABLE_DEALLOC(__HTAB_T, hash_table, 1);
		return 0;
	}
	memset(hash_table->entries, 0, sizeof(__HTAB_NODE_T*) * capacity);
	return hash_table;
}

int HTAB_METHOD(rehash)(__HTAB_T* hash_table, size_t new_capacity) {
	__HTAB_NODE_T** new_entries = HASH_TABLE_ALLOC(__HTAB_NODE_T*, new_capacity);
	if (!new_entries) {
		return 0;
	}
	memset(new_entries, 0, sizeof(__HTAB_NODE_T*) * new_capacity);

	size_t i;
	for (i = 0; i != hash_table->capacity; ++i) {
		__HTAB_NODE_T* next = hash_table->entries[i], *now;
		while (next) {
			now = next;
			next = next->next;

			unsigned hash = __HTAB_H(now->key) % new_capacity;
			now->next = new_entries[hash];
			new_entries[hash] = now;
		}
	}

	HASH_TABLE_DEALLOC(__HTAB_NODE_T*, hash_table->entries, hash_table->capacity);
	hash_table->capacity = new_capacity;
	hash_table->entries = new_entries;
	hash_table->rehash_threshold = (size_t) (hash_table->load_factor * new_capacity);
	return 1;
}

int HTAB_METHOD(put)(__HTAB_T* hash_table, __HTAB_K_T key, __HTAB_V_T value) {
	unsigned hash_raw = __HTAB_H(key);
	unsigned hash = hash_raw % hash_table->capacity;
	__HTAB_NODE_T* node = hash_table->entries[hash];

	while (node) {
		if (__HTAB_COMPARE(key, node->key)) {
		#ifdef HASH_TABLE_VALUE_DESTRUCTOR
			HASH_TABLE_VALUE_DESTRUCTOR(node->value);
		#endif

		#ifdef HASH_TABLE_VALUE_CLONE
			HASH_TABLE_VALUE_CLONE(node->value, value);
		#else
			node->value = value;
		#endif
			return 1;
		}
		node = node->next;
	}

	if (hash_table->size + 1> hash_table->rehash_threshold) {
		if (HTAB_METHOD(rehash)(hash_table, hash_table->capacity * 2)) {
			hash = hash_raw % hash_table->capacity;
		}
		else {
			/* cannot rehash */
		}
	}
	hash_table->entries[hash] = CONCAT3(new_, __HTAB, _node)(key, value, hash_table->entries[hash]);
	++hash_table->size;
	return 1;
}

__HTAB_V_T HTAB_METHOD(get)(__HTAB_T* hash_table, __HTAB_K_T key) {
	return HTAB_METHOD(get_node)(hash_table, key)->value;
}

__HTAB_NODE_T* HTAB_METHOD(get_node)(__HTAB_T* hash_table, __HTAB_K_T key) {
	unsigned hash_raw = __HTAB_H(key);
	unsigned hash = hash_raw % hash_table->capacity;
	__HTAB_NODE_T* node = hash_table->entries[hash];

	while (node) {
		if (__HTAB_COMPARE(key, node->key)) {
			return node;
		}
		node = node->next;
	}

	if (hash_table->size + 1> hash_table->rehash_threshold) {
		if (HTAB_METHOD(rehash)(hash_table, hash_table->capacity * 2)) {
			hash = hash_raw % hash_table->capacity;
		}
		else {
			/* cannot rehash */
		}
	}
	hash_table->entries[hash] = CONCAT3(new_, __HTAB, _node)(key, HASH_TABLE_VALUE_DEFAULT_VALUE, hash_table->entries[hash]);
	++hash_table->size;
	return hash_table->entries[hash];
}

__HTAB_V_T HTAB_METHOD(find)(__HTAB_T* hash_table, __HTAB_K_T key) {
	__HTAB_NODE_T* node = HTAB_METHOD(find_node)(hash_table, key);
	if (!node) return HASH_TABLE_VALUE_DEFAULT_VALUE;
	return node->value;
}

__HTAB_NODE_T* HTAB_METHOD(find_node)(__HTAB_T* hash_table, __HTAB_K_T key) {
	unsigned hash = __HTAB_H(key) % hash_table->capacity;
	__HTAB_NODE_T* node = hash_table->entries[hash];

	while (node) {
		if (__HTAB_COMPARE(key, node->key)) {
			return node;
		}
		node = node->next;
	}

	return 0;
}

int HTAB_METHOD(remove)(__HTAB_T* hash_table, __HTAB_K_T key) {
	unsigned hash = __HTAB_H(key) % hash_table->capacity;
	__HTAB_NODE_T** node_ptr = &(hash_table->entries[hash]);

	while (*node_ptr) {
		if (__HTAB_COMPARE(key, (*node_ptr)->key)) {
			*node_ptr = HTAB_NODE_METHOD(destroy)(*node_ptr);
			--hash_table->size;
			return 1;
		}
		node_ptr = &((*node_ptr)->next);
	}

	return 0;
}

size_t HTAB_METHOD(size)(__HTAB_T* hash_table) {
	return hash_table->size;
}

size_t HTAB_METHOD(capacity)(__HTAB_T* hash_table) {
	return hash_table->capacity;
}

float HTAB_METHOD(load_factor)(__HTAB_T* hash_table) {
	return hash_table->load_factor;
}

void HTAB_METHOD(clear)(__HTAB_T* hash_table) {
	size_t i;
	for (i = 0; i != hash_table->capacity; ++i) {
		__HTAB_NODE_T* node = hash_table->entries[i];
		hash_table->entries[i] = 0;
		while (node) {
			node = HTAB_NODE_METHOD(destroy)(node);
		}
	}
	hash_table->size = 0;
}

int HTAB_METHOD(empty)(__HTAB_T* hash_table) {
	return !hash_table->size;
}

__HTAB_T* HTAB_METHOD(clone)(__HTAB_T* hash_table) {
	__HTAB_T* new_hash_table = CONCAT(new_, __HTAB)(hash_table->capacity, hash_table->load_factor);
	if (!new_hash_table) return 0;

	new_hash_table->size = hash_table->size;

	size_t i;
	for (i = 0; i != hash_table->capacity; ++i) {
		__HTAB_NODE_T* node = hash_table->entries[i];
		while (node) {
			new_hash_table->entries[i] = CONCAT3(new_, __HTAB, _node)
					(node->key, node->value, new_hash_table->entries[i]);
			node = node->next;
		}
	}
	return new_hash_table;
}

__HTAB_NODE_T** HTAB_METHOD(data)(__HTAB_T* hash_table) {
	return hash_table->entries;
}

#if defined(HASH_TABLE_DECLARE_DUMP) && HASH_TABLE_DECLARE_DUMP != 0
int HTAB_METHOD(dump)(char* buffer, int buf_size, __HTAB_T* hash_table) {
	size_t i;
	DUMP_START();
	DUMP(buffer, buf_size, "%s_t, size = %u, capacity = %u, load_factor = %f {\n", STRINGIFY(__HTAB), hash_table->size, hash_table->capacity, hash_table->load_factor);
	for (i =  0; i != hash_table->capacity; ++i) {
		__HTAB_NODE_T* node = hash_table->entries[i];
		while (node) {
			DUMP(buffer, buf_size, "[%u] ", i);
			DUMP_CALL(HTAB_NODE_METHOD(dump), buffer, buf_size, node);
			DUMP(buffer, buf_size, "\n");
			node = node->next;
		}
	}
	DUMP(buffer, buf_size, "}\n");
	DUMP_SUCCESS();
}
#endif

void HTAB_METHOD(destroy)(__HTAB_T* hash_table) {
	HTAB_METHOD(clear)(hash_table);
	HASH_TABLE_DEALLOC(__HTAB_NODE_T*, hash_table->entries, hash_table->capacity);
	HASH_TABLE_DEALLOC(__HTAB_T, hash_table, 1);
}


#undef HASH_TABLE_VALUE_DEFAULT_VALUE
#undef HASH_TABLE_HASH_FUNCTION
#undef __HTAB_H

#undef HASH_TABLE_ALLOC
#undef HASH_TABLE_DEALLOC

#ifdef HASH_TABLE_VALUE_DUMP
#undef HASH_TABLE_VALUE_DUMP
#endif

#ifdef HASH_TABLE_KEY_DUMP
#undef HASH_TABLE_KEY_DUMP
#endif

#ifdef HASH_TABLE_KEY_CLONE
#undef HASH_TABLE_KEY_CLONE
#endif

#ifdef HASH_TABLE_VALUE_CLONE
#undef HASH_TABLE_VALUE_CLONE
#endif

#ifdef HASH_TABLE_KEY_DUMP
#undef HASH_TABLE_KEY_DUMP
#endif

#ifdef HASH_TABLE_VALUE_DUMP
#undef HASH_TABLE_VALUE_DUMP
#endif

#ifdef HASH_TABLE_KEY_COMPARATOR
#undef HASH_TABLE_KEY_COMPARATOR
#endif
#undef __HTAB_COMPARE

#ifdef HASH_TABLE_KEY_DESTRUCTOR
#undef HASH_TABLE_KEY_DESTRUCTOR
#endif


#ifdef HASH_TABLE_VALUE_DESTRUCTOR
#undef HASH_TABLE_VALUE_DESTRUCTOR
#endif

#endif /* !defined(HASH_TABLE_DECLARE_ONLY) || HASH_TABLE_DECLARE_ONLY == 0 */

#ifdef HASH_TABLE_DECLARE_DUMP
#undef HASH_TABLE_DECLARE_DUMP
#endif

#ifdef HASH_TABLE_DECLARE_ONLY
#undef HASH_TABLE_DECLARE_ONLY
#endif

#undef __HTAB
#undef __HTAB_T
#undef __HTAB_K_T
#undef __HTAB_V_T
#undef __HTAB_NODE_T

#undef HTAB_METHOD
#undef HTAB_NODE_METHOD

#undef HASH_TABLE_PREFIX
#undef HASH_TABLE_KEY_TYPE
#undef HASH_TABLE_VALUE_TYPE

#undef HASH_TABLE_DEFINE_STRUCT

#else

#error "HASH_TABLE_PREFIX, HASH_TABLE_KEY_TYPE,\
HASH_TABLE_VALUE_TYPE, must be defined before include hash_table.h"

#endif
/*defined(HASH_TABLE_PREFIX) && defined(HASH_TABLE_KEY_TYPE)
&& defined(HASH_TABLE_VALUE_TYPE) && defined(HASH_TABLE_HASH_FUNCTION) */
