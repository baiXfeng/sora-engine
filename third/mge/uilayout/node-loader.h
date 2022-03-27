//
// Created by baifeng on 2022/1/11.
//

#ifndef SDL2_UI_NODE_LOADER_H
#define SDL2_UI_NODE_LOADER_H

#include <memory>
#include <vector>
#include <map>
#include "layout-variable-assigner.h"

namespace mge {
    class Widget;
    class Texture;
}

#define UI_NODE_LOADER_CREATE(T) \
    Node createNode(mge::Widget *parent, ui::LayoutReader *reader) override { \
        return Node(new T);                             \
    }

namespace ui {

    class LayoutReader;
    class NodeLoader {
    public:
        typedef std::shared_ptr<mge::Widget> Node;
        typedef LayoutSelectorAssigner::Selector Selector;
        typedef std::map<std::string, std::string> Params;
    public:
        virtual ~NodeLoader();
        virtual void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value);
        virtual Selector onResolveSelector(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value);
    public:
        Node loadNode(mge::Widget* parent, LayoutReader* reader);
    public:
        // for param list
        virtual bool hasParamList() const {
            return false;
        }
        virtual void onParamReceiveBegin(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) {}
        virtual void onParamReceiveEnd(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) {}
        virtual void onParamReceive(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, Params const& params) {}
    protected:
        virtual Node createNode(mge::Widget* parent, LayoutReader* reader) = 0;
    };

    class WidgetLoader : public NodeLoader {
        Node createNode(mge::Widget *parent, LayoutReader *reader) override;
    };

    class ImageWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class WindowWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget *parent, LayoutReader *reader) override;
    };

    class TTFLabelLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class ButtonWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class MaskWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class ProgressBarWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        void onParseProperty(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, const char* name, const char* value) override;
    };

    class RenderTargetWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
    };

    class FrameImageWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        bool hasParamList() const override;
        void onParamReceiveBegin(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) override;
        void onParamReceive(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, Params const& params) override;
        void onParamReceiveEnd(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) override;
        typedef std::shared_ptr<mge::Texture> TexturePtr;
        typedef std::vector<TexturePtr> FrameArray;
        FrameArray _frames;
    };

    class FrameAnimationWidgetLoader : public NodeLoader {
        Node createNode(mge::Widget* parent, LayoutReader* reader) override;
        bool hasParamList() const override;
        void onParamReceiveBegin(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) override;
        void onParamReceive(mge::Widget* node, mge::Widget* parent, LayoutReader* reader, Params const& params) override;
        void onParamReceiveEnd(mge::Widget* node, mge::Widget* parent, LayoutReader* reader) override;
        typedef std::shared_ptr<mge::Texture> TexturePtr;
        typedef std::vector<TexturePtr> FrameArray;
        FrameArray _frames;
    };
}

#endif //SDL2_UI_NODE_LOADER_H
