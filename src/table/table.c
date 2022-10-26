#include <table/table.h>
#include <memory/memory.h>
#include <object/object.h>
#include <value/value.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void table_init_table(Table *table)
{
    table->count = 0;
}

void table_free_table(Table *table)
{
    table_init_table(table);
}

int table_find_entry(Table *table, const char *chars, int length)
{
    for (int i = 0; i < table->count; i++)
    {
        Entry *entry = &table->entries[i];

        if (entry->key != NULL &&
            entry->key->length == length &&
            memcmp(entry->key->chars, chars, length) == 0)
        {
            return i;
        }
    }

    return -1;
}

Value table_get(Table *table, int slot)
{
    return table->values[slot];
}

void table_set(Table *table, int slot, Value value)
{
    table->values[slot] = value;
}

int table_declare(Table *table, ObjString *key)
{
    int slot = table_find_entry(table, key->chars, key->length);

    if (slot == -1)
    {
        if (table->count + 1 >= UINT16_COUNT)
        {
            printf("Terminal error: Symbol table overflow.\n");
            exit(1);
        }

        slot = table->count;
        table->entries[slot].key = key;
        table->entries[slot].slot = slot;
        table->count++;
    }

    return slot;
}

ObjString *table_find_string(Table *table, const char *chars, int length)
{
    int slot = table_find_entry(table, chars, length);

    if (slot != -1)
        return table->entries[slot].key;

    return NULL;
}

bool table_delete(Table *table, int slot)
{
    if (table->count == 0)
        return false;

    if (table->entries[slot].key == NULL)
        return false;

    table->entries[slot].key = NULL;
    table->entries[slot].slot = -1;
    table->values[slot] = VALUE_BOOL_VAL(true);

    return true;
}

void table_remove_white(Table *table)
{
    for (int i = 0; i < table->count; i++)
    {
        Entry *entry = &table->entries[i];

        if (entry->key != NULL && !entry->key->obj.is_marked)
        {
            table_delete(table, entry->slot);
        }
    }
}

void table_mark_table(Table *table)
{
    for (int i = 0; i < table->count; i++)
    {
        Entry *entry = &table->entries[i];
        memory_mark_object((Obj *)entry->key);
        memory_mark_value(table->values[i]);
    }
}