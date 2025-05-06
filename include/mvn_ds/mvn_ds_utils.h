// filepath: /Users/validmac/Projects/c_cpp/mvn_ds/include/mvn_ds/mvn_ds_utils.h
/*
 * Copyright (c) 2024 Jake Larson
 */
#ifndef MVN_DS_UTILS_H
#define MVN_DS_UTILS_H

#include <stdio.h>  // For fprintf, stderr
#include <stdlib.h> // For malloc, calloc, realloc, free, size_t
#include <string.h> // For memcpy, memmove

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

// Allow custom memory allocators
#ifndef MVN_DS_MALLOC
#define MVN_DS_MALLOC(sz) malloc(sz)
#endif
#ifndef MVN_DS_CALLOC
#define MVN_DS_CALLOC(n, sz) calloc(n, sz)
#endif
#ifndef MVN_DS_REALLOC
#define MVN_DS_REALLOC(ptr, sz) realloc(ptr, sz)
#endif
#ifndef MVN_DS_FREE
#define MVN_DS_FREE(ptr) free(ptr)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_UTILS_H */
