#ifndef LOGGING_H
#define LOGGING_H

#if VERBOSE

#if _WIN32
#include "windows/logger.h"

#elif defined(__APPLE__) || defined(__linux__)
#include "unix/logger.h"
#endif

#else

/**
 * @brief Log a message in verbose mode
 */
#define LOG(message, ...)

#define ASSERT_F(test, message, ...)
#define ASSERT(test, message)
#define ASSERT_SOFT(test, ...)

inline void init_logger() {}
inline void free_logger() {}

#endif

#endif