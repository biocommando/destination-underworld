#include <test-du.h>
#include "../duscript.h"

TEST(duscript__set_variables)
{
    DuScriptState st = du_script_init();
    DuScriptVariable *var;
    var = du_script_variable(&st, "hello");
    strcpy(var->value, "123");
    int ret = du_script_execute_line(&st, "*=hello\"world\"");
    ASSERT(INT_EQ(0, ret));
    ret = du_script_execute_line(&st, "*=hello2\"world2\"");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ("world", var->value));
    var = du_script_variable(&st, "hello2");
    ASSERT(STR_EQ("world2", var->value));
    // invalid values
    ret = du_script_execute_line(&st, "*=hello\"abc");
    ASSERT(INT_EQ(0, ret));
    ret = du_script_execute_line(&st, "*=hello2");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ("world", du_script_variable(&st, "hello")->value));
    ASSERT(STR_EQ("world2", du_script_variable(&st, "hello2")->value));

    var->read_only = 1;
    ret = du_script_execute_line(&st, "*=hello2\"asdasd\"");
    ASSERT(STR_EQ("world2", du_script_variable(&st, "hello2")->value));
}

TEST(duscript__conditions)
{
    DuScriptState st = du_script_init();
    DuScriptVariable *var, *var2;
    var = du_script_variable(&st, "var");
    var2 = du_script_variable(&st, "var2");

    strcpy(var->value, "hello");
    strcpy(var2->value, "world");

    int ret = du_script_execute_line(&st, "*?var = not_this label_1");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));
    ret = du_script_execute_line(&st, "*?var ! not_this label_1");
    ASSERT(INT_EQ(1, ret));
    ASSERT(STR_EQ(st.goto_label, "label_1"));
    *st.goto_label = 0;
    ret = du_script_execute_line(&st, "*?var = hello label_2");
    ASSERT(INT_EQ(1, ret));
    ASSERT(STR_EQ(st.goto_label, "label_2"));
    *st.goto_label = 0;
    ret = du_script_execute_line(&st, "*?var = hello and var2 = hello label_3");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));
    ret = du_script_execute_line(&st, "*?var = hello and var2 = world label_3");
    ASSERT(INT_EQ(1, ret));
    ASSERT(STR_EQ(st.goto_label, "label_3"));
    *st.goto_label = 0;
    // invalid syntax
    ret = du_script_execute_line(&st, "*?var == hello and var2 = world label_3");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));
    // invalid syntax
    ret = du_script_execute_line(&st, "*?var = hello and ");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));
    ret = du_script_execute_line(&st, "*?var = hello and");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));

    ret = du_script_execute_line(&st, "*?var = hello");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));

    ret = du_script_execute_line(&st, "*?var = hello +fwd_label");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, "+fwd_label"));
}

TEST(duscript__gotos)
{
    DuScriptState st = du_script_init();
    int ret = du_script_execute_line(&st, "data line");
    ASSERT(INT_EQ(-1, ret));
    strcpy(st.goto_label, "LABEL_1");
    ret = du_script_execute_line(&st, "data line");
    ASSERT(INT_EQ(0, ret));
    ret = du_script_execute_line(&st, "*@UNKNOWN_LABEL");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, "LABEL_1"));
    ret = du_script_execute_line(&st, "*@LABEL_1");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, ""));
    ret = du_script_execute_line(&st, "*>LABEL_2");
    ASSERT(INT_EQ(1, ret));
    ASSERT(STR_EQ(st.goto_label, "LABEL_2"));
    *st.goto_label = 0;
    ret = du_script_execute_line(&st, "*>+LABEL_3");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(st.goto_label, "+LABEL_3"));
}