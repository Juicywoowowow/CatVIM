#ifndef camel_table_h
#define camel_table_h

#include "common.h"
#include "value.h"

typedef struct {
    char* key;
    Value value;
} Entry;

typedef struct {
    int count;
    int capacity;
    Entry* entries;
} Table;

void init_table(Table* table);
void free_table(Table* table);
bool table_get(Table* table, const char* key, Value* value);
bool table_set(Table* table, const char* key, Value value);
bool table_delete(Table* table, const char* key);

#endif
