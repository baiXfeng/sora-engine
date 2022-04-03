//
// Created by baifeng on 2022/4/1.
//

#include "lua_apis.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "common/widget.h"
#include "lutok3.h"
#include "lua_objects.h"

static void registerCFunction(lua_State* L, const char* tableName, const char* funcName) {
    lutok3::State state(L);
    auto const top = state.getTop();
    {
        auto type = state.getGlobal(tableName);
        if (type == lutok3::Type::Nil) {
            state.pop();
            state.newTable();
            state.setGlobal(tableName);
            state.getGlobal(tableName);
        }
        auto index = state.getTop();
        state.push(funcName);
        state.getGlobal(funcName);
        state.setTable(index);
        state.pop();
    }
    assert(top == state.getTop());
}

void setValueNil(lua_State* L, const char* valueName) {
    auto const top = lua_gettop(L);
    {
        lua_pushnil(L);
        lua_setglobal(L, valueName);
    }
    assert(top == lua_gettop(L));
}

#define registerCFunction2Table(lua_state, tableName, funcName, cfunction) \
{                                                                \
    ELuna::registerFunction(lua_state, funcName, cfunction);    \
    registerCFunction(lua_state, tableName, funcName);          \
    setValueNil(lua_state, funcName);                           \
}

void openSoraLibs(lua_State* L) {
    ELuna::registerFunction(L, "import", &import);

    registerCFunction2Table(L, "scene", "push", &pushScene);
    registerCFunction2Table(L, "scene", "replace", &replaceScene);
    registerCFunction2Table(L, "scene", "pop", &popScene);

    _game.uilayout().getLoaderPool()->addLoader<LayerLoader>("Layer");
    _game.uilayout().getLoaderPool()->addLoader<NodeLoader>("Node");

    registerClass(L);
}

void import(const char* luaFileName) {
    auto& state = _game.get<lutok3::State>("lua_state");
    auto data = _game.uilayout().getFileReader()->getData(luaFileName);
    if (!data->empty()) {
        auto ret = luaL_loadbuffer(state(), (char*)data->data(), (size_t)data->size(), data->name().c_str());
        if (ret) {
            LOG("error: %s\n", lua_tostring(state(), -1));
            lua_pop(state(), 1);
        } else if (ret = lua_pcall(state(), 0, 0, 0); ret != 0) {
            LOG("error: %s\n", lua_tostring(state(), -1));
            lua_pop(state(), 1);
        }
    }
}

void pushScene(const char* xmlFileName) {
    auto scene = _game.uilayout().readNode(xmlFileName);
    if (scene == nullptr) {
        LOG_ERROR("pushScene fail: %s node is null.\n", xmlFileName);
        return;
    }
    _game.screen().push(scene);
}

void replaceScene(const char* xmlFileName) {
    auto scene = _game.uilayout().readNode(xmlFileName);
    if (scene == nullptr) {
        LOG_ERROR("replaceScene fail: %s node is null.\n", xmlFileName);
        return;
    }
    _game.screen().replace(scene);
}

void popScene() {
    _game.screen().pop();
}