#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linked_list.h"

LinkedList linked_list_create()
{
    LinkedList lst;
    memset(&lst, 0, sizeof(LinkedList));
    return lst;
}

ListNode *linked_list_add(LinkedList *lst, size_t element_size)
{
    ListNode *n = malloc(sizeof(ListNode));
    memset(n, 0, sizeof(ListNode));
    n->obj = malloc(element_size);
    memset(n->obj, 0, element_size);
    if (!lst->first)
    {
        lst->first = lst->last = n;
    }
    else
    {
        lst->last->next = n;
        n->prev = lst->last;
        lst->last = n;
    }
    lst->count++;
    //printf("elements in list %lu\n", lst->count);
    return n;
}

void linked_list_remove(LinkedList *lst, ListNode *n)
{
    if (n->next)
    {
        ((ListNode *)(n->next))->prev = n->prev;
    }
    if (n->prev)
    {
        ((ListNode *)(n->prev))->next = n->next;
    }
    if (n == lst->first)
    {
        lst->first = (ListNode *)n->next;
    }
    if (n == lst->last)
    {
        lst->last = (ListNode *)n->prev;
    }
    lst->count--;
    free(n->obj);
    free(n);
}

void linked_list_remove_by_value(LinkedList *lst, void *obj)
{
    for (ListNode *node = lst->first; node; node = (ListNode*)node->next)
    {
        if (node->obj == obj)
        {
            linked_list_remove(lst, node);
            break;
        }
    }
}

void linked_list_clear(LinkedList *lst)
{
    while (lst->first)
    {
        linked_list_remove(lst, lst->first);
    }
}

LinkedList_it_state linked_list_iteration_start(LinkedList *lst)
{
    LinkedList_it_state state;
    state.lst = lst;
    state.node = NULL;
    state.delete_current = 0;
    return state;
}

void *linked_list_iterate(LinkedList_it_state *state)
{
    if (state->lst == NULL)
        return NULL;
    ListNode *to_be_deleted = NULL;
    if (state->delete_current && state->node)
    {
        state->delete_current = 0;
        to_be_deleted = state->node;
    }
    state->node = !state->node ? state->lst->first : state->node->next;
    if (to_be_deleted)
    {
        linked_list_remove(state->lst, to_be_deleted);
    }
    //printf("next node: %d{next:%d, prev:%d, obj:%d}\n", state->node, !state->node ? 0 : state->node->next, !state->node ? 0 : state->node->prev, !state->node ? 0 : state->node->obj);
    if (!state->node)
    {
        state->lst = NULL;
        return NULL;
    }

    return state->node->obj;
}

void add_managed_list(LinkedList *lst)
{
    static LinkedList _lists;
    static int init = 0;
    if (!init)
    {
        _lists = linked_list_create();
        init = 1;
    }
    LinkedList **el;
    if (!lst)
    {
        //printf("Clearing %d entries\n", (int)_lists.count);
        LINKED_LIST_FOR_EACH(&_lists, LinkedList*, el, 1)
        {
            //printf("Clearing %d sub entries\n", (int)(*el)->count);
            linked_list_clear(*el);
        }
        return;
    }
    LINKED_LIST_FOR_EACH(&_lists, LinkedList*, el, *el == lst)
    {
        if (*el == lst)
        {
            //printf("List already in managed lists! Clearing old entry\n");
            linked_list_clear(*el);
        }
    }
    el = LINKED_LIST_ADD(&_lists, LinkedList *);
    *el = lst;
    *lst = linked_list_create();
}

#ifdef linked_list_test_main

typedef struct
{
    int a;
} A;

void test_manage()
{
    LinkedList lst1;
    LinkedList lst2;
    LinkedList lst3;
    add_managed_list(&lst1);
    LINKED_LIST_ADD(&lst1, A)->a = 1;
    add_managed_list(&lst2);
    LINKED_LIST_ADD(&lst2, A)->a = 1;
    LINKED_LIST_ADD(&lst2, A)->a = 2;
    add_managed_list(&lst3);
    LINKED_LIST_ADD(&lst3, A)->a = 1;
    LINKED_LIST_ADD(&lst3, A)->a = 2;
    LINKED_LIST_ADD(&lst3, A)->a = 3;

    add_managed_list(&lst2);
    add_managed_list(NULL);
}

int main(int argc, char **argv)
{
    LinkedList lst = linked_list_create();
    int delete_state = 0;
    A *el, *_5;
    for (int i = 0; i < 10; i++)
    {
        el = LINKED_LIST_ADD(&lst, A);
        el->a = i;
        if (i == 5)
            _5 = el;
    }
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Remove second last\n");
    linked_list_remove(&lst, lst.last->prev);
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Remove last\n");
    linked_list_remove(&lst, lst.last);
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Remove first\n");
    linked_list_remove(&lst, lst.first);
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Add one with value 123\n");
    LINKED_LIST_ADD(&lst, A)->a = 123;
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Remove %d by value\n", _5->a);
    linked_list_remove_by_value(&lst, _5);
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        delete_state = 0;
        printf("A{%d}\n", el->a);
        if (el->a == 3)
        {
            printf("Delete current!\n");
            delete_state = 1;
        }
    }
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    printf("Clear\n");
    linked_list_clear(&lst);
    LINKED_LIST_FOR_EACH(&lst, A, el, delete_state)
    {
        printf("A{%d}\n", el->a);
    }
    test_manage();
    return 0;
}

#endif