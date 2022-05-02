//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_WIDGET_H
#define SDL2_UI_LUA_WIDGET_H

#include "common/widget.h"
#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"
#include "ELuna.h"

namespace Lua {
    class ObjectScript;
}

namespace ui {
    class LayoutReader;
}

class LuaScriptHelper {
public:
    typedef std::shared_ptr<Lua::ObjectScript> LuaScript;
public:
    LuaScriptHelper();
    virtual ~LuaScriptHelper() {}
public:
    void loadScript(std::string const& fileName, const char* functionNames[], size_t nameSize);
    void initScript();
    LuaScript& script();
protected:
    LuaScript _script;
};

class LuaWidgetHelper {
public:
    LuaWidgetHelper(mge::Widget* target);
public:
    void setLuaPosition(ELuna::LuaTable position);
    ELuna::LuaTable getLuaPosition();
    void setLuaSize(ELuna::LuaTable size);
    ELuna::LuaTable getLuaSize();
    void setLuaScale(ELuna::LuaTable scale);
    ELuna::LuaTable getLuaScale();
    void setLuaAnchor(ELuna::LuaTable anchor);
    ELuna::LuaTable getLuaAnchor();
public:
    float getLuaRotation();
    unsigned char getLuaOpacity();
    bool getLuaVisible();
    int getWidgetParent(lua_State* L);
    int addWidgetFromLayout(lua_State* L);
public:
    virtual bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
        return false;
    }
protected:
    mge::Widget* _widget;
    lua_State* _state;
};

int widgetRunAction(mge::Widget* obj, luabridge::LuaRef action);
int widgetStopAction(mge::Widget* obj, const char* name);
int widgetHasAction(mge::Widget* obj, luabridge::LuaRef name);

int widgetSetPosition(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetSize(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetScale(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetAnchor(mge::Widget* obj, luabridge::LuaRef table);

int widgetAddLayout(mge::Widget* obj, lua_State* L);

#endif //SDL2_UI_LUA_WIDGET_H
