//
// Created by baifeng on 2022/4/3.
//

#include "lua_objects.h"
#include "lua-script/script.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "common/mouse.h"
#include "ELuna.h"

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
        "touch_began",
        "touch_moved",
        "touch_ended",
        "on_assign",
        "on_layout",
};

Node::Node(): FingerResponder(this), LuaActionHelper(this), LuaWidgetHelper(this) {
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
        ELuna::LuaTable table(_script->State());
        table.set("x", x);
        table.set("y", y);
        _script->Call(OBJECT_FUNCTION_ONJOYSTICK, (int)joy_id, table);
    }
}

bool Node::onTouchBegen(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        bool ret = _script->Call<bool>(OBJECT_FUNCTION_ONTOUCHBEGAN, table);
        return ret;
    }
    return false;
}

void Node::onTouchMoved(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        _script->Call(OBJECT_FUNCTION_ONTOUCHMOVED, table);
    }
}

void Node::onTouchEnded(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        _script->Call(OBJECT_FUNCTION_ONTOUCHENDED, table);
    }
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

bool Layer::onTouchBegen(mge::Vector2i const& point) {
    if (_script == nullptr) {
        return true;
    }
    return Node::onTouchBegen(point);
}

//===============================================================================

const char* Image::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Image::Image(): LuaActionHelper(this), LuaWidgetHelper(this) {
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

Label::Label(): LuaActionHelper(this), LuaWidgetHelper(this) {
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

