//
// Created by baifeng on 2022/4/17.
//

#include "lua_widget.h"
#include "lua_action.h"
#include "common/log.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/mouse.h"
#include "lua-script/script.h"
#include "lutok3/lutok3.h"

//===============================================================================

LuaScriptHelper::LuaScriptHelper() {
    initScript();
}

void LuaScriptHelper::loadScript(std::string const& fileName, const char* functionNames[], size_t nameSize) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Scene::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, functionNames, nameSize);
}

void LuaScriptHelper::initScript() {
    if (_script != nullptr) {
        return;
    }
    auto& state = _game.get<lutok3::State>("lua_state");
    _script = std::make_shared<Lua::ObjectScript>(state());
}

LuaScriptHelper::LuaScript& LuaScriptHelper::script() {
    return _script;
}

int widgetRunAction(mge::Widget* obj, luabridge::LuaRef action) {
    LuaActionHelper helper(obj);
    helper.runLuaAction(action);
    return 0;
}

int widgetStopAction(mge::Widget* obj, const char* name) {
    obj->stopAction(name);
    return 0;
}

int widgetHasAction(mge::Widget* obj, luabridge::LuaRef name) {
    lua_pushboolean(name.state(), name.cast<bool>());
    return 1;
}

int widgetSetPosition(mge::Widget* obj, luabridge::LuaRef table) {
    obj->setPosition(table["x"].cast<float>(), table["y"].cast<float>());
    return 0;
}

int widgetSetSize(mge::Widget* obj, luabridge::LuaRef table) {
    obj->setSize(table["x"].cast<float>(), table["y"].cast<float>());
    return 0;
}

int widgetSetScale(mge::Widget* obj, luabridge::LuaRef table) {
    obj->setScale(table["x"].cast<float>(), table["y"].cast<float>());
    return 0;
}

int widgetSetAnchor(mge::Widget* obj, luabridge::LuaRef table) {
    obj->setAnchor(table["x"].cast<float>(), table["y"].cast<float>());
    return 0;
}

int widgetAddLayout(mge::Widget* obj, luabridge::LuaRef xmlFile) {
    if (!xmlFile.isString()) {
        lua_pushnil(xmlFile.state());
        return 1;
    }
    const char* file = xmlFile.cast<const char*>();
    auto node = _game.uilayout().readNode(file);
    if (node == nullptr) {
        lua_pushnil(xmlFile.state());
        return 1;
    }
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node.get());
    if (lua_script_holder == nullptr) {
        lua_pushnil(xmlFile.state());
        return 1;
    }
    obj->addChild(node);
    node->performLayout();
    auto script = lua_script_holder->script();
    luabridge::push(script->State(), script->getRef());
    return 1;
}


//===============================================================================

bool NodeLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<::Node>()->loadScript(value);
        return true;
    } else if (ui::NodeLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<::Node>()->onLayout(parent, reader, name, value);
    }
}

const char* Node::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "update",
        "key_down",
        "key_up",
        "joy_stick",
        "keyboard_down",
        "keyboard_up",
        "mouse_down",
        "mouse_motion",
        "mouse_up",
        "mouse_wheel",
        "mouse_enter",
        "mouse_exit",
        "on_assign",
        "on_layout",
};

Node::Node(): MouseResponder(this), LuaActionHelper(this) {
    connect(ON_ENTER, [this](Widget* sender){
        _game.mouse().add(this);
    });
    connect(ON_EXIT, [this](Widget* sender){
        _game.mouse().remove(this);
    });
    enableUpdate(true);
    _script->Ref(this);
}

Node::~Node() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

void Node::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Node::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

bool Node::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Node::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

void Node::onUpdate(float delta) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_UPDATE, delta);
    }
}

void Node::onButtonDown(int key) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_ONKEYDOWN, key);
    }
}

void Node::onButtonUp(int key) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_ONKEYUP, key);
    }
}

void Node::onJoyAxisMotion(JOYIDX joy_id, int x, int y) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(x);
        table["y"].rawset(y);
        _script->Call(OBJECT_FUNCTION_ONJOYSTICK, (int)joy_id, table);
    }
}

