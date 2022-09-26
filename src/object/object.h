#ifndef _OBJECT_H
#define _OBJECT_H

#include <common/common.h>
#include <value/value.h>

#define OBJECT_OBJ_TYPE(value) (VALUE_AS_OBJ(value)->type)

#define OBJECT_IS_STRING(value) object_is_obj_type(value, OBJ_STRING)

#define OBJECT_AS_STRING(value) ((ObjString *)VALUE_AS_OBJ(value))
#define OBJECT_AS_CSTRING(value) (((ObjString *)VALUE_AS_OBJ(value))->chars)

typedef enum
{
    OBJ_STRING,
} ObjType;

struct Obj
{
    ObjType type;
    struct Obj *next;
};

struct ObjString
{
    Obj obj;
    int length;
    char *chars;
    uint32_t hash;
};

ObjString *object_copy_string(const char *chars, int length);
void object_print_object(Value value);

static inline bool object_is_obj_type(Value value, ObjType type)
{
    return VALUE_IS_OBJ(value) && VALUE_AS_OBJ(value)->type == type;
}

#endif