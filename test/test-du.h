#pragma once
#include <stdio.h>
#include <string.h>

#define _AB_ASSERT_BASE(type, fmt, a, b) \
    0;                                   \
    type _a = (a);                       \
    type _b = (b);                       \
    sprintf(error, "A=" fmt ", B=" fmt, _a, _b);

#define _COMPARE(type, fmt, a, op, b) \
    _AB_ASSERT_BASE(type, fmt, a, b)  \
    _condition_result = (_a op _b)

#define _EQ_DELTA(type, fmt, a, b, d) \
    _AB_ASSERT_BASE(type, fmt, a, b)  \
    _condition_result = ((_a >= _b - (d)) && (_a <= _b + (d)))

#define NOT(x) \
    x;         \
    _condition_result = !_condition_result

#define FLOAT_EQ_DELTA(a, b, d) _EQ_DELTA(float, "%f", a, b, d)

#define FLOAT_EQ(a, b) FLOAT_EQ_DELTA(a, b, 1e-6)

#define INT_EQ(a, b) _COMPARE(int, "%d", a, ==, b)

#define FLOAT_GREATER(less, greater) _COMPARE(float, "%f", less, <, greater)

#define STR_EQ(a, b)                          \
    _AB_ASSERT_BASE(const char *, "%s", a, b) \
    _condition_result = !strcmp(_a, _b)

#define ASSERT(condition)                                                     \
    do                                                                        \
    {                                                                         \
        char error[1024] = "";                                                \
        int _condition_result = condition;                                    \
        if (!_condition_result)                                               \
        {                                                                     \
            *_test_result = 0;                                                \
            printf("-------- ASSERTION FAILED: " #condition "\n");            \
            if (*error)                                                       \
                printf("               >>> REASON: %s\n", error);             \
            extern _TEST_DEBUG_PRINT_VAR;                                     \
            if (*_test_debug_print)                                           \
                printf("               >>>> DEBUG: %s\n", _test_debug_print); \
        }                                                                     \
    } while (0)

#define EXPECT_FLOAT_EQ_DELTA(a, b, d) ASSERT(FLOAT_EQ_DELTA(a, b, d))

#define EXPECT_FLOAT_EQ(a, b) EXPECT_FLOAT_EQ_DELTA(a, b, 1e-6)

#define TEST_NAME(name) \
    test_fn_##name

#define TEST_FN_ARGS_DEF \
    int *_test_result

#define TEST_FN_DEF(name) \
    int TEST_NAME(name)(TEST_FN_ARGS_DEF)

#define TEST(name) \
    TEST_FN_DEF(name)

#define TEST_FN_ARGS _test_result

#define INIT_TESTS                                                   \
    int test_result, n_tests = 0, n_tests_failed = 0,                \
                     n_tests_skipped = 0;                            \
    const char *filter_pos = NULL, *filter_neg = NULL;               \
    for (int i = 1; i < argc; i++)                                   \
    {                                                                \
        if (!strcmp(argv[i], "--include") && i + 1 < argc)           \
            filter_pos = argv[i + 1];                                \
        if (!strcmp(argv[i], "--exclude") && i + 1 < argc)           \
            filter_neg = argv[i + 1];                                \
        if (!strcmp(argv[i], "--help"))                              \
        {                                                            \
            printf("Run unit tests\nAccepted arguments:\n"           \
                   "--help\n"                                        \
                   "    Show this help\n"                            \
                   "--include STRING\n"                              \
                   "    Include only tests containing this string\n" \
                   "--exclude STRING\n"                              \
                   "    Exclude tests containing this string\n");    \
            return 0;                                                \
        }                                                            \
    }

#define END_TESTS                                                               \
    printf("\n\nTotal tests: %d, ok: %d, failed: %d, skipped: %d\n",            \
           n_tests, n_tests - n_tests_failed, n_tests_failed, n_tests_skipped); \
    printf("Test result: %s\n", n_tests_failed ? "FAIL" : "OK");                \
    return n_tests_failed ? 1 : 0

#define TEST_FILTER(name)                        \
    (filter_pos && !strstr(name, filter_pos)) || \
        (filter_neg && strstr(name, filter_neg))

#define TEST_SUITE(name)                                                 \
    int skip_suite_##name = TEST_FILTER(#name);                          \
    printf("\n%s suite " #name "\n", !skip_suite_##name ? "Run" : "Skip"); \
    if (!skip_suite_##name)

#define RUN_TEST(name)                                 \
    do                                                 \
    {                                                  \
        TEST_DEBUG_PRINT_CLEAR();                      \
        if (TEST_FILTER(#name))                        \
        {                                              \
            n_tests_skipped++;                         \
            break;                                     \
        }                                              \
        n_tests++;                                     \
        test_result = 1;                               \
        extern TEST_FN_DEF(name);                      \
        TEST_NAME(name)(&test_result);                 \
        n_tests_failed += test_result ? 0 : 1;         \
        printf("%s : " #name "\n",                     \
               test_result ? "[  OK  ]" : "[ FAIL ]"); \
    } while (0)

#define _TEST_DEBUG_PRINT_VAR char _test_debug_print[1024]

#define TEST_GLOBAL_STATE \
    _TEST_DEBUG_PRINT_VAR = "";

#define TEST_DEBUG_PRINTF(...)                   \
    do                                           \
    {                                            \
        extern _TEST_DEBUG_PRINT_VAR;            \
        sprintf(_test_debug_print, __VA_ARGS__); \
    } while (0)

#define TEST_DEBUG_PRINT_CLEAR() TEST_DEBUG_PRINTF("")