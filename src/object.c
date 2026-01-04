#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"

#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType)\
    (type*)allocateObject(sizeof(type), objectType) 

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    object->type = type;

    object->isMarked = false;
    object->isTenured = false;
    object->isQueued = false;
    object->next = vm.objects;
    vm.objects = object;

    #ifdef DEBUG_LOG_GC
    printf("%p allocate %zu for %d\n", (void*)object, size, type);
    #endif


    return object;
}


static ObjString* allocateString(char* chars, int length, uint32_t hash){
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;

    push(C_TO_OBJ_VALUE(string));
    tableSet(&vm.strings, string, C_TO_NULL_VALUE);
    pop();
    
    return string;
}

ObjFunction* newFunction() {
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative* newNative(NativeFn function) {
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

//TODO: Change the hashing algorithm here 
static uint32_t hashString(const char* key, int length){
    uint32_t hash = 2166136261u;

    for(int i = 0; i < length; i++){
        hash ^= key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString* takeString(char* chars, int length){
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if(interned != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length){
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);
    if (interned != NULL) return interned;

    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

static void printFunction(ObjFunction* function) {
    if (function->name == NULL) {
        printf("<script>");
        return;
    }
    printf("<fn %s>", function->name->chars);
}

void printObject(Value value){
    switch (OBJ_TYPE(value)){
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

void appendRememberedSet(RememberedSet* set, Obj* object) {
    if (set->count + 1 > set->capacity) {
        int oldCapacity = set->capacity;
        set->capacity = GROW_CAPACITY(oldCapacity);
        set->objects = GROW_ARRAY(Obj*, set->objects, oldCapacity, set->capacity);
    }
    object->isQueued = true;
    set->objects[set->count] = object;
    set->count++;
}

void promoteObject(Obj* object, Obj* previous) {
    if (object->isTenured) {
        error("Object is already tenured.");
        return;
    }
    object->isTenured = true;

    if(previous != NULL){
        previous->next = object->next;
    }
    else {
        vm.objects = object->next;
    }

    object->next = vm.tenureObjects;
    vm.tenureObjects = object;
    
    bool pointsToYoung = false;
    switch(object->type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = AS_FUNCTION(C_TO_OBJ_VALUE(object));
            //Name is ObjString*
            if (!function->name->obj.isTenured) {
                pointsToYoung = true;
            }
            for (int i = 0 ; i < function->chunk.constants.count; i++) {
                Value constant = function->chunk.constants.values[i];
                if (IS_OBJ(constant)) {
                    Obj* constObj = OBJ_VALUE_TO_C(constant);
                    if (!constObj->isTenured) {
                        pointsToYoung = true;
                        break;
                    }
                }
            }
        }
    }

    if (pointsToYoung && !object->isQueued) {
        appendRememberedSet(&vm.remSet, object);
    }

}
