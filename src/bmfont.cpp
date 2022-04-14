//
// Created by baifeng on 2021/6/23.
//

#include "bmfont.h"
#include "common/log.h"
#include "common/loadres.h"
#include "common/game.h"
#include "common/xml_layout.h"
#include "common/file-reader.h"
#include <iosfwd>

mge_begin

static std::vector<std::string> _split(std::string const& str, std::string const& pat) {
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

#define _string(s) s.substr(1, s.length()-2)
#define _i(s) atoi(s.c_str())

std::vector<int> _ii(std::string const& s, int n) {
    std::vector<int> r(n);
    auto va = _split(s, ",");
    for (int i = 0; i < va.size(); ++i) {
        if (i >= r.size()) {
            break;
        }
        r[i] = _i(va[i]);
    }
    return r;
}

BMFont::Page::Page(StringArray const& sa):Page() {
    Parse(sa);
}

void BMFont::Page::Parse(StringArray const& sa) {
    for (int i = 1; i < sa.size(); ++i) {
        auto v = _split(sa[i], "=");
        auto const& key = v[0];
        auto const& value = v[1];
        if (key == "id") {
            this->id = _i(value);
        } else if (key == "file") {
            this->file = _string(value);
        }
    }
}

void BMFont::Info::Parse(StringArray const& sa) {
    for (int i = 1; i < sa.size(); ++i) {
        auto v = _split(sa[i], "=");
        auto const& key = v[0];
        auto const& value = v[1];
        if (key == "face") {
            face = _string(value);
        } else if (key == "charset") {
            charset = _string(value);
        } else if (key == "size") {
            size = _i(value);
        } else if (key == "bold") {
            bold = _i(value);
        } else if (key == "italic") {
            italic = _i(value);
        } else if (key == "unicode") {
            unicode = _i(value);
        } else if (key == "stretchH") {
            stretchH = _i(value);
        } else if (key == "smooth") {
            smooth = _i(value);
        } else if (key == "aa") {
            aa = _i(value);
        } else if (key == "outline") {
            outline = _i(value);
        } else if (key == "padding") {
            padding = _ii(value, 4);
        } else if (key == "spacing") {
            spacing = _ii(value, 2);
        }
    }
}

void BMFont::Common::Parse(StringArray const& sa) {
    for (int i = 1; i < sa.size(); ++i) {
        auto v = _split(sa[i], "=");
        auto const &key = v[0];
        auto const &value = v[1];
        if (key == "lineHeight") {
            lineHeight = _i(value);
        } else if (key == "base") {
            base = _i(value);
        } else if (key == "scaleW") {
            scaleW = _i(value);
        } else if (key == "scaleH") {
            scaleH = _i(value);
        } else if (key == "pages") {
            pages = _i(value);
        } else if (key == "packed") {
            packed = _i(value);
        } else if (key == "alphaChnl") {
            alphaChnl = _i(value);
        } else if (key == "redChnl") {
            redChnl = _i(value);
        } else if (key == "greenChnl") {
            greenChnl = _i(value);
        } else if (key == "blueChnl") {
            blueChnl = _i(value);
        }
    }
}

BMFont::Char::Char(const StringArray &sa):Char() {
    Parse(sa);
}

void BMFont::Char::Parse(StringArray const& sa) {
    for (int i = 1; i < sa.size(); ++i) {
        auto v = _split(sa[i], "=");
        auto const &key = v[0];
        auto const &value = v[1];
        if (key == "id") {
            id = _i(value);
        } else if (key == "x") {
            x = _i(value);
        } else if (key == "y") {
            y = _i(value);
        } else if (key == "width") {
            width = _i(value);
        } else if (key == "height") {
            height = _i(value);
        } else if (key == "xoffset") {
            xoffset = _i(value);
        } else if (key == "yoffset") {
            yoffset = _i(value);
        } else if (key == "xadvance") {
            xadvance = _i(value);
        } else if (key == "page") {
            page = _i(value);
        } else if (key == "chnl") {
            chnl = _i(value);
        }
    }
}

BMFont::BMFont(): _color({0, 0, 0, 255}) {

}

BMFont::~BMFont() {

}

void BMFont::loadFile(std::string const& fileName) {
    auto data = _game.uilayout().getFileReader()->getData(fileName);
    if (data == nullptr) {
        LOG_ERROR("bmfont load %s fial, file not exist.\n", fileName.c_str());
        return;
    }
    std::string text((char*)data->data(), data->size());
    this->loadFileData(text, fileName);
}

void BMFont::loadFileData(std::string const& text, std::string const& fileName) {
    this->ParseText(text, fileName);
    this->loadTexture();
}

void BMFont::ParseText(std::string const& text, std::string const& fileName) {
    int lastSlash = fileName.find_last_of("/");
    if (lastSlash > 0) {
        _filePath = fileName.substr(0, lastSlash + 1);
    } else {
        _filePath = "";
    }
    _fileName = fileName;
    auto linearr = _split(text, "\r\n");
    for (int i = 0; i < linearr.size(); ++i) {
        auto arr = _split(linearr[i], " ");
        if (arr[0] == "info") {
            _info.Parse(arr);
        } else if (arr[0] == "common") {
            _common.Parse(arr);
        } else if (arr[0] == "page") {
            _pages.push_back(Page(arr));
        } else if (arr[0] == "char") {
            Char c(arr);
            _charsets.insert(std::make_pair(c.id, c));
        }
    }
}

void BMFont::loadTexture() {
    for (auto& page : _pages) {
        auto fileName =  _filePath + page.file;
        auto img_ptr = mge::res::load_texture(fileName);
        if (img_ptr != nullptr) {
            _texArray.push_back(img_ptr);
        }
    }
    setColor(_color);
}

SDL_Color const& BMFont::getColor() const {
    return _color;
}

void BMFont::setColor(SDL_Color const& c) {
    _color = c;
    for (auto& tex : _texArray) {
        SDL_SetTextureColorMod(tex->data(), c.r, c.g, c.b);
        SDL_SetTextureAlphaMod(tex->data(), c.a);
    }
}

Texture::Ptr BMFont::getTexture(int pageid) const {
    if (pageid >= _texArray.size()) {
        return nullptr;
    }
    return _texArray[pageid];
}

BMFont::Info const& BMFont::info() const {
    return _info;
}

BMFont::Common const& BMFont::common() const {
    return _common;
}

BMFont::Page const& BMFont::page(int i) const {
    return _pages[i];
}

BMFont::Pages const& BMFont::pages() const {
    return _pages;
}

int BMFont::pageSize() const {
    return _pages.size();
}

bool BMFont::hasChar(int id) const {
    auto iter = _charsets.find(id);
    return iter != _charsets.end();
}

BMFont::Char const& BMFont::getChar(int id) const {
    auto iter = _charsets.find(id);
    return iter->second;
}

BMFont::Charsets const& BMFont::charsets() const {
    return _charsets;
}

#undef _string
#undef _i

mge_end