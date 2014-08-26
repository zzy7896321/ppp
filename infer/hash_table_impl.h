#ifndef HASH_TABLE_IMPL
#define HASH_TABLE_IMPL

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DECLARE_HASH_TABLE(prefix, key_type, value_type)	\
	typedef struct prefix##_hash_table_t prefix##_hash_table_t;	\
	typedef struct prefix##_hash_table_node_t prefix##_hash_table_node_t;	\
	struct prefix##_hash_table_t {	\
		size_t size;	\
		size_t capacity;	\
		prefix##_hash_table_node_t* node[];	\
	};	\
		\
	struct prefix##_hash_table_node_t {	\
		key_type key;	\
		value_type value;	\
		prefix##_hash_table_node_t* next;	\
	};	\
		\
	prefix##_hash_table_t* new_##prefix##_hash_table(size_t capacity);	\
	void prefix##_hash_table_destroy(prefix##_hash_table_t* hash_table);	\
	int prefix##_hash_table_put(prefix##_hash_table_t* hash_table, key_type key, value_type value);	\
	value_type prefix##_hash_table_get(prefix##_hash_table_t* hash_table, key_type key);	\
	prefix##_hash_table_node_t* prefix##_hash_table_get_node(prefix##_hash_table_t* hash_table, key_type key);	\
	value_type prefix##_hash_table_find(prefix##_hash_table_t* hash_table, key_type key);	\
	prefix##_hash_table_node_t* prefix##_hash_table_find_node(prefix##_hash_table_t* hash_table, key_type key);	\
	int prefix##_hash_table_remove(prefix##_hash_table_t* hash_table, key_type key);	\
	size_t prefix##_hash_table_size(prefix##_hash_table_t* hash_table);	\
	size_t prefix##_hash_table_capacity(prefix##_hash_table_t* hash_table);	\
	void prefix##_hash_table_clear(prefix##_hash_table_t* hash_table);	\
	int prefix##_hash_table_dump(prefix##_hash_table_t* hash_table, char* buffer, int buf_size);

