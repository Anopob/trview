#pragma once

#include <vector>
#include <memory>
#include <optional>

#include <trview.common/MessageHandler.h>
#include <trview.graphics/Device.h>
#include <trview.graphics/IShaderStorage.h>
#include <trview.graphics/IFontFactory.h>
#include <trview.common/TokenStore.h>
#include "TriggersWindow.h"

namespace trview
{
    class Shortcuts;

    /// Controls and creates TriggersWindows.
    class TriggersWindowManager final : public MessageHandler
    {
    public:
        /// Create an TriggersWindowManager.
        /// @param device The device to use for triggers windows.
        /// @param shader_storage The shader storage for triggers windows.
        /// @param font_factory The font_factory for triggers windows.
        /// @param window The parent window of the triggers window.
        explicit TriggersWindowManager(graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory, const Window& window, IShortcuts& shortcuts);

        /// Destructor for the TriggersWindowManager.
        virtual ~TriggersWindowManager() = default;

        /// Handles a window message.
        /// @param message The message that was received.
        /// @param wParam The WPARAM for the message.
        /// @param lParam The LPARAM for the message.
        virtual void process_message(UINT message, WPARAM wParam, LPARAM lParam) override;

        /// Event raised when an item is selected in one of the trigger windows.
        Event<Item> on_item_selected;

        Event<Trigger*, bool> on_trigger_visibility;

        /// Event raised when a trigger is selected in one of the trigger windows.
        Event<Trigger*> on_trigger_selected;

        /// Event raised when the 'add to route' button is pressed in one of the trigger windows.
        Event<const Trigger*> on_add_to_route;

        /// Render all of the triggers windows.
        /// @param device The device to use to render.
        /// @param vsync Whether to use vsync.
        void render(graphics::Device& device, bool vsync);

        /// Set the items to use in the windows.
        /// @param items The items in the level.
        void set_items(const std::vector<Item>& items);

        /// Set the triggers to use in the windows.
        /// @param triggers The triggers in the level.
        void set_triggers(const std::vector<Trigger*>& triggers);

        /// Set whether a trigger is visible.
        /// @param trigger The trigger.
        /// @param visible Whether the trigger is visible.
        void set_trigger_visible(Trigger* trigger, bool visible);

        /// Set the current room to filter trigger windows.
        /// @param room The current room.
        void set_room(uint32_t room);

        /// Set the currently selected trigger.
        /// @param item The selected trigger.
        void set_selected_trigger(const Trigger* const trigger);

        /// Create a new triggers window.
        TriggersWindow* create_window();
    private:
        std::vector<std::unique_ptr<TriggersWindow>> _windows;
        std::vector<TriggersWindow*> _closing_windows;
        std::vector<Item> _items;
        std::vector<Trigger*> _triggers;
        graphics::Device& _device;
        const graphics::IShaderStorage& _shader_storage;
        const graphics::IFontFactory& _font_factory;
        uint32_t _current_room{ 0u };
        TokenStore _token_store;
        std::optional<const Trigger*> _selected_trigger;
    };
}

