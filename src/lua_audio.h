//
// Created by baifeng on 2022/4/23.
//

#ifndef SDL2_UI_LUA_AUDIO_H
#define SDL2_UI_LUA_AUDIO_H

#include "common/audio.h"

class LuaMusic : protected mge::Music {
public:
    LuaMusic(const char* name);
    ~LuaMusic();
public:
    void load(const char* name);
    void play();
    void pause();
    void resume();
    void rewind();
    void stop();
    void setVolume(int volume);
    int volume();
};

class LuaSound : protected mge::SoundEffect {
public:
    LuaSound(const char* name);
    ~LuaSound();
public:
    void load(const char* name);
    void play();
    void pause();
    void resume();
    void stop();
    void setVolume(int volume);
    int volume();
};

#endif //SDL2_UI_LUA_AUDIO_H
