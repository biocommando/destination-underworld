#pragma once
#include <stdio.h>
#include <string.h>

typedef struct
{
    int test_result, n_tests, n_tests_failed, n_tests_skipped;
    const char *filter_pos, *filter_neg;
} T_test_state;

#define _AB_ASSERT_BASE(type, fmt, a, b) \
    0;                                   \
    type _a = (a);                       \
    type _b = (b);                       \
    sprintf(_test_error, "A=" fmt ", B=" fmt, _a, _b);

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
        extern T_test_state _test_state;                                      \
        char _test_error[1024] = "";                                          \
        int _condition_result = condition;                                    \
        if (!_condition_result)                                               \
        {                                                                     \
            _test_state.test_result = 0;                                      \
            printf("-------- ASSERTION FAILED: " #condition "\n");            \
            if (*_test_error)                                                 \
                printf("               >>> REASON: %s\n", _test_error);       \
            extern _TEST_DEBUG_PRINT_VAR;                                     \
            if (*_test_debug_print)                                           \
                printf("               >>>> DEBUG: %s\n", _test_debug_print); \
        }                                                                     \
    } while (0)

#define EXPECT_FLOAT_EQ_DELTA(a, b, d) ASSERT(FLOAT_EQ_DELTA(a, b, d))

#define EXPECT_FLOAT_EQ(a, b) EXPECT_FLOAT_EQ_DELTA(a, b, 1e-6)

// For backwards compatibility
#define TEST(name) void name()

#define INIT_TESTS                                                   \
    _test_state.n_tests = 0;                                         \
    _test_state.n_tests_failed = 0;                                  \
    _test_state.n_tests_skipped = 0;                                 \
    _test_state.filter_pos = NULL;                                   \
    _test_state.filter_neg = NULL;                                   \
    for (int i = 1; i < argc; i++)                                   \
    {                                                                \
        if (!strcmp(argv[i], "--include") && i + 1 < argc)           \
            _test_state.filter_pos = argv[i + 1];                    \
        if (!strcmp(argv[i], "--exclude") && i + 1 < argc)           \
            _test_state.filter_neg = argv[i + 1];                    \
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

#define END_TESTS                                                                 \
    printf("\n\nTotal tests: %d, ok: %d, failed: %d, skipped: %d\n",              \
           _test_state.n_tests, _test_state.n_tests - _test_state.n_tests_failed, \
           _test_state.n_tests_failed, _test_state.n_tests_skipped);              \
    printf("Test result: %s\n", _test_state.n_tests_failed ? "FAIL" : "OK");      \
    return _test_state.n_tests_failed ? 1 : 0

#define TEST_FILTER(name)                                                \
    (_test_state.filter_pos && !strstr(name, _test_state.filter_pos)) || \
        (_test_state.filter_neg && strstr(name, _test_state.filter_neg))

#define TEST_SUITE(name)                                                   \
    int skip_suite_##name = TEST_FILTER(#name);                            \
    printf("\n%s suite " #name "\n", !skip_suite_##name ? "Run" : "Skip"); \
    if (!skip_suite_##name)

#define RUN_TEST(name)                                                 \
    do                                                                 \
    {                                                                  \
        extern T_test_state _test_state;                               \
        TEST_DEBUG_PRINT_CLEAR();                                      \
        if (TEST_FILTER(#name))                                        \
        {                                                              \
            _test_state.n_tests_skipped++;                             \
            break;                                                     \
        }                                                              \
        _test_state.n_tests++;                                         \
        _test_state.test_result = 1;                                   \
        extern void name();                                            \
        name();                                                        \
        _test_state.n_tests_failed += _test_state.test_result ? 0 : 1; \
        printf("%s : " #name "\n",                                     \
               _test_state.test_result ? "[  OK  ]" : "[ FAIL ]");     \
    } while (0)

#define _TEST_DEBUG_PRINT_VAR char _test_debug_print[1024]

#define TEST_GLOBAL_STATE       \
    _TEST_DEBUG_PRINT_VAR = ""; \
    T_test_state _test_state;

#define TEST_DEBUG_PRINTF(...)                   \
    do                                           \
    {                                            \
        extern _TEST_DEBUG_PRINT_VAR;            \
        sprintf(_test_debug_print, __VA_ARGS__); \
    } while (0)

#define TEST_DEBUG_PRINT_CLEAR() TEST_DEBUG_PRINTF("")