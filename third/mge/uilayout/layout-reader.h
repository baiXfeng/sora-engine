//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_LAYOUT_READER_H
#define SDL2_UI_LAYOUT_READER_H

#include <memory>
#include <vector>
#include <map>

namespace mge {
    class Widget;
    class FileReader;
}

namespace ui {
    class Document;
    class LoaderPool;
    class NodeLoader;
    class LayoutInfo;
    using FileReader = mge::FileReader;
    class LayoutReader {
        typedef std::shared_ptr<mge::Widget> Node;
        typedef std::shared_ptr<LayoutInfo> Info;
        typedef std::vector<Info> InfoStack;
        typedef std::vector<mge::Widget*> OwnerStack;
        typedef std::vector<NodeLoader*> LayoutLoader;
    public:
        LayoutReader(LoaderPool* loader_library, FileReader* r);
        virtual ~LayoutReader();
    public:
        Node readNode(std::string const& fileName, mge::Widget* parent = nullptr);
        void setFileReader(FileReader* reader);
        void setLoaderPool(LoaderPool* loader);
    public:
        LayoutInfo const& info() const;
        mge::Widget* const owner() const;
    private:
        Node readNode(mge::Widget* parent, Document* d, bool owner = false);
        void parseProperties(NodeLoader* loader, mge::Widget* node, mge::Widget* parent, Document* d);
    private:
        LoaderPool* _loader;
        FileReader* _fileReader;
        InfoStack _info;
        OwnerStack _owner;
        LayoutLoader _layoutLoader;
    };
}

#endif //SDL2_UI_LAYOUT_READER_H
