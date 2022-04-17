//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_WIDGET_H
#define SDL2_UI_LUA_WIDGET_H

#include "common/widget.h"
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

int widgetRunAction(mge::Widget* obj, lua_State* L);
int widgetStopAction(mge::Widget* obj, lua_State* L);
int widgetHasAction(mge::Widget* obj, lua_State* L);

int widgetSetPosition(mge::Widget* obj, lua_State* L);
int widgetGetPosition(mge::Widget* obj, lua_State* L);
int widgetSetSize(mge::Widget* obj, lua_State* L);
int widgetGetSize(mge::Widget* obj, lua_State* L);
int widgetSetScale(mge::Widget* obj, lua_State* L);
int widgetGetScale(mge::Widget* obj, lua_State* L);
int widgetSetAnchor(mge::Widget* obj, lua_State* L);
int widgetGetAnchor(mge::Widget* obj, lua_State* L);
int widgetGetRotation(mge::Widget* obj, lua_State* L);
int widgetGetOpacity(mge::Widget* obj, lua_State* L);

int widgetGetVisible(mge::Widget* obj, lua_State* L);
int widgetGetParent(mge::Widget* obj, lua_State* L);
int widgetAddLayout(mge::Widget* obj, lua_State* L);

#endif //SDL2_UI_LUA_WIDGET_H
