//
// Created by baifeng on 2021/6/28.
//

#ifndef MGS_GAME_H
#define MGS_GAME_H

#include <string>
#include <SDL.h>
#include "macro.h"
#include "variable.h"
#include "vector2.h"

#define GAME_SCREEN_WIDTH 960
#define GAME_SCREEN_HEIGHT 544

mge_begin

class Action;
class Game : public GameVariable {
public:
    enum _ScreenFillMode {
        STRETCHING = 0,
        FIT,
    };
    class Delegate {
        friend class Game;
    public:
        virtual ~Delegate() {}
    public:
        virtual std::string windowTitle() = 0;
        virtual Vector2i windowSize() = 0;
        virtual Vector2i displaySize() = 0;
        virtual _ScreenFillMode screenFillMode() const {
            return STRETCHING;
        }
    protected:
        virtual void init() = 0;
        virtual void fini() = 0;
        virtual void update(float delta) = 0;
        virtual void render(SDL_Renderer* renderer) = 0;
    };
    class App : public Game::Delegate {
    public:
        typedef _ScreenFillMode ScreenFillMode;
    public:
        App();
        int run();
    protected:
        std::string windowTitle() override {
            return "my_game";
        }
        Vector2i displaySize() override {
            return {GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT};
        }
        Vector2i windowSize() override {
            return displaySize();
        }
        void init() override {}
        void fini() override {}
        void update(float delta) override {}
        void render(SDL_Renderer* renderer) override {}
    };
public:
    static Game& instance();
    SDL_Renderer* renderer() const;
    SDL_Window* window() const;
    Delegate* delegate() const;
    void setDelegate(Delegate* p);
    void setRenderColor(SDL_Color const& c);
    int run();
public:
    typedef std::shared_ptr<Action> ActionPtr;
    void runActionOnMainThread(ActionPtr const& action);
private:
    Game();
    void input();
private:
    Delegate* _delegate;
    SDL_Color _color;
};

mge_end

#define _game mge::Game::instance()

#endif //MGS_GAME_H
