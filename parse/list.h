#ifndef LIST_H
#define LIST_H

typedef struct list_entry_t
{
    void* data;
    struct list_entry_t* next;
} list_entry_t;

typedef struct list_t
{
    struct list_entry_t* entry;
} list_t;

list_t* new_list();
void list_append(list_t* list, void* data);
void list_entry_append(list_entry_t* entry, void* data);

#endif

