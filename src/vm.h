#ifndef graphiC_vm_h
#define graphiC_vm_h


#include "chunk.h"
#include "table.h"
#include "value.h"
#include "object.h"
#include "natives.h"
#include "raylib.h"
#include <time.h>

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjFunction* function;
    uint8_t* ip;
    Value* slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frameCount;

    Value stack[STACK_MAX];
    Value* stackTop;
    Table strings;
    ObjString* initString;
    ObjString* drawString;
    Table globals;

    Obj* objects;

    size_t bytesAllocated;
    size_t nextGC;

    //GC BENCHMARKING TIME STATS
    time_t totalMinorTime;
    time_t totalMajorTime;
        
    int grayCount;
    int grayCapacity;
    Obj** grayStack;

    //OLD objects
    bool isMajor;
    bool freeingTenured;
    RememberedSet remSet;
    Obj* tenureObjects;

    size_t bytesAllocatedTenure;
    size_t nextGCTenure;

    //VECTOR STUFF
    ObjString* strVector2;
    ObjString* strX;
    ObjString* strY;
    ObjEntity* vector2Entity;
    /*
    
    */
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);

void push(Value value);
Value pop();


#endif