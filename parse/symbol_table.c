#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct symbol_table_t* new_symbol_table()
{
    struct symbol_table_t* symbol_table;

    symbol_table = malloc(sizeof(struct symbol_table_t));
    symbol_table->last_symbol = 0;
    symbol_table->entry = 0;

    return symbol_table;
}

struct symbol_table_entry_t* symbol_table_find_entry_by_string(struct symbol_table_t* symbol_table, const char* string)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table->entry;
    while (table_entry) {
        if (strcmp(table_entry->string, string) == 0) {
            return table_entry;
        }
        table_entry = table_entry->next;
    }
    return 0;
}

symbol_t symbol_table_lookup_symbol(struct symbol_table_t* symbol_table, const char* string)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table_find_entry_by_string(symbol_table, string);
    if (table_entry)
        return table_entry->symbol;

    /* entry not found, create a new entry */
    table_entry = malloc(sizeof(struct symbol_table_entry_t));
    strcpy(table_entry->string, string);
    table_entry->symbol = symbol_table->last_symbol + 1;
    table_entry->next = symbol_table->entry;

    symbol_table->last_symbol = table_entry->symbol;
    symbol_table->entry = table_entry;

    /*fprintf(stderr, "symbol_table_lookup_symbol (%d): create a new entry (%s, %u)\n", __LINE__, string, table_entry->symbol); */
    return table_entry->symbol;
}

struct symbol_table_entry_t* symbol_table_find_entry_by_symbol(struct symbol_table_t* symbol_table, symbol_t symbol)
{
    struct symbol_table_entry_t* table_entry;

    table_entry = symbol_table->entry;
    while (table_entry) {
        if (table_entry->symbol == symbol) {
            return table_entry;
        }
        table_entry = table_entry->next;
    }
    return 0;
}
