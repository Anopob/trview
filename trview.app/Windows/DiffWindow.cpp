#include "DiffWindow.h"
#include <trview.ui/StackPanel.h>
#include <trview.ui/Label.h>
#include <trview.ui/GroupBox.h>
#include <trview.common/Strings.h>
#include <trview.ui/Button.h>
#include <trview.ui/Grid.h>

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
        : CollapsiblePanel(device, shader_storage, font_factory, parent, L"DiffWindow", L"Diff", Size(480, 400))
    {
        set_panels(create_left_panel(), create_right_panel());
    }

    std::unique_ptr<ui::Control> DiffWindow::create_left_panel()
    {
        using namespace ui;
        auto panel = std::make_unique<StackPanel>(Size(180, window().size().height), Colours::LeftPanel);

        _diff_list = panel->add_child(std::make_unique<Listbox>(Size(180, 400), Colours::Diffs));
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
                start_item_diff(_diff.changes[std::stoi(item.value(L"#"))]);
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
        auto panel = std::make_unique<ui::StackPanel>(Size(300, window().size().height), Colours::RightPanel);

        auto switcher = panel->add_child(std::make_unique<ui::Grid>(Size(300, 32), Colour::Blue, 2, 1));
        _a_button = switcher->add_child(std::make_unique<ui::Button>(Size(150, 32), L"A"));
        _token_store += _a_button->on_click += [&]()
        {
            _a_button->set_text_background_colour(Colour::LightGrey);
            _b_button->set_text_background_colour(Colour::Grey);
            on_version_selected(Version::Left);
        };
        _b_button = switcher->add_child(std::make_unique<ui::Button>(Size(150, 32), L"B"));
        _token_store += _b_button->on_click += [&]()
        {
            _a_button->set_text_background_colour(Colour::Grey);
            _b_button->set_text_background_colour(Colour::LightGrey);
            on_version_selected(Version::Right);
        };

        _a_button->set_text_background_colour(Colour::LightGrey);
        _b_button->set_text_background_colour(Colour::Grey);

        auto area = panel->add_child(std::make_unique<ui::Window>(Size(300, window().size().height - switcher->size().height), Colours::RightPanel));

        // Different panels for different diff types.
        _items_diff = area->add_child(create_item_diff_panel());

        _triggers_diff = area->add_child(std::make_unique<StackPanel>(Size(300, window().size().height), Colours::RightPanel));
        _triggers_diff->set_visible(false);
        _triggers_diff->add_child(std::make_unique<Label>(Size(100, 20), Colours::RightPanel, L"Triggers Diff...", 8));

        _geometry_diff = area->add_child(std::make_unique<StackPanel>(Size(300, window().size().height), Colours::RightPanel));
        _geometry_diff->set_visible(false);
        _geometry_diff->add_child(std::make_unique<Label>(Size(100, 20), Colours::RightPanel, L"Geometry Diff...", 8));

        return std::move(panel);
    }

    std::unique_ptr<ui::Control> DiffWindow::create_item_diff_panel()
    {
        using namespace ui;
        auto panel = std::make_unique<StackPanel>(Size(300, window().size().height), Colours::RightPanel);
        panel->set_visible(false);

        auto diff_pair = panel->add_child(std::make_unique<StackPanel>(Size(300, 300), Colours::RightPanel, Size(), StackPanel::Direction::Horizontal));
        _item_diff = diff_pair->add_child(std::make_unique<Listbox>(Size(300, 300), Colours::RightPanel));
        _item_diff->set_show_scrollbar(false);
        _item_diff->set_columns({
                { Listbox::Column::Type::String, L"Property", 60 },
                { Listbox::Column::Type::String, L"Left", 120 },
                { Listbox::Column::Type::String, L"Right", 120 },
            });
        return std::move(panel);
    }

    void DiffWindow::set_diff(const Diff& diff)
    {
        _diff = diff;

        using namespace ui;
        std::vector<Listbox::Item> list_items;
        std::transform(diff.changes.begin(), diff.changes.end(), std::back_inserter(list_items), create_listbox_item);
        _diff_list->set_items(list_items);
    }

    void DiffWindow::set_items(const std::vector<Item>& left, const std::vector<Item>& right)
    {
        _left_items = left;
        _right_items = right;
    }

    namespace
    {
        std::wstring to_string(const DirectX::SimpleMath::Vector3& pos)
        {
            std::wstringstream pos_string;
            pos_string << pos.x * trlevel::Scale_X << L", " << pos.y * trlevel::Scale_Y << L", " << pos.z * trlevel::Scale_Z;
            return pos_string.str();
        }
    }

    void DiffWindow::start_item_diff(const Diff::Change& change)
    {
        _items_diff->set_visible(true);
        _triggers_diff->set_visible(false);
        _geometry_diff->set_visible(false);

        uint32_t i = 0;
        std::vector<ui::Listbox::Item> items;
        auto add_row = [&](auto name, auto left_value, auto right_value)
        {
            items.push_back(
                { {{ L"#", std::to_wstring(i++) },
                { L"Property", name },
                { L"Left", left_value },
                { L"Right", right_value }} });
        };

        if (change.type == Diff::Change::Type::Edit)
        {
            const auto left = _left_items[change.index];
            const auto right = _right_items[change.index];

            add_row(L"Type", left.type(), right.type());
            add_row(L"Position", to_string(left.position()), to_string(right.position()));
            add_row(L"Type ID", std::to_wstring(left.type_id()), std::to_wstring(right.type_id()));
            add_row(L"Room", std::to_wstring(left.room()), std::to_wstring(right.room()));
            add_row(L"Clear Body", format_bool(left.clear_body_flag()), format_bool(right.clear_body_flag()));
            add_row(L"Invisible", format_bool(left.invisible_flag()), format_bool(right.invisible_flag()));
            add_row(L"Flags", format_binary(left.activation_flags()), format_binary(right.activation_flags()));
            add_row(L"OCB", std::to_wstring(left.ocb()), std::to_wstring(right.ocb()));
        }

        _item_diff->set_items(items);
    }
}
