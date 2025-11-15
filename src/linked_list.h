#pragma once

#include <stddef.h>

// A doubly-linked list node
typedef struct
{
    // The contained object
    void *obj;
    // Next node or NULL
    void *next;
    // Next node or NULL
    void *prev;
} ListNode;

// Linked list container
typedef struct
{
    // First node or NULL if list is empty
    ListNode *first;
    // Last node or NULL if list is empty
    ListNode *last;
    // Number of elements in the list (correct only if provided modification
    // functions or macros are used).
    size_t count;
} LinkedList;

// Linked list iteration state for iterating over the list.
// Set delete_current = 1 to delete the current node. Other removals
// during iteration are not supported.
typedef struct
{
    LinkedList *lst;
    ListNode *node;
    int delete_current;
} LinkedList_it_state;

// Create an empty linked list
LinkedList linked_list_create();

// Type safe shorthand for calling linked_list_add.
// Returns the obj field of the created list node, cast to the correct type.
// Usage: `int *new_element = LINKED_LIST_ADD(&my_list, int);`
#define LINKED_LIST_ADD(lst, type) \
    ((type *)(linked_list_add(lst, sizeof(type))->obj))

// Add a node to the end of the linked list. Allocates element_size bytes
// to the newly created node's obj field.
ListNode *linked_list_add(LinkedList *lst, size_t element_size);

// Remove the node from the list. Automatically deallocates the node's obj field.
// Does not check if the node actually exists in the list.
void linked_list_remove(LinkedList *lst, ListNode *n);

// Remove the node from the list if the node with the requested obj field exists in the list.
// Automatically deallocates the node's obj field (so obj will be a dangling pointer after
// the removal).
void linked_list_remove_by_value(LinkedList *lst, void *obj);

// Removes all the elements in the list. Uses linked_list_remove internally.
void linked_list_clear(LinkedList *lst);

// Start new iteration. Actual iteration is done by linked_list_iterate.
LinkedList_it_state linked_list_iteration_start(LinkedList *lst);
// Does the following steps:
// - Move to the next list item
// - If delete_current is set, delete the previous item
// - If no more items, clear iteration state (next calls will always return NULL)
// - Returns current node's obj element or NULL if at end
// Items can be added during iteration but not removed directly (only using delete_current).
// Nesting iterations over the same list that modify the list results in undefined behavior.
void *linked_list_iterate(LinkedList_it_state *state);

// Shorthand for a for loop that iterates over the list.
// Usage example: `int *element; LINKED_LIST_FOR_EACH(&my_list, int, element, *element == 0);`
// This will iterate through my_list (containing int type values) and delete all nodes
// with value == 0.
#define LINKED_LIST_FOR_EACH(list, type, i, delete_state)                        \
    for (LinkedList_it_state _it_state__##i = linked_list_iteration_start(list); \
         (i = (type *)linked_list_iterate(&_it_state__##i));                     \
         _it_state__##i.delete_current = delete_state)

/* Add a "managed" list. The list lst is added as an element to the list pointed by the root_list
 and its memory is freed when add_managed_list is called with lst parameter set to NULL.
 The same list can be added multple times, but note that the list will be cleared when it's added
 again. The function manages also the root_list so the correct way to use the API would be:
 ```
LinkedList *root = NULL;
LinkedList a, b;
add_managed_list(&a, &root);
linked_list_add(&a, 100);
add_managed_list(&b, &root);
linked_list_add(&b, 100);
add_managed_list(NULL, &root);
// All memory cleared:
// a and b will contain empty lists and root == NULL.
```*/
void add_managed_list(LinkedList *lst, LinkedList **root_list);
