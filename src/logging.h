#ifndef LOGGING_H
#define LOGGING_H

#ifdef ENABLE_LOGGING
#define LOG printf
#else
#define LOG(...)
#endif
#endif