#include <memory/memory.h>
#include <vm/vm.h>
#include <compiler/compiler.h>

#include <stdlib.h>

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include <debug/debug.h>
#endif

#define GC_HEAP_GROW_FACTOR 2

Memory memory;

static void memory_mark_roots(VM *vm);

void *memory_reallocate(void *pointer, size_t old_size, size_t new_size)
{
    memory.bytes_allocated += new_size - old_size;

    if (new_size > old_size)
    {
#ifdef DEBUG_STRESS_GC
        memory_collect_garbage();
#endif

        if (memory.bytes_allocated > memory.next_gc)
        {
            memory_collect_garbage();
        }
    }

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

void memory_mark_object(Obj *object)
{
    if (object == NULL)
        return;

    if (object->is_marked)
        return;

#ifdef DEBUG_LOG_GC
    printf("%p mark ", (void *)object);
    value_print_value(VALUE_OBJ_VAL(object));
    printf("\n");
#endif

    object->is_marked = true;

    if (memory.gray_capacity < memory.gray_count + 1)
    {
        memory.gray_capacity = MEMORY_GROW_CAPACITY(memory.gray_capacity);
        memory.gray_stack = (Obj **)realloc(memory.gray_stack, sizeof(Obj *) * memory.gray_capacity);

        if (memory.gray_stack == NULL)
            exit(1);
    }

    memory.gray_stack[memory.gray_count++] = object;

    if (object->type == OBJ_CONTINUATION)
    {
        ObjContinuation *cont = (ObjContinuation *)object;
        object_mark_continuation(cont);
    }
}

void memory_mark_value(Value value)
{
    if (VALUE_IS_OBJ(value))
        memory_mark_object(VALUE_AS_OBJ(value));
}

static void memory_mark_array(ValueArray *array)
{
    for (int i = 0; i < array->count; i++)
    {
        memory_mark_value(array->values[i]);
    }
}

static void memory_blacken_object(Obj *object)
{
#ifdef DEBUG_LOG_GC
    printf("%p blacken ", (void *)object);
    value_print_value(VALUE_OBJ_VAL(object));
    printf("\n");
#endif

    switch (object->type)
    {
    case OBJ_CLOSURE:
    {
        ObjClosure *closure = (ObjClosure *)object;
        memory_mark_object((Obj *)closure->function);
        for (int i = 0; i < closure->upvalue_count; i++)
        {
            memory_mark_object((Obj *)closure->upvalues[i]);
        }
        break;
    }
    case OBJ_CONTINUATION:
    {
        ObjContinuation *cont = (ObjContinuation *)object;
        object_mark_continuation(cont);
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        memory_mark_array(&function->chunk.constants);
        break;
    }
    case OBJ_UPVALUE:
        memory_mark_value(((ObjUpvalue *)object)->closed);
        break;
    case OBJ_NATIVE:
    case OBJ_STRING:
        break;
    }
}

static void memory_free_object(Obj *object)
{
#ifdef DEBUG_LOG_GC
    printf("%p free type %d\n", (void *)object, object->type);

    // printf("%p free ", (void *)object);
    // value_print_value(VALUE_OBJ_VAL(object));
    // printf("\n");
#endif

    switch (object->type)
    {
    case OBJ_CLOSURE:
    {
        ObjClosure *closure = (ObjClosure *)object;
        MEMORY_FREE_ARRAY(ObjUpvalue *, closure->upvalues, closure->upvalue_count);
        MEMORY_FREE(ObjClosure, object);
        break;
    }
    case OBJ_CONTINUATION:
    {
        ObjContinuation *cont = (ObjContinuation *)object;
        object_free_continuation(cont);
        break;
    }
    case OBJ_FUNCTION:
    {
        ObjFunction *function = (ObjFunction *)object;
        chunk_free_chunk(&function->chunk);
        MEMORY_FREE(ObjFunction, object);
        break;
    }
    case OBJ_NATIVE:
        MEMORY_FREE(ObjNative, object);
        break;
    case OBJ_STRING:
    {
        ObjString *string = (ObjString *)object;
        MEMORY_FREE_ARRAY(char, string->chars, string->length + 1);
        MEMORY_FREE(ObjString, object);
        break;
    }
    case OBJ_UPVALUE:
        MEMORY_FREE(ObjUpvalue, object);
        break;
    }
}

static void memory_mark_roots(VM *vm)
{
    for (Value *slot = vm->stack; slot < vm->stack_top; slot++)
    {
        memory_mark_value(*slot);
    }

    for (int i = 0; i < vm->frame_count; i++)
    {
        memory_mark_object((Obj *)vm->call_frames[i].closure);
    }

    for (ObjUpvalue *upvalue = vm->open_upvalues; upvalue != NULL; upvalue = upvalue->next)
    {
        memory_mark_object((Obj *)upvalue);
    }

    table_mark_table(&vm->globals);
}

static void memory_trace_references()
{
    while (memory.gray_count > 0)
    {
        Obj *object = memory.gray_stack[--memory.gray_count];
        memory_blacken_object(object);
    }
}

static void memory_sweep()
{
    Obj *previous = NULL;
    Obj *object = memory.objects;
    while (object != NULL)
    {
        if (object->is_marked)
        {
            object->is_marked = false;
            previous = object;
            object = object->next;
        }
        else
        {
            Obj *unreached = object;
            object = object->next;

            if (previous != NULL)
            {
                previous->next = object;
            }
            else
            {
                memory.objects = object;
            }

            memory_free_object(unreached);
        }
    }
}

void memory_collect_garbage()
{
#ifdef DEBUG_LOG_GC
    printf("-- gc begin\n");
    size_t before = memory.bytes_allocated;
#endif

    memory_mark_roots(&vm);
    compiler_mark_compiler_roots();
    memory_trace_references();
    table_remove_white(&vm.strings);
    memory_sweep();

    memory.next_gc = memory.bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
    printf("-- gc end\n");
    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
           before - memory.bytes_allocated,
           before, memory.bytes_allocated,
           memory.next_gc);
#endif
}

void memory_free_objects()
{
    Obj *object = memory.objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        memory_free_object(object);
        object = next;
    }

    free(memory.gray_stack);
}

void memory_init_memory()
{
    memory.objects = NULL;

    memory.gray_count = 0;
    memory.gray_capacity = 0;
    memory.gray_stack = NULL;

    memory.bytes_allocated = 0;
    memory.next_gc = 1024 * 1024;
}