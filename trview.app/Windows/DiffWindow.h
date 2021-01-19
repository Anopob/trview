#pragma once

#include "CollapsiblePanel.h"
#include <trview.app/Diff/Diff.h>
#include <trview.ui/Listbox.h>
#include <trview.app/Elements/Item.h>

namespace trview
{
    class DiffWindow final : public CollapsiblePanel
    {
    public:
        explicit DiffWindow(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& parent);
        virtual ~DiffWindow() = default;
        void set_diff(const Diff& diff);

        enum class Version
        {
            Left,
            Right
        };

        Event<Version> on_version_selected;
        Event<Item> on_item_selected;
        Event<const Trigger*> on_trigger_selected;
    private:
        void start_item_diff(const Diff::Change& change);
        void start_trigger_diff(const Diff::Change& change);

        std::unique_ptr<ui::Control> create_left_panel();
        std::unique_ptr<ui::Control> create_right_panel();
        std::unique_ptr<ui::Control> create_item_diff_panel();

        Diff _diff;
        ui::Listbox* _diff_list{ nullptr };
        ui::Control* _items_diff{ nullptr };
        ui::Window* _geometry_diff{ nullptr };
        ui::Listbox* _item_diff{ nullptr };
        ui::Button* _a_button{ nullptr };
        ui::Button* _b_button{ nullptr };
    };
}
