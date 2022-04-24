//
// Created by baifeng on 2022/4/1.
//

#include "lua_apis.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "lutok3.h"
#include "lua_objects.h"
#include "lua_widget.h"
#include "lua_audio.h"
#include "story-script.hpp"

static void registerCFunction(lua_State* L, const char* tableName, const char* funcName) {
    lutok3::State state(L);
    auto const top = state.getTop();
    {
        auto type = state.getGlobal(tableName);
        if (type == lutok3::Type::Nil) {
            state.pop();
            state.newTable();
            state.setGlobal(tableName);
            state.getGlobal(tableName);
        }
        auto index = state.getTop();
        state.push(funcName);
        state.getGlobal(funcName);
        state.setTable(index);
        state.pop();
    }
    assert(top == state.getTop());
}

static void setValueNil(lua_State* L, const char* valueName) {
    auto const top = lua_gettop(L);
    {
        lua_pushnil(L);
        lua_setglobal(L, valueName);
    }
    assert(top == lua_gettop(L));
}

#define registerCFunction2Table(lua_state, tableName, funcName, cfunction) \
{                                                                \
    ELuna::registerFunction(lua_state, funcName, cfunction);    \
    registerCFunction(lua_state, tableName, funcName);          \
    setValueNil(lua_state, funcName);                           \
}

template<typename T>
static void registerMacro(lua_State* L, const char* name, T value) {
    auto const top = lua_gettop(L);
    {
        ELuna::push2lua(L, value);
        lua_setglobal(L, name);
    }
    assert(top == lua_gettop(L));
}

void openSoraLibs(lua_State* L) {

    // 按键值
    registerMacro(L, "BUTTON_UP", (int)mge::GamePadListener::UP);
    registerMacro(L, "BUTTON_DOWN", (int)mge::GamePadListener::DOWN);
    registerMacro(L, "BUTTON_LEFT", (int)mge::GamePadListener::LEFT);
    registerMacro(L, "BUTTON_RIGHT", (int)mge::GamePadListener::RIGHT);
    registerMacro(L, "BUTTON_L1", (int)mge::GamePadListener::L1);
    registerMacro(L, "BUTTON_R1", (int)mge::GamePadListener::R1);
    registerMacro(L, "BUTTON_SELECT", (int)mge::GamePadListener::SELECT);
    registerMacro(L, "BUTTON_START", (int)mge::GamePadListener::START);
    registerMacro(L, "BUTTON_A", (int)mge::GamePadListener::A);
    registerMacro(L, "BUTTON_B", (int)mge::GamePadListener::B);
    registerMacro(L, "BUTTON_X", (int)mge::GamePadListener::X);
    registerMacro(L, "BUTTON_Y", (int)mge::GamePadListener::Y);

    // 系统
    ELuna::registerFunction(L, "import", &import);
    registerCFunction2Table(L, "scene", "push", &pushScene);
    registerCFunction2Table(L, "scene", "replace", &replaceScene);
    registerCFunction2Table(L, "scene", "pop", &popScene);

    // UI
    _game.uilayout().setLoader(mge::XmlLayout::LoaderPool(new ui::LoaderPool));
    _game.uilayout().getLoaderPool()->addLoader<ui::WidgetLoader>("Layout");
    _game.uilayout().getLoaderPool()->addLoader<LayerLoader>("Layer");
    _game.uilayout().getLoaderPool()->addLoader<NodeLoader>("Node");
    _game.uilayout().getLoaderPool()->addLoader<ImageLoader>("Image");
    _game.uilayout().getLoaderPool()->addLoader<LabelLoader>("Label");
    _game.uilayout().getLoaderPool()->addLoader<MaskLoader>("Mask");

    // 类
    registerClass(L);
}

