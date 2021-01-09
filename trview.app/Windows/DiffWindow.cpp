#include "DiffWindow.h"
#include <trview.ui/StackPanel.h>

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
        : CollapsiblePanel(device, shader_storage, font_factory, parent, L"DiffWindow", L"Diff", Size(800, 800))
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

        return std::move(panel);
    }

    std::unique_ptr<ui::Control> DiffWindow::create_right_panel()
    {
        using namespace ui;
        return std::make_unique<StackPanel>(Size(550, window().size().height), Colours::RightPanel);
    }

    void DiffWindow::set_diff(const Diff& diff)
    {
        using namespace ui;
        std::vector<Listbox::Item> list_items;
        std::transform(diff.changes.begin(), diff.changes.end(), std::back_inserter(list_items), create_listbox_item);
        _diff_list->set_items(list_items);
    }
}
