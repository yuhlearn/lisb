#include <chunk/chunk.h>
#include <memory/memory.h>

#include <stdlib.h>

void chunk_init_chunk(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    value_init_value_array(&chunk->constants);
}

void chunk_free_chunk(Chunk *chunk)
{
    MEMORY_FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    MEMORY_FREE_ARRAY(int, chunk->lines, chunk->capacity);
    value_free_value_array(&chunk->constants);
    chunk_init_chunk(chunk);
}

void chunk_write_chunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int old_capacity = chunk->capacity;
        chunk->capacity = MEMORY_GROW_CAPACITY(old_capacity);
        chunk->code = MEMORY_GROW_ARRAY(uint8_t, chunk->code, old_capacity, chunk->capacity);
        chunk->lines = MEMORY_GROW_ARRAY(int, chunk->lines, old_capacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int chunk_add_constant(Chunk *chunk, Value value)
{
    value_write_value_array(&chunk->constants, value);
    return chunk->constants.count - 1;
}