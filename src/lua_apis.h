//
// Created by baifeng on 2022/4/1.
//

#ifndef SDL2_UI_LUA_APIS_H
#define SDL2_UI_LUA_APIS_H

#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"

template<typename T>
void registerMacro(lua_State* L, const char* name, T value) {
    luabridge::push(L, value);
    lua_setglobal(L, name);
}

void doBuffer(lua_State *L, const char* buffer, const size_t size, const char* name);
int import(lua_State* L);
void pushScene(const char* xmlFileName);
void replaceScene(const char* xmlFileName);
void popScene();

#endif //SDL2_UI_LUA_APIS_H
