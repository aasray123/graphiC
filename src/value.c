#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "object.h"

void initValueArray(ValueArray* array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void writeValueArray(ValueArray* array, Value value) {
    if (array->capacity < array->count + 1) {
        int oldCapacity = array->capacity;
        array->capacity = GROW_CAPACITY(oldCapacity);
        array->values = GROW_ARRAY(Value, array->values, 
                        oldCapacity, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}

void freeValueArray(ValueArray* array){
    FREE_ARRAY(Value, array->values, array->capacity);
    initValueArray(array);
}

void printValue(Value value){
    switch(value.type) {
        case VAL_BOOL:
            printf(BOOL_VALUE_TO_C(value) ? "true" : "false");
            break; 
        case VAL_NULL: printf("null"); break;
        case VAL_NUMBER: printf("%g", NUMBER_VALUE_TO_C(value)); break;
        case VAL_OBJ: printObject(value); break;
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_BOOL: return BOOL_VALUE_TO_C(a) == BOOL_VALUE_TO_C(b);
        case VAL_NIL: return true; // Both are nil
        case VAL_NUMBER: return NUMBER_VALUE_TO_C(a) == NUMBER_VALUE_TO_C(b);
        case VAL_OBJ: return OBJ_VALUE_TO_C(a) == OBJ_VALUE_TO_C(b);
        default: return false; // Unreachable, but keeps the compiler happy.
    }
}