bool Node::onMouseDown(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        table["button"].rawset((int)event.button);
        bool ret = _script->Call<bool>(OBJECT_FUNCTION_ONMOUSEDOWN, table);
        return ret;
    }
    return false;
}

void Node::onMouseMotion(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        table["button"].rawset((int)event.button);
        _script->Call(OBJECT_FUNCTION_ONMOUSEMOTION, table);
    }
}

void Node::onMouseUp(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        table["button"].rawset((int)event.button);
        _script->Call(OBJECT_FUNCTION_ONMOUSEUP, table);
    }
}

void Node::onMouseEnter(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        _script->Call(OBJECT_FUNCTION_ONMOUSEENTER, table);
    }
}

void Node::onMouseExit(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        _script->Call(OBJECT_FUNCTION_ONMOUSEEXIT, table);
    }
}

bool Node::onMouseWheel(mge::MouseEvent const& event) {
    if (_script != nullptr) {
        auto table = luabridge::LuaRef::newTable(_script->State());
        table["x"].rawset(event.x);
        table["y"].rawset(event.y);
        return _script->Call<bool>(OBJECT_FUNCTION_ONMOUSEWHEEL, table);
    }
    return false;
}

bool Node::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

//===============================================================================

Layer::Layer() {
    connect(ON_ENTER, [](Widget* sender){
        _game.gamepad().add(sender->ptr());
    });
    connect(ON_EXIT, [](Widget* sender){
        _game.gamepad().remove(sender->ptr());
    });
}

bool Layer::onMouseDown(mge::MouseEvent const& event) {
    if (_script == nullptr) {
        return true;
    }
    return Node::onMouseDown(event);
}

void Layer::onKeyboardDown(int key) {
    if (_script != nullptr and _script->hasFunction(OBJECT_FUNCTION_ONKEYBOARDDOWN)) {
        _script->Call(OBJECT_FUNCTION_ONKEYBOARDDOWN, key);
        return;
    }
    Node::onKeyboardDown(key);
}

void Layer::onKeyboardUp(int key) {
    if (_script != nullptr and _script->hasFunction(OBJECT_FUNCTION_ONKEYBOARDUP)) {
        _script->Call(OBJECT_FUNCTION_ONKEYBOARDUP, key);
        return;
    }
    Node::onKeyboardUp(key);
}

//===============================================================================

const char* Image::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Image::Image(): LuaActionHelper(this) {
    _script->Ref(this);
}

Image::~Image() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Image::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Image::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Image::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

void Image::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

bool Image::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

bool ImageLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Image>()->loadScript(value);
        return true;
    } else if (ImageWidgetLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<Image>()->onLayout(parent, reader, name, value);
    }
}

//===============================================================================

const char* Label::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Label::Label(): LuaActionHelper(this) {
    _script->Ref(this);
}

Label::~Label() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Label::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Label::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Label::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

void Label::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

bool Label::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

bool LabelLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Label>()->loadScript(value);
        return true;
    } else if (TTFLabelLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<Label>()->onLayout(parent, reader, name, value);
    }
}

//===============================================================================

const char* Mask::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Mask::Mask():MaskWidget({0, 0, 0, 80}), LuaActionHelper(this) {
    _script->Ref(this);
    enableUpdate(true);
}

Mask::~Mask() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Mask::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Mask::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Mask::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

void Mask::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

bool Mask::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

void Mask::setColor(luabridge::LuaRef color) {
    MaskWidget::setColor({
                                 color[1].cast<unsigned char>(),
                                 color[2].cast<unsigned char>(),
                                 color[3].cast<unsigned char>(),
                                 color[4].cast<unsigned char>(),
                         });
}

luabridge::LuaRef Mask::getColor() {
    auto c = luabridge::LuaRef::newTable(_script->State());
    c[1].rawset(_color.r);
    c[2].rawset(_color.g);
    c[3].rawset(_color.b);
    c[4].rawset(_color.a);
    return c;
}

bool MaskLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Label>()->loadScript(value);
        return true;
    } else if (MaskWidgetLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<Label>()->onLayout(parent, reader, name, value);
    }
}