#define DEFINE_HASH_TABLE(prefix, key_type, value_type)	\
	prefix##_hash_table_t* new_##prefix##_hash_table(size_t capacity) { \
		prefix##_hash_table_t* hash_table = malloc(sizeof(prefix##_hash_table_t) + sizeof(prefix##_hash_table_node_t*) * capacity); \
			\
		hash_table->size = 0;	\
		hash_table->capacity = capacity;	\
		memset(hash_table->node, 0, sizeof(prefix##_hash_table_node_t*) * capacity);	\
		return hash_table;	\
	} \
		\
	void prefix##_hash_table_destroy(prefix##_hash_table_t* hash_table) {	\
		if (hash_table) {	\
			prefix##_hash_table_clear(hash_table);	\
			free(hash_table);	\
		}	\
	}	\
		\
	int prefix##_hash_table_put(prefix##_hash_table_t* hash_table, key_type key, value_type value) {	\
		unsigned hash = HASH_TABLE_HASH_FUNCTION(key) & hash_table->capacity; \
		prefix##_hash_table_node_t* node = hash_table->node[hash];	\
			\
		while (node) {	\
			if (HASH_TABLE_COMPARATOR(key, node->key)) {	\
				HASH_TABLE_DESTROY_VALUE(node->value);	\
				node->value = value;	\
				return 1;	\
			}	\
			node = node->next;	\
		}	\
			\
		node = malloc(sizeof(prefix##_hash_table_node_t));	\
		node->key = key;	\
		node->value = value;	\
		node->next = hash_table->node[hash];	\
		hash_table->node[hash] = node;	\
		++hash_table->size;	\
			\
		return 0;	\
	}	\
		\
	value_type prefix##_hash_table_get(prefix##_hash_table_t* hash_table, key_type key) {	\
		return prefix##_hash_table_get_node(hash_table, key)->value;	\
	}	\
		\
	prefix##_hash_table_node_t* prefix##_hash_table_get_node(prefix##_hash_table_t* hash_table, key_type key) {	\
		unsigned hash = HASH_TABLE_HASH_FUNCTION(key) & hash_table->capacity;	\
		prefix##_hash_table_node_t* node = hash_table->node[hash];	\
			\
		while (node) {	\
			if (HASH_TABLE_COMPARATOR(key, node->key)) {	\
				return node;	\
			}	\
			node = node->next;	\
		}	\
			\
		node = malloc(sizeof(prefix##_hash_table_node_t));	\
		node->key = key;	\
		node->value = HASH_TABLE_VALUE_DEFAULT;	\
		node->next= hash_table->node[hash];	\
		hash_table->node[hash] = node;	\
		++hash_table->size;	\
			\
		return node;	\
	}	\
		\
	value_type prefix##_hash_table_find(prefix##_hash_table_t* hash_table, key_type key) {	\
		prefix##_hash_table_node_t* node = prefix##_hash_table_find_node(hash_table, key);	\
		if (node) {	\
			return node->value;	\
		}	\
		return HASH_TABLE_KEY_NOT_FOUND_VALUE;	\
	}	\
		\
	prefix##_hash_table_node_t* prefix##_hash_table_find_node(prefix##_hash_table_t* hash_table, key_type key) {	\
		unsigned hash = HASH_TABLE_HASH_FUNCTION(key) & hash_table->capacity;	\
		prefix##_hash_table_node_t* node = hash_table->node[hash];	\
			\
		while (node) {	\
			if (HASH_TABLE_COMPARATOR(key, node->key))	{	\
				return node;	\
			}	\
			node = node->next;	\
		}	\
		return 0;	\
	}	\
		\
	int prefix##_hash_table_remove(prefix##_hash_table_t* hash_table, key_type key) {	\
		unsigned hash = HASH_TABLE_HASH_FUNCTION(key) & hash_table->capacity;	\
		prefix##_hash_table_node_t** node_ptr = &(hash_table->node[hash]);	\
			\
		while (*node_ptr) {	\
			if (HASH_TABLE_COMPARATOR(key, (*node_ptr)->key)) {	\
				prefix##_hash_table_node_t* to_free = *node_ptr;	\
				*node_ptr = (*node_ptr) -> next;	\
				HASH_TABLE_DESTROY_KEY(to_free->key);	\
				HASH_TABLE_DESTROY_VALUE(to_free->value);	\
				free(to_free);	\
				--hash_table->size;	\
				return 0;	\
			}	\
			node_ptr = &((*node_ptr) -> next);	\
		}	\
		return 1;	\
	}	\
		\
	size_t prefix##_hash_table_size(prefix##_hash_table_t* hash_table) {	\
		return hash_table->size;	\
	}	\
	size_t prefix## hash_table_capacity(prefix##_hash_table_t* hash_table) {	\
		return hash_table->capacity;	\
	}	\
		\
	void prefix##_hash_table_clear(prefix##_hash_table_t* hash_table) {	\
		prefix##_hash_table_node_t** begin = hash_table->node;	\
		prefix##_hash_table_node_t** end = begin + hash_table->capacity;	\
		for (; begin != end; ++begin) {	\
			prefix##_hash_table_node_t* node = *begin;	\
				\
			while (node) {	\
				prefix##_hash_table_node_t* to_free = node;	\
				node = node->next;	\
				HASH_TABLE_DESTROY_KEY(to_free->key);	\
				HASH_TABLE_DESTROY_VALUE(to_free->value);	\
				free(to_free);	\
			}	\
		}	\
			\
		hash_table->size = 0;	\
		\
	}	\
		\
	int prefix##_hash_table_dump(prefix##_hash_table_t* hash_table, char* buffer, int buf_size) {	\
		if (buf_size <= 0) return -1;	\
		buffer[0] = '\0';	\
		int num_written = 0;	\
		num_written += snprintf(buffer + num_written, buf_size - num_written, #prefix "_hash_table_t, size = %d, capacity = %d\n", hash_table->size, hash_table->capacity);	\
		for (size_t i = 0; i != hash_table->capacity; ++i) {	\
			prefix##_hash_table_node_t* node = hash_table->node[i];	\
			while (node) {	\
				num_written += HASH_TABLE_KEY_DUMP_FUNCTION(buffer + num_written, buf_size - num_written, node->key);	\
				num_written += snprintf(buffer + num_written, buf_size - num_written, " = ");	\
				num_written += HASH_TABLE_VALUE_DUMP_FUNCTION(buffer + num_written, buf_size - num_written, node->value);	\
				num_written += snprintf(buffer + num_written, buf_size - num_written, "\n");	\
				node = node->next;	\
			}	\
		}	\
		return num_written;	\
	}

#endif
