#ifndef UTIL_H
#define UTIL_H

#if _WIN32
#include <windows.h>
/**
 * @brief Universal character type.
 *
 * Universal character is either UTF-8 or Unicode depending on the build target.
 */
typedef TCHAR char_t;

/**
 * @brief Boolean type
 */
typedef BOOL bool_t;

#define PATH_SEP TEXT(";")

#elif defined(__APPLE__) || defined(__linux__)
#include <stddef.h>
typedef char char_t;
typedef int bool_t;

#define TRUE 1
#define FALSE 0
#ifndef NULL
#define NULL 0
#endif

#define TEXT(text) text

#define PATH_SEP TEXT(":")
#endif

#define STR_LEN(str) (sizeof(str) / sizeof((str)[0]))

/**
 * @brief Converts UTF-8 string to universal string.
 *
 * @remark Return value must be freed by caller.
 *
 * @param str UTF-8 string to convert.
 * @return char_t* Universal string of the passed string.
 */
char_t *widen(const char *str);

/**
 * @brief Convert universal string to UTF-8 string.
 *
 * @remark Return value must be freed by caller.
 *
 * @param str Universal string to convert.
 * @return char* UTF-8 string of the passed string.
 */
char *narrow(const char_t *str);

/**
 * @brief Get the full path to the given module.
 *
 * @remark Module path string must be freed by caller.
 *
 * @param module Module to get the path of. If NULL, gets path of the current
 *               module.
 * @param result Reference to variable which will receive the address to the
 *               module path.
 * @param size Reference to variable which will receive the length of the module
 *             path.
 * @param free_space How much additional space to allocate after the module path
 *                   string.
 * @return size_t Length of the allocated array (size + free_space)
 */
size_t get_module_path(void *module, char_t **result, size_t *size,
                       size_t free_space);

/**
 * @brief Get the full path to the relative path.
 *
 * @remark Return value must be freed by caller.
 *
 * @param path Path to resolve relative path for.
 * @return char_t* Absolute path to the given path.
 */
char_t *get_full_path(char_t *path);

/**
 * @brief Get the directory name of the given path.
 *
 * Splits the path into file name (after last path separator) and directory path
 * (before last path separator) and returns the directory path part.
 *
 * @remark Returned value must be freed by the caller.
 *
 * @param path Full path to get directory name of.
 * @return char_t* Directory part of the path (part before last path separator).
 */
char_t *get_folder_name(char_t *path);

/**
 * @brief Get the file name from the full path
 *
 * @remark Returned value must be freed by the caller.
 *
 * @param path Full path to get file name from.
 * @param with_ext Whether to include file extension or not.
 * @return char_t* File name part of the path.
 */
char_t *get_file_name(char_t *path, bool_t with_ext);

/**
 * @brief Check if a file exists.
 *
 * @param file File path to check.
 * @return bool_t TRUE if file exists, otherwise FALSE.
 */
bool_t file_exists(char_t *file);

/**
 * @brief Check if a folder exists.
 *
 * @param folder Folder path to check.
 * @return bool_t TRUE if folder exsits, otherwise FALSE.
 */
bool_t folder_exists(char_t *folder);

/**
 * @brief Get the current working directory.
 *
 * @remark Returned value must be freed by caller.
 *
 * @return char_t* Absolute path to current working directory.
 */
char_t *get_working_dir();

/**
 * @brief Get path to the current executable.
 *
 * @remark Returned value must be freed by caller.
 *
 * @return char_t* Absolue path to the current executable.
 */
char_t *program_path();

size_t get_file_size(void *file);

#endif