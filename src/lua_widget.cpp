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
    auto ref = lua_script_holder->script()->getRef();
    luabridge::push(ref.state(), ref);
    return 1;
}
