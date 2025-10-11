#pragma once
#include <stdio.h>
#include <string.h>

#ifndef TEST_DEBUG_STR_LENGTH
#define TEST_DEBUG_STR_LENGTH 1024
#endif

#define TEST_NAME_SZ 4096

typedef struct
{
    int test_result, n_tests, n_tests_failed, n_tests_skipped;
    const char *filter[32];
    int filter_type[32];
    int filter_count;
    char debug[TEST_DEBUG_STR_LENGTH];
    char test_name[TEST_NAME_SZ];
} T_test_state;

#define _AB_ASSERT_BASE(type, fmt, a, b) \
    0;                                   \
    type _a = (a);                       \
    type _b = (b);                       \
    snprintf(_test_error, 1023, "A=" fmt ", B=" fmt, _a, _b);

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
            if (*_test_state.debug)                                           \
                printf("               >>>> DEBUG: %s\n", _test_state.debug); \
        }                                                                     \
    } while (0)

#define INIT_TESTS                                                       \
    memset(&_test_state, 0, sizeof(_test_state));                        \
    for (int i = 1; i < argc; i++)                                       \
    {                                                                    \
        const char *arg = argv[i];                                       \
        if (!strcmp(arg, "--help"))                                      \
        {                                                                \
            printf("Run unit tests\nAccepted arguments:\n"               \
                   "--help\n"                                            \
                   "    Show this help\n"                                \
                   "--include STRING\n"                                  \
                   "    Include only tests containing this string.\n"    \
                   "--exclude STRING\n"                                  \
                   "    Exclude tests containing this string\n"          \
                   "Prefix include/exclude with 0-9 to affect "          \
                   "only names at certain level.\n"                      \
                   "Example: '%s --include 0suite_name' will only check" \
                   "filter for the outermost test suite name.\n",        \
                   argv[0]);                                             \
            return 0;                                                    \
        }                                                                \
        if (i + 1 >= argc || _test_state.filter_count == 32)             \
            continue;                                                    \
        if (!strcmp(arg, "--include") || !strcmp(arg, "--exclude"))      \
        {                                                                \
            _test_state.filter_type[_test_state.filter_count] = arg[2];  \
            _test_state.filter[_test_state.filter_count++] = argv[++i];  \
        }                                                                \
    }                                                                    \
    for (int i = 0; i < _test_state.filter_count; i++)                   \
        printf("%sTYPE '%c', VALUE '%s'\n",                              \
               i == 0 ? "Running unit tests with filters:\n" : "",       \
               _test_state.filter_type[i], _test_state.filter[i]);

#define END_TESTS                                                                 \
    printf("\n\nTotal tests: %d, ok: %d, failed: %d, skipped: %d\n",              \
           _test_state.n_tests, _test_state.n_tests - _test_state.n_tests_failed, \
           _test_state.n_tests_failed, _test_state.n_tests_skipped);              \
    printf("Test result: %s\n", _test_state.n_tests_failed ? "FAIL" : "OK");      \
    return _test_state.n_tests_failed ? 1 : 0

