//
// Created by baifeng on 2022/3/30.
//

#ifndef SDL2_UI_SCRIPT_H
#define SDL2_UI_SCRIPT_H

#include "lutok3.h"
#include "ELuna.h"
#include <memory>

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

    class ObjectScript {
    public:
        ObjectScript(lua_State* L, std::shared_ptr<mge::FileData> const& data);
        ~ObjectScript();
    public:
        ELuna::LuaFunction<void> getFunction(ObjectFunction f);
    protected:
        lua_State* L;
        int funcRef[OBJECT_FUNCTION_MAX];
    };
}

#endif //SDL2_UI_SCRIPT_H
