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
#include "src/lua_objects.h"
#include "lutok3/lutok3.h"
#include <unistd.h>
#include "LuaBridge/LuaBridge.h"
#include "src/lua_audio.h"
#include "src/story-script.hpp"

class Test {
public:
    void print() {
        printf("hello world.\n");
    }
    void testTable(luabridge::LuaRef table) {
        //printf("%s\n", lua_typename(L, -1));
        //auto table = luabridge::LuaRef::fromStack(L, -1);
        float x = table["x"].cast<float>();
        float y = table["y"].cast<float>();
        printf("x = %f, y = %f\n", x, y);
    }
};

class MyApp : public mge::Game::App {
    lua_State* _state;
public:
    MyApp():_state(ELuna::openLua()) {}
    void init() override {
        LOG_INIT();

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
                .addFunction("removeFromParent", &mge::Widget::removeFromParent)
                .addFunction("parent", &mge::Widget::parent)
            .endClass();

        luabridge::getGlobalNamespace(_state)
            .beginNamespace("user")
                .beginClass<Test>("Test")
                    .addConstructor<void(*)()>()
                    .addFunction("print", &Test::print)
                    .addFunction("testTable", &Test::testTable)
                .endClass()
            .endNamespace();

        _game.set<lutok3::State>("lua_state", _state);
        auto data = _game.uilayout().getFileReader()->getData("assets/startup.lua");
        ELuna::doBuffer(_state, (char*)data->data(), data->size(), data->name().c_str());

        /*
        openSoraLibs(_state);
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
            ELuna::doBuffer(_state, (char*)data->data(), data->size(), data->name().c_str());
        }

        // 设置窗口标题
        lutok3::State s(_state);
        if (auto type = s.getGlobal("WINDOW_TITLE"); type == lutok3::Type::String) {
            std::string name = s.get();
            SDL_SetWindowTitle(_game.window(), name.c_str());
        }
        s.pop();
         */
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
            ELuna::closeLua(_state);
        }
        LOG_FINI();
    }
    std::string windowTitle() override {
        return "sora-engine";
    }
};

Startup(MyApp);

#endif //SDL2_UI_MYGAME_H
