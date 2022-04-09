//
// Created by baifeng on 2022/4/3.
//

#include "lua_objects.h"
#include "lua-script/script.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "common/mouse.h"
#include "ELuna.h"

LuaScriptHelper::LuaScriptHelper() {
    initScript();
}

void LuaScriptHelper::loadScript(std::string const& fileName, const char* functionNames[], size_t nameSize) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Scene::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, functionNames, nameSize);
}

void LuaScriptHelper::initScript() {
    if (_script != nullptr) {
        return;
    }
    auto& state = _game.get<lutok3::State>("lua_state");
    _script = std::make_shared<Lua::ObjectScript>(state());
}

LuaScriptHelper::LuaScript& LuaScriptHelper::script() {
    return _script;
}

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

//===============================================================================

LuaWidgetHelper::LuaWidgetHelper(mge::Widget* target): _widget(target), _state(_game.get<lutok3::State>("lua_state")()) {

}

void LuaWidgetHelper::setLuaPosition(ELuna::LuaTable position) {
    _widget->setPosition(position.get<const char*, float>("x"), position.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaPosition() {
    auto const& pos = _widget->position();
    ELuna::LuaTable table(_state);
    table.set("x", pos.x);
    table.set("y", pos.y);
    return table;
}

void LuaWidgetHelper::setLuaSize(ELuna::LuaTable size) {
    _widget->setSize(size.get<const char*, float>("x"), size.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaSize() {
    auto const& size = _widget->size();
    ELuna::LuaTable table(_state);
    table.set("x", size.x);
    table.set("y", size.y);
    return table;
}

void LuaWidgetHelper::setLuaScale(ELuna::LuaTable scale) {
    _widget->setScale(scale.get<const char*, float>("x"), scale.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaScale() {
    auto const& scale = _widget->scale();
    ELuna::LuaTable table(_state);
    table.set("x", scale.x);
    table.set("y", scale.y);
    return table;
}

void LuaWidgetHelper::setLuaAnchor(ELuna::LuaTable anchor) {
    _widget->setAnchor(anchor.get<const char*, float>("x"), anchor.get<const char*, float>("y"));
}

ELuna::LuaTable LuaWidgetHelper::getLuaAnchor() {
    auto const& anchor = _widget->anchor();
    ELuna::LuaTable table(_state);
    table.set("x", anchor.x);
    table.set("y", anchor.y);
    return table;
}

float LuaWidgetHelper::getLuaRotation() {
    return _widget->rotation();
}

unsigned char LuaWidgetHelper::getLuaOpacity() {
    return _widget->opacity();
}

bool LuaWidgetHelper::getLuaVisible() {
    return _widget->visible();
}

int LuaWidgetHelper::getWidgetParent(lua_State* L) {
    if (_widget->parent() == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(_widget->parent());
    if (lua_script_holder == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    ELuna::push2lua_ref(L, lua_script_holder->script()->getRef());
    return 1;
}

int LuaWidgetHelper::addWidgetFromLayout(lua_State* L) {
    auto file = ELuna::read2cpp<const char*>(L, -1);
    if (file == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto node = _game.uilayout().readNode(file);
    if (node == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node.get());
    if (lua_script_holder == nullptr) {
        lua_pushnil(L);
        return 1;
    }
    _widget->addChild(node);
    node->performLayout();
    ELuna::push2lua_ref(L, lua_script_holder->script()->getRef());
    return 1;
}

//===============================================================================

bool NodeLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<::Node>()->loadScript(value);
        return true;
    } else if (ui::NodeLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<::Node>()->onLayout(parent, reader, name, value);
    }
}

const char* Node::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "update",
        "key_down",
        "key_up",
        "joy_stick",
        "touch_began",
        "touch_moved",
        "touch_ended",
        "on_assign",
        "on_layout",
};

Node::Node(): FingerResponder(this), LuaActionHelper(this), LuaWidgetHelper(this) {
    connect(ON_ENTER, [this](Widget* sender){
        _game.mouse().add(this);
    });
    connect(ON_EXIT, [this](Widget* sender){
        _game.mouse().remove(this);
    });
    enableUpdate(true);
    _script->Ref(this);
}

Node::~Node() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

void Node::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Node::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

bool Node::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Node::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

void Node::onUpdate(float delta) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_UPDATE, delta);
    }
}

void Node::onButtonDown(int key) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_ONKEYDOWN, key);
    }
}

void Node::onButtonUp(int key) {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_ONKEYUP, key);
    }
}

void Node::onJoyAxisMotion(JOYIDX joy_id, int x, int y) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", x);
        table.set("y", y);
        _script->Call(OBJECT_FUNCTION_ONJOYSTICK, (int)joy_id, table);
    }
}

bool Node::onTouchBegen(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        bool ret = _script->Call<bool>(OBJECT_FUNCTION_ONTOUCHBEGAN, table);
        return ret;
    }
    return false;
}

void Node::onTouchMoved(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        _script->Call(OBJECT_FUNCTION_ONTOUCHMOVED, table);
    }
}

void Node::onTouchEnded(mge::Vector2i const& point) {
    if (_script != nullptr) {
        ELuna::LuaTable table(_script->State());
        table.set("x", point.x);
        table.set("y", point.y);
        _script->Call(OBJECT_FUNCTION_ONTOUCHENDED, table);
    }
}

bool Node::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

//===============================================================================

Layer::Layer() {
    connect(ON_ENTER, [](Widget* sender){
        _game.gamepad().add(sender->ptr());
    });
    connect(ON_EXIT, [](Widget* sender){
        _game.gamepad().remove(sender->ptr());
    });
}

bool Layer::onTouchBegen(mge::Vector2i const& point) {
    if (_script == nullptr) {
        return true;
    }
    return Node::onTouchBegen(point);
}

//===============================================================================

const char* Image::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Image::Image(): LuaActionHelper(this), LuaWidgetHelper(this) {
    _script->Ref(this);
}

Image::~Image() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Image::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Image::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Image::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

void Image::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

bool Image::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

bool ImageLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Image>()->loadScript(value);
        return true;
    } else if (ImageWidgetLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<Image>()->onLayout(parent, reader, name, value);
    }
}

//===============================================================================

const char* Label::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "on_assign",
        "on_layout",
};

Label::Label(): LuaActionHelper(this), LuaWidgetHelper(this) {
    _script->Ref(this);
}

Label::~Label() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

bool Label::onAssignMember(mge::Widget* target, const char* name, mge::Widget* node) {
    auto lua_script_holder = dynamic_cast<LuaScriptHelper*>(node);
    if (lua_script_holder != nullptr) {
        auto& script = lua_script_holder->script();
        _script->CallAssign(OBJECT_FUNCTION_ONASSIGN, name, script->getRef());
        return true;
    }
    return false;
}

void Label::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Label::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    _script->Load(data, FunctionNames, OBJECT_FUNCTION_MAX);
}

void Label::onLayoutLoaded() {
    if (_script != nullptr) {
        _script->Call(OBJECT_FUNCTION_INIT);
    }
}

bool Label::onLayout(mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (_script != nullptr) {
        return _script->Call<bool>(OBJECT_FUNCTION_ONLAYOUT, parent, name, value);
    }
    return false;
}

bool LabelLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Label>()->loadScript(value);
        return true;
    } else if (TTFLabelLoader::onParseProperty(node, parent, reader, name, value)) {
        return true;
    } else {
        return node->fast_to<Label>()->onLayout(parent, reader, name, value);
    }
}

