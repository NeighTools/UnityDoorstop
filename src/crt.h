#ifndef CRT_H
#define CRT_H

#if _WIN32
#include "windows/wincrt.h"
// Better default to account for longer name support
#define MAX_PATH 1024

#if _WIN64
#define ENV64
#else
#define ENV32
#endif

#elif defined(__APPLE__) || defined(__linux__)
#define _GNU_SOURCE
#include <dlfcn.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif

#if __x86_64__ || __ppc64__
#define ENV64
#else
#define ENV32
#endif

#define MAX_PATH PATH_MAX

#define shutenv(val)

#endif

#endif