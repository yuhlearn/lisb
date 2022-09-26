#include <memory/memory.h>
#include <vm/vm.h>

#include <stdlib.h>

void *memory_reallocate(void *pointer, size_t old_size, size_t new_size)
{
    if (new_size == 0)
    {
        free(pointer);
        return NULL;
    }

    void *result = realloc(pointer, new_size);

    if (result == NULL)
        exit(1);

    return result;
}

static void memory_free_object(Obj *object)
{
    switch (object->type)
    {
    case OBJ_STRING:
    {
        ObjString *string = (ObjString *)object;
        MEMORY_FREE_ARRAY(char, string->chars, string->length + 1);
        MEMORY_FREE(ObjString, object);
        break;
    }
    }
}

void memory_free_objects()
{
    Obj *object = vm.objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        memory_free_object(object);
        object = next;
    }
}