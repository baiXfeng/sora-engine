//
// Created by baifeng on 2022/4/3.
//

#include "lua_objects.h"
#include "lua-script/script.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include "common/log.h"
#include "ELuna.h"

void registerClass(lua_State* L) {
    ELuna::registerClass<Scene>(L, "Scene", ELuna::constructor<Scene>);

    //ELuna::registerMetatable<>()
}

void SceneLoader::onParseProperty(mge::Widget* node, mge::Widget* parent, ui::LayoutReader* reader, const char* name, const char* value) {
    if (strcmp(name, "Script") == 0) {
        node->fast_to<Scene>()->loadScript(value);
    } else {
        NodeLoader::onParseProperty(node, parent, reader, name, value);
    }
}

const char* Scene::FunctionNames[OBJECT_FUNCTION_MAX] = {
        "init",
        "release",
        "update",
        "keydown",
        "keyup",
        "touch_began",
        "touch_moved",
        "touch_ended",
};

Scene::Scene() {

}

Scene::~Scene() {
    if (_script.get()) {
        _script->Call(OBJECT_FUNCTION_RELEASE);
    }
}

void Scene::loadScript(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data->empty()) {
        LOG_ERROR("error: Scene::loadScript fail, <%s> not exist.", fileName.c_str());
        return;
    }
    auto& state = _game.get<lutok3::State>("lua_state");
    _script = std::make_shared<Lua::ObjectScript>(state(), data, FunctionNames, OBJECT_FUNCTION_MAX);
    if (_script.get()) {
        _script->Ref(this);
    }
}

void Scene::onLayoutLoaded() {
    if (_script == nullptr) {
        return;
    }
    _script->Call(OBJECT_FUNCTION_INIT);
}