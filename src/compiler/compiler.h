#ifndef _COMPILER_H
#define _COMPILER_H

#include <vm/vm.h>

bool compiler_compile(const char *source, Chunk *chunk);

#endif