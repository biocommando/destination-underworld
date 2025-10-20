#include <unittests.h>
#include <stdio.h>
#include <string.h>

#include "../variables.h"

void variables__create_VarState__creates_0_state()
{
    VarState st = create_VarState();
    ASSERT(INT_EQ(st.num_vars, 0));
    ASSERT(st.vars == NULL);
}

void variables__set_get__works()
{
    VarState st = create_VarState();

    ASSERT(get_var("hello", &st) == NULL);
    ASSERT(set_var("hello", "123", &st) == 0);
    ASSERT(set_var("world", "universe", &st) == 0);
    ASSERT(set_var("hello world", "masters of the universe", &st) == 0);
    
    ASSERT(STR_EQ(get_var("hello", &st), "123"));
    ASSERT(set_var("hello", "321", &st) == 0);
    
    ASSERT(STR_EQ(get_var("hello", &st), "321"));
    ASSERT(STR_EQ(get_var("world", &st), "universe"));
    ASSERT(STR_EQ(get_var("hello world", &st), "masters of the universe"));
    
    free_VarState(&st);
}

void variables__set_readonly__works()
{
    VarState st = create_VarState();

    ASSERT(NOT(set_var_readonly("hello", &st) == 0));
    ASSERT(set_var("hello", "123", &st) == 0);
    ASSERT(set_var_readonly("hello", &st) == 0);
    
    ASSERT(STR_EQ(get_var("hello", &st), "123"));
    ASSERT(NOT(set_var("hello", "321", &st) == 0));
    
    free_VarState(&st);
}

void variables__set__too_long_name__returns_nonzero()
{
    VarState st = create_VarState();

    char name[257];
    memset(name, 'x', 256);
    name[256] = 0;
    ASSERT(NOT(set_var(name, "123", &st) == 0));
    free_VarState(&st);
}

void variables__set__too_long_value__returns_nonzero()
{
    VarState st = create_VarState();

    char value[257];
    memset(value, 'x', 256);
    value[256] = 0;
    ASSERT(NOT(set_var("hello", value, &st) == 0));
    free_VarState(&st);
}

void variables__free_VarState__resets_state_to_0_state()
{
    VarState st = create_VarState();
    ASSERT(set_var("hello", "123", &st) == 0);
    free_VarState(&st);
    ASSERT(INT_EQ(st.num_vars, 0));
    ASSERT(st.vars == NULL);
    // This should work too
    free_VarState(&st);
    ASSERT(INT_EQ(st.num_vars, 0));
    ASSERT(st.vars == NULL);
}

void test_suite__variables()
{
    RUN_TEST(variables__create_VarState__creates_0_state);
    RUN_TEST(variables__set_get__works);
    RUN_TEST(variables__set_readonly__works);
    RUN_TEST(variables__set__too_long_name__returns_nonzero);
    RUN_TEST(variables__free_VarState__resets_state_to_0_state);
}