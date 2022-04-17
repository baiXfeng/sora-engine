//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_ACTION_H
#define SDL2_UI_LUA_ACTION_H

#include "common/widget.h"
#include "ELuna.h"

class LuaActionHelper {
public:
    LuaActionHelper(mge::Widget* target);
    virtual ~LuaActionHelper() {}
public:
    void runLuaAction(ELuna::LuaTable action);
    void stopLuaAction(const char* name);
    bool hasLuaAction(const char* name);
protected:
    mge::Widget* _actionTarget;
};

#endif //SDL2_UI_LUA_ACTION_H
