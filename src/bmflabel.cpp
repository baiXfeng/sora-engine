//
// Created by baifeng on 2022/4/14.
//

#include "bmflabel.h"
#include "bmfont.h"
#include "utf8_to_unicode.h"
#include "common/log.h"

mge_begin

BMFChar::BMFChar(TexturePtr const& texture, SDL_Rect const& srcrect):ImageWidget(texture, srcrect) {

}

BMFLabel::BMFLabel():_textBox(new Widget), _padding({0, 0, 0, 0}), _dirty(false), _width(0), _visibleCount(0xffff) {
    /*auto mask = Ptr(new MaskWidget({255, 255, 255, 100}));
    mask->setName("mask");
    addChild(mask);*/
    addChild(Ptr(_textBox));
}

void BMFLabel::setFont(BMFontPtr const& font) {
    _font = font;
    _dirty = true;
}

BMFLabel::BMFontPtr& BMFLabel::getFont() {
    return _font;
}

void BMFLabel::setString(std::string const& text) {
    if (_text == text) {
        return;
    }
    _text = text;
    _dirty = true;
    _unicodeString.clear();

    auto i = 0;
    while (i < text.length()) {
        auto n = utf8_get_size(&text[i]);
        if (n == -1) {
            n++;
            LOG_ERROR("BMFLabel::setString text[%d] error.\n", i);
            continue;
        }
        _unicodeString.push_back(utf8_convert_unicode(text.c_str()+i, n));
        i += n;
    }
}

void BMFLabel::setPadding(Padding const& padding) {
    _padding = padding;
    _dirty = true;
}

void BMFLabel::setWidth(int width) {
    _width = width;
    _dirty = true;
}

void BMFLabel::setSpacing(Vector2i const& spacing) {
    _spacing = spacing;
    _dirty = true;
}

void BMFLabel::setVisibleCount(size_t count) {
    _visibleCount = count;
    for (int i = 0; i < _textBox->children().size(); ++i) {
        auto& child = _textBox->children()[i];
        child->setVisible(i < _visibleCount);
    }
}

void BMFLabel::draw_at_once() {
    if (!_dirty) {
        return;
    }
    refresh();
    _dirty = false;
}

BMFChar* BMFLabel::getChar(uint32_t index) {
    if (index < _textBox->children().size()) {
        return _textBox->children()[index]->fast_to<BMFChar>();
    }
    return nullptr;
}

void BMFLabel::onUpdate(float delta) {
    draw_at_once();
}

void BMFLabel::refresh() {
    if (_font == nullptr) {
        return;
    }
    Vector2f size;
    size.x = _padding.left + _padding.right + _width;
    _textBox->setPosition(_padding.left, _padding.top);
    _textBox->removeAllChildren();

    Vector2f offset;
    auto& common = _font->common();
    for (int i = 0; i < _unicodeString.size(); ++i) {
        auto c = _unicodeString[i];
        if (!_font->hasChar(c)) {
            continue;
        }
        auto const& charset = _font->getChar(c);
        if (offset.x + charset.xoffset + charset.width > _width) {
            offset.x = 0;
            offset.y += common.base + _spacing.y;
        }
        auto tex = _font->getTexture(charset.page);
        auto bmfchar = New<BMFChar>(tex, SDL_Rect{charset.x, charset.y, charset.width, charset.height});
        bmfchar->setPosition(offset.x + charset.xoffset, offset.y + charset.yoffset);
        bmfchar->setVisible(i < _visibleCount);
        _textBox->addChild(bmfchar);
        offset.x += charset.width + charset.xoffset + _spacing.x;
    }

    size.y = common.base + offset.y + _padding.top + _padding.bottom;

    this->setSize(size);
    _textBox->setSize(size.x - _padding.left - _padding.right, size.y - _padding.top - _padding.bottom);
    //find("mask")->setSize(size);
}

mge_end