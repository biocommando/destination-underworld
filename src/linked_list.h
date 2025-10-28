#pragma once

#include <stddef.h>

typedef struct
{
    void *obj;
    void *next;
    void *prev;
} ListNode;

typedef struct
{
    ListNode *first;
    ListNode *last;
    size_t count;
} LinkedList;

typedef struct
{
    LinkedList *lst;
    ListNode *node;
    int delete_current;
} LinkedList_it_state;

LinkedList linked_list_create();

#define LINKED_LIST_ADD(lst, type) \
    ((type *)(linked_list_add(lst, sizeof(type))->obj))

ListNode *linked_list_add(LinkedList *lst, size_t element_size);

void linked_list_remove(LinkedList *lst, ListNode *n);

void linked_list_remove_by_value(LinkedList *lst, void *obj);

void linked_list_clear(LinkedList *lst);

LinkedList_it_state linked_list_iteration_start(LinkedList *lst);
void *linked_list_iterate(LinkedList_it_state *state);

#define LINKED_LIST_FOR_EACH(list, type, i, delete_state)                        \
    for (LinkedList_it_state _it_state__##i = linked_list_iteration_start(list); \
         (i = (type *)linked_list_iterate(&_it_state__##i));                     \
         _it_state__##i.delete_current = delete_state)

void add_managed_list(LinkedList *lst, LinkedList **root_list);