template<typename T>
void registerClass(lua_State* L, const char* className) {
    ELuna::registerClass<T>(L, className, ELuna::constructor<T>);
    ELuna::registerMethod<T, void, ELuna::LuaTable>(L, "runAction", &T::runLuaAction);
    ELuna::registerMethod<T, void, const char*>(L, "stopAction", &T::stopLuaAction);
    ELuna::registerMethod<T, bool, const char*>(L, "hasAction", &T::hasLuaAction);
    ELuna::registerMethod<T, void, ELuna::LuaTable>(L, "setPosition", &T::setLuaPosition);
    ELuna::registerMethod<T, ELuna::LuaTable>(L, "position", &T::getLuaPosition);
    ELuna::registerMethod<T, void, ELuna::LuaTable>(L, "setSize", &T::setLuaSize);
    ELuna::registerMethod<T, ELuna::LuaTable>(L, "size", &T::getLuaSize);
    ELuna::registerMethod<T, void, ELuna::LuaTable>(L, "setScale", &T::setLuaScale);
    ELuna::registerMethod<T, ELuna::LuaTable>(L, "scale", &T::getLuaScale);
    ELuna::registerMethod<T, void, ELuna::LuaTable>(L, "setAnchor", &T::setLuaAnchor);
    ELuna::registerMethod<T, ELuna::LuaTable>(L, "anchor", &T::getLuaAnchor);
    ELuna::registerMethod<T, void, float>(L, "setRotation", &T::setRotation);
    ELuna::registerMethod<T, float>(L, "rotation", &T::getLuaRotation);
    ELuna::registerMethod<T, void, unsigned char>(L, "setOpacity", &T::setOpacity);
    ELuna::registerMethod<T, unsigned char>(L, "opacity", &T::getLuaOpacity);
    ELuna::registerMethod<T, void, bool>(L, "setVisible", &T::setVisible);
    ELuna::registerMethod<T, bool>(L, "visible", &T::getLuaVisible);
    ELuna::registerMethod<T, void, bool>(L, "setClip", &T::enableClip);
    ELuna::registerMethod<T, void>(L, "removeFromParent", &T::removeFromParent);
    ELuna::registerMethod<T>(L, "parent", &T::getWidgetParent);
    ELuna::registerMethod<T>(L, "add", &T::addWidgetFromLayout);
}

void registerWidget(lua_State* L) {
    using namespace mge;
    ELuna::registerClass<Widget>(L, "Widget", ELuna::constructor<Widget>);
    ELuna::registerMethod<Widget>(L, "runAction", &widgetRunAction);
    ELuna::registerMethod<Widget>(L, "stopAction", &widgetStopAction);
    ELuna::registerMethod<Widget>(L, "hasAction", &widgetHasAction);
    ELuna::registerMethod<Widget>(L, "setPosition", &widgetSetPosition);
    ELuna::registerMethod<Widget>(L, "position", &widgetGetPosition);
    ELuna::registerMethod<Widget>(L, "setSize", &widgetSetSize);
    ELuna::registerMethod<Widget>(L, "size", &widgetGetSize);
    ELuna::registerMethod<Widget>(L, "setScale", &widgetSetScale);
    ELuna::registerMethod<Widget>(L, "scale", &widgetGetScale);
    ELuna::registerMethod<Widget>(L, "setAnchor", &widgetSetAnchor);
    ELuna::registerMethod<Widget>(L, "anchor", &widgetGetAnchor);
    ELuna::registerMethod<Widget>(L, "setRotation", &Widget::setRotation);
    ELuna::registerMethod<Widget>(L, "rotation", &widgetGetRotation);
    ELuna::registerMethod<Widget>(L, "setOpacity", &Widget::setOpacity);
    ELuna::registerMethod<Widget>(L, "opacity", &widgetGetOpacity);
    ELuna::registerMethod<Widget, void, bool>(L, "setVisible", &Widget::setVisible);
    ELuna::registerMethod<Widget>(L, "visible", &widgetGetVisible);
    ELuna::registerMethod<Widget, void, bool>(L, "setClip", &Widget::enableClip);
    ELuna::registerMethod<Widget, void>(L, "removeFromParent", &Widget::removeFromParent);
    ELuna::registerMethod<Widget>(L, "parent", &widgetGetParent);
    ELuna::registerMethod<Widget>(L, "add", &widgetAddLayout);
}

