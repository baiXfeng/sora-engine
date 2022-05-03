//
// Created by baifeng on 2022/4/17.
//

#ifndef SDL2_UI_LUA_WIDGET_H
#define SDL2_UI_LUA_WIDGET_H

#include "common/widget.h"
#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"
#include "ui-layout/ui-layout.h"
#include "lua_action.h"

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

//===============================================================================

class Node : public mge::Widget,
             public mge::MouseResponder,
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
        OBJECT_FUNCTION_ONKEYBOARDDOWN,
        OBJECT_FUNCTION_ONKEYBOARDUP,
        OBJECT_FUNCTION_ONMOUSEDOWN,
        OBJECT_FUNCTION_ONMOUSEMOTION,
        OBJECT_FUNCTION_ONMOUSEUP,
        OBJECT_FUNCTION_ONMOUSEWHEEL,
        OBJECT_FUNCTION_ONMOUSEENTER,
        OBJECT_FUNCTION_ONMOUSEEXIT,
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
    bool onMouseDown(mge::MouseEvent const& event) override;
    void onMouseMotion(mge::MouseEvent const& event) override;
    void onMouseUp(mge::MouseEvent const& event) override;
    void onMouseEnter(mge::MouseEvent const& event) override;
    void onMouseExit(mge::MouseEvent const& event) override;
    bool onMouseWheel(mge::MouseEvent const& event) override;
};

class NodeLoader : public ui::NodeLoader {
    UI_NODE_LOADER_CREATE(::Node);
    bool onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

class Layer : public Node {
public:
    Layer();
private:
    bool onMouseDown(mge::MouseEvent const& event) override;
    void onKeyboardDown(int key) override;
    void onKeyboardUp(int key) override;
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

class Mask : public mge::MaskWidget,
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
    Mask();
    ~Mask();
public:
    void loadScript(std::string const& fileName);
    bool onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
    void setColor(luabridge::LuaRef color);
    luabridge::LuaRef getColor();
private:
    bool onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) override;
    void onLayoutLoaded() override;
};

class MaskLoader : public ui::MaskWidgetLoader {
    UI_NODE_LOADER_CREATE(Mask);
    bool onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) override;
};

#endif //SDL2_UI_LUA_WIDGET_H
