#include <stdlib.h>
#include "memory.h"
#include "vm.h"
#include "compiler.h"
#include "object.h"

#define GC_HEAP_GROW_FACTOR 2

#ifdef DEBUG_LOG_GC
#include <stdio.h>
#include "debug.h"
#endif

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    vm.bytesAllocated += newSize - oldSize;

    if (newSize > oldSize) {
        #ifdef DEBUG_STRESS_GC
        collectGarbage(true);
        #endif

        if(vm.bytesAllocatedTenure > vm.nextGCTenure){
            collectGarbage(true);
        }
        else if (vm.bytesAllocated > vm.nextGC) {
            collectGarbage(false);
        }
        
    }

    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);

    return result;
}

static void freeObject(Obj* object){
    #ifdef DEBUG_LOG_GC
        printf("%p free type %d\n", (void*)object, object->type);
    #endif
    switch (object->type){
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, object);
            break;
        }
        case OBJ_NATIVE: {
            FREE(ObjNative, object);
            break;
        }
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE_ARRAY(char, string->chars, string->length + 1);
            FREE(ObjString, object);
            break;
        }
    }
}

void freeObjects() {
    Obj* object = vm.objects;
    while (object != NULL) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }

    free(vm.grayStack);
}



void markObject(Obj* object) {
    if (object == NULL) return;
    if (object->isMarked) return;
    #ifdef DEBUG_LOG_GC
    printf("%p mark ", (void*)object);
    printValue(C_TO_OBJ_VALUE(object));
    printf("\n");
    #endif

    object->isMarked = true;
    if (vm.grayCapacity < vm.grayCount + 1) {
        vm.grayCapacity = GROW_CAPACITY(vm.grayCapacity);
        vm.grayStack = (Obj**)realloc(vm.grayStack,
                                    sizeof(Obj*) * vm.grayCapacity);
        if (vm.grayStack == NULL) exit(1);

    }
    vm.grayStack[vm.grayCount++] = object;
}


void markValue(Value value) {
    if (IS_OBJ(value)) markObject(OBJ_VALUE_TO_C(value));
}


static void markArray(ValueArray* array) {
    for (int i = 0; i < array->count; i++) {
        markValue(array->values[i]);
    }
}

static void markRoots(bool isMajor) {
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
        markValue(*slot);
    }

    for (int i = 0; i < vm.frameCount; i++) {
        markObject((Obj*)vm.frames[i].function);
    }

    if(!isMajor){
        for (int i = 0; i < vm.remSet.count; i++) {
            blackenObject((Obj*)vm.remSet.objects[i]);
        }
    }

    markTable(&vm.globals);
    markCompilerRoots();
    markObject((Obj*)vm.initString);

}

static void blackenObject(Obj* object) {
    #ifdef DEBUG_LOG_GC
        printf("%p blacken ", (void*)object);
        printValue(C_TO_OBJ_VALUE(object));
        printf("\n");
    #endif
    switch (object->type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            markObject((Obj*)function->name);
            markArray(&function->chunk.constants);
            break;
        }
        case OBJ_NATIVE:
        case OBJ_STRING:
        break;
    }
}

static void traceReferences() {
    while (vm.grayCount > 0) {
        Obj* object = vm.grayStack[--vm.grayCount];
        blackenObject(object);
    }
}

bool remSetChecker(Obj* object);

static void sweep(bool isMajor) {
    Obj** cursor = &vm.objects;

    while (*cursor != NULL) {
        Obj* object = *cursor;
        if (object->isMarked) {

            object->isMarked = false;
            cursor* = object->next;
            promoteObject(object);
            
        } 
        else {
            cursor* = object->next;
            freeObject(object);
        }
    }

    for (int i = 0; i < vm.remSet.count; i++) {
        vm.remSet.objects[i]->isQueued = false;
    }
    vm.remSet.count = 0;

    if (isMajor) {
        cursor = &vm.tenureObjects;
        while (*cursor != NULL) {
            Obj* object = *cursor;
            if (object->isMarked) {
                object->isMarked = false;
                if(remSetChecker(object)){
                    appendRememberedSet(&vm.remSet, object);
                }
                cursor = &object->next;
            } else {
                *cursor = object->next;
                freeObject(object);
            }
        }
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

void promoteObject(Obj* object) {
    if (object->isTenured) {
        error("Object is already tenured.");
        return;
    }
    object->isTenured = true;
    object->next = vm.tenureObjects;
    vm.tenureObjects = object;
    
    bool pointsToYoung = remSetChecker(object);
    

    if (pointsToYoung && !object->isQueued) {
        appendRememberedSet(&vm.remSet, object);
    }

}

bool remSetChecker(Obj* object){
    switch(object->type) {
        case OBJ_FUNCTION: {
            ObjFunction* function = AS_FUNCTION(C_TO_OBJ_VALUE(object));
            //Name is ObjString*
            if (!function->name->obj.isTenured) {
                return true;
            }
            for (int i = 0 ; i < function->chunk.constants.count; i++) {
                Value constant = function->chunk.constants.values[i];
                if (IS_OBJ(constant)) {
                    Obj* constObj = OBJ_VALUE_TO_C(constant);
                    if (!constObj->isTenured) {
                        return true;
                        
                    }
                }
            }
        }
        default:
            return false;
    }
}

void writeBarrier(Obj* source, Value value){
    if(!IS_OBJ(value)) return;
    Obj* target = OBJ_VALUE_TO_C(value);

    if(source->isTenured && !target->isTenured && !source->isQueued){
        source->isQueued = true;
        appendRememberedSet(&vm.remSet, source);
    }

}

void collectGarbage(bool isMajor) {
    #ifdef DEBUG_LOG_GC
    printf("-- gc begin (%s)\n", isMajor ? "major" : "minor");
    size_t before = vm.bytesAllocated;

    #endif

    markRoots(isMajor);
    traceReferences();
    tableRemoveWhite(&vm.strings);
    sweep(isMajor);

    vm.nextGC = vm.bytesAllocated * GC_HEAP_GROW_FACTOR;
    #ifdef DEBUG_LOG_GC
    printf("-- gc end\n");

    printf("   collected %zu bytes (from %zu to %zu) next at %zu\n",
            before - vm.bytesAllocated, before, vm.bytesAllocated,
            vm.nextGC);

    #endif
}


