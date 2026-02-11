
#include "natives.h"

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