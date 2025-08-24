#ifndef graphiC_compiler_h
#define graphiC_compiler_h

#include "vm.h"
#include "object.h"

ObjFunction* compile(const char* source);

#endif