#ifndef _COMPILER_H
#define _COMPILER_H

#include <vm/vm.h>

typedef enum
{
    COMPILER_OK,
    COMPILER_EOF,
    COMPILER_COMPILE_ERROR,
} CompileResult;

CompileResult compiler_compile(const char *source, Chunk *chunk);

#endif