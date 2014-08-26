#ifndef ILIST_H
#define ILIST_H

typedef struct ilist_entry_t
{
    int data;
    struct ilist_entry_t* next;
} ilist_entry_t;

typedef struct ilist_t
{
    struct ilist_entry_t* entry;
} ilist_t;

ilist_t* new_ilist();
void ilist_append(ilist_t* list, int data);
void ilist_entry_append(ilist_entry_t* list_entry, int data);

#endif
