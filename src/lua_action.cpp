//
// Created by baifeng on 2022/4/17.
//

#include "lua_action.h"
#include "common/action.h"
#include "common/log.h"
#include "lutok3.h"

//===============================================================================

mge::Action::Ptr newAction(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newSequence(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newRepeat(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newDelay(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newCallback(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newScaleTo(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newScaleBy(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newMoveTo(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newMoveBy(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newRotationTo(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newRotationBy(mge::Widget* target, ELuna::LuaTable action);
mge::Action::Ptr newBlink(mge::Widget* target, ELuna::LuaTable action);

static std::map<std::string, std::function<mge::Action::Ptr(mge::Widget*, ELuna::LuaTable)>> LuaActionBuilder = {
        {"Sequence", &newSequence},
        {"Repeat", &newRepeat},
        {"Delay", &newDelay},
        {"Callback", &newCallback},
        {"ScaleTo", &newScaleTo},
        {"ScaleBy", &newScaleBy},
        {"MoveTo", &newMoveTo},
        {"MoveBy", &newMoveBy},
        {"RotationTo", &newRotationTo},
        {"RotationBy", &newRotationBy},
        {"Blink", &newBlink},
};

mge::Action::Ptr newSequence(mge::Widget* target, ELuna::LuaTable action) {
    mge::Sequence::Actions actions;
    {
        auto actions_table = action.get("actions");
        auto L = actions_table.m_luaState;
        auto const index = actions_table.m_stackPos;
        auto const top = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, index)) {
            lua_pushvalue(L, -2);
            auto key = lua_tostring(L, -1);
            auto value = lua_tostring(L, -2);
            if (lua_type(L, -2) == LUA_TTABLE) {
                ELuna::LuaTable table(L, lua_gettop(L) - 1);
                auto act = newAction(target, table);
                if (act != nullptr) {
                    actions.emplace_back(act);
                }
            } else {
                lua_pop(L, 2);
                continue;
            }
            lua_pop(L, 1);
        }
        assert(top == lua_gettop(L));
    }
    return mge::Action::Ptr(new mge::Sequence(actions));
}

mge::Action::Ptr newRepeat(mge::Widget* target, ELuna::LuaTable action) {
    int repeatCount = action.get<const char*, int>("repeatCount");
    auto act = newAction(target, action.get("action"));
    return mge::Action::Ptr(new mge::Repeat(act, repeatCount));
}

mge::Action::Ptr newDelay(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    return mge::Action::Ptr(new mge::Delay(duration));
}

class LuaFunctionBind {
public:
    LuaFunctionBind():func_ref(LUA_NOREF), object_ref(LUA_NOREF), L(nullptr) {}
    ~LuaFunctionBind() {
        unref(func_ref);
        unref(object_ref);
        L = nullptr;
    }
    void Call() {
        if (func_ref == LUA_NOREF or L == nullptr) {
            return;
        }
        auto const top = lua_gettop(L);
        {
            lua_rawgeti(L, LUA_REGISTRYINDEX, func_ref);
            if (object_ref != LUA_NOREF) {
                lua_rawgeti(L, LUA_REGISTRYINDEX, object_ref);
            }
            int argsCount = object_ref != LUA_NOREF ? 1 : 0;
            int ret = lua_pcall(L, argsCount, 0, 0);
            if (ret != 0) {
                LOG_ERROR("error: %s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
        assert(top == lua_gettop(L));
    }
    lua_State* L;
    int func_ref;
    int object_ref;
private:
    void unref(int& ref) {
        if (ref != LUA_NOREF) {
            luaL_unref(L, LUA_REGISTRYINDEX, ref);
            ref = LUA_NOREF;
        }
    }
};

class LuaCallBackVoid : public mge::CallBackVoid {
public:
    LuaCallBackVoid():CallBackVoid(std::bind(&LuaFunctionBind::Call, &_bind)) {}
    LuaFunctionBind& getBind() {
        return _bind;
    }
private:
    LuaFunctionBind _bind;
};

mge::Action::Ptr newCallback(mge::Widget* target, ELuna::LuaTable action) {
    auto act = new LuaCallBackVoid;
    {
        auto L = action.m_luaState;
        auto const top = lua_gettop(L);
        auto& bind = act->getBind();
        bind.L = L;
        lua_getfield(bind.L, action.m_stackPos, "func");
        if (lua_type(L, -1) == LUA_TFUNCTION) {
            bind.func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
            LOG_ERROR("error: newCallback 'func' is not function.");
            lua_pop(L, 1);
        }
        lua_getfield(bind.L, action.m_stackPos, "object");
        if (lua_type(L, -1) != LUA_TNIL) {
            bind.object_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        } else {
            lua_pop(L, 1);
        }
        assert(top == lua_gettop(L));
    }
    return mge::Action::Ptr(act);
}

mge::Action::Ptr newScaleTo(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    lutok3::State state(action.m_luaState);
    auto const top = state.getTop();
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action.get("scale");
        state.getI(scale.m_stackPos, 1);
        vec2f.y = vec2f.x = state.get();
        state.getI(scale.m_stackPos, 2);
        if (state.type(-1) != lutok3::Type::Nil) {
            vec2f.y = state.get();
        }
        state.pop(2);
    }
    assert(top == state.getTop());
    return mge::Action::Ptr(new mge::ScaleTo(target, vec2f, duration));
}

mge::Action::Ptr newScaleBy(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    lutok3::State state(action.m_luaState);
    auto const top = state.getTop();
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action.get("scale");
        state.getI(scale.m_stackPos, 1);
        vec2f.y = vec2f.x = state.get();
        state.getI(scale.m_stackPos, 2);
        if (state.type(-1) != lutok3::Type::Nil) {
            vec2f.y = state.get();
        }
        state.pop(2);
    }
    assert(top == state.getTop());
    return mge::Action::Ptr(new mge::ScaleBy(target, vec2f, duration));
}

mge::Action::Ptr newMoveTo(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    lutok3::State state(action.m_luaState);
    auto const top = state.getTop();
    mge::Vector2f vec2f = target->scale();
    {
        auto position = action.get("position");
        state.getI(position.m_stackPos, 1);
        vec2f.y = vec2f.x = state.get();
        state.getI(position.m_stackPos, 2);
        if (state.type(-1) != lutok3::Type::Nil) {
            vec2f.y = state.get();
        }
        state.pop(2);
    }
    assert(top == state.getTop());
    return mge::Action::Ptr(new mge::MoveTo(target, vec2f, duration));
}

mge::Action::Ptr newMoveBy(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    lutok3::State state(action.m_luaState);
    auto const top = state.getTop();
    mge::Vector2f vec2f = target->scale();
    {
        auto position = action.get("position");
        state.getI(position.m_stackPos, 1);
        vec2f.y = vec2f.x = state.get();
        state.getI(position.m_stackPos, 2);
        if (state.type(-1) != lutok3::Type::Nil) {
            vec2f.y = state.get();
        }
        state.pop(2);
    }
    assert(top == state.getTop());
    return mge::Action::Ptr(new mge::MoveBy(target, vec2f, duration));
}


mge::Action::Ptr newRotationTo(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    auto rotation = action.get<const char*, float>("rotation");
    return mge::Action::Ptr(new mge::RotationTo(target, rotation, duration));
}

mge::Action::Ptr newRotationBy(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    auto rotation = action.get<const char*, float>("rotation");
    return mge::Action::Ptr(new mge::RotationBy(target, rotation, duration));
}

mge::Action::Ptr newBlink(mge::Widget* target, ELuna::LuaTable action) {
    auto duration = action.get<const char*, float>("duration");
    auto times = action.get<const char*, int>("times");
    return mge::Action::Ptr(new mge::Blink(target, times, duration));
}

mge::Action::Ptr newAction(mge::Widget* target, ELuna::LuaTable action) {
    using namespace mge;
    if (!action.isValid()) {
        return nullptr;
    }
    auto const type = action.get<const char*, const char*>("type");
    auto iter = LuaActionBuilder.find(type);
    if (iter != LuaActionBuilder.end()) {
        auto act = iter->second(target, action);
        if (act != nullptr and action.has("name")) {
            act->setName(action.get<const char*, const char*>("name"));
        }
        return act;
    }
    return nullptr;
}

LuaActionHelper::LuaActionHelper(mge::Widget* target):_actionTarget(target) {

}

void LuaActionHelper::runLuaAction(ELuna::LuaTable action) {
    auto _action = newAction(_actionTarget, action);
    if (_action != nullptr) {
        _actionTarget->runAction(_action);
    }
}

void LuaActionHelper::stopLuaAction(const char* name) {
    _actionTarget->stopAction(name);
}

bool LuaActionHelper::hasLuaAction(const char* name) {
    return _actionTarget->hasAction(name);
}
