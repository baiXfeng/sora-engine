//
// Created by baifeng on 2022/3/30.
//

#include "script.h"
#include "common/log.h"
#include "common/file-reader.h"
#include "lutok3.h"

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

    ObjectScript::ObjectScript(lua_State* L, std::shared_ptr<mge::FileData> const& data):L(L) {
        for (int i = 0; i < OBJECT_FUNCTION_MAX; ++i) {
            funcRef[i] = LUA_NOREF;
        }
        setObjectFuncAllNil(L);
        auto const top = lua_gettop(L);
        auto ret = luaL_loadbuffer(L, (char*)data->data(), data->size(), data->name().c_str());
        if (ret != 0) {
            LOG_ERROR("error: %s\n", lua_tostring(L, -1));
        } else {
            if (auto ret = lua_pcall(L, 0, 0, 0); ret != 0) {
                LOG_ERROR("error: %s\n", lua_tostring(L, -1));
            }
            for (int i = 0; i < OBJECT_FUNCTION_MAX; ++i) {
                lua_getglobal(L, ObjectFunctionNames[i]);
                if (not lua_isnil(L, -1)) {
                    if (lua_type(L, -1) == LUA_TFUNCTION) {
                        funcRef[i] = luaL_ref(L, LUA_REGISTRYINDEX);
                    } else {
                        LOG_ERROR("error: the global name %s in %s must be function.\n", ObjectFunctionNames[i], data->name().c_str());
                        lua_pop(L, 1);
                        continue;
                    }
                } else {
                    funcRef[i] = LUA_NOREF;
                    lua_pop(L, 1);
                }
            }
        }
        assert(top == lua_gettop(L));
    }

    ObjectScript::~ObjectScript() {
        for (int i = 0; i <OBJECT_FUNCTION_MAX; ++i) {
            if (funcRef[i] != LUA_NOREF) {
                luaL_unref(L, LUA_REGISTRYINDEX, funcRef[i]);
            }
        }
    }

    ELuna::LuaFunction<void> ObjectScript::getFunction(ObjectFunction i) {
        if (i < OBJECT_FUNCTION_INIT or i >= OBJECT_FUNCTION_MAX) {
            LOG_ERROR("error: object function index [%d] overstep.", i);
        } else if (funcRef[i] == LUA_NOREF) {
            LOG_ERROR("error: object function %s noref.", ObjectFunctionNames[i]);
        }
        return ELuna::LuaFunction<void>(L, funcRef[i]);
    }
}
