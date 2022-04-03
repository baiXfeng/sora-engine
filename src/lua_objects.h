//
// Created by baifeng on 2022/4/3.
//

#ifndef SDL2_UI_LUA_OBJECTS_H
#define SDL2_UI_LUA_OBJECTS_H

#include "common/widget.h"
#include "uilayout/ui-layout.h"
#include "lutok3.h"

namespace Lua {
    class ObjectScript;
}

namespace ui {
    class LayoutReader;
}

class Scene : public mge::GamePadWidget, public ui::LayoutNodeListener {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_UPDATE,
        OBJECT_FUNCTION_ONKEYDOWN,
        OBJECT_FUNCTION_ONKEYUP,
        OBJECT_FUNCTION_ONTOUCHBEGAN,
        OBJECT_FUNCTION_ONTOUCHMOVED,
        OBJECT_FUNCTION_ONTOUCHENDED,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
    typedef std::shared_ptr<Lua::ObjectScript> LuaScript;
public:
    Scene();
    ~Scene();
public:
    void test() {}
    void loadScript(std::string const& fileName);
private:
    void onLayoutLoaded() override;
private:
    LuaScript _script;
};

class SceneLoader : public ui::NodeLoader {
    UI_NODE_LOADER_CREATE(Scene);
    void onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

void registerClass(lua_State* L);

#endif //SDL2_UI_LUA_OBJECTS_H
