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
    ELuna::registerClass<Node>(L, "Layer", ELuna::constructor<Node>);

    //ELuna::registerMetatable<>()
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
};

Node::Node():FingerResponder(this) {
    enableUpdate(true);
}

Node::~Node() {
    if (_script.get()) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

void Node::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Scene::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    auto& state = _game.get<lutok3::State>("lua_state");
    _script = std::make_shared<Lua::ObjectScript>(state(), data, FunctionNames, OBJECT_FUNCTION_MAX);
    if (_script.get()) {
        _script->Ref(this);
    }
}

bool Node::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    return false;
}

void Node::onLayoutLoaded() {
    _script->Call(OBJECT_FUNCTION_INIT);
}

void Node::update(float delta) {
    _script->Call(OBJECT_FUNCTION_UPDATE, delta);
}

void Node::onButtonDown(int key) {
    _script->Call(OBJECT_FUNCTION_ONKEYDOWN, key);
}

void Node::onButtonUp(int key) {
    _script->Call(OBJECT_FUNCTION_ONKEYUP, key);
}

bool Node::onTouchBegen(mge::Vector2i const& point) {
    ELuna::LuaTable table(_script->State());
    table.set("x", point.x);
    table.set("y", point.y);
    bool ret = _script->CallRet<bool>(OBJECT_FUNCTION_ONTOUCHBEGAN, table);
    return ret;
}

void Node::onTouchMoved(mge::Vector2i const& point) {
    ELuna::LuaTable table(_script->State());
    table.set("x", point.x);
    table.set("y", point.y);
    _script->Call(OBJECT_FUNCTION_ONTOUCHMOVED, table);
}

void Node::onTouchEnded(mge::Vector2i const& point) {
    ELuna::LuaTable table(_script->State());
    table.set("x", point.x);
    table.set("y", point.y);
    _script->Call(OBJECT_FUNCTION_ONTOUCHENDED, table);
}

Layer::Layer() {
    connect(ON_ENTER, [](Widget* sender){
        _game.gamepad().add(sender->ptr());
    });
    connect(ON_EXIT, [](Widget* sender){
        _game.gamepad().remove(sender->ptr());
    });
}
