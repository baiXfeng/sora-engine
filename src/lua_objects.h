//
// Created by baifeng on 2022/4/3.
//

#ifndef SDL2_UI_LUA_OBJECTS_H
#define SDL2_UI_LUA_OBJECTS_H

#include "common/widget.h"
#include "ui-layout/ui-layout.h"
#include "lua_action.h"
#include "lua_widget.h"

class Node : public mge::Widget,
             public mge::FingerResponder,
             public ui::LayoutVariableAssigner,
             public ui::LayoutNodeListener,
             public LuaScriptHelper,
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
        OBJECT_FUNCTION_ONLAYOUT,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Node();
    ~Node();
public:
    void loadScript(std::string const& fileName);
    bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
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
    bool onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
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
              public LuaScriptHelper,
              public LuaActionHelper,
              public LuaWidgetHelper {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_ONASSIGN,
        OBJECT_FUNCTION_ONLAYOUT,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Image();
    ~Image();
public:
    void loadScript(std::string const& fileName);
    bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
};

class ImageLoader : public ui::ImageWidgetLoader {
    UI_NODE_LOADER_CREATE(Image);
    bool onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

class Label : public mge::TTFLabel,
              public ui::LayoutVariableAssigner,
              public ui::LayoutNodeListener,
              public LuaScriptHelper,
              public LuaActionHelper,
              public LuaWidgetHelper {
public:
    enum Function {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_ONASSIGN,
        OBJECT_FUNCTION_ONLAYOUT,
        OBJECT_FUNCTION_MAX,
    };
    static const char* FunctionNames[OBJECT_FUNCTION_MAX];
public:
    Label();
    ~Label();
public:
    void loadScript(std::string const& fileName);
    bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
};

class LabelLoader : public ui::TTFLabelLoader {
    UI_NODE_LOADER_CREATE(Label);
    bool onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

#endif //SDL2_UI_LUA_OBJECTS_H
