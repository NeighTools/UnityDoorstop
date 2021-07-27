#ifndef LOGGING_H
#define LOGGING_H

/**
 * @brief Log a message in verbose mode
 */
#define LOG(message, ...)

#define ASSERT_F(test, message, ...)
#define ASSERT(test, message)
#define ASSERT_SOFT(test, ...)

void init_logger();
void free_logger();

#if VERBOSE

#if _WIN32
#include "windows/logger.h"

#elif defined(__APPLE__) || defined(__linux__)
#include "unix/logger.h"
#endif

#else

inline void init_logger() {}
inline void free_logger() {}

#endif

#endif