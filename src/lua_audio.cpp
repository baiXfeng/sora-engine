//
// Created by baifeng on 2022/4/23.
//

#include "lua_audio.h"

LuaMusic::LuaMusic(const char* name):_channel(-1), _pause(false) {
    load(name);
}

LuaMusic::~LuaMusic() {
    _channel = -1;
}

void LuaMusic::load(const char* name) {
    if (name == nullptr) {
        return;
    }
    _pause = false;
    mge::Music::load(name);
}

void LuaMusic::play(bool restart) {
    if (restart) {
        Mix_RewindMusic();
    }
    if (_pause) {
        Mix_ResumeMusic();
        return;
    }
    _channel = mge::Music::play(-1);
}

void LuaMusic::stop() {
    _pause = true;
    Mix_PauseMusic();
}

LuaSound::LuaSound(const char* name) {
    load(name);
}

LuaSound::~LuaSound() {

}

void LuaSound::load(const char* name) {
    if (name == nullptr) {
        return;
    }
    mge::SoundEffect::load(name);
}

void LuaSound::play() {
    mge::SoundEffect::play();
}

void LuaSound::pause() {
    mge::SoundEffect::pause();
}

void LuaSound::resume() {
    mge::SoundEffect::resume();
}
