//
// Created by baifeng on 2022/4/14.
//

#ifndef SDL2_UI_BMFLABEL_H
#define SDL2_UI_BMFLABEL_H

#include "common/widget.h"
#include "common/texture.h"

mge_begin

class BMFChar : public ImageWidget {
public:
    BMFChar(TexturePtr const& texture, SDL_Rect const& srcrect);
};

class BMFont;
class BMFLabel : public Widget {
public:
    typedef std::shared_ptr<BMFont> BMFontPtr;
    typedef std::vector<unsigned short> UnicodeString;
    struct Padding {
        int top;
        int right;
        int bottom;
        int left;
    };
public:
    BMFLabel();
public:
    void setFont(BMFontPtr const& font);
    BMFontPtr& getFont();
    void setString(std::string const& text);
    void setPadding(Padding const& padding);    // 边距
    void setWidth(int width);                   // 画布宽
    void setSpacing(Vector2i const& spacing);   // 行列间距
    void setVisibleCount(size_t count);         // 显示字数
    void draw_at_once();
    BMFChar* getChar(uint32_t index);
private:
    void onUpdate(float delta) override;
    void refresh();
private:
    bool _dirty;
    int _width;
    Vector2i _spacing;
    size_t _visibleCount;
    Widget* _textBox;
    BMFontPtr _font;
    std::string _text;
    Padding _padding;
    UnicodeString _unicodeString;
};

mge_end

#endif //SDL2_UI_BMFLABEL_H
