#pragma once

#include "CollapsiblePanel.h"
#include <trview.app/Diff/Diff.h>
#include <trview.ui/Listbox.h>

namespace trview
{
    class DiffWindow final : public CollapsiblePanel
    {
    public:
        explicit DiffWindow(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& parent);
        virtual ~DiffWindow() = default;
        void set_diff(const Diff& diff);
    private:
        std::unique_ptr<ui::Control> create_left_panel();
        std::unique_ptr<ui::Control> create_right_panel();
        std::unique_ptr<ui::Control> create_item_diff_panel();
        ui::Listbox* _diff_list{ nullptr };
        ui::Control* _items_diff{ nullptr };
        ui::Window* _triggers_diff{ nullptr };
        ui::Window* _geometry_diff{ nullptr };

    };
}
