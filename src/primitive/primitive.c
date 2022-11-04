#include <primitive/primitive.h>
#include <vm/vm.h>

#include <stdbool.h>
#include <time.h>
#include <stdio.h>

Value primitive_display(int arg_count, Value *args)
{
    if (arg_count != 1)
    {
        vm_runtime_error("Expected 1 argument but got %d.", arg_count);
        return VALUE_VOID_VAL;
    }

    value_print_value(args[0]);
    return VALUE_VOID_VAL;
}

Value primitive_displayln(int arg_count, Value *args)
{
    Value result = primitive_display(arg_count, args);
    printf("\n");
    return result;
}

Value primitive_clock(int arg_count, Value *args)
{
    return VALUE_NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

Value primitive_add(int arg_count, Value *args)
{
    double sum = 0;

    for (int i = 0; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            sum += VALUE_AS_NUMBER(args[i]);
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_NUMBER_VAL(sum);
}

Value primitive_sub(int arg_count, Value *args)
{
    double diff;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        diff = VALUE_AS_NUMBER(args[0]);
    else
        vm_runtime_error("Expected number at argument position 1.");

    if (arg_count == 1)
        return VALUE_NUMBER_VAL(-diff);

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            diff -= VALUE_AS_NUMBER(args[i]);
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_NUMBER_VAL(diff);
}

Value primitive_mup(int arg_count, Value *args)
{
    double prod = 1;

    for (int i = 0; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            prod *= VALUE_AS_NUMBER(args[i]);
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_NUMBER_VAL(prod);
}

Value primitive_div(int arg_count, Value *args)
{
    double fract = 1;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    for (int i = 0; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            fract /= VALUE_AS_NUMBER(args[i]);
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_NUMBER_VAL(fract);
}

Value primitive_num_eq(int arg_count, Value *args)
{
    double prev;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            if (prev != VALUE_AS_NUMBER(args[i]))
                return VALUE_BOOL_VAL(false);
        }
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_le(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    else
        vm_runtime_error("Expected number at argument position 1.");

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev < current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_ge(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    else
        vm_runtime_error("Expected number at argument position 1.");

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev > current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_leq(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    else
        vm_runtime_error("Expected number.");

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev <= current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_geq(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        vm_runtime_error("Expected at least 1 argument, got %d.", arg_count);
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    else
        vm_runtime_error("Expected number at argument position 1.");

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev >= current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        else
            vm_runtime_error("Expected number, at argument position %d.", i + 1);
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_car(int arg_count, Value *args)
{
    if (arg_count != 1)
        vm_runtime_error("Expected 1 argument, got %d.", arg_count);

    else if (!OBJECT_IS_CONS(args[0]))
        vm_runtime_error("Expected cons, at argument position %d.", 1);

    return OBJECT_CAR(args[0]);
}

Value primitive_cdr(int arg_count, Value *args)
{
    if (arg_count != 1)
        vm_runtime_error("Expected 1 argument, got %d.", arg_count);

    else if (!OBJECT_IS_CONS(args[0]))
        vm_runtime_error("Expected cons, at argument position %d.", 1);

    return OBJECT_CDR(args[0]);
}

Value primitive_cons(int arg_count, Value *args)
{
    if (arg_count != 2)
        vm_runtime_error("Expected at 2 arguments, got %d.", arg_count);

    return VALUE_OBJ_VAL(object_new_cons(args[0], args[1]));
}

Value primitive_list(int arg_count, Value *args)
{
    Value last = VALUE_NULL_VAL;
    vm_push(last);

    for (int i = arg_count - 1; i >= 0; i--)
    {
        ObjCons *current = object_new_cons(args[i], last);
        last = VALUE_OBJ_VAL(current);

        vm_pop();
        vm_push(last);
    }
    vm_pop();

    return last;
}

Value primitive_append(int arg_count, Value *args)
{
    Value head = VALUE_NULL_VAL;

    if (arg_count > 0)
    {
        Value tail = args[arg_count - 1];
        Value last = VALUE_NULL_VAL;

        for (int i = 0; i < arg_count - 1; i++)
        {
            Value tmp = args[i];

            for (; OBJECT_IS_CONS(tmp); tmp = OBJECT_CDR(last))
            {
                tmp = VALUE_OBJ_VAL(object_new_cons(OBJECT_CAR(tmp), OBJECT_CDR(tmp)));

                if (VALUE_IS_NULL(last))
                    vm_push(head = tmp);
                else
                    OBJECT_CDR(last) = tmp;
                last = tmp;
            }

            if (!VALUE_IS_NULL(tmp))
                vm_runtime_error("Expected list, at argument position %d.", i + 1);
        }

        if (VALUE_IS_NULL(last))
            return tail;

        OBJECT_CDR(last) = tail;
        vm_pop();
    }

    return head;
}