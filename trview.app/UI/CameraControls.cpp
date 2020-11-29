#include "CameraControls.h"

#include <trview.ui/GroupBox.h>
#include <trview.ui/Checkbox.h>
#include <trview.ui/Button.h>
#include <trview.ui/Slider.h>
#include <trview.ui/Label.h>
#include <trview.ui/Grid.h>

namespace trview
{
    CameraControls::CameraControls(ui::Control& parent)
    {
        using namespace ui;

        auto camera_window = std::make_unique<GroupBox>(Size(150, 92), Colour::Transparent, Colour::Grey, L"Camera");

#if 1
        // Use the new grid control.
        auto grid = camera_window->add_child(std::make_unique<Grid>(Size(150, 92), Colour::Transparent, Grid::InsertOrder::Column));

        // Make a button with a label next to it, until this kind of control exists.
        auto create_labelled_button = [](Event<>& on_click, const std::wstring& text)
        {
            auto panel = std::make_unique<StackPanel>(Size(16, 16), Colour::Transparent, Size(), StackPanel::Direction::Horizontal);
            auto button = panel->add_child(std::make_unique<Button>(Size(16, 16)));
            button->on_click += on_click;
            panel->add_child(std::make_unique<Label>(Size(40, 16), Colour::Transparent, text, 8, graphics::TextAlignment::Left, graphics::ParagraphAlignment::Centre));
            return std::move(panel);
        };

        grid->add_child(create_labelled_button(on_reset, L"Reset"));

        _orbit = grid->add_child(std::make_unique<Checkbox>(Point(74, 0), Colour::Transparent, L"Orbit"));
        _token_store += _orbit->on_state_changed += [&](auto) { change_mode(CameraMode::Orbit); };

        _free = grid->add_child(std::make_unique<Checkbox>(Point(0, 21), Colour::Transparent, L"Free"));
        _token_store += _free->on_state_changed += [&](auto) { change_mode(CameraMode::Free); };

        _axis = grid->add_child(std::make_unique<Checkbox>(Point(74, 21), Colour::Transparent, L"Axis"));
        _token_store += _axis->on_state_changed += [&](auto) { change_mode(CameraMode::Axis); };

        _ortho = grid->add_child(std::make_unique<Checkbox>(Point(0, 43), Colour::Transparent, L"Ortho"));
        _token_store += _ortho->on_state_changed += [&](auto ortho_enabled) { change_projection(ortho_enabled ? ProjectionMode::Orthographic : ProjectionMode::Perspective); };
#else

        auto reset_camera = std::make_unique<Button>(Size(16, 16));
        reset_camera->on_click += on_reset;

        auto reset_camera_label = std::make_unique<Label>(Point(20, 0), Size(40, 16), Colour::Transparent, L"Reset", 8, graphics::TextAlignment::Left, graphics::ParagraphAlignment::Centre);

        auto orbit_camera = std::make_unique<Checkbox>(Point(74, 0), Colour::Transparent, L"Orbit");
        _token_store += orbit_camera->on_state_changed += [&](auto) { change_mode(CameraMode::Orbit); };

        auto free_camera = std::make_unique<Checkbox>(Point(0, 21), Colour::Transparent, L"Free");
        _token_store += free_camera->on_state_changed += [&](auto) { change_mode(CameraMode::Free); };

        auto axis_camera = std::make_unique<Checkbox>(Point(74, 21), Colour::Transparent, L"Axis");
        _token_store += axis_camera->on_state_changed += [&](auto) { change_mode(CameraMode::Axis); };

        auto ortho = std::make_unique<Checkbox>(Point(0, 43), Colour::Transparent, L"Ortho");
        _token_store += ortho->on_state_changed += [&](auto ortho_enabled) { change_projection(ortho_enabled ? ProjectionMode::Orthographic : ProjectionMode::Perspective); };

        camera_window->add_child(std::move(reset_camera));
        camera_window->add_child(std::move(reset_camera_label));
        _orbit = camera_window->add_child(std::move(orbit_camera));
        _free = camera_window->add_child(std::move(free_camera));
        _axis = camera_window->add_child(std::move(axis_camera));
        _ortho = camera_window->add_child(std::move(ortho));
#endif

        parent.add_child(std::move(camera_window));
    }

    // Set the current camera mode and raise the on_mode_selected event.
    // mode: The new camera mode.
    void CameraControls::change_mode(CameraMode mode)
    {
        set_mode(mode);
        on_mode_selected(mode);
    }

    void CameraControls::change_projection(ProjectionMode mode)
    {
        set_projection_mode(mode);
        on_projection_mode_selected(mode);
    }

    // Set the current camera mode. This will not raise the on_mode_selected event.
    // mode: The camera mode to change to.
    void CameraControls::set_mode(CameraMode mode)
    {
        _orbit->set_state(mode == CameraMode::Orbit);
        _free->set_state(mode == CameraMode::Free);
        _axis->set_state(mode == CameraMode::Axis);
    }

    void CameraControls::set_projection_mode(ProjectionMode mode)
    {
        _ortho->set_state(mode == ProjectionMode::Orthographic);
    }
}
