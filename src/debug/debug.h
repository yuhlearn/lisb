#ifndef _DEBUG_H
#define _DEBUG_H

#include <chunk/chunk.h>

void disassembleChunk(Chunk *chunk, const char *name);
int disassembleInstruction(Chunk *chunk, int offset);

#endif