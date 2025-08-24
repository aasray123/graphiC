#include <stdlib.h>
#include "memory.h"
#include "vm.h"
#include "compiler.h"
#include "object.h"

#define GC_HEAP_GROW_FACTOR 2


void* reallocate(void* pointer, size_t oldSize, size_t newSize){
    //TODO: GC
    // vm.bytesAllocated += newSize - oldSize;

    // if (newSize > oldSize) {
    //     #ifdef DEBUG_STRESS_GC
    //     collectGarbage();
    //     #endif
    //     if (vm.bytesAllocated > vm.nextGC) {
    //         collectGarbage();
    //     }
    // }

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

    // free(vm.grayStack);
}