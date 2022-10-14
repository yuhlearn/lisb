#ifndef _VM_H
#define _VM_H

#include <chunk/chunk.h>
#include <value/value.h>
#include <table/table.h>
#include <object/object.h>

#define VM_FRAMES_MAX 64
#define VM_STACK_MAX (VM_FRAMES_MAX * UINT8_COUNT)

typedef struct
{
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct
{
    CallFrame frames[VM_FRAMES_MAX];
    int frame_count;

    Value stack[VM_STACK_MAX];
    Value *stack_top;
    Table globals;
    Table strings;
    ObjUpvalue *open_upvalues;

    size_t bytes_allocated;
    size_t next_gc;
    Obj *objects;

    int gray_count;
    int gray_capacity;
    Obj **gray_stack;
} VM;

typedef enum
{
    VM_OK,
    VM_COMPILE_ERROR,
    VM_RUNTIME_ERROR,
} InterpretResult;

extern VM vm;

void vm_init_vm();
void vm_free_vm();
InterpretResult vm_interpret(const char *source);

Value vm_pop();
void vm_push(Value value);

#endif