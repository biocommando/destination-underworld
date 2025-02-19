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
#define LOG(...)
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