#ifndef LOGGING_H
#define LOGGING_H

#ifdef ENABLE_LOGGING
#define LOG printf
#else
#define LOG(...)
#endif
#endif

#ifdef TRACE_LOG
#define LOG_TRACE LOG
#else
#define LOG_TRACE(...)
#endif