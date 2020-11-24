#ifndef CRT_H
#define CRT_H

#if _WIN32
#include "windows/wincrt.h"

#elif defined(__APPLE__) || defined(__linux__)
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

#endif

#endif