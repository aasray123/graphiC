#ifndef graphiC_debug_h
#define graphiC_debug_h

#include "chunk.h"
#include "object.h"

void disassembleChunk(Chunk* chunk, const char* name);
int disassembleInstruction(Chunk* chunk, int offset);

#endif 