//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_ACTION_H
#define SDL2_UI_LUA_ACTION_H

#include "common/widget.h"
#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"

class LuaActionHelper {
public:
    LuaActionHelper(mge::Widget* target);
    virtual ~LuaActionHelper() {}
public:
    void runLuaAction(luabridge::LuaRef action);
protected:
    mge::Widget* _actionTarget;
};

#endif //SDL2_UI_LUA_ACTION_H
