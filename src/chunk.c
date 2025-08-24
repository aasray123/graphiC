#include "chunk.h"
#include "memory.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>


void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk* chunk){
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value){
    push(value);
    writeValueArray(&chunk->constants, value);
    pop(value);
    return chunk->constants.count - 1;
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  // 1. Check if the array has enough capacity.
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
  }

  // 2. Add the new byte and line number to the end of the arrays.
  chunk->code[chunk->count] = byte;
  chunk->lines[chunk->count] = line;
  chunk->count++;
}

int writeConstant(Chunk* chunk, Value value, int line){
    int constantIndex = addConstant(chunk, value);
    if(constantIndex < 256){
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, constantIndex, line);
    }
    else{
        // writeChunk(chunk, OP_CONSTANT_LONG, line);
        // for (int i = 0; i < 3; i++){
        //     writeChunk(chunk, (uint8_t)((constantIndex >> (2-i)*8) & 0xff), line);
        // }
        //TODO: ADD CONSTANT_LONG
        printf("\n\n\n\nHELPPPPPPPPP\n\n\n\n");
    }
    return (int)chunk->constants.count - 1;
}