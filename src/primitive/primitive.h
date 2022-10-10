#ifndef _PRIMITIVE_H
#define _PRIMITIVE_H

#include <value/value.h>

Value primitive_clock(int arg_count, Value *args);

Value primitive_add(int arg_count, Value *args);
Value primitive_sub(int arg_count, Value *args);
Value primitive_mup(int arg_count, Value *args);
Value primitive_div(int arg_count, Value *args);

Value primitive_num_eq(int arg_count, Value *args);
Value primitive_num_le(int arg_count, Value *args);
Value primitive_num_ge(int arg_count, Value *args);
Value primitive_num_leq(int arg_count, Value *args);
Value primitive_num_geq(int arg_count, Value *args);

#endif