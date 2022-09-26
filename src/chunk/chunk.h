#ifndef _CHUNK_H
#define _CHUNK_H

#include <common/common.h>
#include <value/value.h>

typedef enum
{
    OP_CONSTANT,
    OP_NULL,
    OP_TRUE,
    OP_FALSE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,
} OpCode;

typedef struct
{
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;

void chunk_init_chunk(Chunk *chunk);
void chunk_free_chunk(Chunk *chunk);
void chunk_write_chunk(Chunk *chunk, uint8_t byte, int line);
int chunk_add_constant(Chunk *chunk, Value value);

#endif