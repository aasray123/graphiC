#ifndef graphiC_natives_h
#define graphiC_natives_h

#include "value.h"
#include "vm.h"
#include "raylib.h"
#include "memory.h"
#include "common.h"
#include "chunk.h"

//VECTORS
Value nativeVector2(int argCount, Value* args);
Vector2 valueToVector2(Value value);
Value nativeInitWindow(int argCount, Value* args);
Value nativeCloseWindow(int argCount, Value* args);
Value nativeColor(int argCount, Value* args);
Color valueToColor(Value value);
//void ClearBackground(Color color);           
Value NativeClearBackground(int argCount, Value* args);
Value NativeBeginDrawing(int argCount, Value* args);                
Value NativeEndDrawing(int argCount, Value* args);                    
Value NativeDrawCircle(int argCount, Value* args);

#endif
