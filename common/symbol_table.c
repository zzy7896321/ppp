
#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static PPP_INLINE unsigned bkdr_hash(const char* str) {
	unsigned hash = 0;
	for ( ; *str != '\0'; ++str) {
		hash = (hash * 131 + *str);
	}
	return hash;
}

typedef struct symbol_table_entry_t symbol_table_entry_t;

struct symbol_table_entry_t
{
    symbol_t symbol;
    char* name;
};

static PPP_INLINE symbol_table_entry_t* new_symbol_table_entry(symbol_t symbol, const char* name) {
	symbol_table_entry_t* sym_entry = malloc(sizeof(symbol_table_entry_t));

	sym_entry->symbol = symbol;
	sym_entry->name = strdup(name);

	return sym_entry;
}

static PPP_INLINE void symbol_table_entry_destroy(symbol_table_entry_t* sym_entry) {
	if (sym_entry == 0) return;

	free(sym_entry->name);
	free(sym_entry);
}

static PPP_INLINE int symbol_table_entry_dump(char* buffer, int buf_size, symbol_table_entry_t* sym_entry) {
	return snprintf(buffer, buf_size, "%d: %s", sym_entry->symbol, sym_entry->name);
}

#define HASH_TABLE_PREFIX symbol_table_hash_map
#define HASH_TABLE_KEY_TYPE const char*
#define HASH_TABLE_VALUE_TYPE symbol_table_entry_t*
#define HASH_TABLE_DEFINE_STRUCT 1
#define HASH_TABLE_VALUE_DEFAULT_VALUE 0
#define HASH_TABLE_HASH_FUNCTION(key) (bkdr_hash(key))
#ifdef ENABLE_MEM_PROFILE
#define HASH_TABLE_ALLOC(type, count) PROFILE_MEM_ALLOC(type, count)
#define HASH_TABLE_DEALLOC(type, ptr, count) PROFILE_MEM_FREE(type, ptr, count)
#endif
#define HASH_TABLE_KEY_DUMP(buffer, buf_size, key) (snprintf((buffer), (buf_size), "%s", key))
#define HASH_TABLE_VALUE_DUMP(buffer, buf_size, value) (snprintf((buffer), (buf_size), "%d", (value)->symbol))
#define HASH_TABLE_KEY_COMPARATOR(key1, key2) (!strcmp((key1), (key2)))
#define HASH_TABLE_KEY_DESCTRUCTOR(key) ((void) 0)
#define HASH_TABLE_VALUE_DESTRUCTOR(value) ((void) 0)
#include "hash_table.h"

#define VECTOR_PREFIX symbol_table_vector
#define VECTOR_VALUE_TYPE symbol_table_entry_t*
#define VECTOR_DEFINE_STRUCT 1
#define VECTOR_VALUE_DEFAULT_VALUE 0
#ifdef ENABLE_MEM_PROFILE
#define VECTOR_ALLOC(type, count) PROFILE_MEM_ALLOC(type, count)
#define VECTOR_DEALLOC(type, ptr, count) PROFILE_MEM_FREE(type, ptr, count)
#endif
#define VECTOR_VALUE_DESTRUCTOR(var) symbol_table_entry_destroy(var)
#define VECTOR_VALUE_DUMP(buffer, buf_size, value) symbol_table_entry_dump(buffer, buf_size, value)
#include "vector.h"

struct symbol_table_t {
	symbol_t last_symbol;
	symbol_table_vector_t* symbols;
	symbol_table_hash_map_t* hmap;
};

symbol_table_t* new_symbol_table() {
	symbol_table_t* symbol_table = malloc(sizeof(symbol_table_t));

	symbol_table->last_symbol = 0;
	symbol_table->symbols = new_symbol_table_vector(16);
	symbol_table->hmap = new_symbol_table_hash_map(16, 0.8);

	return symbol_table;
}

symbol_t symbol_table_insert(symbol_table_t* symbol_table, const char* name) {
	symbol_table_entry_t* sym_entry;
	sym_entry = symbol_table_hash_map_find(symbol_table->hmap, name);
	if (!sym_entry) {
		sym_entry = new_symbol_table_entry(symbol_table->last_symbol++, name);

		symbol_table_vector_push_back(symbol_table->symbols, sym_entry);
		symbol_table_hash_map_put(symbol_table->hmap, sym_entry->name, sym_entry);
	}
	return sym_entry->symbol;
}

symbol_t symbol_table_lookup(symbol_table_t* symbol_table, const char* name) {
	symbol_table_entry_t* sym_entry;
	sym_entry = symbol_table_hash_map_find(symbol_table->hmap, name);
	if (!sym_entry) return SYMBOL_NULL;
	return sym_entry->symbol;
}

const char* symbol_table_get_name(symbol_table_t* symbol_table, symbol_t symbol) {
	symbol_table_entry_t* sym_entry = symbol_table_vector_at(symbol_table->symbols, symbol);
	if (sym_entry) {
		return sym_entry->name;
	}
	return 0;
}

size_t symbol_table_size(symbol_table_t* symbol_table) {
	return symbol_table->last_symbol;
}

void symbol_table_destroy(symbol_table_t* symbol_table) {
	symbol_table_hash_map_destroy(symbol_table->hmap);
	symbol_table_vector_destroy(symbol_table->symbols);
	free(symbol_table);
}

int symbol_table_dump(char* buffer, int buf_size, symbol_table_t* symbol_table) {
	return symbol_table_vector_dump(buffer, buf_size, symbol_table->symbols);
}
