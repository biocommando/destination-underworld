#include <test-du.h>
#include "../linked_list.h"

static void str_append_number(char *str, int data)
{
    char num[16];
    sprintf(num, "%d,", data);
    strcat(str, num);
}

TEST(linked_list)
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
    ASSERT(STR_EQ(test_str, "0,1,2,3,4,5,6,7,8,9,"));
    linked_list_remove(&lst, lst.last->prev);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "0,1,2,3,4,5,6,7,9,"));
    linked_list_remove(&lst, lst.last);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "0,1,2,3,4,5,6,7,"));
    linked_list_remove(&lst, lst.first);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "1,2,3,4,5,6,7,"));
    *LINKED_LIST_ADD(&lst, int) = 123;
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "1,2,3,4,5,6,7,123,"));
    linked_list_remove_by_value(&lst, _5);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "1,2,3,4,6,7,123,"));
    LINKED_LIST_FOR_EACH(&lst, int, el, *el == 3);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, "1,2,4,6,7,123,"));
    linked_list_clear(&lst);
    *test_str = 0;
    LINKED_LIST_FOR_EACH(&lst, int, el, 0)
    {
        str_append_number(test_str, *el);
    }
    ASSERT(STR_EQ(test_str, ""));
    return 0;
}