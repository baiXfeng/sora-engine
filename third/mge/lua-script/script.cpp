//
// Created by baifeng on 2022/3/30.
//

#include "script.h"
#include "common/log.h"
#include "common/file-reader.h"

namespace Lua {

    const char* ObjectFunctionNames[OBJECT_FUNCTION_MAX] = {
            "init",
            "release",
            "update",
            "keydown",
            "keyup",
            "touch_began",
            "touch_moved",
            "touch_ended",
    };

    void setObjectFuncAllNil(lua_State* L) {
        auto const top = lua_gettop(L);
        for (int i = 0; i < OBJECT_FUNCTION_MAX; ++i) {
            lua_pushnil(L);
            lua_setglobal(L, ObjectFunctionNames[i]);
        }
        assert(top == lua_gettop(L));
    }

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

    ObjectScript::ObjectScript(lua_State* L, std::shared_ptr<mge::FileData> const& data, const char* functionNames[], size_t nameSize):L(L), object_ref(LUA_NOREF) {
        this->func_refs.resize(nameSize, LUA_NOREF);
        this->func_names.resize(nameSize);
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
                            func_refs[i] = luaL_ref(L, LUA_REGISTRYINDEX);
                        } else {
                            LOG_ERROR("error: the global name %s in %s must be function.\n", functionNames[i], data->name().c_str());
                            lua_pop(L, 1);
                            continue;
                        }
                    } else {
                        func_refs[i] = LUA_NOREF;
                        lua_pop(L, 1);
                    }
                }
            }
            assert(top == lua_gettop(L));
        }
    }

    ObjectScript::~ObjectScript() {
        for (int i = 0; i <OBJECT_FUNCTION_MAX; ++i) {
            if (func_refs[i] != LUA_NOREF) {
                luaL_unref(L, LUA_REGISTRYINDEX, func_refs[i]);
            }
        }
        unRef();
    }

    void ObjectScript::unRef() {
        if (object_ref != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, object_ref);
            object_ref = LUA_NOREF;
        }
    }

    void ObjectScript::Call(ObjectFunction function) {
        auto const func_ref = func_refs[function];
        if (func_ref == LUA_NOREF or this->object_ref == LUA_NOREF) {
            return;
        }
        auto const top = lua_gettop(L);
        {
            lua_pushcclosure(L, error_log, 0);
            int stackTop = lua_gettop(L);
            lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
            lua_rawgeti(L, LUA_REGISTRYINDEX, this->object_ref);
            lua_pcall(L, 1, 1, stackTop);
            lua_settop(L, -3);
        }
        assert(top == lua_gettop(L));
    }
}
