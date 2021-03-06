//
// Created by baifeng on 2021/9/24.
//

#include "widget.h"
#include "render.h"
#include "game.h"
#include "assert.h"
#include "loadres.h"
#include "action.h"
#include "font.h"
#include "mouse.h"
#include <string.h>

mge_begin

static int _widgetCount = 0;

void TestWidget() {
    auto root = std::make_shared<Widget>();
    auto first = std::make_shared<Widget>();
    auto second = std::make_shared<Widget>();
    root->addChild(first);
    first->addChild(second);
    assert(first->children().size() == 1 and "widget children size error.");

    first->addChild(second);
    assert(first->children().size() == 1 and "widget children size error.");

    first->setPosition(100, 100);
    second->setPosition(20, 20);
    first->update(0.0f);
    auto second_position = second->global_position().to<int>();
    Vector2i target_position{120, 120};
    assert(second_position == target_position and "widget global position error.");

    root->removeChild(first);
    root = nullptr;
    first = nullptr;
    second = nullptr;
    assert(_widgetCount == 0 and "widget memory leak.");
}

//=====================================================================================

WidgetSignal::SignalType::slot_type WidgetSignal::connect(int type, SignalType::observer_type const& obs) {
    return signal(type).connect(obs);
}

void WidgetSignal::disconnect(int type, SignalType::slot_type const& obs) {
    signal(type).disconnect(obs);
}

WidgetSignal::SignalType& WidgetSignal::signal(int type) {
    return _signals[type];
}

//=====================================================================================

std::vector<SDL_Rect> Widget::_clipStack;

Widget::Widget():
_parent(nullptr),
_userdata(nullptr),
_visible(true),
_update(false),
_clip(false),
_renderTarget(false),
_touchEnable(true),
_pause_action_when_hidden(false),
_dirty(true),
_action(ActionExecuterPtr(new ActionExecuter)),
_rotation(0.0f),
_position({0.0f, 0.0f}),
_global_position({0.0f, 0.0f}),
_size({0.0f, 0.0f}),
_global_size({0.0f, 0.0f}),
_anchor({0.0f, 0.0f}),
_scale({1.0f, 1.0f}),
_opacity(255) {
    _children.reserve(10);
    ++_widgetCount;
    setSize(_game.delegate()->displaySize().to<float>());
    _global_size = _size;
}

Widget::~Widget() {
    removeAllChildren();
    --_widgetCount;
    //printf("widget size = %d\n", _widgetCount);
}

Widget* Widget::parent() const {
    return _parent;
}

Widget* Widget::root() {
    Widget* _root = this;
    while (_root->_parent != nullptr) {
        _root = _root->_parent;
    }
    return _root;
}

Widget::Ptr Widget::ptr() const {
    if (_parent != nullptr) {
        for (auto& child : _parent->_children) {
            if (child.get() == this) {
                return child;
            }
        }
    }
    return nullptr;
}

bool Widget::visible() const {
    return _visible;
}

void Widget::defer(std::function<void()> const& func, float delay) {
    auto delay_action = Action::Ptr(new Delay(delay));
    auto callback = Action::Ptr(new CallBackVoid(func));
    auto action = Action::Ptr(new Sequence({delay_action, callback}));
    this->runAction(action);
}

void Widget::defer(Widget* sender, std::function<void(Widget*)> const& func, float delay) {
    auto delay_action = Action::Ptr(new Delay(delay));
    auto callback = Action::Ptr(new CallBackSender(sender, func));
    auto action = Action::Ptr(new Sequence({delay_action, callback}));
    this->runAction(action);
}

void Widget::enableUpdate(bool update) {
    _update = update;
}

void Widget::enableClip(bool clip) {
    _clip = clip;
}

void Widget::enableRenderTarget(bool e, bool force) {
    if (e) {
        if (_render != nullptr and !force) {
            return;
        }
        _render = std::make_shared<RenderCopyEx>();
        auto const& size = this->size();
        assert(size.x > 0 and size.y > 0 and "Widget::enableRenderTarget error.");
        auto texture = SDL_CreateTexture(
                _game.renderer(),
                SDL_PIXELFORMAT_RGBA8888,
                SDL_TEXTUREACCESS_TARGET,
                size.x,
                size.y
        );
        if (texture) {
            //some problem on psvita
            //SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
        }
        _render->setTexture(Texture::Ptr(new Texture(texture)));
    } else {
        _render = nullptr;
    }
    _renderTarget = e;
}

void Widget::setVisible(bool visible) {
    _visible = visible;
    this->onVisible(_visible);
}

