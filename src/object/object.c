#include <object/object.h>
#include <memory/memory.h>
#include <value/value.h>
#include <vm/vm.h>
#include <table/table.h>

#include <stdio.h>
#include <string.h>

size_t next_id = 1;

#define OBJECT_ALLOCATE_OBJ(type, objectType) \
    (type *)object_allocate_object(sizeof(type), objectType)

static Obj *object_allocate_object(size_t size, ObjType type)
{
    Obj *object = (Obj *)memory_reallocate(NULL, 0, size);
    object->type = type;
    object->is_marked = false;
    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void *)object, size, type);
#endif

    return object;
}

ObjClosure *object_new_closure(ObjFunction *function)
{
    ObjUpvalue **upvalues = MEMORY_ALLOCATE(ObjUpvalue *,
                                            function->upvalue_count);
    for (int i = 0; i < function->upvalue_count; i++)
    {
        upvalues[i] = NULL;
    }

    ObjClosure *closure = OBJECT_ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalue_count = function->upvalue_count;
    return closure;
}

ObjFunction *object_new_script()
{
    ObjFunction *function = OBJECT_ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalue_count = 0;
    function->id = 0;
    chunk_init_chunk(&function->chunk);
    return function;
}

ObjFunction *object_new_function()
{
    ObjFunction *function = object_new_script();
    function->id = next_id++;
    return function;
}

static ObjString *object_allocate_string(char *chars, int length)
{
    ObjString *string = OBJECT_ALLOCATE_OBJ(ObjString, OBJ_STRING);

    string->length = length;
    string->chars = chars;

    // Push then pop the string to the constant table so it doesn't get
    // garbage collected before being interned
    vm_push(VALUE_OBJ_VAL(string));
    int slot = table_declare(&vm.strings, string);
    table_set(&vm.strings, slot, VALUE_NULL_VAL);
    vm_pop();

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
    ObjString *interned = table_find_string(&vm.strings, chars, length);
    if (interned != NULL)
    {
        MEMORY_FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return object_allocate_string(chars, length);
}

ObjString *object_copy_string(const char *chars, int length)
{
    ObjString *interned = table_find_string(&vm.strings, chars, length);
    if (interned != NULL)
        return interned;

    char *heap_chars = MEMORY_ALLOCATE(char, length + 1);
    memcpy(heap_chars, chars, length);
    heap_chars[length] = '\0';
    return object_allocate_string(heap_chars, length);
}

ObjUpvalue *object_new_upvalue(Value *slot)
{
    ObjUpvalue *upvalue = OBJECT_ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->location = slot;
    upvalue->closed = VALUE_NULL_VAL;
    upvalue->next = NULL;
    return upvalue;
}

static void object_print_function(ObjFunction *function)
{
    printf("<fn %u>", (unsigned)function->id);
}

ObjNative *object_new_native(NativeFn function)
{
    ObjNative *native = OBJECT_ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

void object_print_object(Value value)
{
    switch (OBJECT_OBJ_TYPE(value))
    {
    case OBJ_CLOSURE:
        object_print_function(OBJECT_AS_CLOSURE(value)->function);
        break;
    case OBJ_FUNCTION:
        object_print_function(OBJECT_AS_FUNCTION(value));
        break;
    case OBJ_NATIVE:
        printf("<fn prim>");
        break;
    case OBJ_STRING:
        printf("%s", OBJECT_AS_CSTRING(value));
        break;
    case OBJ_UPVALUE:
        printf("upvalue");
        break;
    }
}