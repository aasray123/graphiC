#include "common.h"
#include "vm.h"
#include "debug.h"
#include "object.h"
#include "memory.h"
#include "compiler.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

VM vm;

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}


static void runtimeError(const char* format, ...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;

        // -1 because the IP is sitting on the next instruction to be executed.
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", function->chunk.lines[instruction]);

        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        }
        else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

void initVM() {
    resetStack();
    vm.objects = NULL;

    //TODO: GC
    // vm.grayCount = 0;
    // vm.grayCapacity = 0;
    // vm.grayStack = NULL;
    // vm.bytesAllocated = 0;
    // vm.nextGC = 1024 * 1024;
    // vm.nextGC = 2500;

    initTable(&vm.strings);
    initTable(&vm.globals);

    vm.initString = NULL;
    vm.initString = copyString("init", 4);

    vm.drawString = NULL;
    vm.drawString = copyString("draw", 4);

    //TODO: NATIVE FUNCTION SETUP
    // defineNative("clock", clockNative);
}

void freeVM(){
    freeTable(&vm.strings);
    freeTable(&vm.globals);
    vm.initString = NULL;
    freeObjects();
}

void push (Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    return *vm.stackTop;
}

//Look into the stack
static Value peek(int distance){
    return vm.stackTop[-1 - distance];
}

static bool call(ObjFunction* function, int argCount) {
    if (argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.", function->arity, argCount);
        return false;
    }

    if(vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack Overflow.");
        return false;
    }

    CallFrame* frame = &vm.frames[vm.frameCount++];
    frame->function = function;
    frame->ip = function->chunk.code;

    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            //TODO: FUNCTION INSTEAD OF CLOSURE
            // case OBJ_CLOSURE:
            //     return call(AS_FUNCTION(callee), argCount);
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), argCount);
            case OBJ_NATIVE:{
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            default:
                // Non-callable object type.
                break;
            }
    }
    //TODO: FUNCTIONS AND etc..
    runtimeError("Can only call functions.");
    return false;
}

static bool isFalsey(Value value){
    return IS_NULL(value) || (IS_BOOL(value) && !BOOL_VALUE_TO_C(value));
}

static void concatenate(){
    ObjString* b = AS_STRING(peek(0));
    ObjString* a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length+1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pop(); pop();
    push(C_TO_OBJ_VALUE(result));
}

static InterpretResult run() {
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

    #define READ_BYTE() (*frame->ip++)

    #define READ_SHORT() \
        (frame->ip += 2, \
        (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
    
    #define READ_CONSTANT() \
        (frame->function->chunk.constants.values[READ_BYTE()])

    #define READ_STRING() AS_STRING(READ_CONSTANT())

    #define BINARY_OP(valueType, op)\
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {\
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            }\
            double b = NUMBER_VALUE_TO_C(pop()); \
            double a = NUMBER_VALUE_TO_C(pop()); \
            push(valueType(a op b)); \
        } while (false)

    for(;;){
        #ifdef DEBUG_TRACE_EXECUTION
            //Get offset of the instruciton from base of code
            printf("        ");
            for (Value* slot = vm.stack; slot < vm.stackTop; slot++){
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n"); 

            disassembleInstruction(&frame->function->chunk, (int)(frame->ip - frame->function->chunk.code));
        #endif

        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NULL: push(C_TO_NULL_VALUE); break;
            case OP_TRUE: push(C_TO_BOOL_VALUE(true)); break;
            case OP_FALSE: push(C_TO_BOOL_VALUE(false)); break;
            case OP_POP: pop(); break;
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                if(!tableGet(&vm.globals, name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;
            }
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(C_TO_BOOL_VALUE(valuesEqual(a, b)));
                break;
            }
            case OP_GREATER:     BINARY_OP(C_TO_BOOL_VALUE, >); break;
            case OP_LESS:        BINARY_OP(C_TO_BOOL_VALUE, <); break;
            case OP_POST_INCREMENT: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double value = NUMBER_VALUE_TO_C(pop());
                push(C_TO_NUMBER_VALUE(value + 1));
                break;
            }
            case OP_POST_DECREMENT: {
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                double value = NUMBER_VALUE_TO_C(pop());
                push(C_TO_NUMBER_VALUE(value - 1));
                break;
            }
            case OP_ADD:         {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))){
                    concatenate(); 
                }
                else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))){    
                    BINARY_OP(C_TO_NUMBER_VALUE, +);
                }
                else {
                    runtimeError(       
                    "Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SUBTRACT:    BINARY_OP(C_TO_NUMBER_VALUE, -); break;
            case OP_MULTIPLY:    BINARY_OP(C_TO_NUMBER_VALUE, *); break;
            case OP_DIVIDE:      BINARY_OP(C_TO_NUMBER_VALUE, /); break;
            case OP_NOT: push(C_TO_BOOL_VALUE(isFalsey(pop())));  break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(C_TO_NUMBER_VALUE(-NUMBER_VALUE_TO_C(pop())));
                break;
            case OP_PRINT: {
                printValue(pop());
                printf("\n");
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }   
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT(); 
                frame->ip -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();
                if(!callValue(peek(argCount), argCount)){
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
            case OP_RETURN: {
                Value result = pop();

                // closeUpvalues(frame->slots);

                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);

                frame = &vm.frames[vm.frameCount - 1];
                break;
            }
        }
    }
    #undef READ_STRING
    #undef BINARY_OP
    #undef READ_CONSTANT
    #undef READ_SHORT
    #undef READ_BYTE
}

InterpretResult interpret(const char* source) {
    ObjFunction* function = compile(source);
    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    push(C_TO_OBJ_VALUE(function));
    
    call(function, 0);
    // push(C_TO_OBJ_VALUE(closure));
    // callValue(C_TO_OBJ_VALUE(closure), 0);
    InterpretResult result = run();
    if(result != INTERPRET_OK) return result;

    Value drawValue;
    if (tableGet(&vm.globals, vm.drawString, &drawValue)) {
        while(true) {
            vm.stackTop = vm.stack;
            push(drawValue);
            if(!callValue(drawValue, 0)) {
                return INTERPRET_RUNTIME_ERROR;
            }

            result = run();
            if(result != INTERPRET_OK) return result;
        }
    }
    return INTERPRET_OK;
}