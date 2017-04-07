#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>

#define DEBUG_ENABLE
#define LOG_ENALBE

#define MAJOR    0
#define MINOR    0
#define PATCH    0
#define RELEASE  1
#define VERSION() (RELEASE | (PATCH << 8) | (MINOR << 16) | (MAJOR << 24))
#define VERSION_STR() ("V"##MAJOR##"."##MINOR##"."##PATCH##"."##RELEASE)

#define MALLOC(size) malloc(size)
#define FREE(p) free(p)

#define ASSERT(x) do {if (!(x))  abort();} while (0)

#ifdef LOG_ENALBE
#define DEBUG_LOG(args...) printf(args)
#else
#define DEBUG_LOG(args...) 
#endif

#ifdef DEBUG_ENABLE
#define DEBUG_PRINT(args...) printf(args)
#else
#define DEBUG_PRINT(args...)
#endif


#ifndef bool
#define bool unsigned int
#endif

#ifndef true
#define true (!!1)
#endif

#ifndef false
#define false (!!0)
#endif


#endif
