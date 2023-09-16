#pragma once

#ifdef ENABLE_LOGGING
#define LOG printf
#else
#define LOG(...)
#endif

#ifdef TRACE_LOG
#define LOG_TRACE LOG
#else
#define LOG_TRACE(...)
#endif