void Widget::performLayout() {
    this->modifyLayout();
}

bool Widget::isTouchEnabled() const {
    return _touchEnable;
}

void Widget::setTouchEnable(bool b) {
    _touchEnable = b;
}

Vector2f Widget::covertToWorldPosition(Vector2f const& local_position) {
    return global_position() + local_position;
}

Vector2f Widget::covertToLocalPosition(Vector2f const& world_position) {
    return world_position - global_position();
}

void Widget::addChild(WidgetPtr const& widget) {
    addChild(widget, _children.size());
}

void Widget::addChild(WidgetPtr const& widget, int index) {
    if (widget->_parent) {
        widget->_parent->removeChild(widget);
        widget->_parent = nullptr;
    }
    if (index >= _children.size()) {
        _children.push_back(widget);
    } else {
        _children.insert(_children.begin() + (index <= 0 ? 0 : index), widget);
    }
    widget->_parent = this;
    widget->enter();
}

void Widget::removeChild(WidgetPtr const& widget) {
    this->removeChild(widget.get());
}

void Widget::removeChild(Widget* widget) {
    for (auto iter = _children.begin(); iter != _children.end(); iter++) {
        if (iter->get() == widget) {
            root()->runAction(Action::Ptr(new KeepAlive<Widget>(*iter))); // ???????????????????????????
            widget->exit();
            widget->_parent = nullptr;
            _children.erase(iter);
            return;
        }
    }
}

void Widget::removeAllChildren() {
    for (int i = _children.size()-1; i >= 0; --i) {
        auto& child = _children[i];
        child->exit();
        child->_parent = nullptr;
    }
    _children.clear();
}

void Widget::removeFromParent() {
    if (_parent) {
        _parent->removeChild(this);
    }
}

Widget::WidgetArray& Widget::children() {
    return _children;
}

Widget::WidgetArray const& Widget::children() const {
    return _children;
}

void Widget::update(float delta) {
    bool update = _visible and _update;
    bool action_update = not _pause_action_when_hidden;
    if (_visible or action_update) {
        _action->update(delta);
    }
    if (_dirty) {
        this->modifyLayout();
        _dirty = false;
    }
    if (not update) {
        return;
    }
    this->onUpdate(delta);
    if (_children.empty()) {
        return;
    }
    WidgetArray list(_children);
    for (auto child : list) {
        if (child->parent() == nullptr) {
            // child is removed, but KeepAlive.
            continue;
        }
        child->update(delta);
    }
}

void Widget::draw(SDL_Renderer* renderer) {
    if (not _visible) {
        return;
    }
    if (_clip) {
        if (_renderTarget) {
            this->push_clip({
                int(0),
                int(0),
                int(_global_size.x),
                int(_global_size.y),
            });
            drawRenderTarget(renderer);
        } else {
            this->push_clip({
                int(_global_position.x),
                int(_global_position.y),
                int(_global_size.x),
                int(_global_size.y),
            });
            this->onDraw(renderer);
            this->onChildrenDraw(renderer);
        }
        this->pop_clip();
    } else {
        if (_renderTarget) {
            drawRenderTarget(renderer);
        } else {
            this->onDraw(renderer);
            this->onChildrenDraw(renderer);
        }
    }
}

void Widget::drawRenderTarget(SDL_Renderer* renderer) {

    if (not _visible) {
        return;
    }

    auto scale = _scale;
    auto anchor = _anchor;
    auto position = _position;
    auto parent = _parent;
    this->_scale = {1.0f, 1.0f};
    this->_anchor = {0.0f, 0.0f};
    this->_position = {0.0f, 0.0f};
    this->_parent = nullptr;
    this->modifyLayout();

    auto render_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, _render->texture()->data());
    SDL_RenderClear(renderer);
    this->onDraw(renderer);
    this->onChildrenDraw(renderer);
    SDL_SetRenderTarget(renderer, render_target);

    _parent = parent;
    _position = position;
    _anchor = anchor;
    _scale = scale;
    this->modifyLayout();

    _render->setAnchor(_anchor);
    _render->setScale(_scale);
    _render->setSize(_size.to<int>());
    _render->setOpacity(_opacity);
    _render->setAngle(_rotation);
    _render->draw(renderer, _position.to<int>());
}

void Widget::onChildrenDraw(SDL_Renderer* renderer) {
    for (auto& child : _children) {
        child->draw(renderer);
    }
}

void Widget::dirty() {
    onDirty();
}

