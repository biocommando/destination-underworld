#pragma once

#ifdef ENABLE_LOGGING
#define LOG(...)                    \
    do                              \
    {                               \
        extern int logging_enabled; \
        if (logging_enabled)        \
            printf(__VA_ARGS__);    \
    } while (0)
#else
#define LOG(...)       \
    do                 \
    { /* do nothing */ \
    } while (0)
#endif

#ifdef TRACE_LOG
#define LOG_TRACE LOG
#else
#define LOG_TRACE(...)
#endif

#define LOG_PREFIX(prefix, ...) \
    do                          \
    {                           \
        printf(prefix ": ");    \
        printf(__VA_ARGS__);    \
    } while (0);

#define LOG_ERROR(...) LOG_PREFIX("ERROR", __VA_ARGS__)
#define LOG_FATAL(...) LOG_PREFIX("FATAL", __VA_ARGS__)

#define FATAL(expression, ...)                  \
    do                                          \
    {                                           \
        if (expression)                         \
        {                                       \
            extern int exit_due_to_fatal_error; \
            exit_due_to_fatal_error = 1;        \
            LOG_FATAL(__VA_ARGS__);             \
            exit(1);                            \
        }                                       \
    } while (0)
