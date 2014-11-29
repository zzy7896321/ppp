#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../config.h"
#include <stddef.h>

typedef int symbol_t;

typedef struct symbol_table_t symbol_table_t;

#define SYMBOL_NULL (-1)

symbol_table_t* new_symbol_table();

/**
 * Inserts the symbol into the symbol table if it is not present in the table.
 * Returns its symbol number.
 */
symbol_t symbol_table_insert(symbol_table_t* symbol_table, const char* name);
symbol_t symbol_table_lookup(symbol_table_t* symbol_table, const char* name);
const char* symbol_table_get_name(symbol_table_t* symbol_table, symbol_t symbol);

/* Kept just for compatibility. */
//const char* symbol_to_string(symbol_table_t* symbol_table, symbol_t symbol);
#define symbol_to_string symbol_table_get_name

/* don't want to expose the internal implementation */
/* symbol_table_entry_t* symbol_table_find_entry_by_string(symbol_table_t* symbol_table, const char* string); */
/* symbol_table_entry_t* symbol_table_find_entry_by_symbol(symbol_table_t* symbol_table, symbol_t symbol); */

size_t symbol_table_size(symbol_table_t* symbol_table);

void symbol_table_destroy(symbol_table_t* symbol_table);

int symbol_table_dump(char* buffer, int buf_size, symbol_table_t* symbol_table);

#endif
