//
// Created by baifeng on 2022/4/17.
//

#include "lua_widget.h"
#include "lua_action.h"
#include "common/log.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "lua-script/script.h"
#include "lutok3.h"

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

//===============================================================================

LuaWidgetHelper::LuaWidgetHelper(mge::Widget* target): _widget(target), _state(_game.get<lutok3::State>("lua_state")()) {

}

void LuaWidgetHelper::setLuaPosition(ELuna::LuaTable position) {
    _widget->setPosition(position.get<const char*, float>("x"), position.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaPosition() {
    auto const& pos = _widget->position();
    ELuna::LuaTable table(_state);
    table.set("x", pos.x);
    table.set("y", pos.y);
    return table;
}

void LuaWidgetHelper::setLuaSize(ELuna::LuaTable size) {
    _widget->setSize(size.get<const char*, float>("x"), size.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaSize() {
    auto const& size = _widget->size();
    ELuna::LuaTable table(_state);
    table.set("x", size.x);
    table.set("y", size.y);
    return table;
}

void LuaWidgetHelper::setLuaScale(ELuna::LuaTable scale) {
    _widget->setScale(scale.get<const char*, float>("x"), scale.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaScale() {
    auto const& scale = _widget->scale();
    ELuna::LuaTable table(_state);
    table.set("x", scale.x);
    table.set("y", scale.y);
    return table;
}

void LuaWidgetHelper::setLuaAnchor(ELuna::LuaTable anchor) {
    _widget->setAnchor(anchor.get<const char*, float>("x"), anchor.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaAnchor() {
    auto const& anchor = _widget->anchor();
    ELuna::LuaTable table(_state);
    table.set("x", anchor.x);
    table.set("y", anchor.y);
    return table;
}

float LuaWidgetHelper::getLuaRotation() {
    return _widget->rotation();
}

unsigned char LuaWidgetHelper::getLuaOpacity() {
    return _widget->opacity();
}

bool LuaWidgetHelper::getLuaVisible() {
    return _widget->visible();
}

int LuaWidgetHelper::getWidgetParent(lua_State* L) {
    if (_widget->parent() == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(_widget->parent());
    if (lua_script_holder == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    ELuna::push2lua_ref(L, lua_script_holder->script()->getRef());
    return 1;
}

int LuaWidgetHelper::addWidgetFromLayout(lua_State* L) {
    auto file = ELuna::read2cpp<const char*>(L, -1);
    if (file == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto node = _game.uilayout().readNode(file);
    if (node == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node.get());
    if (lua_script_holder == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    _widget->addChild(node);
    node->performLayout();
    ELuna::push2lua_ref(L, lua_script_holder->script()->getRef());
    return 1;
}


int widgetRunAction(mge::Widget* obj, lua_State* L) {
    ELuna::LuaTable action(L, -1);
    LuaActionHelper helper(obj);
    helper.runLuaAction(action);
    return 0;
}

int widgetStopAction(mge::Widget* obj, lua_State* L) {
    auto name = lua_tostring(L, -1);
    if (name) {
        obj->stopAction(name);
    }
    return 0;
}

int widgetHasAction(mge::Widget* obj, lua_State* L) {
    auto name = lua_tostring(L, -1);
    if (name) {
        ELuna::push2lua(L, obj->hasAction(name));
        return 1;
    }
    return 0;
}

int widgetSetPosition(mge::Widget* obj, lua_State* L) {
    ELuna::LuaTable table(L, -1);
    LuaWidgetHelper helper(obj);
    helper.setLuaPosition(table);
    return 0;
}

int widgetGetPosition(mge::Widget* obj, lua_State* L) {
    LuaWidgetHelper helper(obj);
    ELuna::push2lua(L, helper.getLuaPosition());
    return 1;
}

int widgetSetSize(mge::Widget* obj, lua_State* L) {
    ELuna::LuaTable table(L, -1);
    LuaWidgetHelper helper(obj);
    helper.setLuaSize(table);
    return 0;
}

int widgetGetSize(mge::Widget* obj, lua_State* L) {
    LuaWidgetHelper helper(obj);
    ELuna::push2lua(L, helper.getLuaSize());
    return 1;
}

int widgetSetScale(mge::Widget* obj, lua_State* L) {
    ELuna::LuaTable table(L, -1);
    LuaWidgetHelper helper(obj);
    helper.setLuaScale(table);
    return 0;
}

int widgetGetScale(mge::Widget* obj, lua_State* L) {
    LuaWidgetHelper helper(obj);
    ELuna::push2lua(L, helper.getLuaScale());
    return 1;
}

int widgetSetAnchor(mge::Widget* obj, lua_State* L) {
    ELuna::LuaTable table(L, -1);
    LuaWidgetHelper helper(obj);
    helper.setLuaAnchor(table);
    return 0;
}

int widgetGetAnchor(mge::Widget* obj, lua_State* L) {
    LuaWidgetHelper helper(obj);
    ELuna::push2lua(L, helper.getLuaAnchor());
    return 1;
}

int widgetGetRotation(mge::Widget* obj, lua_State* L) {
    ELuna::push2lua(L, obj->rotation());
    return 1;
}

int widgetGetOpacity(mge::Widget* obj, lua_State* L) {
    ELuna::push2lua(L, obj->opacity());
    return 1;
}

int widgetGetVisible(mge::Widget* obj, lua_State* L) {
    ELuna::push2lua(L, obj->visible());
    return 1;
}

int widgetGetParent(mge::Widget* obj, lua_State* L) {
    ELuna::push2lua(L, obj->parent());
    return 1;
}

int widgetAddLayout(mge::Widget* obj, lua_State* L) {
    LuaWidgetHelper helper(obj);
    return helper.addWidgetFromLayout(L);
}
