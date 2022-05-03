//
// Created by baifeng on 2022/3/30.
//

#include "script.h"
#include "common/log.h"
#include "common/file-reader.h"

namespace Lua {

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

    int error_log(lua_State *L) {
        LOG_ERROR("error : %s\n", lua_tostring(L, -1));
        traceStack(L, 0);
        return 0;
    }

    ObjectScript::ObjectScript(lua_State* L):L(L), object(L) {

    }

    ObjectScript::~ObjectScript() {
        funcs.clear();
    }

    luabridge::LuaRef ObjectScript::getRef() const {
        return object;
    }

    void ObjectScript::Load(std::shared_ptr<mge::FileData> const& data, const char* functionNames[], size_t nameSize) {
        // 释放旧引用
        this->funcs.clear();
        this->funcs.resize(nameSize, L);
        this->func_names.resize(nameSize);

        // 保留旧self引用
        auto self = luabridge::LuaRef::getGlobal(L, "self");

        this->getRef().push();
        lua_setglobal(L, "self");

        {
            auto const top = lua_gettop(L);
            for (int i = 0; i < nameSize; ++i) {
                lua_pushnil(L);
                lua_setglobal(L, functionNames[i]);
                this->func_names[i] = functionNames[i];
            }
            assert(top == lua_gettop(L));
        }
        {
            auto const top = lua_gettop(L);
            auto ret = luaL_loadbuffer(L, (char*)data->data(), data->size(), data->name().c_str());
            if (ret != 0) {
                LOG_ERROR("error: %s\n", lua_tostring(L, -1));
            } else {
                if (auto ret = lua_pcall(L, 0, 0, 0); ret != 0) {
                    LOG_ERROR("error: %s\n", lua_tostring(L, -1));
                }
                for (int i = 0; i < nameSize; ++i) {
                    lua_getglobal(L, functionNames[i]);
                    if (not lua_isnil(L, -1)) {
                        if (lua_type(L, -1) == LUA_TFUNCTION) {
                            this->funcs[i] = luabridge::LuaRef::fromStack(L);
                        } else {
                            LOG_ERROR("error: the global name %s in %s must be function.\n", functionNames[i], data->name().c_str());
                            lua_pop(L, 1);
                            continue;
                        }
                    } else {
                        lua_pop(L, 1);
                    }
                }
            }
            assert(top == lua_gettop(L));
        }

        // 还原旧self引用
        self.push();
        lua_setglobal(L, "self");

    }
}
