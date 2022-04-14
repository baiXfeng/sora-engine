//
//  script.cpp
//  HelloTuiCpp
//
//  Created by baifeng on 2018/4/10.
//

#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <list>
#include "story-script.hpp"
#include "common/file-reader.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/log.h"

namespace story {

    static std::vector<std::string> split(std::string const& str, std::string const& pat) {
        using namespace std;
        vector<string> strvec;
        string::size_type pos1 = 0, pos2 = str.find(pat);
        while (string::npos != pos2) {
            strvec.push_back(str.substr(pos1, pos2 - pos1));
            pos1 = pos2 + pat.length();
            pos2 = str.find(pat, pos1);
        }
        strvec.push_back(str.substr(pos1));
        return strvec;
    }

//=====================================================================

    ScriptData::ScriptData():mData(NULL),mDataSize(0),mLineSize(0) {

    }

    ScriptData::~ScriptData() {
        clear();
    }

    void ScriptData::clear() {
        if (this->mData != NULL) {
            free(this->mData);
            this->mData = NULL;
        }
        this->mDataSize = 0;
        this->mLineSize = 0;
    }

    void ScriptData::load(std::shared_ptr<mge::FileData> const& data) {
        load(data->data(), data->size(), data->name());
    }

    void ScriptData::load(unsigned char* buffer, long bufSize) {

        std::string s((char*)buffer, bufSize);
        //分割换行符
        auto list = split(s, "\n");
        //printf("line: %d\n", (int)list.size());
        /*for (int i=0; i < (int)list.size(); i++) {
            std::string& line = list[i];
            // 过滤无效字符
            removeInvalidSymbol(line);
        }*/

        std::vector<std::string> array;
        array.reserve(list.size());
        for (int i=0; i < (int)list.size(); i++) {
            if (list[i] == "[[") {
                std::string line;
                do {
                    ++i;
                    assert(i < (int) list.size() || "ScriptData::load block([[...]]) no match.");
                    if (list[i] == "]]") {
                        break;
                    }
                    line += list[i];
                } while (true);
                if (line.length() != 0) {
                    array.push_back(line);
                }
                continue;
            } else if (list[i] == "\n" or list[i] == "\r\n") {
                continue;
            }
            if (list[i].length() != 0) {
                array.push_back(list[i]);
            }
        }

        int realSize = 0;
        std::vector<int> indexArray;
        indexArray.reserve(array.size());

        for (int i=0; i < array.size(); i++) {
            std::string const& line = array[i];
            int size = (int)line.size();
            if (size != 0) {
                indexArray.push_back(i);
                realSize += size;
            }
        }
        clear();

        this->mDataSize = realSize+sizeof(int)*indexArray.size();
        this->mData = (unsigned char*)malloc(this->mDataSize);
        this->mLineSize = indexArray.size();
        memset(this->mData, 0, this->mDataSize);

        int offset = 0;
        for (int i=0; i < indexArray.size(); i++) {
            int index = (int)indexArray[i];
            std::string const& line = array[index];
            int size = (int)line.size();
            unsigned char* curr = this->mData + offset;
            memcpy(curr, &size, sizeof(int));
            curr += sizeof(int);
            memcpy(curr, line.data(), line.size());
            offset += sizeof(int) + size;
        }
    }

    void ScriptData::load(unsigned char* buffer, long bufSize, std::string const& filename) {
        this->load(buffer, bufSize);
        this->mFileName = filename;
    }

    void ScriptData::travel(TraveralFunc func) const {
        int offset = 0;
        for (int i=0; i < this->mLineSize; i++) {
            unsigned char* buffer = this->mData + offset;
            int bufSize = 0;
            memcpy(&bufSize, buffer, sizeof(int));
            func(i, buffer+sizeof(int), bufSize);
            offset += bufSize + sizeof(int);
        }
    }

    long ScriptData::getDataSize() const {
        return this->mDataSize;
    }

    std::string const& ScriptData::getFileName() const {
        return this->mFileName;
    }

//=====================================================================

    Script::Script():mIndex(0), mCurTime(0), mEndTime(0) {
        mScript.reserve(1024);
    }

    Script::~Script() {
        clear();
    }

    void Script::load(ScriptData const& data) {
        clear();
        data.travel(std::bind(
                &Script::onAdd, this,
                std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
        );
        this->mName = data.getFileName();
    }

    void Script::back(int step) {
        mIndex -= fabs((float)step);
    }

    void Script::seek(std::string const& tag) {
        int linenum = getTagValue(tag);
        if (linenum >= 0) {
            this->seek(linenum);
        }
    }

    void Script::seek(int index) {
        if (index >= 0) {
            mIndex = index;
        }
    }

    void Script::step(ScriptStepFunc func) {
        if (isEnd()) {
            return;
        }
        std::string const& line = mScript[mIndex];
        func(line, mIndex++, mName);
    }

    bool Script::isEnd() {
        return mIndex >= mScript.size();
    }

    void Script::update(float dt) {
        if (isPause()) {
            this->mCurTime += dt * 1000;
            if (this->mCurTime >= this->mEndTime) {
                this->mCurTime = 0;
                this->mEndTime = 0;
            }
        }
    }

    void Script::pause(float seconds) {
        this->mCurTime = 0;
        this->mEndTime = seconds * 1000;
        if (this->mEndTime < 0) {
            this->mEndTime = 0x0fffffff;
        }
    }

    bool Script::isPause() const {
        return this->mCurTime != this->mEndTime;
    }

    void Script::setPauseTime(int curTime, int endTime) {
        this->mCurTime = curTime;
        this->mEndTime = endTime;
    }

    int Script::getIndex() const {
        return mIndex;
    }
    int Script::getCurTime() const {
        return mCurTime;
    }
    int Script::getEndTime() const {
        return mEndTime;
    }
    std::string const& Script::getName() const {
        return mName;
    }

    void Script::clear() {
        mScript.clear();
        mTags.clear();
        mIndex = 0;
    }

    int Script::getTagValue(std::string const& tag) const {
        TagMap::const_iterator iter = mTags.find(tag);
        if (mTags.end() == iter) {
            return -1;
        }
        return iter->second;
    }

    void Script::onAdd(int index, unsigned char* curr_buffer, long buf_size) {
        std::string line((char*)curr_buffer, buf_size);
        mScript.push_back(line);
        if (line[0] == '*') {
            mTags[line] = index;
        }
    }

    LuaStoryScript::LuaStoryScript(const char* evName) {
        if (evName == nullptr) {
            return;
        }
        load(evName);
    }

    void LuaStoryScript::load(const char* evName) {
        auto data = _game.uilayout().getFileReader()->getData(evName);
        if (data->empty()) {
            LOG_DEBUG("LuaScript::load %s fail, file not exist.\n", evName);
            return;
        }
        ScriptData sd;
        sd.load(data);
        Script::load(sd);
    }

    void LuaStoryScript::seek(const char* tag) {
        Script::seek(tag);
    }

    void LuaStoryScript::next(int step) {
        Script::seek(getIndex() + step);
    }

    int LuaStoryScript::current() {
        return getIndex();
    }

    int LuaStoryScript::step(lua_State* L) {
        if (isEnd()) {
            return 0;
        }
        std::string const& line = mScript[mIndex];
        if (line[0] == '*') {
            return 0;
        }
        ELuna::doBuffer(L, line.data(), line.size(), (mName+"-index-"+std::to_string(mIndex)).c_str());
        return 0;
    }

    const char* LuaStoryScript::file() {
        return mName.c_str();
    }

}
