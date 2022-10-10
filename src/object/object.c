#include <object/object.h>
#include <memory/memory.h>
#include <value/value.h>
#include <vm/vm.h>
#include <table/table.h>

#include <stdio.h>
#include <string.h>

#define OBJECT_ALLOCATE_OBJ(type, objectType) \
    (type *)object_allocate_object(sizeof(type), objectType)

static Obj *object_allocate_object(size_t size, ObjType type)
{
    Obj *object = (Obj *)memory_reallocate(NULL, 0, size);
    object->type = type;
    object->next = vm.objects;
    vm.objects = object;
    return object;
}

ObjFunction *object_new_function()
{
    ObjFunction *function = OBJECT_ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->name = NULL;
    chunk_init_chunk(&function->chunk);
    return function;
}

static ObjString *object_allocate_string(char *chars, int length, uint32_t hash)
{
    ObjString *string = OBJECT_ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    table_set(&vm.strings, string, VALUE_NULL_VAL);
    return string;
}

static uint32_t object_hash_string(const char *key, int length)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

ObjString *object_take_string(char *chars, int length)
{
    uint32_t hash = object_hash_string(chars, length);
    ObjString *interned = table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL)
    {
        MEMORY_FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return object_allocate_string(chars, length, hash);
}

ObjString *object_copy_string(const char *chars, int length)
{
    uint32_t hash = object_hash_string(chars, length);
    ObjString *interned = table_find_string(&vm.strings, chars, length, hash);
    if (interned != NULL)
        return interned;

    char *heap_chars = MEMORY_ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';
    return object_allocate_string(heap_chars, length, hash);
}

static void object_print_function(ObjFunction *function)
{
    if (function->name == NULL)
    {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void object_print_object(Value value)
{
    switch (OBJECT_OBJ_TYPE(value))
    {
    case OBJ_FUNCTION:
        object_print_function(OBJECT_AS_FUNCTION(value));
        break;
    case OBJ_STRING:
        printf("%s", OBJECT_AS_CSTRING(value));
        break;
    }
}