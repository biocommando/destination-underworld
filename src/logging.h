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