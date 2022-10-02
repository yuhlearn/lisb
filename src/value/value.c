#include <value/value.h>
#include <memory/memory.h>
#include <object/object.h>

#include <stdio.h>

void value_init_value_array(ValueArray *array)
{
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void value_write_value_array(ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int old_capacity = array->capacity;
        array->capacity = MEMORY_GROW_CAPACITY(old_capacity);
        array->values = MEMORY_GROW_ARRAY(Value, array->values, old_capacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void value_free_value_array(ValueArray *array)
{
    MEMORY_FREE_ARRAY(Value, array->values, array->capacity);
    value_init_value_array(array);
}

void value_print_value(Value value)
{
    switch (value.type)
    {
    case VALUE_BOOL:
        printf(VALUE_AS_BOOL(value) ? "#t" : "#f");
        break;
    case VALUE_NULL:
        printf("()");
        break;
    case VALUE_VOID:
        printf("#<void>");
        break;
    case VALUE_NUMBER:
        printf("%g", VALUE_AS_NUMBER(value));
        break;
    case VALUE_OBJ:
        object_print_object(value);
        break;
    }
}

bool value_values_equal(Value a, Value b)
{
    if (a.type != b.type)
        return false;
    switch (a.type)
    {
    case VALUE_BOOL:
        return VALUE_AS_BOOL(a) == VALUE_AS_BOOL(b);
    case VALUE_NULL:
        return true;
    case VALUE_VOID:
        return true;
    case VALUE_NUMBER:
        return VALUE_AS_NUMBER(a) == VALUE_AS_NUMBER(b);
    default:
        return false; // Unreachable.
    }
}