void Widget::enter() {
    onEnter();
    signal(ON_ENTER)(this);
}

void Widget::exit() {
    signal(ON_EXIT)(this);
    onExit();
}

Widget* Widget::find(std::string const& name) {
    if (name.empty()) {
        return nullptr;
    }
    if (this->name() == name) {
        return this;
    }
    for (auto& child : _children) {
        if (child->name() == name) {
            return child.get();
        }
    }
    return nullptr;
}

Widget* _findWidget(Widget* widget, std::string const& name) {
    if (widget->name() == name) {
        return widget;
    }
    for (auto& child : widget->children()) {
        auto r = _findWidget(child.get(), name);
        if (r != nullptr) {
            return r;
        }
    }
    return nullptr;
}

Widget* Widget::gfind(std::string const& name) {
    return name.empty() ? nullptr : _findWidget(this, name);
}

void Widget::setName(std::string const& name) {
    _name = name;
}

std::string const& Widget::name() const {
    return _name;
}

void Widget::set_userdata(void* data) {
    _userdata = data;
}

void* Widget::userdata() const {
    return _userdata;
}

void Widget::modifyLayout() {
    _global_size = _size * _scale.self_abs();
    _global_position = (_parent ? _parent->_global_position : Vector2f{0, 0}) + (_position - _global_size * _anchor);
    this->dirty();
    _dirty = false;
    for (auto& child : _children) {
        child->modifyLayout();
    }
}

void Widget::push_clip(SDL_Rect const& clip) {
    SDL_RenderSetClipRect(_game.renderer(), &clip);
    _clipStack.emplace_back(clip);
}

void Widget::pop_clip() {
    _clipStack.pop_back();
    SDL_RenderSetClipRect(_game.renderer(), &_clipStack.back());
}

void Widget::onModifyPosition(Vector2f const& position) {

}

void Widget::setPosition(Vector2f const& position) {
    _dirty = true;
    this->onModifyPosition(_position = position);
}

void Widget::setPosition(float dx, float dy) {
    _position.x = dx;
    _position.y = dy;
    _dirty = true;
    this->onModifyPosition(_position);
}

void Widget::setPositionX(float dx) {
    setPosition(dx, _position.y);
}

void Widget::setPositionY(float dy) {
    setPosition(_position.x, dy);
}

Vector2f const& Widget::position() const {
    return _position;
}

Vector2f const& Widget::global_position() const {
    return _global_position;
}

void Widget::onModifySize(Vector2f const& size) {

}

void Widget::setSize(Vector2f const& size) {
    _dirty = true;
    this->onModifySize(_size = size);
}

void Widget::setSize(float sx, float sy) {
    _size.x = sx;
    _size.y = sy;
    _dirty = true;
    this->onModifySize(_size);
}

Vector2f const& Widget::size() const {
    return _size;
}

Vector2f const& Widget::global_size() const {
    return _global_size;
}

void Widget::onModifyAnchor(Vector2f const& anchor) {

}

void Widget::setAnchor(Vector2f const& anchor) {
    _dirty = true;
    this->onModifyAnchor(_anchor = anchor);
}

void Widget::setAnchor(float x, float y) {
    _anchor.x = x;
    _anchor.y = y;
    _dirty = true;
    this->onModifyAnchor(_anchor);
}

Vector2f const& Widget::anchor() const {
    return _anchor;
}

void Widget::onModifyScale(Vector2f const& scale) {

}

void Widget::setScale(Vector2f const& scale) {
    _dirty = true;
    this->onModifyScale(_scale = scale);
}

void Widget::setScale(float x, float y) {
    _scale.x = x;
    _scale.y = y;
    _dirty = true;
    this->onModifyScale(_scale);
}

void Widget::setScale(float scale) {
    setScale(scale, scale);
}

Vector2f const& Widget::scale() const {
    return _scale;
}

void Widget::setOpacity(unsigned char opacity) {
    this->onModifyOpacity(_opacity = opacity);
}

unsigned char Widget::opacity() const {
    return _opacity;
}

void Widget::onModifyOpacity(unsigned char opacity) {

}

void Widget::setRotation(float rotation) {
    this->onModifyRotation(_rotation = rotation);
}

float Widget::rotation() const {
    return _rotation;
}

void Widget::onModifyRotation(float rotation) {

}

void Widget::runAction(ActionPtr const& action) {
    _action->add(action);
}

void Widget::stopAction(ActionPtr const& action) {
    _action->remove(action);
}

void Widget::stopAction(std::string const& name) {
    _action->remove(name);
}

