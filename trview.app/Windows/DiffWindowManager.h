#pragma once

#include <trview.common/MessageHandler.h>
#include <trview.graphics/Device.h>
#include <trview.graphics/IShaderStorage.h>
#include <trview.graphics/IFontFactory.h>
#include <trview.common/Windows/Shortcuts.h>
#include "DiffWindow.h"

namespace trview
{
    class DiffWindowManager final : public MessageHandler
    {
    public:
        explicit DiffWindowManager(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& window, Shortcuts& shortcuts);
        virtual ~DiffWindowManager() = default;
        virtual void process_message(UINT message, WPARAM wParam, LPARAM lParam) override;
        void render(graphics::Device& device, bool vsync);
        void create_window();

        void set_items(const std::vector<Item>& left, const std::vector<Item>& right);
        void set_diff(const Diff& diff);
        void clear_diff();

        Event<DiffWindow::Version> on_version_selected;
        Event<Item> on_item_selected;
    private:
        graphics::Device& _device;
        const graphics::IShaderStorage& _shader_storage;
        const graphics::IFontFactory& _font_factory;
        TokenStore _token_store;
        std::unique_ptr<DiffWindow> _diff_window;
        std::vector<Item> _left_items;
        std::vector<Item> _right_items;
        std::optional<Diff> _diff;
        bool _closing{ false };
    };
}
