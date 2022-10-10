#include <primitive/primitive.h>

#include <stdbool.h>
#include <time.h>

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
        // else runtime error!
    }

    return VALUE_NUMBER_VAL(sum);
}

Value primitive_sub(int arg_count, Value *args)
{
    double diff;

    if (arg_count < 1)
    {
        // runtime error!
    }

    if (VALUE_IS_NUMBER(args[0]))
        diff = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    if (arg_count == 1)
        return VALUE_NUMBER_VAL(-diff);

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            diff -= VALUE_AS_NUMBER(args[i]);
        // else runtime error!
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
        // else runtime error!
    }

    return VALUE_NUMBER_VAL(prod);
}

Value primitive_div(int arg_count, Value *args)
{
    double fract = 1;

    if (arg_count < 1)
    {
        // runtime error!
    }

    for (int i = 0; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
            fract /= VALUE_AS_NUMBER(args[i]);
        // else runtime error!
    }

    return VALUE_NUMBER_VAL(fract);
}

Value primitive_num_eq(int arg_count, Value *args)
{
    double prev;

    if (arg_count < 1)
    {
        // runtime error!
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
        // else runtime error!
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_le(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        // runtime error!
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev < current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        // else runtime error!
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_ge(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        // runtime error!
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev > current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        // else runtime error!
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_leq(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        // runtime error!
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev <= current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        // else runtime error!
    }

    return VALUE_BOOL_VAL(true);
}

Value primitive_num_geq(int arg_count, Value *args)
{
    double prev, current;

    if (arg_count < 1)
    {
        // runtime error!
    }

    if (VALUE_IS_NUMBER(args[0]))
        prev = VALUE_AS_NUMBER(args[0]);
    // else runtime error!

    for (int i = 1; i < arg_count; i++)
    {
        if (VALUE_IS_NUMBER(args[i]))
        {
            current = VALUE_AS_NUMBER(args[i]);
            if (!(prev >= current))
                return VALUE_BOOL_VAL(false);
            prev = current;
        }
        // else runtime error!
    }

    return VALUE_BOOL_VAL(true);
}