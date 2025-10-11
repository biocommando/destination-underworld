#include <test-du.h>
#include "../linked_list.h"

static void str_append_number(char *str, int data)
{
    char num[16];
    sprintf(num, "%d,", data);
    strcat(str, num);
}

static void assert_int_list_contents_equal(LinkedList *lst, const char *cmp_str)
{
    char test_str[256] = "";
    int *el;
    LINKED_LIST_FOR_EACH(lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, cmp_str));
}

TEST(linked_list__list_add_remove_clear_access)
{
    LinkedList lst = linked_list_create();
    int *el, *_5;
    for (int i = 0; i < 10; i++)
    {
        el = LINKED_LIST_ADD(&lst, int);
        *el = i;
        if (i == 5)
            _5 = el;
    }
    char test_str[256] = "";
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    assert_int_list_contents_equal(&lst, "0,1,2,3,4,5,6,7,8,9,");

    linked_list_remove(&lst, lst.last->prev);
    assert_int_list_contents_equal(&lst, "0,1,2,3,4,5,6,7,9,");

    linked_list_remove(&lst, lst.last);
    assert_int_list_contents_equal(&lst, "0,1,2,3,4,5,6,7,");

    linked_list_remove(&lst, lst.first);
    assert_int_list_contents_equal(&lst, "1,2,3,4,5,6,7,");

    *LINKED_LIST_ADD(&lst, int) = 123;
    assert_int_list_contents_equal(&lst, "1,2,3,4,5,6,7,123,");

    linked_list_remove_by_value(&lst, _5);
    assert_int_list_contents_equal(&lst, "1,2,3,4,6,7,123,");

    LINKED_LIST_FOR_EACH(&lst, int, el, *el == 3);
    assert_int_list_contents_equal(&lst, "1,2,4,6,7,123,");

    linked_list_clear(&lst);
    assert_int_list_contents_equal(&lst, "");
    linked_list_clear(&lst);
    assert_int_list_contents_equal(&lst, "");
    return 0;
}

void test_managed_lists(LinkedList *lists)
{
    LinkedList *mgmt = NULL;
    for (int i = 0; i < 3; i++)
    {
        add_managed_list(&lists[i], &mgmt);
        for (int j = 0; j < 3; j++)
        {
            *(LINKED_LIST_ADD(&lists[i], int)) = j;
        }
        assert_int_list_contents_equal(&lists[i], "0,1,2,");
    }
    // Add already managed -> should clear the original entry
    add_managed_list(&lists[1], &mgmt);
    for (int i = 0; i < 3; i++)
    {
        assert_int_list_contents_equal(&lists[i], i == 1 ? "" : "0,1,2,");
    }
    // Add the fourth list
    add_managed_list(&lists[3], &mgmt);
    // Add elements back
    for (int j = 0; j < 3; j++)
    {
        *(LINKED_LIST_ADD(&lists[1], int)) = j;
        // Also init the fourth list
        *(LINKED_LIST_ADD(&lists[3], int)) = j;
    }
    for (int i = 0; i < 4; i++)
    {
        assert_int_list_contents_equal(&lists[i], "0,1,2,");
    }
    // Passing NULL clears all lists
    add_managed_list(NULL, &mgmt);

    for (int i = 0; i < 4; i++)
    {
        assert_int_list_contents_equal(&lists[i], "");
    }
    ASSERT(mgmt == NULL);
}

TEST(linked_list__managed_lists)
{
    LinkedList lists[4];
    // Both executions should work exactly in the same manner.
    test_managed_lists(lists);
    test_managed_lists(lists);
}