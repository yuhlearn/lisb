#ifndef _TABLE_H
#define _TABLE_H

#include <common/common.h>
#include <value/value.h>

typedef struct
{
    ObjString *key;
    int slot;
} Entry;

typedef struct
{
    int count;
    Entry entries[UINT16_COUNT];
    Value values[UINT16_COUNT];
} Table;

void table_init_table(Table *table);
void table_free_table(Table *table);
int table_find_entry(Table *table, const char *chars, int length);
int table_declare(Table *table, ObjString *key);
Value table_get(Table *table, int slot);
void table_set(Table *table, int slot, Value value);
void table_remove_white(Table *table);
void table_mark_table(Table *table);
ObjString *table_find_string(Table *table, const char *chars, int length);

#endif