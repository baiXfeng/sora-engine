//
// Created by baifeng on 2022/4/1.
//

#ifndef SDL2_UI_LUA_APIS_H
#define SDL2_UI_LUA_APIS_H

#include "ELuna.h"

void openSoraLibs(lua_State* L);
void import(const char* luaFileName);
void pushScene(const char* xmlFileName);
void replaceScene(const char* xmlFileName);
void popScene();

#endif //SDL2_UI_LUA_APIS_H
