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
    ObjFunction *function;
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
    Obj *objects;
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

#endif