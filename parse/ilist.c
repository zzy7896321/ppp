#include "ilist.h"
#include <stdlib.h>
#include <assert.h>

/******************************************************************************
Integer list
******************************************************************************/

struct ilist_t* new_ilist()
{
    struct ilist_t* list;
    list = malloc(sizeof(struct ilist_t));
    list->entry = 0;
    return list;
}

void ilist_entry_append(struct ilist_entry_t* list_entry, int data)
{
    assert(list_entry);
    if (list_entry->next)
        ilist_entry_append(list_entry->next, data);
    else {
        list_entry->next = malloc(sizeof(struct ilist_entry_t));
        list_entry->next->data = data;
        list_entry->next->next = 0;
    }
}

void ilist_append(struct ilist_t* list, int data)
{
    assert(list);
    if (list->entry)
        ilist_entry_append(list->entry, data);
    else {
        list->entry = malloc(sizeof(struct ilist_entry_t));
        list->entry->data = data;
        list->entry->next = 0;
    }
}
