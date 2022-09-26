#ifndef _VALUE_H
#define _VALUE_H

#include <common/common.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum
{
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ
} ValueType;

typedef struct
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj *obj;
    } as;
} Value;

#define VALUE_IS_BOOL(value) ((value).type == VAL_BOOL)
#define VALUE_IS_NULL(value) ((value).type == VAL_NULL)
#define VALUE_IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define VALUE_IS_OBJ(value) ((value).type == VAL_OBJ)

#define VALUE_AS_OBJ(value) ((value).as.obj)
#define VALUE_AS_BOOL(value) ((value).as.boolean)
#define VALUE_AS_NUMBER(value) ((value).as.number)

#define VALUE_BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define VALUE_NULL_VAL ((Value){VAL_NULL, {.number = 0}})
#define VALUE_NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define VALUE_OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj *)object}})

typedef struct
{
    int capacity;
    int count;
    Value *values;
} ValueArray;

void value_init_value_array(ValueArray *array);
void value_write_value_array(ValueArray *array, Value value);
void value_free_value_array(ValueArray *array);
void value_print_value(Value value);

#endif