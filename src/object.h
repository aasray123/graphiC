#ifndef graphiC_object_h
#define graphiC_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

#define OBJ_TYPE(value)     (OBJ_VALUE_TO_C(value) -> type)

#define IS_ENTITY(value)        isObjType(value, OBJ_ENTITY)
#define IS_INSTANCE(value)    isObjType(value, OBJ_INSTANCE)
#define IS_FUNCTION(value)      isObjType(value, OBJ_FUNCTION)
#define IS_NATIVE(value)        isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)        isObjType(value, OBJ_STRING)

#define AS_ENTITY(value)        ((ObjEntity*)OBJ_VALUE_TO_C(value))
#define AS_INSTANCE(value)      ((ObjInstance*)OBJ_VALUE_TO_C(value))
#define AS_FUNCTION(value)      ((ObjFunction*)OBJ_VALUE_TO_C(value))
#define AS_NATIVE(value)        (((ObjNative*)OBJ_VALUE_TO_C(value))->function)
#define AS_STRING(value)        ((ObjString*)OBJ_VALUE_TO_C(value))
//THE POINT is to have the char* for things like printf("%s", AS_CSTRING(val))
#define AS_CSTRING(value)       (((ObjString*)OBJ_VALUE_TO_C(value))->chars)

typedef enum {
    OBJ_ENTITY,
    OBJ_INSTANCE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
} ObjType;

struct Obj {
    bool isMarked;
    bool isTenured;
    bool isQueued;
    ObjType type;
    struct Obj* next;
};

typedef struct {
    Obj obj;
    int arity;
    Chunk chunk;
    ObjString* name;
} ObjFunction;

typedef struct {
    Obj obj;
    ObjString* name;
} ObjEntity;

typedef struct {
    Obj obj;
    ObjEntity* entity;
    Table fields;
} ObjInstance;

//A fn pointer to a nativefn that returns a Value
typedef Value (*NativeFn)(int argCount, Value* args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

typedef struct {
    int capacity;
    int count;
    Obj** objects;
} RememberedSet;

ObjEntity* newEntity(ObjString* name);
ObjInstance* newInstance(ObjEntity* entity);
ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);
ObjString* takeString(char* chars, int length);

// size_t sizeOfObject(Obj* object);

void printObject(Value value);
ObjString* copyString(const char* chars, int length);

static inline bool isObjType(Value value, ObjType type){
    return IS_OBJ(value) && OBJ_VALUE_TO_C(value)->type == type;
}

#endif