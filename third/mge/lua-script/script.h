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

    int error_log(lua_State *L);

    class ObjectScript {
    public:
        typedef int32_t Function;
    public:
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
        int getRef() const;
        void Call(Function function) {
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
        template<typename T>
        void Call(Function function, T value) {
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
        void Call(Function function, T1 value1, T2 value2) {
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
