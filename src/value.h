#ifndef graphiC_value_h
#define graphiC_value_h

#include "common.h"

typedef struct Obj Obj;
typedef struct ObjString ObjString;

typedef enum {
    VAL_BOOL,
    VAL_NULL,
    VAL_NUMBER,
    VAL_OBJ,
} ValueType;

typedef struct Value {
    ValueType type;
    union {
        bool boolean;
        double number;
        Obj* obj;
    } as;
} Value;

typedef struct 
{
    int capacity;
    int count;
    Value* values;
} ValueArray;

//CHECK THE TYPE OF A VALUE
#define IS_BOOL(value)      ((value).type == VAL_BOOL)
#define IS_NULL(value)       ((value).type == VAL_NULL)
#define IS_NUMBER(value)    ((value).type == VAL_NUMBER)
#define IS_OBJ(value)       ((value).type == VAL_OBJ)

//RETURN THE RAW C VALUE GIVEN THE VALUE
#define BOOL_VALUE_TO_C(value)   ((value).as.boolean)
#define NUMBER_VALUE_TO_C(value) ((value).as.number)
#define OBJ_VALUE_TO_C(value)    ((value).as.obj)

//GIVEN RAW C VALUE, ENTER IT INTO THE RELEVANT TYPE 
#define C_TO_BOOL_VALUE(value) ((Value){VAL_BOOL, {.boolean = value}})
#define C_TO_NULL_VALUE ((Value){VAL_NULL, {.number = 0}})
#define C_TO_NUMBER_VALUE(value) ((Value){VAL_NUMBER, {.number = value}})
#define C_TO_OBJ_VALUE(object)      ((Value){VAL_OBJ, {.obj = (Obj*)object}})


bool valuesEqual(Value a, Value b);
void initValueArray(ValueArray* array);
void writeValueArray(ValueArray* array, Value value);
void freeValueArray(ValueArray* array);
void printValue(Value value);




#endif