bool Widget::hasAction(std::string const& name) const {
    return _action->has(name);
}

void Widget::stopAllActions() {
    _action->clear();
}

void Widget::pauseAllActionWhenHidden(bool yes) {
    _pause_action_when_hidden = yes;
}

void Widget::pauseAllActions() {
    _action->pause(true);
}

void Widget::resumeAllActions() {
    _action->pause(false);
}

//=====================================================================================

LayerWidget::LayerWidget(): MouseResponder(this) {
    connect(ON_ENTER, [this](Widget* sender){
        _game.mouse().add(this);
    });
    connect(ON_EXIT, [this](Widget* sender){
        _game.mouse().remove(this);
    });
    enableUpdate(true);
}

//=====================================================================================

WindowWidget::WindowWidget() {
    enableUpdate(true);
}

//=====================================================================================

RenderTargetWidget::RenderTargetWidget():_hasRender(false), _render(std::make_shared<Render>()) {
    enableUpdate(true);
}

void RenderTargetWidget::setRenderTargetSize(Vector2i const& size) {
    assert(size.x > 0 and size.y > 0 and "RenderTargetWidget::setRenderTargetSize error.");
    auto texture = SDL_CreateTexture(
            _game.renderer(),
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_TARGET,
            size.x,
            size.y
    );
    if (texture) {
        //some problem on psvita
        //SDL_SetTextureScaleMode(texture, SDL_ScaleModeLinear);
    }
    _render->setTexture(TexturePtr(new Texture(texture)));
    _hasRender = true;
}

void RenderTargetWidget::setRenderTargetNull() {
    _hasRender = false;
    _render->setTexture(nullptr);
}

void RenderTargetWidget::draw(SDL_Renderer* renderer) {
    if (_hasRender) {
        drawRenderTarget(renderer);
    } else {
        Widget::draw(renderer);
    }
}

void RenderTargetWidget::drawRenderTarget(SDL_Renderer* renderer) {

    if (not _visible) {
        return;
    }

    auto scale = _scale;
    auto anchor = _anchor;
    auto position = _position;
    auto parent = _parent;
    this->_scale = {1.0f, 1.0f};
    this->_anchor = {0.0f, 0.0f};
    this->_position = {0.0f, 0.0f};
    this->_parent = nullptr;
    this->modifyLayout();

    auto render_target = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, _render->texture()->data());
    SDL_RenderClear(renderer);

    this->onDraw(renderer);
    for (auto& child : _children) {
        child->draw(renderer);
    }
    SDL_SetRenderTarget(renderer, render_target);

    _parent = parent;
    _position = position;
    _anchor = anchor;
    _scale = scale;
    this->modifyLayout();

    _render->setAnchor(_anchor);
    _render->setScale(_scale);
    _render->setSize(_size.to<int>());
    _render->setOpacity(_opacity);
    _render->setAngle(_rotation);
    _render->draw(renderer, _position.to<int>());
}

//=====================================================================================

GamePadWidget::GamePadWidget() {
    connect(ON_ENTER, [](Widget* sender){
        _game.gamepad().add(sender->ptr());
    });
    connect(ON_EXIT, [](Widget* sender){
        _game.gamepad().remove(sender->ptr());
    });
    enableUpdate(true);
}

void GamePadWidget::sleep_gamepad(float seconds) {
    _game.gamepad().sleep(seconds);
    _game.mouse().sleep(seconds);
}

//=====================================================================================

ImageWidget::ImageWidget():_target(std::make_shared<Render>()) {

}

ImageWidget::ImageWidget(TexturePtr const& texture):_target(std::make_shared<Render>()) {
    this->setTexture(texture);
}

ImageWidget::ImageWidget(TexturePtr const& texture, SDL_Rect const& srcrect):_target(std::make_shared<Render>()) {
    this->setTexture(texture, srcrect);
}

void ImageWidget::setTexture(TexturePtr const& texture) {
    if (texture == nullptr) {
        setSize(0.0f, 0.0f);
        return;
    }
    if (_target->texture().get() == texture.get()) {
        return;
    }
    _target->setTexture(texture);
    this->setSize(_target->size().to<float>());
}

void ImageWidget::setTexture(TexturePtr const& texture, SDL_Rect const& srcrect) {
    if (texture == nullptr) {
        return;
    }
    _target->setTexture(texture, srcrect);
    this->setSize(_target->size().to<float>());
}

Texture::Ptr ImageWidget::getTexture() const {
    return _target->texture();
}

