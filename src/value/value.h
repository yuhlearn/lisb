#ifndef _VALUE_H
#define _VALUE_H

#include <common/common.h>

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum
{
    VALUE_BOOL,
    VALUE_NULL,
    VALUE_VOID,
    VALUE_NUMBER,
    VALUE_OBJ,
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

#define VALUE_IS_BOOL(value) ((value).type == VALUE_BOOL)
#define VALUE_IS_NULL(value) ((value).type == VALUE_NULL)
#define VALUE_IS_VOID(value) ((value).type == VALUE_VOID)
#define VALUE_IS_NUMBER(value) ((value).type == VALUE_NUMBER)
#define VALUE_IS_OBJ(value) ((value).type == VALUE_OBJ)

#define VALUE_AS_OBJ(value) ((value).as.obj)
#define VALUE_AS_BOOL(value) ((value).as.boolean)
#define VALUE_AS_NUMBER(value) ((value).as.number)

#define VALUE_BOOL_VAL(value) ((Value){VALUE_BOOL, {.boolean = value}})
#define VALUE_NULL_VAL ((Value){VALUE_NULL, {.number = 0}})
#define VALUE_VOID_VAL ((Value){VALUE_VOID, {.number = 0}})
#define VALUE_NUMBER_VAL(value) ((Value){VALUE_NUMBER, {.number = value}})
#define VALUE_OBJ_VAL(object) ((Value){VALUE_OBJ, {.obj = (Obj *)object}})

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
bool value_values_equal(Value a, Value b);

#endif