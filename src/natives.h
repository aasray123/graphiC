#ifndef graphiC_natives_h
#define graphiC_natives_h

#include "value.h"
#include "common.h"
#include "object.h"
#include "vm.h"
#include "raylib.h"

//VECTORS
Value nativeVector2(int argCount, Value* args);
Vector2 valueToVector2(Value value);


#endif