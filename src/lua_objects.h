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

class Node : public mge::Widget, public mge::FingerResponder, public ui::LayoutVariableAssigner, public ui::LayoutNodeListener {
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
    Node();
    ~Node();
public:
    void loadScript(std::string const& fileName);
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
    void update(float delta) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    bool onTouchBegen(mge::Vector2i const& point) override;
    void onTouchMoved(mge::Vector2i const& point) override;
    void onTouchEnded(mge::Vector2i const& point) override;
private:
    LuaScript _script;
};

class NodeLoader : public ui::NodeLoader {
    UI_NODE_LOADER_CREATE(::Node);
    void onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

class Layer : public Node {
public:
    Layer();
};

class LayerLoader : public NodeLoader {
    UI_NODE_LOADER_CREATE(Layer);
};

void registerClass(lua_State* L);

#endif //SDL2_UI_LUA_OBJECTS_H
