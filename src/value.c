#include "value.h"
#include "memory.h"

void init_value_array(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void write_value_array(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, oldCapacity, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void free_value_array(ValueArray* array) {
    FREE_ARRAY(Value, array->values, array->capacity);
    init_value_array(array);
}

void print_value(Value value) {
    switch (value.type) {
        case VAL_NIL:    printf("nil"); break;
        case VAL_BOOL:   printf(AS_BOOL(value) ? "true" : "false"); break;
        case VAL_I8:     printf("%d", value.as.i8); break;
        case VAL_I16:    printf("%d", value.as.i16); break;
        case VAL_I32:    printf("%d", value.as.i32); break;
        case VAL_I64:    printf("%lld", (long long)value.as.i64); break;
        case VAL_U8:     printf("%u", value.as.u8); break;
        case VAL_U16:    printf("%u", value.as.u16); break;
        case VAL_U32:    printf("%u", value.as.u32); break;
        case VAL_U64:    printf("%llu", (unsigned long long)value.as.u64); break;
        case VAL_F32:    printf("%g", value.as.f32); break;
        case VAL_F64:    printf("%g", value.as.f64); break;
        case VAL_STRING: printf("%s", AS_STRING(value)); break;
    }
}

bool values_equal(Value a, Value b) {
    if (a.type != b.type) return false;
    
    switch (a.type) {
        case VAL_NIL:    return true;
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_I8:     return a.as.i8 == b.as.i8;
        case VAL_I16:    return a.as.i16 == b.as.i16;
        case VAL_I32:    return a.as.i32 == b.as.i32;
        case VAL_I64:    return AS_I64(a) == AS_I64(b);
        case VAL_U8:     return a.as.u8 == b.as.u8;
        case VAL_U16:    return a.as.u16 == b.as.u16;
        case VAL_U32:    return a.as.u32 == b.as.u32;
        case VAL_U64:    return a.as.u64 == b.as.u64;
        case VAL_F32:    return AS_F32(a) == AS_F32(b);
        case VAL_F64:    return AS_F64(a) == AS_F64(b);
        case VAL_STRING: return strcmp(AS_STRING(a), AS_STRING(b)) == 0;
        default:         return false;
    }
}
