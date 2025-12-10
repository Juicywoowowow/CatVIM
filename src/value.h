#ifndef camel_value_h
#define camel_value_h

#include "common.h"

typedef enum {
    VAL_NIL,
    VAL_BOOL,
    VAL_I8,
    VAL_I16,
    VAL_I32,
    VAL_I64,
    VAL_U8,
    VAL_U16,
    VAL_U32,
    VAL_U64,
    VAL_F32,
    VAL_F64,
    VAL_STRING,
} ValueType;

typedef struct {
    ValueType type;
    union {
        bool boolean;
        int8_t i8;
        int16_t i16;
        int32_t i32;
        int64_t i64;
        uint8_t u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float f32;
        double f64;
        char* string;
    } as;
} Value;

// Value constructors
#define NIL_VAL           ((Value){VAL_NIL, {.i64 = 0}})
#define BOOL_VAL(value)   ((Value){VAL_BOOL, {.boolean = value}})
#define I64_VAL(value)    ((Value){VAL_I64, {.i64 = value}})
#define I32_VAL(value)    ((Value){VAL_I32, {.i32 = value}})
#define F64_VAL(value)    ((Value){VAL_F64, {.f64 = value}})
#define F32_VAL(value)    ((Value){VAL_F32, {.f32 = value}})
#define STRING_VAL(value) ((Value){VAL_STRING, {.string = value}})

// Value type checks
#define IS_NIL(value)    ((value).type == VAL_NIL)
#define IS_BOOL(value)   ((value).type == VAL_BOOL)
#define IS_I64(value)    ((value).type == VAL_I64)
#define IS_I32(value)    ((value).type == VAL_I32)
#define IS_F64(value)    ((value).type == VAL_F64)
#define IS_F32(value)    ((value).type == VAL_F32)
#define IS_STRING(value) ((value).type == VAL_STRING)

// Value extractors
#define AS_BOOL(value)   ((value).as.boolean)
#define AS_I64(value)    ((value).as.i64)
#define AS_I32(value)    ((value).as.i32)
#define AS_F64(value)    ((value).as.f64)
#define AS_F32(value)    ((value).as.f32)
#define AS_STRING(value) ((value).as.string)

// Dynamic array for constants
typedef struct {
    int count;
    int capacity;
    Value* values;
} ValueArray;

void init_value_array(ValueArray* array);
void write_value_array(ValueArray* array, Value value);
void free_value_array(ValueArray* array);
void print_value(Value value);
bool values_equal(Value a, Value b);

#endif
