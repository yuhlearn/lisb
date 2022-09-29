#ifndef _DEBUG_H
#define _DEBUG_H

#include <chunk/chunk.h>
#include <parser/parser.h>

void debug_disassemble_chunk(Chunk *chunk, const char *name);
int debug_disassemble_instruction(Chunk *chunk, int offset);
void debug_disassemble_sexpression(SExpr *sexpr);

#endif