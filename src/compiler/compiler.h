#ifndef _COMPILER_H
#define _COMPILER_H

#include <vm/vm.h>

ObjFunction *compiler_compile(const char *source);

#endif