//
// Created by baifeng on 2022/3/30.
//

#ifndef SDL2_UI_SCRIPT_H
#define SDL2_UI_SCRIPT_H

#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"
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
        ObjectScript(lua_State* L);
        ~ObjectScript();
    public:
        template<typename T>
        void Ref(T* object) {
            luabridge::push(L, object);
            this->object = luabridge::LuaRef::fromStack(L);
        }
        luabridge::LuaRef getRef() const;
        void Load(std::shared_ptr<mge::FileData> const& data, const char* functionNames[], size_t nameSize);
        void Call(Function function) {
            if (funcs.empty()) {
                return;
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return;
            }
            func(object);
        }
        template<typename T>
        void Call(Function function, T value) {
            if (funcs.empty()) {
                return;
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return;
            }
            func(object, value);
        }
        template<typename RL, typename T>
        RL Call(Function function, T value) {
            if (funcs.empty()) {
                return RL();
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return RL();
            }
            auto ret = func(object, value);
            return ret.template cast<RL>();
        }
        template<typename RL, typename T1, typename T2>
        RL Call(Function function, T1 value1, T2 value2) {
            if (funcs.empty()) {
                return RL();
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return RL();
            }
            auto ret = func(object, value1, value2);
            return ret.template cast<RL>();
        }
        template<typename RL, typename T1, typename T2, typename T3>
        RL Call(Function function, T1 value1, T2 value2, T3 value3) {
            if (funcs.empty()) {
                return RL();
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return RL();
            }
            auto ret = func(object, value1, value2, value3);
            return ret.template cast<RL>();
        }
        template<typename T1, typename T2>
        void Call(Function function, T1 value1, T2 value2) {
            if (funcs.empty()) {
                return;
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil()) {
                return;
            }
            func(object, value1, value2);
        }
        lua_State* State() const {
            return L;
        }
        void CallAssign(Function function, const char* name, luabridge::LuaRef assign) {
            if (funcs.empty()) {
                return;
            }
            auto func = funcs[function];
            if (func.isNil() or object.isNil() or assign.isNil()) {
                return;
            }
            func(object, name, assign);
        }
        bool hasFunction(int function) const {
            if (function < funcs.size()) {
                return !funcs[function].isNil();
            }
            return false;
        }
    protected:
        lua_State* L;
        luabridge::LuaRef object;
        std::vector<luabridge::LuaRef> funcs;
        std::vector<const char*> func_names;
    };
}

#endif //SDL2_UI_SCRIPT_H