void ImageWidget::setColor(SDL_Color const& c) {
    _target->setColor(c.r, c.g, c.b);
    _target->setOpacity(c.a);
}

void ImageWidget::onModifyOpacity(unsigned char opacity) {
    _target->setOpacity(opacity);
}

void ImageWidget::onModifyRotation(float rotation) {
    _target->setAngle(rotation);
}

void ImageWidget::onModifySize(Vector2f const& size) {
    _target->setSize(size.to<int>());
}

void ImageWidget::onModifyScale(Vector2f const& scale) {
    _target->setScale(scale);
}

void ImageWidget::onModifyAnchor(Vector2f const& anchor) {
    _target->setAnchor(anchor);
}

void ImageWidget::onDraw(SDL_Renderer* renderer) {
    auto position = _parent ? _parent->global_position() + _position : _position;
    _target->draw(renderer, position.to<int>());
}

//=====================================================================================

ButtonWidget::ButtonWidget():
        ImageWidget(nullptr),
        MouseResponder(this),
        _state(UNKNOWN),
        _enable(true) {

}

ButtonWidget::ButtonWidget(TexturePtr const& normal, TexturePtr const& pressed, TexturePtr const& disabled):
        ImageWidget(normal),
        MouseResponder(this),
        _state(NORMAL),
        _enable(true) {
    setNormalTexture(normal);
    setPressedTexture(pressed);
    setDisabledTexture(disabled);
}

void ButtonWidget::setNormalTexture(TexturePtr const& normal) {
    _texture[NORMAL] = normal;
    if (getTexture() == nullptr) {
        setTexture(normal);
    }
}

void ButtonWidget::setPressedTexture(TexturePtr const& pressed) {
    _texture[PRESSED] = pressed;
}

void ButtonWidget::setDisabledTexture(TexturePtr const& disabled) {
    _texture[DISABLED] = disabled;
}

void ButtonWidget::setEnable(bool enable) {
    setState((_enable = enable) ? NORMAL : DISABLED);
}

void ButtonWidget::setPressed(bool pressed) {
    if (not _enable) {
        return;
    }
    setState(pressed ? PRESSED : NORMAL);
}

bool ButtonWidget::enable() const {
    return _enable;
}

bool ButtonWidget::pressed() const {
    return _state == PRESSED;
}

void ButtonWidget::setSelector(CallBack const& cb) {
    _selector = cb;
}

void ButtonWidget::click() {
    if (_enable and _selector != nullptr) {
        _selector(this);
    }
}

void ButtonWidget::setState(State state) {
    if (_state == state) {
        return;
    }
    auto size = this->size();
    setTexture(_texture[state]);
    setSize(size);
    _state = state;
}

void ButtonWidget::onEnter() {
    ImageWidget::onEnter();
    _game.mouse().add(this);
}

void ButtonWidget::onExit() {
    _game.mouse().remove(this);
    ImageWidget::onExit();
}

bool ButtonWidget::onMouseDown(MouseEvent const& event) {
    if (!_enable) {
        return false;
    }
    setState(PRESSED);
    return true;
}

void ButtonWidget::onMouseUp(MouseEvent const& event) {
    setState(NORMAL);
    if (event.x > 0 and event.x < _global_size.x and event.y > 0 and event.y < _global_size.y) {
        if (_selector != nullptr) {
            _selector(this);
        }
    }
}

void ButtonWidget::onMouseMotion(MouseEvent const& event) {

}

//=====================================================================================

ProgressBarWidget::ProgressBarWidget():_value(1.0f), _clipBox(new Widget) {
    addChild(Ptr(_clipBox));
    _clipBox->enableClip(true);
    memset(_image, 0, sizeof(_image));
}

ProgressBarWidget::ProgressBarWidget(TexturePtr const& bg, TexturePtr const& fg, TexturePtr const& selector):
_value(1.0f),
_clipBox(new Widget) {
    addChild(Ptr(_clipBox));
    _clipBox->enableClip(true);
    memset(_image, 0, sizeof(_image));
    setBgTexture(bg);
    setBarTexture(fg);
    setDotTexture(selector);
}

void ProgressBarWidget::setBgTexture(TexturePtr const& bg) {
    if (auto view = _image[0]; view == nullptr) {
        _image[0] = new ImageWidget(bg);
        addChild(Ptr(_image[0]), 0);
        setSize(_image[0]->size());
    } else {
        _image[0]->setTexture(bg);
    }
    _image[0]->setSize(size());
}

