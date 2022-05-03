//
// Created by baifeng on 2022/4/1.
//

#include "lua_apis.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "lua_widget.h"
#include "lua_audio.h"

inline void traceStack(lua_State* L, int n) {
    lua_Debug ar;
    if (lua_getstack(L, n, &ar)) {
        lua_getinfo(L, "Sln", &ar);
        if (ar.name) {
            LOG_ERROR("\tstack[%d] -> line %d : %s()[%s : line %d]\n", n, ar.currentline, ar.name, ar.short_src, ar.linedefined);
        } else {
            LOG_ERROR("\tstack[%d] -> line %d : unknown[%s : line %d]\n", n, ar.currentline, ar.short_src, ar.linedefined);
        }
        traceStack(L, n+1);
    }
}

inline int error_log(lua_State *L) {
    LOG_ERROR("error : %s\n", lua_tostring(L, -1));
    traceStack(L, 0);
    return 0;
}

void doBuffer(lua_State *L, const char* buffer, const size_t size, const char* name) {
    auto const top = lua_gettop(L);
    {
        lua_pushcclosure(L, error_log, 0);
        int stackTop = lua_gettop(L);
        if (luaL_loadbuffer(L, buffer, size, name) == 0) {
            if (lua_pcall(L, 0, 0, stackTop)) {
                lua_pop(L, 1);
            }
        } else {
            LOG_ERROR("dobuffer error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        lua_pop(L, 1);
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
