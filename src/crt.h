#ifndef CRT_H
#define CRT_H

#if _WIN32
#include "windows/wincrt.h"
// Better default to account for longer name support
#define MAX_PATH 1024

#elif defined(__APPLE__) || defined(__linux__)
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>

#define shutenv(val)

#endif

#endif