void ProgressBarWidget::setBarTexture(TexturePtr const& fg) {
    if (auto view = _image[1]; view == nullptr) {
        _image[1] = new ImageWidget(fg);
        _clipBox->addChild(Ptr(_image[1]));
    } else {
        _image[1]->setTexture(fg);
    }
    _image[1]->setSize(size());
}

void ProgressBarWidget::setDotTexture(TexturePtr const& selector) {
    if (auto view = _image[2]; view == nullptr) {
        _image[2] = new ImageWidget(selector);
        _image[2]->setAnchor(0.5f, 0.5f);
        addChild(Ptr(_image[2]));
    } else {
        _image[2]->setTexture(selector);
    }
    _image[2]->setPosition({size().x * _value, size().y * 0.5f});
}

void ProgressBarWidget::setSelector(CallBack const& cb) {
    _selector = cb;
}

void ProgressBarWidget::setValue(float value) {
    _value = value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
    if (int(_value * 1000) <= 10) {
        _clipBox->setVisible(false);
    } else {
        _clipBox->setSize({size().x * _value, size().y});
        _clipBox->setVisible(true);
    }
    if (auto selector = _image[2]; selector) {
        selector->setPosition({global_size().x * _value, global_size().y * 0.5f});
    }
}

float ProgressBarWidget::value() const {
    return _value;
}

void ProgressBarWidget::onEnter() {
    LayerWidget::onEnter();
    setSize(size());
    setScale(scale());
    performLayout();
    setValue(_value);
}

void ProgressBarWidget::onModifyScale(Vector2f const& scale) {
    for (int i = 0; i < 3; ++i) {
        if (_image[i]) {
            _image[i]->setScale(scale);
        }
    }
    _clipBox->setScale(scale);
    setValue(_value);
}

void ProgressBarWidget::onModifySize(Vector2f const& size) {
    for (int i = 0; i < 2; ++i) {
        if (_image[i]) {
            _image[i]->setSize(size);
        }
    }
    _clipBox->setSize(size);
    if (_image[0] and _image[2]) {
        if (auto texture0 = _image[0]->getTexture(); texture0.get()) {
            if (auto texture1 = _image[2]->getTexture(); texture1.get()) {
                auto scale = size / texture0->size().to<float>();
                _image[2]->setSize(scale * texture1->size().to<float>());
            }
        }
    }
    setValue(_value);
}

bool ProgressBarWidget::onMouseDown(MouseEvent const& event) {
    setValue(event.x / global_size().x);
    if (_selector != nullptr) {
        _selector(this, BEGEN);
    }
    return true;
}

void ProgressBarWidget::onMouseMotion(MouseEvent const& event) {
    setValue(event.x / global_size().x);
    if (_selector != nullptr) {
        _selector(this, MOVED);
    }
}

void ProgressBarWidget::onMouseUp(MouseEvent const& event) {
    setValue(event.x / global_size().x);
    if (_selector != nullptr) {
        _selector(this, ENDED);
    }
}

//=====================================================================================

MaskWidget::MaskWidget(SDL_Color const& c):_color(c) {
    setOpacity(c.a);
}

void MaskWidget::setColor(SDL_Color const& c) {
    _color = c;
    setOpacity(c.a);
}

SDL_Color const& MaskWidget::color() const {
    return _color;
}

