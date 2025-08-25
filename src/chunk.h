#ifndef graphiC_chunk_h
#define graphiC_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_RETURN, 
    OP_NEGATE, 
    OP_PRINT,
    OP_JUMP, 
    OP_JUMP_IF_FALSE, 
    OP_LOOP, 
    OP_CALL, 
    OP_POST_INCREMENT,
    OP_POST_DECREMENT,
    OP_ADD, 
    OP_SUBTRACT, 
    OP_MULTIPLY, 
    OP_DIVIDE, 
    OP_NOT, 
    OP_CONSTANT, //1 byte for operand 
    OP_NULL, 
    OP_TRUE, 
    OP_FALSE, 
    OP_POP, 
    OP_GET_LOCAL, 
    OP_GET_GLOBAL, 
    OP_DEFINE_GLOBAL, 
    OP_SET_GLOBAL, 
    OP_SET_LOCAL, 
    OP_EQUAL,   
    OP_GREATER, 
    OP_LESS, 
}OpCode;
/*
    1. Allocate a new array with more capacity.
    2. Copy the existing elements from the old array to the new one.
    3. Store the new capacity.
    4. Delete the old array.
    5. Update code to point to the new array.
    6. Store the element in the new array now that there is room.
    7. Update the count
*/

/*
The code holds the Operation code like OP_RETURN, the constants hold the values
When doing OP_CONSTANT it will show that then the index in the array that that value is in
*/

typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    int* lines;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);
int writeConstant(Chunk* chunk, Value value, int line);

#endif