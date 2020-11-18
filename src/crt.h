#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include "windows/wincrt.h"
#elif defined(__APPLE__) || defined(__linux__)
#include <stdio.h>
#include <stdlib.h>
#endif