#define TEST_SUITE(name) \
    if (_check_test_suite_enabled(#name))

#define END_TEST_SUITE         \
    do                         \
    {                          \
        _edit_test_name(NULL); \
    } while (0)

#define RUN_TEST_SUITE(name) \
    TEST_SUITE(name)         \
    {                        \
        extern void name();  \
        name();              \
        END_TEST_SUITE;      \
    }

#define RUN_TEST_FN_DEF void _run_test__execute(void (*fn)(), const char *name)
RUN_TEST_FN_DEF;
#define CHECK_TEST_SUITE_FN_DEF int _check_test_suite_enabled(const char *name)
CHECK_TEST_SUITE_FN_DEF;
#define EDIT_TEST_NAME_FN_DEF void _edit_test_name(const char *append)
EDIT_TEST_NAME_FN_DEF;

#define RUN_TEST(name)                   \
    do                                   \
    {                                    \
        extern void name();              \
        _run_test__execute(name, #name); \
    } while (0)

#define TEST_GLOBAL_STATE                          \
    T_test_state _test_state;                      \
    static int _count_depth()                      \
    {                                              \
        int depth = 0;                             \
        const char *s = _test_state.test_name;     \
        while (s)                                  \
        {                                          \
            s = strstr(s, ".");                    \
            if (s)                                 \
            {                                      \
                s++;                               \
                depth++;                           \
            }                                      \
        }                                          \
        return depth;                              \
    }                                              \
    static int _check_filter()                     \
    {                                              \
        T_test_state *ts = &_test_state;           \
        int pos = 0, neg = 1;                      \
        int has_pos = 0;                           \
        for (int i = 0; i < ts->filter_count; i++) \
        {                                          \
            const char *f = ts->filter[i];         \
            if (f[0] >= '0' && f[0] <= '9')        \
            {                                      \
                if (_count_depth() != f[0] - '0')  \
                    continue;                      \
                f++;                               \
            }                                      \
            const char *r =                        \
                strstr(ts->test_name, f);          \
            if (ts->filter_type[i] == 'i')         \
            {                                      \
                has_pos = 1;                       \
                pos = pos || r != NULL;            \
            }                                      \
            else                                   \
                neg = neg && r == NULL;            \
        }                                          \
        int run = (!has_pos || pos) && neg;        \
        return !run;                               \
    }                                              \
    CHECK_TEST_SUITE_FN_DEF                        \
    {                                              \
        _edit_test_name(name);                     \
        int skip = _check_filter();                \
        if (skip)                                  \
            _edit_test_name(NULL);                 \
        else                                       \
            printf("\nRun suite %s\n", name);      \
        return !skip;                              \
    }                                              \
    RUN_TEST_FN_DEF                                \
    {                                              \
        TEST_DEBUG_PRINT_CLEAR();                  \
        _edit_test_name(name);                     \
        if (_check_filter())                       \
        {                                          \
            _test_state.n_tests_skipped++;         \
            goto end;                              \
        }                                          \
        _test_state.n_tests++;                     \
        _test_state.test_result = 1;               \
        fn();                                      \
        if (_test_state.test_result)               \
            printf("[  OK  ] : ");                 \
        else                                       \
        {                                          \
            _test_state.n_tests_failed++;          \
            printf("[ FAIL ] : ");                 \
        }                                          \
        printf("%s\n",                             \
               _test_state.test_name);             \
    end:                                           \
        _edit_test_name(NULL);                     \
    }                                              \
    EDIT_TEST_NAME_FN_DEF                          \
    {                                              \
        char *tn = _test_state.test_name;          \
        if (!append)                               \
        {                                          \
            unsigned i = strlen(tn);               \
            while (--i && tn[i] != '.')            \
                ;                                  \
            tn[i] = 0;                             \
            return;                                \
        }                                          \
        unsigned sz = strlen(tn) +                 \
                      strlen(append) + 2;          \
        if (append && sz < TEST_NAME_SZ)           \
        {                                          \
            if (*tn)                               \
                strcat(tn, ".");                   \
            strcat(tn, append);                    \
        }                                          \
    }

#define TEST_DEBUG_PRINTF(...)                   \
    do                                           \
    {                                            \
        extern T_test_state _test_state;         \
        sprintf(_test_state.debug, __VA_ARGS__); \
    } while (0)

#define TEST_DEBUG_PRINT_CLEAR() TEST_DEBUG_PRINTF("")

#define TEST_MAIN(main_function_name)     \
    TEST_GLOBAL_STATE                     \
    int main(int argc, char **argv)       \
    {                                     \
        INIT_TESTS                        \
        extern void main_function_name(); \
        main_function_name();             \
        END_TESTS;                        \
    }
