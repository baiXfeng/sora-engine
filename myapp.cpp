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
#include "ELuna.h"
#include "lutok3.h"

class MyApp : public mge::Game::App {
    lua_State* _state;
public:
    MyApp():_state(ELuna::openLua()) {}
    void init() override {
        LOG_INIT();
        auto data = _game.uilayout().getFileReader()->getData("assets/startup.lua");
        if (data->empty()) {
            LOG_ERROR("assets/startup.lua not exist.\n");
        } else {
            openSoraLibs(_state);
            _game.set<lutok3::State>("lua_state", _state);
            ELuna::doBuffer(_state, (char*)data->data(), data->size(), data->name().c_str());
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
