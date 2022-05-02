//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_WIDGET_H
#define SDL2_UI_LUA_WIDGET_H

#include "common/widget.h"
#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"

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
    virtual ~LuaWidgetHelper() {}
    virtual bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
        return false;
    }
};

int widgetRunAction(mge::Widget* obj, luabridge::LuaRef action);
int widgetStopAction(mge::Widget* obj, const char* name);
int widgetHasAction(mge::Widget* obj, luabridge::LuaRef name);

int widgetSetPosition(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetSize(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetScale(mge::Widget* obj, luabridge::LuaRef table);
int widgetSetAnchor(mge::Widget* obj, luabridge::LuaRef table);

int widgetAddLayout(mge::Widget* obj, luabridge::LuaRef xmlFile);

#endif //SDL2_UI_LUA_WIDGET_H
