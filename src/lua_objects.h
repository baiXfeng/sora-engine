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

class LuaScriptHolder {
public:
    typedef std::shared_ptr<Lua::ObjectScript> LuaScript;
public:
    LuaScriptHolder();
    virtual ~LuaScriptHolder() {}
public:
    void loadScript(std::string const& fileName, const char* functionNames[], size_t nameSize);
    void initScript();
    LuaScript& script();
protected:
    LuaScript _script;
};

class Node : public mge::Widget, public mge::FingerResponder, public ui::LayoutVariableAssigner, public ui::LayoutNodeListener,
             public LuaScriptHolder {
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
        OBJECT_FUNCTION_ONASSIGN,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Node();
    ~Node();
public:
    void loadScript(std::string const& fileName);
protected:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
    void update(float delta) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    bool onTouchBegen(mge::Vector2i const& point) override;
    void onTouchMoved(mge::Vector2i const& point) override;
    void onTouchEnded(mge::Vector2i const& point) override;
};

class NodeLoader : public ui::NodeLoader {
    UI_NODE_LOADER_CREATE(::Node);
    void onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

class Layer : public Node {
public:
    Layer();
private:
    bool onTouchBegen(mge::Vector2i const& point) override;
};

class LayerLoader : public NodeLoader {
    UI_NODE_LOADER_CREATE(Layer);
};

class Image : public mge::ImageWidget, public ui::LayoutVariableAssigner, public ui::LayoutNodeListener, public LuaScriptHolder {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_ONASSIGN,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Image();
    ~Image();
public:
    void loadScript(std::string const& fileName);
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
};

class ImageLoader : public ui::ImageWidgetLoader {
    UI_NODE_LOADER_CREATE(Image);
    void onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

void registerClass(lua_State* L);

#endif //SDL2_UI_LUA_OBJECTS_H
