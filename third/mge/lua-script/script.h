//
// Created by baifeng on 2022/3/30.
//

#ifndef SDL2_UI_SCRIPT_H
#define SDL2_UI_SCRIPT_H

#include "ELuna.h"
#include <memory>
#include <vector>

namespace mge {
    class FileData;
}

namespace Lua {

    enum ObjectFunction {
        OBJECT_FUNCTION_INIT = 0,
        OBJECT_FUNCTION_RELEASE,
        OBJECT_FUNCTION_UPDATE,
        OBJECT_FUNCTION_ONKEYDOWN,
        OBJECT_FUNCTION_ONKEYUP,
        OBJECT_FUNCTION_ONTOUCHBEGAN,
        OBJECT_FUNCTION_ONTOUCHMOVED,
        OBJECT_FUNCTION_ONTOUCHENDED,
        OBJECT_FUNCTION_MAX,
    };

    extern const char* ObjectFunctionNames[OBJECT_FUNCTION_MAX];

    int error_log(lua_State *L);

    class ObjectScript {
    public:
        ObjectScript(lua_State* L, std::shared_ptr<mge::FileData> const& data);
        ObjectScript(lua_State* L, std::shared_ptr<mge::FileData> const& data, const char* functionNames[], size_t nameSize);
        ~ObjectScript();
    public:
        template<typename T>
        void Ref(T* object) {
            auto const top = lua_gettop(L);
            {
                this->unRef();
                ELuna::push2lua(L, object);
                object_ref = luaL_ref(L, LUA_REGISTRYINDEX);
            }
            assert(top == lua_gettop(L));
        }
        void unRef();
        void Call(ObjectFunction function);
        template<typename T>
        void Call(ObjectFunction function, T value) {
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
                ELuna::push2lua(L, value);
                lua_pcall(L, 2, 1, stackTop);
                lua_settop(L, -3);
            }
            assert(top == lua_gettop(L));
        }
        template<typename T1, typename T2>
        void Call(ObjectFunction function, T1 value1, T2 value2) {
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
                ELuna::push2lua(L, value1);
                ELuna::push2lua(L, value2);
                lua_pcall(L, 3, 1, stackTop);
                lua_settop(L, -3);
            }
            assert(top == lua_gettop(L));
        }
    protected:
        lua_State* L;
        int object_ref;
        std::vector<int> func_refs;
        std::vector<const char*> func_names;
    };
}

#endif //SDL2_UI_SCRIPT_H
