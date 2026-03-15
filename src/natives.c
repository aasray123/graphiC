
#include "natives.h"

#include "value.h"

Value nativeVector2(int argCount, Value* args){
    if (argCount !=2) return C_TO_NULL_VALUE;

    ObjInstance* instance = newInstance(vm.vector2Entity);

    push(C_TO_OBJ_VALUE(instance));

    tableSet(&instance->fields, vm.strX, args[0]);
    tableSet(&instance->fields, vm.strY, args[1]);

    pop();
    
    return C_TO_OBJ_VALUE(instance);

}

Vector2 valueToVector2(Value value){
    if (!IS_INSTANCE(value)) return (Vector2){0, 0};
    ObjInstance* instance = AS_INSTANCE(value);
    
    if(strcmp(instance->entity->name->chars, "Vector2") != 0) return (Vector2){0, 0};

    Vector2 vec;
    Value val;

    if(tableGet(&instance->fields, vm.strX, &val)) {
        vec.x = (float)NUMBER_VALUE_TO_C(val);
    }
    else {
        vec.x = 0.0f;
    }

    if (tableGet(&instance->fields, vm.strY, &val)) {
        vec.y = (float)NUMBER_VALUE_TO_C(val);
    } else {
        vec.y = 0.0f;
    }

    return vec;
}

Value nativeInitWindow(int argCount, Value* args){
    if (argCount != 3) return C_TO_NULL_VALUE;

    int width = (int)NUMBER_VALUE_TO_C(args[0]);
    int height = (int)NUMBER_VALUE_TO_C(args[1]);
    const char* title = AS_CSTRING(args[2]);

    InitWindow(width, height, title);

    return C_TO_NULL_VALUE;
}


Value nativeCloseWindow(int argCount, Value* args){
   if (argCount != 0) return C_TO_NULL_VALUE;

    CloseWindow();

   return C_TO_NULL_VALUE;
}


Value nativeColor(int argCount, Value* args) {
    if (argCount != 4) return C_TO_NULL_VALUE;

    ObjInstance* instance = newInstance(vm.colorEntity);

    // Anchor the instance so the GC doesn't free it during tableSet
    push(C_TO_OBJ_VALUE(instance));

    tableSet(&instance->fields, vm.strR, args[0]);
    tableSet(&instance->fields, vm.strG, args[1]);
    tableSet(&instance->fields, vm.strB, args[2]);
    tableSet(&instance->fields, vm.strA, args[3]);

    // Remove the anchor
    pop();

    return C_TO_OBJ_VALUE(instance);
}

Color valueToColor(Value value) {
    // Default to Black (opaque) if the cast fails
    if (!IS_INSTANCE(value)) return (Color){0, 0, 0, 255}; 
    ObjInstance* instance = AS_INSTANCE(value);
    
    if (strcmp(instance->entity->name->chars, "Color") != 0) return (Color){0, 0, 0, 255};

    Color color;
    Value val;

    // Raylib Colors use unsigned chars (0-255)
    if (tableGet(&instance->fields, vm.strR, &val)) {
        color.r = (unsigned char)NUMBER_VALUE_TO_C(val);
    } else color.r = 0;

    if (tableGet(&instance->fields, vm.strG, &val)) {
        color.g = (unsigned char)NUMBER_VALUE_TO_C(val);
    } else color.g = 0;

    if (tableGet(&instance->fields, vm.strB, &val)) {
        color.b = (unsigned char)NUMBER_VALUE_TO_C(val);
    } else color.b = 0;

    if (tableGet(&instance->fields, vm.strA, &val)) {
        color.a = (unsigned char)NUMBER_VALUE_TO_C(val);
    } else color.a = 255; // Default alpha to 255 (visible)

    return color;
}

Value NativeClearBackground(int argCount, Value* args){
    if(argCount != 1) return C_TO_NULL_VALUE;
        
	Color color = valueToColor(args[0]);

	ClearBackground(color);

	return C_TO_NULL_VALUE;
}

Value NativeBeginDrawing(int argCount, Value* args){
    if(argCount != 0) return C_TO_NULL_VALUE;

    BeginDrawing();

    return C_TO_NULL_VALUE;
}   

Value NativeEndDrawing(int argCount, Value* args){
    if(argCount != 0) return C_TO_NULL_VALUE;

    EndDrawing();

    return C_TO_NULL_VALUE;
}

Value NativeDrawCircle(int argCount, Value* args){
    if (argCount != 3) return C_TO_NULL_VALUE;

    Vector2 vector2 = valueToVector2(args[0]);
    float radius = NUMBER_VALUE_TO_C(args[1]);
    Color color = valueToColor(args[2]);

    DrawCircleV(vector2, radius, color);
    // void DrawCircleV(Vector2 center, float radius, Color color);   
    return C_TO_NULL_VALUE;
}

Value NativeMouseX(int argCount, Value* args){
    if(argCount != 0 ) return C_TO_NULL_VALUE;

    return C_TO_NUMBER_VALUE(GetMouseX());
}

Value NativeMouseY(int argCount, Value* args){
    if(argCount != 0) return C_TO_NULL_VALUE;

    return C_TO_NUMBER_VALUE(GetMouseY());
}

Value NativeGetMousePosition(int argCount, Value* args){
    if (argCount != 0) return C_TO_NULL_VALUE;

    // Call the Raylib function
    Vector2 mousePos = GetMousePosition();

    // Create a new graphiC Vector2 instance
    ObjInstance* instance = newInstance(vm.vector2Entity);

    // Anchor the instance to protect it from the GC during tableSet
    push(C_TO_OBJ_VALUE(instance));

    // Convert the C floats to VM Number Values and set the fields
    tableSet(&instance->fields, vm.strX, C_TO_NUMBER_VALUE(mousePos.x));
    tableSet(&instance->fields, vm.strY, C_TO_NUMBER_VALUE(mousePos.y));

    // Remove the GC anchor
    pop();

    return C_TO_OBJ_VALUE(instance);
}
