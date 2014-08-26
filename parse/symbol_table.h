#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

/* FIXME should allow arbitrary length */
#define MAX_NAME_SIZE 250

typedef unsigned int symbol_t;

typedef struct symbol_table_entry_t
{
    char string[MAX_NAME_SIZE];
    symbol_t symbol;
    struct symbol_table_entry_t* next;
} symbol_table_entry_t;

typedef struct symbol_table_t
{
    symbol_t last_symbol;
    struct symbol_table_entry_t* entry;
} symbol_table_t;

symbol_table_t* new_symbol_table();
symbol_table_entry_t* symbol_table_find_entry_by_string(symbol_table_t* symbol_table, const char* string);
symbol_t symbol_table_lookup_symbol(symbol_table_t* symbol_table, const char* string);
symbol_table_entry_t* symbol_table_find_entry_by_symbol(symbol_table_t* symbol_table, symbol_t symbol);

const char* symbol_to_string(symbol_table_t* symbol_table, symbol_t symbol);

#endif
