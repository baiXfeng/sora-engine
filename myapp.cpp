//
// Created by baifeng on 2021/7/12.
//

#ifndef SDL2_UI_MYGAME_H
#define SDL2_UI_MYGAME_H

#include "common/game.h"
#include "common/log.h"
#include "common/widget.h"

class MyApp : public mge::Game::App {
public:
    MyApp() {}
    void init() override {
        LOG_INIT();
    }
    void update(float delta) override {
        _game.screen().update(delta);
    }
    void render(SDL_Renderer* renderer) override {
        _game.screen().render(renderer);
    }
    void fini() override {
        _game.screen().popAll();
        LOG_FINI();
    }
};

Startup(MyApp);

#endif //SDL2_UI_MYGAME_H
