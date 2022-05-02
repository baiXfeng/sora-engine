//
// Created by baifeng on 2022/4/17.
//

#include "lua_action.h"
#include "common/action.h"
#include "common/log.h"
#include "lutok3/lutok3.h"

//===============================================================================

mge::Action::Ptr newAction(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newSequence(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newRepeat(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newDelay(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newCallback(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newScaleTo(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newScaleBy(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newMoveTo(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newMoveBy(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newRotationTo(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newRotationBy(mge::Widget* target, luabridge::LuaRef action);
mge::Action::Ptr newBlink(mge::Widget* target, luabridge::LuaRef action);

static std::map<std::string, std::function<mge::Action::Ptr(mge::Widget*, luabridge::LuaRef action)>> LuaActionBuilder = {
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

mge::Action::Ptr newSequence(mge::Widget* target, luabridge::LuaRef action) {
    mge::Sequence::Actions actions;
    {
        auto actions_table = action["actions"];
        auto L = action.state();
        auto const top = lua_gettop(L);
        action.push();
        auto const index = lua_gettop(L);
        lua_pushnil(L);
        while (lua_next(L, index)) {
            lua_pushvalue(L, -2);
            auto key = lua_tostring(L, -1);
            auto value = lua_tostring(L, -2);
            if (lua_type(L, -2) == LUA_TTABLE) {
                auto table = luabridge::LuaRef::fromStack(L);
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
        lua_pop(L, 1);
        assert(top == lua_gettop(L));
    }
    return mge::Action::Ptr(new mge::Sequence(actions));
}

mge::Action::Ptr newRepeat(mge::Widget* target, luabridge::LuaRef action) {
    int repeatCount = action["repeatCount"].cast<int>();
    auto act = newAction(target, action["action"]);
    return mge::Action::Ptr(new mge::Repeat(act, repeatCount));
}

mge::Action::Ptr newDelay(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    return mge::Action::Ptr(new mge::Delay(duration));
}

class LuaFunctionBind {
public:
    LuaFunctionBind(lua_State* L):func(L), object(L) {}
    ~LuaFunctionBind() {}
    void Call() {
        if (func.isNil()) {
            return;
        }
        auto L = func.state();
        auto const top = lua_gettop(L);
        {
            func.push();
            if (!object.isNil()) {
                object.push();
            }
            int argsCount = !object.isNil() ? 1 : 0;
            int ret = lua_pcall(L, argsCount, 0, 0);
            if (ret != 0) {
                LOG_ERROR("error: %s\n", lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
        assert(top == lua_gettop(L));
    }
    luabridge::LuaRef func;
    luabridge::LuaRef object;
};

class LuaCallBackVoid : public mge::CallBackVoid {
public:
    LuaCallBackVoid(lua_State* L):_bind(L), CallBackVoid(std::bind(&LuaFunctionBind::Call, &_bind)) {}
    LuaFunctionBind& getBind() {
        return _bind;
    }
private:
    LuaFunctionBind _bind;
};

mge::Action::Ptr newCallback(mge::Widget* target, luabridge::LuaRef action) {
    auto act = new LuaCallBackVoid(action.state());
    {
        auto L = action.state();
        auto const top = lua_gettop(L);
        auto& bind = act->getBind();
        bind.func = action["func"];
        if (bind.func.isNil()) {
            LOG_ERROR("error: newCallback 'func' is nilnot function.");
        } else if (!bind.func.isFunction()) {
            LOG_ERROR("error: newCallback 'func' is not function.");
        }
        bind.object = action["object"];
        assert(top == lua_gettop(L));
    }
    return mge::Action::Ptr(act);
}

mge::Action::Ptr newScaleTo(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    auto L = action.state();
    auto const top = lua_gettop(L);
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action["scale"];
        vec2f.y = vec2f.x = scale[1].cast<float>();
        if (auto y = scale[2]; !y.isNil()) {
            vec2f.y = y.cast<float>();
        }
    }
    assert(top == lua_gettop(L));
    return mge::Action::Ptr(new mge::ScaleTo(target, vec2f, duration));
}

mge::Action::Ptr newScaleBy(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    auto L = action.state();
    auto const top = lua_gettop(L);
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action["scale"];
        vec2f.y = vec2f.x = scale[1].cast<float>();
        if (auto y = scale[2]; !y.isNil()) {
            vec2f.y = y.cast<float>();
        }
    }
    assert(top == lua_gettop(L));
    return mge::Action::Ptr(new mge::ScaleBy(target, vec2f, duration));
}

mge::Action::Ptr newMoveTo(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"];
    auto L = action.state();
    auto const top = lua_gettop(L);
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action["position"];
        vec2f.y = vec2f.x = scale[1].cast<float>();
        if (auto y = scale[2]; !y.isNil()) {
            vec2f.y = y.cast<float>();
        }
    }
    assert(top == lua_gettop(L));
    return mge::Action::Ptr(new mge::MoveTo(target, vec2f, duration));
}

mge::Action::Ptr newMoveBy(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"];
    auto L = action.state();
    auto const top = lua_gettop(L);
    mge::Vector2f vec2f = target->scale();
    {
        auto scale = action["position"];
        vec2f.y = vec2f.x = scale[1].cast<float>();
        if (auto y = scale[2]; !y.isNil()) {
            vec2f.y = y.cast<float>();
        }
    }
    assert(top == lua_gettop(L));
    return mge::Action::Ptr(new mge::MoveBy(target, vec2f, duration));
}


mge::Action::Ptr newRotationTo(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    auto rotation = action["rotation"].cast<float>();
    return mge::Action::Ptr(new mge::RotationTo(target, rotation, duration));
}

mge::Action::Ptr newRotationBy(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    auto rotation = action["rotation"].cast<float>();
    return mge::Action::Ptr(new mge::RotationBy(target, rotation, duration));
}

mge::Action::Ptr newBlink(mge::Widget* target, luabridge::LuaRef action) {
    auto duration = action["duration"].cast<float>();
    auto times = action["times"].cast<int>();
    return mge::Action::Ptr(new mge::Blink(target, times, duration));
}

mge::Action::Ptr newAction(mge::Widget* target, luabridge::LuaRef action) {
    using namespace mge;
    auto const type = action["type"].cast<const char*>();
    auto iter = LuaActionBuilder.find(type);
    if (iter != LuaActionBuilder.end()) {
        auto act = iter->second(target, action);
        if (act != nullptr and !action["name"].isNil()) {
            act->setName( action["name"].cast<const char*>() );
        }
        return act;
    }
    return nullptr;
}

LuaActionHelper::LuaActionHelper(mge::Widget* target):_actionTarget(target) {

}

void LuaActionHelper::runLuaAction(luabridge::LuaRef action) {
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
