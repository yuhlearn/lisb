#ifndef _OBJECT_H
#define _OBJECT_H

#include <common/common.h>
#include <value/value.h>
#include <chunk/chunk.h>

#define OBJECT_OBJ_TYPE(value) (VALUE_AS_OBJ(value)->type)

#define OBJECT_IS_CLOSURE(value) isObjType(value, OBJ_CLOSURE)
#define OBJECT_IS_FUNCTION(value) object_is_obj_type(value, OBJ_FUNCTION)
#define OBJECT_IS_NATIVE(value) object_is_obj_type(value, OBJ_NATIVE)
#define OBJECT_IS_STRING(value) object_is_obj_type(value, OBJ_STRING)

#define OBJECT_AS_CLOSURE(value) ((ObjClosure *)VALUE_AS_OBJ(value))
#define OBJECT_AS_FUNCTION(value) ((ObjFunction *)VALUE_AS_OBJ(value))
#define OBJECT_AS_NATIVE(value) (((ObjNative *)VALUE_AS_OBJ(value))->function)
#define OBJECT_AS_STRING(value) ((ObjString *)VALUE_AS_OBJ(value))
#define OBJECT_AS_CSTRING(value) (((ObjString *)VALUE_AS_OBJ(value))->chars)

typedef enum
{
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} ObjType;

struct Obj
{
    ObjType type;
    bool is_marked;
    struct Obj *next;
};

typedef struct
{
    Obj obj;
    int arity;
    int upvalue_count;
    Chunk chunk;
    ObjString *name;
} ObjFunction;

typedef Value (*NativeFn)(int arcg_cout, Value *args);

typedef struct
{
    Obj obj;
    NativeFn function;
} ObjNative;

struct ObjString
{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

typedef struct ObjUpvalue
{
    Obj obj;
    Value *location;
    Value closed;
    struct ObjUpvalue *next;
} ObjUpvalue;

typedef struct
{
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalue_count;
} ObjClosure;

ObjClosure *object_new_closure(ObjFunction *function);
ObjFunction *object_new_function();
ObjNative *object_new_native(NativeFn function);
ObjString *object_take_string(char *chars, int length);
ObjString *object_copy_string(const char *chars, int length);
ObjUpvalue *object_new_upvalue(Value *slot);
void object_print_object(Value value);

static inline bool object_is_obj_type(Value value, ObjType type)
{
    return VALUE_IS_OBJ(value) && VALUE_AS_OBJ(value)->type == type;
}

#endif