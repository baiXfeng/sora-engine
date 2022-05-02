//
// Created by baifeng on 2022/4/1.
//

#include "lua_apis.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "lutok3/lutok3.h"
#include "lua_objects.h"
#include "lua_widget.h"
#include "lua_audio.h"
#include "story-script.hpp"

template<typename T>
static void registerMacro(lua_State* L, const char* name, T value) {
    auto const top = lua_gettop(L);
    {
        luabridge::push(L, value);
        lua_setglobal(L, name);
    }
    assert(top == lua_gettop(L));
}

void openSoraLibs(lua_State* L) {

    // 按键值
    registerMacro(L, "BUTTON_UP", (int)mge::GamePadListener::UP);
    registerMacro(L, "BUTTON_DOWN", (int)mge::GamePadListener::DOWN);
    registerMacro(L, "BUTTON_LEFT", (int)mge::GamePadListener::LEFT);
    registerMacro(L, "BUTTON_RIGHT", (int)mge::GamePadListener::RIGHT);
    registerMacro(L, "BUTTON_L1", (int)mge::GamePadListener::L1);
    registerMacro(L, "BUTTON_R1", (int)mge::GamePadListener::R1);
    registerMacro(L, "BUTTON_SELECT", (int)mge::GamePadListener::SELECT);
    registerMacro(L, "BUTTON_START", (int)mge::GamePadListener::START);
    registerMacro(L, "BUTTON_A", (int)mge::GamePadListener::A);
    registerMacro(L, "BUTTON_B", (int)mge::GamePadListener::B);
    registerMacro(L, "BUTTON_X", (int)mge::GamePadListener::X);
    registerMacro(L, "BUTTON_Y", (int)mge::GamePadListener::Y);
}

void doBuffer(lua_State *L, const char* buffer, const size_t size, const char* name) {
    auto const top = lua_gettop(L);
    {
        auto ret = luaL_loadbuffer(L, buffer, size, name);
        if (ret) {
            LOG_ERROR("error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        } else if (ret = lua_pcall(L, 0, 0, 0); ret != 0) {
            LOG_ERROR("error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    assert(top == lua_gettop(L));
}

int import(lua_State* L) {
    const char* luaFileName = luaL_checkstring(L, -1);
    if (luaFileName == nullptr) {
        return 0;
    }
    auto data = _game.uilayout().getFileReader()->getData(luaFileName);
    if (!data->empty()) {
        doBuffer(L, (char*)data->data(), data->size(), data->name().c_str());
    } else {
        LOG_ERROR("error: import %s not exist.", luaFileName);
    }
    return 0;
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
