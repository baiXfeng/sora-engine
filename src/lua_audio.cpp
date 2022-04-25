//
// Created by baifeng on 2022/4/23.
//

#include "lua_audio.h"
#include "common/log.h"

LuaMusic::LuaMusic(const char* name) {
    load(name);
}

LuaMusic::~LuaMusic() {

}

void LuaMusic::load(const char* name) {
    if (name == nullptr) {
        return;
    }
    if (!mge::Music::load(name)) {
        LOG_ERROR("load music %s not exist.\n", name);
    }
}

void LuaMusic::play() {
    mge::Music::play(-1);
}

void LuaMusic::pause() {
    Mix_PauseMusic();
}

void LuaMusic::resume() {
    Mix_ResumeMusic();
}

void LuaMusic::rewind() {
    Mix_RewindMusic();
}

void LuaMusic::stop() {
    this->free();
}

void LuaMusic::setVolume(int volume) {
    Mix_VolumeMusic(volume);
}

int LuaMusic::volume() {
    return Mix_VolumeMusic(-1);
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
    if (!mge::SoundEffect::load(name)) {
        LOG_ERROR("load sound %s not exist.\n", name);
    }
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

void LuaSound::stop() {
    this->free();
}

void LuaSound::setVolume(int volume) {
    Mix_Volume(_channel, volume);
}

int LuaSound::volume() {
    return Mix_Volume(_channel, -1);
}
