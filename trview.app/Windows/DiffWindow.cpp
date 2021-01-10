#include "DiffWindow.h"
#include <trview.ui/StackPanel.h>
#include <trview.ui/Label.h>
#include <trview.ui/GroupBox.h>

namespace trview
{
    namespace Colours
    {
        const Colour LeftPanel{ 1.0f, 0.25f, 0.25f, 0.25f };
        const Colour RightPanel{ 1.0f, 0.225f, 0.225f, 0.225f };
        const Colour Diffs{ 1.0f, 0.20f, 0.20f, 0.20f };
    }

    namespace
    {
        std::wstring convert_change_type(Diff::Change::Type type)
        {
            switch (type)
            {
                case Diff::Change::Type::Add:
                    return L"Add";
                case Diff::Change::Type::Edit:
                    return L"Edit";
                case Diff::Change::Type::Delete:
                    return L"Delete";
            }
            return L"Unknown";
        }

        std::wstring convert_change_subject(Diff::Change::Subject subject)
        {
            switch (subject)
            {
                case Diff::Change::Subject::Item:
                    return L"Item";
                case Diff::Change::Subject::Trigger:
                    return L"Trigger";
            }
            return L"Unknown";
        }

        ui::Listbox::Item create_listbox_item(const Diff::Change& change)
        {
            return { {{ L"#", std::to_wstring(change.number) },
                     { L"Type", convert_change_type(change.type) },
                     { L"Subject", convert_change_subject(change.subject) },
                     { L"Index", std::to_wstring(change.index) }} };
        }
    }

    DiffWindow::DiffWindow(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& parent)
        : CollapsiblePanel(device, shader_storage, font_factory, parent, L"DiffWindow", L"Diff", Size(800, 400))
    {
        set_panels(create_left_panel(), create_right_panel());
    }

    std::unique_ptr<ui::Control> DiffWindow::create_left_panel()
    {
        using namespace ui;
        auto panel = std::make_unique<StackPanel>(Size(250, window().size().height), Colours::LeftPanel);

        _diff_list = panel->add_child(std::make_unique<Listbox>(Size(250, 400), Colours::Diffs));
        _diff_list->set_columns({
                { Listbox::Column::Type::Number, L"#", 35 },
                { Listbox::Column::Type::String, L"Type", 45 },
                { Listbox::Column::Type::String, L"Subject", 45 },
                { Listbox::Column::Type::Number, L"Index", 45 },
            }
        );
        _token_store += _diff_list->on_item_selected += [this](const auto& item)
        {
            const auto type = item.value(L"Subject");
            if (type == L"Item")
            {
                _items_diff->set_visible(true);
                _triggers_diff->set_visible(false);
                _geometry_diff->set_visible(false);
            }
            else if (type == L"Trigger")
            {
                _items_diff->set_visible(false);
                _triggers_diff->set_visible(true);
                _geometry_diff->set_visible(false);
            }
            else if (type == L"Geometry")
            {
                _items_diff->set_visible(false);
                _triggers_diff->set_visible(false);
                _geometry_diff->set_visible(true);
            }
        };

        return std::move(panel);
    }

    std::unique_ptr<ui::Control> DiffWindow::create_right_panel()
    {
        using namespace ui;
        auto panel = std::make_unique<ui::Window>(Size(550, window().size().height), Colours::RightPanel);

        // Different panels for different diff types.
        _items_diff = panel->add_child(create_item_diff_panel());

        _triggers_diff = panel->add_child(std::make_unique<StackPanel>(Size(550, window().size().height), Colours::RightPanel));
        _triggers_diff->set_visible(false);
        _triggers_diff->add_child(std::make_unique<Label>(Size(100, 20), Colours::RightPanel, L"Triggers Diff...", 8));

        _geometry_diff = panel->add_child(std::make_unique<StackPanel>(Size(550, window().size().height), Colours::RightPanel));
        _geometry_diff->set_visible(false);
        _geometry_diff->add_child(std::make_unique<Label>(Size(100, 20), Colours::RightPanel, L"Geometry Diff...", 8));

        return std::move(panel);
    }

    std::unique_ptr<ui::Control> DiffWindow::create_item_diff_panel()
    {
        using namespace ui;
        auto panel = std::make_unique<StackPanel>(Size(550, window().size().height), Colours::RightPanel);
        panel->set_visible(false);

        auto diff_pair = panel->add_child(std::make_unique<StackPanel>(Size(550, 300), Colours::RightPanel, Size(), StackPanel::Direction::Horizontal));
        auto left = diff_pair->add_child(std::make_unique<Listbox>(Size(225, 300), Colour::Green));
        left->set_columns({
                { Listbox::Column::Type::Number, L"#", 35 },
                { Listbox::Column::Type::String, L"Left", 45 },
                { Listbox::Column::Type::String, L"Right", 45 },
            });

        return std::move(panel);
    }

    void DiffWindow::set_diff(const Diff& diff)
    {
        using namespace ui;
        std::vector<Listbox::Item> list_items;
        std::transform(diff.changes.begin(), diff.changes.end(), std::back_inserter(list_items), create_listbox_item);
        _diff_list->set_items(list_items);
    }
}
