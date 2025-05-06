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

/**
 * @internal
 * @brief Reallocates memory, handling potential errors.
 * Uses MVN_DS_REALLOC and MVN_DS_FREE.
 * @param pointer Existing pointer (or NULL).
 * @param old_size Current allocated size (ignored by standard realloc).
 * @param new_size Desired new size. If 0, frees the pointer.
 * @return Pointer to the reallocated memory, or NULL if new_size is 0 or allocation fails.
 */
static inline void *mvn_reallocate(void *pointer, size_t old_size, size_t new_size)
{
    (void)old_size; // Unused in this basic implementation
    if (new_size == 0) {
        MVN_DS_FREE(pointer);
        return NULL;
    }
    void *result = MVN_DS_REALLOC(pointer, new_size);
    if (result == NULL && new_size > 0) { // Check if allocation actually failed
        fprintf(stderr, "[MVN_DS] Memory reallocation failed!\n");
    }
    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MVN_DS_UTILS_H */
