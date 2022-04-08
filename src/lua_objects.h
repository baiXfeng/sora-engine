//
// Created by baifeng on 2022/4/3.
//

#ifndef SDL2_UI_LUA_OBJECTS_H
#define SDL2_UI_LUA_OBJECTS_H

#include "common/widget.h"
#include "common/action.h"
#include "uilayout/ui-layout.h"
#include "lutok3.h"
#include "ELuna.h"

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
protected:
    mge::Widget* _widget;
    lua_State* _state;
};

class Node : public mge::Widget,
             public mge::FingerResponder,
             public ui::LayoutVariableAssigner,
             public ui::LayoutNodeListener,
             public LuaScriptHolder,
             public LuaActionHelper,
             public LuaWidgetHelper {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_UPDATE,
        OBJECT_FUNCTION_ONKEYDOWN,
        OBJECT_FUNCTION_ONKEYUP,
        OBJECT_FUNCTION_ONJOYSTICK,
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
    void onUpdate(float delta) override;
    void onButtonDown(int key) override;
    void onButtonUp(int key) override;
    void onJoyAxisMotion(JOYIDX joy_id, int x, int y) override;
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

class Image : public mge::ImageWidget,
              public ui::LayoutVariableAssigner,
              public ui::LayoutNodeListener,
              public LuaScriptHolder,
              public LuaActionHelper,
              public LuaWidgetHelper {
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

class Label : public mge::TTFLabel,
              public ui::LayoutVariableAssigner,
              public ui::LayoutNodeListener,
              public LuaScriptHolder,
              public LuaActionHelper,
              public LuaWidgetHelper {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_ONASSIGN,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Label();
    ~Label();
public:
    void loadScript(std::string const& fileName);
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
};

class LabelLoader : public ui::TTFLabelLoader {
    UI_NODE_LOADER_CREATE(Label);
    void onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

#endif //SDL2_UI_LUA_OBJECTS_H
