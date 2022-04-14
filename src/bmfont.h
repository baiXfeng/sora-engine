//
// Created by baifeng on 2021/6/23.
// tools download: http://www.angelcode.com/products/bmfont/
//

#ifndef SE_BMFONT_H
#define SE_BMFONT_H

#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <SDL.h>
#include "common/macro.h"
#include "common/texture.h"

mge_begin

class BMFont {
public:
    typedef std::vector<std::string> StringArray;
    class Info {
    public:
        typedef std::vector<int> IntArray;
    public:
        std::string face;
        std::string charset;
        int size;
        int bold;
        int italic;
        int unicode;
        int stretchH;
        int smooth;
        int aa;
        int outline;
        IntArray padding;
        IntArray spacing;
        Info():size(0), bold(0), italic(0), unicode(0), stretchH(0), smooth(0), aa(0), outline(0) {}
        void Parse(StringArray const& sa);
    };
    class Common {
    public:
        int lineHeight;
        int base;
        int scaleW;
        int scaleH;
        int pages;
        int packed;
        int alphaChnl;
        int redChnl;
        int greenChnl;
        int blueChnl;
        Common():lineHeight(0), base(0), scaleW(0), scaleH(0), pages(0), packed(0), alphaChnl(0), redChnl(0), greenChnl(0), blueChnl(0) {}
        void Parse(StringArray const& sa);
    };
    class Page {
    public:
        int id;
        std::string file;
        Page():id(0) {}
        Page(StringArray const& sa);
        void Parse(StringArray const& sa);
    };
    typedef std::vector<Page> Pages;
    class Char {
    public:
        int id;
        int x;
        int y;
        int width;
        int height;
        int xoffset;
        int yoffset;
        int xadvance;
        int page;
        int chnl;
        Char():id(0), x(0), y(0), width(0), height(0), xoffset(0), yoffset(0), xadvance(0), page(0), chnl(0) {}
        Char(StringArray const& sa);
        void Parse(StringArray const& sa);
    };
    typedef std::unordered_map<int, Char> Charsets;
    typedef std::vector<Texture::Ptr> TextureArray;
    typedef Texture::Ptr TexturePtr;
public:
    BMFont();
    ~BMFont();
public:
    void loadFile(std::string const& fileName);
    void loadFileData(std::string const& text, std::string const& fileName);
public:
    Info const& info() const;
    Common const& common() const;
    Page const& page(int i) const;
    Pages const& pages() const;
    int pageSize() const;
    bool hasChar(int id) const;
    Char const& getChar(int id) const;
    Charsets const& charsets() const;
    Texture::Ptr getTexture(int pageid) const;
public:
    SDL_Color const& getColor() const;
    void setColor(SDL_Color const& c);
private:
    void ParseText(std::string const& text, std::string const& fileName);
    void loadTexture();
private:
    Info _info;
    Common _common;
    Pages _pages;
    Charsets _charsets;
    TextureArray _texArray;
    std::string _filePath, _fileName;
    SDL_Color _color;
};

mge_end

#endif //SE_BMFONT_H
