#ifndef _COMPILER_H
#define _COMPILER_H

#include <vm/vm.h>

typedef enum
{
    COMPILE_OK,
    COMPILE_EOF,
    COMPILE_COMPILE_ERROR,
} CompileResult;

CompileResult compiler_compile(const char *source, Chunk *chunk);

#endif