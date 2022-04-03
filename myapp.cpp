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
#include "lua-script/script.h"
#include "src/lua_apis.h"
#include "src/lua_objects.h"
#include "ELuna.h"
#include "lutok3.h"

class MyApp : public mge::Game::App {
    lua_State* _state;
public:
    MyApp():_state(0) {}
    void init() override {
        LOG_INIT();
        auto data = _game.uilayout().getFileReader()->getData("assets/startup.lua");
        if (data->empty()) {
            LOG("assets/startup.lua not exist.\n");
        } else {
            _state = ELuna::openLua();
            openSoraLibs(_state);
            _game.set<lutok3::State>("lua_state", _state);
            auto const top = lua_gettop(_state);
            {
                auto ret = luaL_loadbuffer(_state, (char*)data->data(), (size_t)data->size(), data->name().c_str());
                auto a = lua_gettop(_state);
                if (ret) {
                    LOG("error: %s\n", lua_tostring(_state, -1));
                    lua_pop(_state, 1);
                } else if (ret = lua_pcall(_state, 0, 0, 0); ret != 0) {
                    LOG("error: %s\n", lua_tostring(_state, -1));
                    lua_pop(_state, 1);
                }
                auto b = lua_gettop(_state);
            }
            assert(top == lua_gettop(_state));
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
            ELuna::closeLua(_state);
        }
        LOG_FINI();
    }
};

Startup(MyApp);

#endif //SDL2_UI_MYGAME_H