void MaskWidget::onDraw(SDL_Renderer* renderer) {
    DrawColor dc(renderer);
    SDL_FRect dst{
        global_position().x,
        global_position().y,
        global_size().x,
        global_size().y,
    };
    _color.a = _opacity;
    dc.setColor(_color);

    if (auto render_target = SDL_GetRenderTarget(renderer); render_target) {
        SDL_BlendMode blend_mode;
        SDL_GetRenderDrawBlendMode(renderer, &blend_mode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderFillRectF(renderer, &dst);
        SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    } else {
        SDL_RenderFillRectF(renderer, &dst);
    }
}

MaskBoxWidget::MaskBoxWidget(SDL_Color const& c):MaskWidget(c) {}

void MaskBoxWidget::onDraw(SDL_Renderer* renderer) {
    DrawColor dc(renderer);
    SDL_FRect dst{
            global_position().x,
            global_position().y,
            global_size().x,
            global_size().y,
    };
    _color.a = _opacity;
    dc.setColor(_color);

    if (auto render_target = SDL_GetRenderTarget(renderer); render_target) {
        SDL_BlendMode blend_mode;
        SDL_GetRenderDrawBlendMode(renderer, &blend_mode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
        SDL_RenderDrawRectF(renderer, &dst);
        SDL_SetRenderDrawBlendMode(renderer, blend_mode);
    } else {
        SDL_RenderDrawRectF(renderer, &dst);
    }
}

//=====================================================================================

CurtainWidget::CurtainWidget(SDL_Color const& c) {
    Vector2f anchor[2] = {
            {0.5f, 1.0f},
            {0.5f, 0.0f},
    };
    for (int i = 0; i < 2; ++i) {
        addChild(_mask[i]= Widget::Ptr(new MaskWidget(c)));
        _mask[i]->setVisible(false);
        _mask[i]->setAnchor(anchor[i]);
    }
    enableUpdate(true);
}

void CurtainWidget::setState(State s) {
    if (s == ON) {
        for (int i = 0; i < 2; ++i) {
            _mask[i]->setPosition(size().x * 0.5f, size().y * 0.5f);
            _mask[i]->setVisible(true);
        }
    } else {
        Vector2f position[2] = {
                {size().x * 0.5f, 0.0f},
                {size().x * 0.5f, size().y},
        };
        for (int i = 0; i < 2; ++i) {
            _mask[i]->setPosition(position[i]);
            _mask[i]->setVisible(false);
        }
    }
}

void CurtainWidget::fadeIn(float duration) {
    Vector2f position[2] = {
            {size().x * 0.5f, 0.0f},
            {size().x * 0.5f, size().y},
    };
    float step[2] = {size().y * 0.5f, size().y * -0.5f};
    for (int i = 0; i < 2; ++i) {
        _mask[i]->setVisible(true);
        _mask[i]->setPosition(position[i]);
        moveMaskVertical(_mask[i]->to<MaskWidget>(), step[i], duration);
    }
}

void CurtainWidget::fadeOut(float duration) {
    float step[2] = {size().y * -0.5f, size().y * 0.5f};
    for (int i = 0; i < 2; ++i) {
        _mask[i]->setVisible(true);
        _mask[i]->setPosition(size().x * 0.5f, size().y * 0.5f);
        moveMaskVertical(_mask[i]->to<MaskWidget>(), step[i], duration);
    }
}

void CurtainWidget::moveMaskVertical(MaskWidget* target, float yStep, float duration) {
    auto move = Action::Ptr(new MoveBy(target, {0, yStep}, duration));
    target->runAction(move);
}

//=====================================================================================

ScreenWidget::ScreenWidget():_curtain(nullptr), _root(nullptr) {

    Widget::Ptr box(new RenderTargetWidget);
    Widget::Ptr window(new WindowWidget);
    Widget::Ptr curtain(new CurtainWidget);

    addChild(box);
    box->addChild(window);
    box->addChild(curtain);

    _root = window->to<WindowWidget>();
    _curtain = curtain->to<CurtainWidget>();
    _action = ActionExecuterPtr(new SafeActionExecuter); // make action executer thread safe
}

void ScreenWidget::push(Widget::Ptr const& widget) {
    _root->addChild(widget);
    for (auto& child : _root->children()) {
        if (child.get() == widget.get()) {
            break;
        }
        child->setVisible(false);
    }
    if (auto p = dynamic_cast<ScreenWidgetListener*>(widget.get()); p) {
        p->onScreenLoaded();
    }
    if (_root->children().size() >= 2) {
        auto child = _root->children()[ _root->children().size() - 2 ];
        if (auto p = dynamic_cast<ScreenWidgetListener*>(child.get()); p) {
            p->onScreenSleep();
        }
    }
    widget->performLayout();
}

void ScreenWidget::push(Widget::Ptr const& widget, std::function<void(Widget*, Widget*)> const& transform) {
    _root->addChild(widget);
    if (auto p = dynamic_cast<ScreenWidgetListener*>(widget.get()); p) {
        p->onScreenLoaded();
    }
    if (_root->children().size() >= 2) {
        auto child = _root->children()[ _root->children().size() - 2 ];
        if (auto p = dynamic_cast<ScreenWidgetListener*>(child.get()); p) {
            p->onScreenSleep();
        }
    }
    Widget* curr = _root->children().size() >= 2 ? _root->children()[ _root->children().size() - 2 ].get() : nullptr;
    Widget* next = _root->children().back().get();
    transform(curr, next);
    widget->performLayout();
}

void ScreenWidget::replace(Widget::Ptr const& widget) {
    this->pop(false);
    this->push(widget);
}

void ScreenWidget::pop(bool wake_last) {
    auto& c = _root->children();
    if (c.size()) {
        _root->removeChild(c.back());
    }
    if (c.size()) {
        c.back()->setVisible(true);
        if (auto p = dynamic_cast<ScreenWidgetListener*>(c.back().get()); p and wake_last) {
            p->onScreenWake();
        }
    }
}

void ScreenWidget::popAll() {
    _root->removeAllChildren();
}

void ScreenWidget::update(float delta) {
    WindowWidget::update(delta);
}

void ScreenWidget::render(SDL_Renderer* renderer) {
    draw(renderer);
}

int ScreenWidget::scene_size() const {
    return _root->children().size();
}

Widget::Ptr& ScreenWidget::scene_at(int index) const {
    return _root->children()[index];
}

Widget::Ptr& ScreenWidget::scene_back() const {
    return scene_at(scene_size()-1);
}

Widget::Ptr ScreenWidget::find(std::string const& name) const {
    for (auto& child : _root->children()) {
        if (child->name() == name) {
            return child;
        }
    }
    return nullptr;
}

Vector2f const& ScreenWidget::size() const {
    return WindowWidget::size();
}

bool ScreenWidget::hasAction(std::string const& name) const {
    return WindowWidget::hasAction(name);
}

void ScreenWidget::runAction(Action::Ptr const& action) {
    WindowWidget::runAction(action);
}

void ScreenWidget::stopAction(Action::Ptr const& action) {
    WindowWidget::stopAction(action);
}

void ScreenWidget::stopAction(std::string const& name) {
    WindowWidget::stopAction(name);
}

//=====================================================================================

TTFLabel::TTFLabel():ImageWidget(nullptr) {}

TTFLabel::Ptr TTFLabel::New(std::string const& text, TTFontPtr const& font, Vector2f const& anchor) {
    auto label = new TTFLabel;
    label->setFont(font);
    label->setString(text);
    label->setAnchor(anchor);
    return Ptr(label);
}

void TTFLabel::setFont(TTFontPtr const& font) {
    _font = font;
}

TTFont::Ptr const& TTFLabel::font() const {
    return _font;
}

void TTFLabel::setString(std::string const& s) {
    if (_font == nullptr or _s == s) {
        return;
    }
    _font->setColor( _target->color() );
    setTexture(_font->createWithUTF8(_game.renderer(), s.empty() ? " " : s.c_str()));
    _s = s;
}

void TTFLabel::setString(std::string const& s, SDL_Color const& color) {
    setColor(color);
    setString(s);
}

std::string const& TTFLabel::str() const {
    return _s;
}

//=====================================================================================

FrameImageWidget::FrameImageWidget():_index(0) {

}

FrameImageWidget::FrameArray const& FrameImageWidget::frames() const {
    return _frames;
}

void FrameImageWidget::setFrames(FrameArray const& frames) {
    _frames = frames;
    if (_frames.size()) {
        setTexture(_frames[0]);
    }
}

void FrameImageWidget::setFrameIndex(uint32_t index) {
    if (index < _frames.size()) {
        _index = index;
        setTexture(_frames[_index]);
    }
}

//=====================================================================================

FrameAnimationWidget::FrameAnimationWidget():
_frame_tick(0.0f),
_frame_time(0.0f),
_loop(false) {
    enableUpdate(true);
}

void FrameAnimationWidget::play(float duration, bool loop) {
    if (_frames.empty()) {
        return;;
    }
    _loop = loop;
    _frame_time = duration / _frames.size();
    _frame_tick = 0.0f;
    _index = 0;
    setTexture(_frames[0]);
    startAnimate();
}

void FrameAnimationWidget::play_once(float duration) {
    this->play(duration, false);
}

void FrameAnimationWidget::stop() {
    if (_frames.empty()) {
        return;;
    }
    setTexture(_frames[0]);
    stopAction("animate");
}

void FrameAnimationWidget::startAnimate() {
    if (hasAction("animate")) {
        return;
    }
    auto call = Action::Ptr(new CallBackDelta(std::bind(&FrameAnimationWidget::onAnimate, this, std::placeholders::_1)));
    auto action = Action::New<Repeat>(call);
    action->setName("animate");
    runAction(action);
}

void FrameAnimationWidget::onAnimate(float delta) {
    bool modify = false;
    while ((_frame_tick += delta) >= _frame_time) {
        _frame_tick -= _frame_time;
        if (++_index >= _frames.size()) {
            if (_loop) {
                _index = 0;
            } else {
                _index = _frames.size() - 1;
                stopAction("animate");
            }
        }
        modify = true;
    }
    if (modify) {
        setTexture(_frames[_index]);
    }
}

mge_end
