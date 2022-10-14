#ifndef _MEMORY_H
#define _MEMORY_H

#include <common/common.h>
#include <object/object.h>

#define MEMORY_ALLOCATE(type, count) \
    (type *)memory_reallocate(NULL, 0, sizeof(type) * (count))

#define MEMORY_FREE(type, pointer) memory_reallocate(pointer, sizeof(type), 0)

#define MEMORY_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity)*2)

#define MEMORY_GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type *)memory_reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define MEMORY_FREE_ARRAY(type, pointer, oldCount) \
    memory_reallocate(pointer, sizeof(type) * (oldCount), 0)

void *memory_reallocate(void *pointer, size_t oldSize, size_t newSize);
void memory_mark_object(Obj *object);
void memory_mark_value(Value value);
void memory_collect_garbage();
void memory_free_objects();

#endif