#ifndef LOGGING_H
#define LOGGING_H

/**
 * @brief Log a message in verbose mode
 */
#define LOG(message, ...)

/**
 * @brief Execute a block when verbose mode is enabled
 *
 */
#define VERBOSE_ONLY(block)

#define ASSERT_F(test, message, ...)
#define ASSERT(test, message)
#define ASSERT_SOFT(test, ...)

#endif