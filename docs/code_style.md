# Recommended C style and coding rules

- Set Column limit to 100
- Comments should use 4 spaces after a statement when single line and allow double forward slashes (`//`)
- Change to use `stdbool.h` library for `true` or `false` values insteed of `0` or `1`
- Add Copyright section. Use `Copyright (c) {year} Jake Larson` for copyright notice
- Declare all local variables of the same type on a separate line
- Prefer using MVN_DS_MALLOC, MVN_DS_CALLOC, MVN_DS_REALLOC, and MVN_DS_FREE for memory managment as they are aliases for SDL memory management functions
- Include instructions to document macros in header file
- Ensure that all variables are at least 3 characters long
- Ensure that code is compatible with MSVC, GCC, and Clang
- Never include `.c` files in another `.c` file
- `.c` file should first include corresponding `.h` file, later others, unless otherwise explicitly necessary
- Do not include module private declarations in header file
- Header file example (no license for sake of an example)
- Ensure Doxygen comments are used for implementation only and not in the header file unless explicitly necessary for code only defined in the header

```c
/* License comes here */
#ifndef TEMPLATE_HDR_H
#define TEMPLATE_HDR_H

/* Include headers */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* File content here */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEMPLATE_HDR_H */
```
