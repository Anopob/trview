#include "DiffWindowManager.h"
#include <trview.app/Windows/WindowIDs.h>

namespace trview
{
    DiffWindowManager::DiffWindowManager(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& window, Shortcuts& shortcuts)
        : MessageHandler(window), _device(device), _font_factory(font_factory), _shader_storage(shader_storage)
    {
        _token_store += shortcuts.add_shortcut(true, 'D') += [&]() { create_window(); };
    }

    void DiffWindowManager::process_message(UINT message, WPARAM wParam, LPARAM)
    {
        if (message == WM_COMMAND && LOWORD(wParam) == ID_APP_WINDOWS_DIFF)
        {
            create_window();
        }
    }

    void DiffWindowManager::create_window()
    {
        // If the window already exists, just focus on the window.
        if (_diff_window)
        {
            SetForegroundWindow(_diff_window->window());
            return;
        }

        // Otherwise create the window.
        _diff_window = std::make_unique<DiffWindow>(_device, _shader_storage, _font_factory, window());
        _diff_window->on_item_selected += on_item_selected;
        _diff_window->on_version_selected += on_version_selected;
        _token_store += _diff_window->on_window_closed += [&]() { _closing = true; };

        _diff_window->set_items(_left_items, _right_items);
        if (_diff.has_value())
        {
            _diff_window->set_diff(_diff.value());
        }
    }

    void DiffWindowManager::set_items(const std::vector<Item>& left, const std::vector<Item>& right)
    {
        _left_items = left;
        _right_items = right;
        if (_diff_window)
        {
            _diff_window->set_items(_left_items, _right_items);
        }
    }

    void DiffWindowManager::set_diff(const Diff& diff)
    {
        _diff = diff;
        if (_diff_window)
        {
            _diff_window->set_diff(_diff.value());
        }
    }

    void DiffWindowManager::render(graphics::Device& device, bool vsync)
    {
        if (_closing)
        {
            _diff_window.reset();
            _closing = false;
        }

        if (_diff_window)
        {
            _diff_window->render(device, vsync);
        }
    }
}