void registerStoryScript(lua_State* L) {
    using namespace story;
    ELuna::registerClass<LuaStoryScript>(L, "StoryScript", ELuna::constructor<LuaStoryScript, const char*>);
    ELuna::registerMethod<LuaStoryScript>(L, "load", &LuaStoryScript::load);
    ELuna::registerMethod<LuaStoryScript, void, int>(L, "back", &LuaStoryScript::back);
    ELuna::registerMethod<LuaStoryScript, void, int>(L, "next", &LuaStoryScript::next);
    ELuna::registerMethod<LuaStoryScript, void, const char*>(L, "seek", &LuaStoryScript::seek);
    ELuna::registerMethod<LuaStoryScript, bool>(L, "end", &LuaStoryScript::isEnd);
    ELuna::registerMethod<LuaStoryScript, int, lua_State*>(L, "step", &LuaStoryScript::step);
    ELuna::registerMethod<LuaStoryScript, int>(L, "curr", &LuaStoryScript::current);
    ELuna::registerMethod<LuaStoryScript, const char*>(L, "file", &LuaStoryScript::file);
}

void registerSoundSystem(lua_State* L) {
    ELuna::registerClass<LuaMusic>(L, "Music", ELuna::constructor<LuaMusic, const char*>);
    ELuna::registerMethod<LuaMusic>(L, "load", &LuaMusic::load);
    ELuna::registerMethod<LuaMusic>(L, "play", &LuaMusic::play);
    ELuna::registerMethod<LuaMusic>(L, "stop", &LuaMusic::stop);

    ELuna::registerClass<LuaSound>(L, "Sound", ELuna::constructor<LuaSound, const char*>);
    ELuna::registerMethod<LuaSound>(L, "load", &LuaSound::load);
    ELuna::registerMethod<LuaSound>(L, "play", &LuaSound::play);
    ELuna::registerMethod<LuaSound>(L, "pause", &LuaSound::pause);
    ELuna::registerMethod<LuaSound>(L, "resume", &LuaSound::resume);
}

void registerClass(lua_State* L) {
    registerClass<Layer>(L, "Layer");
    registerClass<Node>(L, "Node");
    registerClass<Image>(L, "Image");
    registerClass<Label>(L, "Label");
    registerClass<Mask>(L, "Mask");

    registerSoundSystem(L);
    registerStoryScript(L);
    registerWidget(L);

    ELuna::registerMethod<Mask>(L, "setColor", &Mask::setColor);
    ELuna::registerMethod<Mask>(L, "color", &Mask::getColor);
}

void import(const char* luaFileName) {
    auto& state = _game.get<lutok3::State>("lua_state");
    auto data = _game.uilayout().getFileReader()->getData(luaFileName);
    if (!data->empty()) {
        auto ret = luaL_loadbuffer(state(), (char*)data->data(), (size_t)data->size(), data->name().c_str());
        if (ret) {
            LOG("error: %s\n", lua_tostring(state(), -1));
            lua_pop(state(), 1);
        } else if (ret = lua_pcall(state(), 0, 0, 0); ret != 0) {
            LOG("error: %s\n", lua_tostring(state(), -1));
            lua_pop(state(), 1);
        }
    } else {
        LOG_ERROR("error: import %s not exist.", luaFileName);
    }
}

void pushScene(const char* xmlFileName) {
    auto scene = _game.uilayout().readNode(xmlFileName);
    if (scene == nullptr) {
        LOG_ERROR("pushScene fail: %s node is null.\n", xmlFileName);
        return;
    }
    _game.screen().push(scene);
}

void replaceScene(const char* xmlFileName) {
    auto scene = _game.uilayout().readNode(xmlFileName);
    if (scene == nullptr) {
        LOG_ERROR("replaceScene fail: %s node is null.\n", xmlFileName);
        return;
    }
    _game.screen().replace(scene);
}

void popScene() {
    _game.screen().pop();
}
