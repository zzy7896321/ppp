#include "list.h"
#include <stdlib.h>
#include <assert.h>

/******************************************************************************
Pointer list
******************************************************************************/
struct list_t* new_list()
{
   struct list_t* list;
   list = malloc(sizeof(struct list_t));
   list->entry = 0;
   return list;
}

void list_entry_append(struct list_entry_t* list_entry, void* data)
{
   assert(list_entry);
   if (list_entry->next)
       list_entry_append(list_entry->next, data);
   else {
       list_entry->next = malloc(sizeof(struct list_entry_t));
       list_entry->next->data = data;
       list_entry->next->next = 0;
   }
}

void list_append(struct list_t* list, void* data)
{
   assert(list);
   if (list->entry)
       list_entry_append(list->entry, data);
   else {
       list->entry = malloc(sizeof(struct list_entry_t));
       list->entry->data = data;
       list->entry->next = 0;
   }
}
