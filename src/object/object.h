#ifndef _OBJECT_H
#define _OBJECT_H

#include <common/common.h>
#include <value/value.h>
#include <chunk/chunk.h>
#include <scanner/scanner.h>

#define OBJECT_CAR(cons) ((OBJECT_AS_CONS(cons))->car)
#define OBJECT_CDR(cons) ((OBJECT_AS_CONS(cons))->cdr)

#define OBJECT_CAAR(cons) (OBJECT_CAR(OBJECT_CAR(cons)))
#define OBJECT_CADR(cons) (OBJECT_CDR(OBJECT_CAR(cons)))
#define OBJECT_CDAR(cons) (OBJECT_CAR(OBJECT_CDR(cons)))
#define OBJECT_CDDR(cons) (OBJECT_CDR(OBJECT_CDR(cons)))

#define OBJECT_CAAAR(cons) (OBJECT_CAR(OBJECT_CAAR(cons)))
#define OBJECT_CAADR(cons) (OBJECT_CDR(OBJECT_CAAR(cons)))
#define OBJECT_CADAR(cons) (OBJECT_CAR(OBJECT_CADR(cons)))
#define OBJECT_CADDR(cons) (OBJECT_CDR(OBJECT_CADR(cons)))
#define OBJECT_CDAAR(cons) (OBJECT_CAR(OBJECT_CDAR(cons)))
#define OBJECT_CDADR(cons) (OBJECT_CDR(OBJECT_CDAR(cons)))
#define OBJECT_CDDAR(cons) (OBJECT_CAR(OBJECT_CDDR(cons)))
#define OBJECT_CDDDR(cons) (OBJECT_CDR(OBJECT_CDDR(cons)))

#define OBJECT_CAAAAR(cons) (OBJECT_CAAR(OBJECT_CAAR(cons)))
#define OBJECT_CAADAR(cons) (OBJECT_CDAR(OBJECT_CAAR(cons)))
#define OBJECT_CAADDR(cons) (OBJECT_CDDR(OBJECT_CAAR(cons)))
#define OBJECT_CADAAR(cons) (OBJECT_CAAR(OBJECT_CADR(cons)))
#define OBJECT_CADADR(cons) (OBJECT_CADR(OBJECT_CADR(cons)))
#define OBJECT_CADDAR(cons) (OBJECT_CDAR(OBJECT_CADR(cons)))
#define OBJECT_CADDDR(cons) (OBJECT_CDDR(OBJECT_CADR(cons)))
#define OBJECT_CDAAAR(cons) (OBJECT_CAAR(OBJECT_CDAR(cons)))
#define OBJECT_CDAADR(cons) (OBJECT_CADR(OBJECT_CDAR(cons)))
#define OBJECT_CDADAR(cons) (OBJECT_CDAR(OBJECT_CDAR(cons)))
#define OBJECT_CDADDR(cons) (OBJECT_CDDR(OBJECT_CDAR(cons)))
#define OBJECT_CDDAAR(cons) (OBJECT_CAAR(OBJECT_CDDR(cons)))
#define OBJECT_CDDADR(cons) (OBJECT_CADR(OBJECT_CDDR(cons)))
#define OBJECT_CDDDAR(cons) (OBJECT_CDAR(OBJECT_CDDR(cons)))
#define OBJECT_CDDDDR(cons) (OBJECT_CDDR(OBJECT_CDDR(cons)))

#define OBJECT_OBJ_TYPE(value) (VALUE_AS_OBJ(value)->type)

#define OBJECT_IS_CLOSURE(value) object_is_obj_type(value, OBJ_CLOSURE)
#define OBJECT_IS_FUNCTION(value) object_is_obj_type(value, OBJ_FUNCTION)
#define OBJECT_IS_PRIMITIVE(value) object_is_obj_type(value, OBJ_PRIMITIVE)
#define OBJECT_IS_CONTINUATION(value) object_is_obj_type(value, OBJ_CONTINUATION)
#define OBJECT_IS_CONS(value) object_is_obj_type(value, OBJ_CONS)
#define OBJECT_IS_SYMBOL(value) object_is_obj_type(value, OBJ_SYMBOL)
#define OBJECT_IS_STRING(value) object_is_obj_type(value, OBJ_STRING)

#define OBJECT_AS_CLOSURE(value) ((ObjClosure *)VALUE_AS_OBJ(value))
#define OBJECT_AS_FUNCTION(value) ((ObjFunction *)VALUE_AS_OBJ(value))
#define OBJECT_AS_PRIMITIVE(value) (((ObjPrimitive *)VALUE_AS_OBJ(value))->function)
#define OBJECT_AS_CONTINUATION(value) ((ObjContinuation *)VALUE_AS_OBJ(value))
#define OBJECT_AS_CONS(value) ((ObjCons *)VALUE_AS_OBJ(value))
#define OBJECT_AS_SYMBOL(value) ((ObjSymbol *)VALUE_AS_OBJ(value))
#define OBJECT_AS_STRING(value) ((ObjString *)VALUE_AS_OBJ(value))
#define OBJECT_AS_CSTRING(value) (((ObjString *)VALUE_AS_OBJ(value))->chars)

typedef enum
{
    OBJ_CLOSURE,
    OBJ_CONS,
    OBJ_CONTINUATION,
    OBJ_FUNCTION,
    OBJ_PRIMITIVE,
    OBJ_STRING,
    OBJ_SYMBOL,
    OBJ_UPVALUE
} ObjType;

struct Obj
{
    ObjType type;
    bool is_marked;
    struct Obj *next;
};

struct ObjString
{
    Obj obj;
    int length;
    char *chars;
};

typedef struct
{
    Obj obj;
    Value car;
    Value cdr;
} ObjCons;

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
    TokenType token;
    int length;
    char *chars;
    int line;
    int row;
} ObjSymbol;

typedef struct
{
    Obj obj;
    int arity;
    int upvalue_count;
    Chunk chunk;
    size_t id;
} ObjFunction;

typedef Value (*PrimitiveFn)(int arcg_cout, Value *args);

typedef struct
{
    Obj obj;
    PrimitiveFn function;
} ObjPrimitive;

typedef struct
{
    Obj obj;
    ObjFunction *function;
    ObjUpvalue **upvalues;
    int upvalue_count;
} ObjClosure;

// Forward declaration to avoid circular dependancies
typedef struct ObjContinuation ObjContinuation;

ObjClosure *object_new_closure(ObjFunction *function);
ObjContinuation *object_new_continuation();
ObjFunction *object_new_script();
ObjFunction *object_new_function();
ObjPrimitive *object_new_native(PrimitiveFn function);
ObjCons *object_new_cons(Value car, Value cdr);
ObjString *object_take_string(char *chars, int length);
ObjString *object_copy_string(const char *chars, int length);
ObjUpvalue *object_new_upvalue(Value *slot);
void object_load_continuation(ObjContinuation *cont);
void object_mark_continuation(ObjContinuation *cont);
void object_free_continuation(ObjContinuation *cont);
void object_print_object(Value value);

static inline bool object_is_obj_type(Value value, ObjType type)
{
    return VALUE_IS_OBJ(value) && VALUE_AS_OBJ(value)->type == type;
}

#endif