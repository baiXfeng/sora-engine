//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_MYGAME_H
#define SDL2_UI_MYGAME_H

#include "common/game.h"
#include "common/log.h"
#include "common/widget.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "src/lua_apis.h"
#include "lutok3/lutok3.h"
#include "LuaBridge/LuaBridge.h"
#include "src/lua_audio.h"
#include "src/story-script.hpp"
#include "src/lua_widget.h"
#include <unistd.h>

class MyApp : public mge::Game::App {
    lua_State* _state;
public:
    MyApp():_state(luaL_newstate()) {}
    void init() override {
        LOG_INIT();

        luaopen_base(_state);
        luaL_openlibs(_state);

        std::function<luabridge::LuaRef(mge::Widget*)> widgetGetPosition = [this](mge::Widget* w) {
            auto table = luabridge::newTable(_state);
            table["x"].rawset(w->position().x);
            table["y"].rawset(w->position().y);
            return table;
        };

        std::function<luabridge::LuaRef(mge::Widget*)> widgetGetSize = [this](mge::Widget* w) {
            auto table = luabridge::newTable(_state);
            table["x"].rawset(w->size().x);
            table["y"].rawset(w->size().y);
            return table;
        };

        std::function<luabridge::LuaRef(mge::Widget*)> widgetGetScale = [this](mge::Widget* w) {
            auto table = luabridge::newTable(_state);
            table["x"].rawset(w->scale().x);
            table["y"].rawset(w->scale().y);
            return table;
        };

        std::function<luabridge::LuaRef(mge::Widget*)> widgetGetAnchor = [this](mge::Widget* w) {
            auto table = luabridge::newTable(_state);
            table["x"].rawset(w->anchor().x);
            table["y"].rawset(w->anchor().y);
            return table;
        };

        luabridge::getGlobalNamespace(_state)
            .addFunction("import", &import)
            .beginNamespace("scene")
                .addFunction("push", &pushScene)
                .addFunction("replace", &replaceScene)
                .addFunction("pop", &popScene)
            .endNamespace()
            .beginClass<LuaMusic>("Music")
                .addConstructor<void(*)(const char*)>()
                .addFunction("load", &LuaMusic::load)
                .addFunction("play", &LuaMusic::play)
                .addFunction("pause", &LuaMusic::pause)
                .addFunction("resume", &LuaMusic::resume)
                .addFunction("rewind", &LuaMusic::rewind)
                .addFunction("stop", &LuaMusic::stop)
                .addFunction("setVolume", &LuaMusic::setVolume)
                .addFunction("volume", &LuaMusic::volume)
            .endClass()
            .beginClass<LuaSound>("Sound")
                .addConstructor<void(*)(const char*)>()
                .addFunction("load", &LuaSound::load)
                .addFunction("play", &LuaSound::play)
                .addFunction("pause", &LuaSound::pause)
                .addFunction("resume", &LuaSound::resume)
                .addFunction("stop", &LuaSound::stop)
                .addFunction("setVolume", &LuaSound::setVolume)
                .addFunction("volume", &LuaSound::volume)
            .endClass()
            .beginClass<story::LuaStoryScript>("StoryScript")
                .addConstructor<void(*)(const char*)>()
                .addFunction("load", &story::LuaStoryScript::load)
                .addFunction("back", &story::LuaStoryScript::back)
                .addFunction("next", &story::LuaStoryScript::next)
                .addFunction("seek", &story::LuaStoryScript::seek)
                .addFunction("isEnd", &story::LuaStoryScript::isEnd)
                .addFunction("step", &story::LuaStoryScript::step)
                .addFunction("current", &story::LuaStoryScript::current)
                .addFunction("file", &story::LuaStoryScript::file)
            .endClass()
            .beginClass<mge::Widget>("Widget")
                .addConstructor<void(*)()>()
                .addFunction("runAction", &widgetRunAction)
                .addFunction("stopAction", &widgetStopAction)
                .addFunction("hasAction", &widgetHasAction)
                .addFunction("setPosition", &widgetSetPosition)
                .addFunction("position", std::move(widgetGetPosition))
                .addFunction("setSize", &widgetSetSize)
                .addFunction("size", std::move(widgetGetSize))
                .addFunction("setScale", &widgetSetScale)
                .addFunction("scale", std::move(widgetGetScale))
                .addFunction("setAnchor", &widgetSetAnchor)
                .addFunction("anchor", std::move(widgetGetAnchor))
                .addFunction("setRotation", &mge::Widget::setRotation)
                .addFunction("rotation", &mge::Widget::rotation)
                .addFunction("setOpacity", &mge::Widget::setOpacity)
                .addFunction("opacity", &mge::Widget::opacity)
                .addFunction("setVisible", &mge::Widget::setVisible)
                .addFunction("visible", &mge::Widget::visible)
                .addFunction("setClip", &mge::Widget::enableClip)
                .addFunction("setRenderTarget", &mge::Widget::enableRenderTarget)
                .addFunction("removeFromParent", &mge::Widget::removeFromParent)
                .addFunction("parent", &mge::Widget::parent)
                .addFunction("add", &widgetAddLayout)
            .endClass()
            .deriveClass<Node, mge::Widget>("Node")
                .addConstructor<void(*)()>()
            .endClass()
            .deriveClass<Layer, Node>("Layer")
                .addConstructor<void(*)()>()
            .endClass()
            .deriveClass<Image, mge::Widget>("Image")
                .addConstructor<void(*)()>()
            .endClass()
            .deriveClass<Label, mge::Widget>("Label")
                .addConstructor<void(*)()>()
            .endClass()
            .deriveClass<Mask, mge::Widget>("Mask")
                .addConstructor<void(*)()>()
                .addFunction("setColor", &Mask::setColor)
                .addFunction("color", &Mask::getColor)
            .endClass();

        registerMacro(_state, "BUTTON_UP", (int)mge::GamePadListener::UP);
        registerMacro(_state, "BUTTON_DOWN", (int)mge::GamePadListener::DOWN);
        registerMacro(_state, "BUTTON_LEFT", (int)mge::GamePadListener::LEFT);
        registerMacro(_state, "BUTTON_RIGHT", (int)mge::GamePadListener::RIGHT);
        registerMacro(_state, "BUTTON_L1", (int)mge::GamePadListener::L1);
        registerMacro(_state, "BUTTON_R1", (int)mge::GamePadListener::R1);
        registerMacro(_state, "BUTTON_SELECT", (int)mge::GamePadListener::SELECT);
        registerMacro(_state, "BUTTON_START", (int)mge::GamePadListener::START);
        registerMacro(_state, "BUTTON_A", (int)mge::GamePadListener::A);
        registerMacro(_state, "BUTTON_B", (int)mge::GamePadListener::B);
        registerMacro(_state, "BUTTON_X", (int)mge::GamePadListener::X);
        registerMacro(_state, "BUTTON_Y", (int)mge::GamePadListener::Y);

        _game.uilayout().setLoader(mge::XmlLayout::LoaderPool(new ui::LoaderPool));
        _game.uilayout().getLoaderPool()->addLoader<ui::WidgetLoader>("Layout");
        _game.uilayout().getLoaderPool()->addLoader<LayerLoader>("Layer");
        _game.uilayout().getLoaderPool()->addLoader<NodeLoader>("Node");
        _game.uilayout().getLoaderPool()->addLoader<ImageLoader>("Image");
        _game.uilayout().getLoaderPool()->addLoader<LabelLoader>("Label");
        _game.uilayout().getLoaderPool()->addLoader<MaskLoader>("Mask");

        _game.set<lutok3::State>("lua_state", _state);
        auto data = _game.uilayout().getFileReader()->getData("assets/startup.lua");

#if defined(WIN32) or defined(__WIN32__)
        // 重定向根目录
        if (data->empty()) {
            chdir("../../");
            data = _game.uilayout().getFileReader()->getData("assets/startup.lua");
        }
#endif

        if (data->empty()) {
            LOG_ERROR("assets/startup.lua not exist.\n");
        } else {
            doBuffer(_state, (char*)data->data(), data->size(), data->name().c_str());
        }
    }
    void update(float delta) override {
        _game.screen().update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        _game.screen().render(renderer);
    }
    void fini() override {
        _game.screen().popAll();
        if (_state != 0) {
            lua_close(_state);
            _state = 0;
        }
        LOG_FINI();
    }
    std::string windowTitle() override {
        return "sora-engine";
    }
};

Startup(MyApp);

#endif //SDL2_UI_MYGAME_H
