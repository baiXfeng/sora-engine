//
//  script.hpp
//
//  Created by baifeng on 2018/4/10.
//

#ifndef __STORY_SCRIPT_HPP__
#define __STORY_SCRIPT_HPP__

#include <stdio.h>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include "ELuna.h"

namespace mge {
    class FileData;
}

namespace story {

    class ScriptData {
    public:
        typedef std::function<void(int index, unsigned char* curr_buffer, long buf_size)> TraveralFunc;
    public:
        ScriptData();
        ~ScriptData();
    public:
        void load(std::shared_ptr<mge::FileData> const& data);
        void load(unsigned char* buffer, long bufSize);
        void load(unsigned char* buffer, long bufSize, std::string const& filename);
        void travel(TraveralFunc func) const;
    public:
        long getDataSize() const;
        std::string const& getFileName() const;
    private:
        void clear();
    private:
        unsigned char* mData;
        long mDataSize;
        long mLineSize;
        std::string mFileName;
    };

    class Script {
    public:
        typedef std::vector<std::string> ScriptList;
        typedef std::map<std::string, int> TagMap;
        typedef std::function<void(std::string const& line, int lineIndex, std::string const& scriptName)> ScriptStepFunc;
    public:
        Script();
        virtual ~Script();
    public:
        void load(ScriptData const& data);
    public:
        void back(int step = 1);
        void seek(std::string const& tag);
        void seek(int index);
        void step(ScriptStepFunc func);
        bool isEnd() const;
    public:
        void update(float dt);
        void pause(float seconds);
        bool isPause() const;
        void setPauseTime(int curTime, int endTime);
    public:
        int getIndex() const;
        int getCurTime() const;
        int getEndTime() const;
        std::string const& getName() const;
    protected:
        void clear();
        int getTagValue(std::string const& tag) const;
    protected:
        void onAdd(int index, unsigned char* curr_buffer, long buf_size);
    protected:
        int mIndex; // 脚本游标
        int mCurTime, mEndTime;
        std::string mName; // 脚本名字
        TagMap mTags; // 标签行号映射
        ScriptList mScript; // 剧本脚本集合
    };

    class LuaStoryScript : public Script {
    public:
        LuaStoryScript(const char* evName);
        ~LuaStoryScript();
    public:
        void load(const char* evName);
        void seek(const char* tag);
        void back(int step = 1);
        void next(int step = 1);
        int current() const;
        int step(lua_State* L);
        const char* file() const;
        bool isEnd() const;
    };

}

#endif /* script_hpp */
