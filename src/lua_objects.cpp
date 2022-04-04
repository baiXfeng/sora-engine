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

void registerClass(lua_State* L) {
    ELuna::registerClass<Layer>(L, "Layer", ELuna::constructor<Layer>);
    ELuna::registerClass<Node>(L, "Node", ELuna::constructor<Node>);

    ELuna::registerClass<Image>(L, "Image", ELuna::constructor<Image>);

    //ELuna::registerMetatable<>()
}

LuaScriptHolder::LuaScriptHolder() {
    initScript();
}

void LuaScriptHolder::loadScript(std::string const& fileName, const char* functionNames[], size_t nameSize) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Scene::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, functionNames, nameSize);
}

void LuaScriptHolder::initScript() {
    if (_script != nullptr) {
        return;
    }
    auto& state = _game.get<lutok3::State>("lua_state");
    _script = std::make_shared<Lua::ObjectScript>(state());
}

LuaScriptHolder::LuaScript& LuaScriptHolder::script() {
    return _script;
}

void NodeLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<::Node>()->loadScript(value);
    } else {
        NodeLoader::onParseProperty(node, parent, reader, name, value);
    }
}

const char* Node::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "update",
        "key_down",
        "key_up",
        "touch_began",
        "touch_moved",
        "touch_ended",
        "on_assign",
};

Node::Node():FingerResponder(this) {
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
    auto lua_script_holder = dynamic_cast<LuaScriptHolder*>(node);
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

void Node::update(float delta) {
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

bool Node::onTouchBegen(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        bool ret = _script->CallRet<bool>(OBJECT_FUNCTION_ONTOUCHBEGAN, table);
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

const char* Image::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
};

Image::Image() {
    _script->Ref(this);
}

Image::~Image() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Image::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHolder*>(node);
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

void ImageLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Image>()->loadScript(value);
    } else {
        ImageWidgetLoader::onParseProperty(node, parent, reader, name, value);
    }
}
