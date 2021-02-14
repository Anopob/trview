/// @file Viewer.h
/// @brief Class that brings together the component parts of the application.
/// 
/// Class that coordinates all the parts of the application.

#pragma once

#include <Windows.h>
#include <cstdint>
#include <memory>
#include <string>

#include <trview.common/Timer.h>
#include <trview.common/Window.h>
#include <trview.graphics/Device.h>
#include <trview.input/Keyboard.h>
#include <trview.input/Mouse.h>
#include <trview.common/TokenStore.h>

#include <trview.app/Camera/FreeCamera.h>
#include <trview.app/Camera/OrbitCamera.h>
#include <trview.app/Camera/CameraInput.h>
#include <trview.app/Camera/CameraMode.h>
#include <trview.app/Elements/Level.h>
#include <trview.app/Settings/UserSettings.h>
#include <trview.app/Windows/WindowResizer.h>
#include <trview.app/Windows/ItemsWindowManager.h>
#include <trview.app/Windows/TriggersWindowManager.h>
#include <trview.app/Windows/RoomsWindowManager.h>
#include <trview.app/Tools/Measure.h>
#include <trview.app/Tools/Compass.h>
#include <trview.app/Menus/AlternateGroupToggler.h>
#include <trview.app/Routing/Route.h>
#include <trview.app/Windows/RouteWindowManager.h>
#include <trview.app/Menus/ViewMenu.h>
#include <trview.app/Geometry/Picking.h>
#include <trview.app/Graphics/SectorHighlight.h>
#include <trview.app/UI/ViewerUI.h>
#include <trview.app/Elements/ITypeNameLookup.h>
#include <trview.app/Menus/MenuDetector.h>
#include <trview.app/Lua/Lua.h>
#include <trview.common/Windows/Shortcuts.h>

namespace trview
{
    struct ITextureStorage;

    namespace graphics
    {
        struct IShaderStorage;
        struct IFontFactory;
        class DeviceWindow;
    }

    /// Class that coordinates all the parts of the application.
    class Viewer : public MessageHandler
    {
    public:
        /// Create a new viewer.
        /// @param window The window that the viewer should use.
        explicit Viewer(const Window& window, graphics::Device& device, const graphics::IShaderStorage& shader_storage, const graphics::IFontFactory& font_factory);

        /// Destructor for the viewer.
        virtual ~Viewer() = default;

        /// Render the viewer.
        void render();

        /// Attempt to open the specified level file.
        /// @param filename The level file to open.
        void open(Level* level);

        virtual void process_message(UINT message, WPARAM wParam, LPARAM lParam) override;

        /// Event raised when the user settings have changed.
        /// @remarks The settings is passed as a parameter to the listener functions.
        Event<UserSettings> on_settings;

        void set_settings(const UserSettings& settings);
    private:
        void initialise_input();
        void toggle_highlight();
        void update_camera();
        void render_scene();
        void select_room(uint32_t room, bool force_orbit = false);
        void select_item(const Item& item);
        void select_trigger(const Trigger* const trigger);
        void select_waypoint(uint32_t index);
        void select_next_waypoint();
        void select_previous_waypoint();
        void set_item_visibility(const Item& item, bool visible);
        void set_trigger_visibility(Trigger* trigger, bool visible);
        void remove_waypoint(uint32_t index);
        bool should_pick() const;
        const ICamera& current_camera() const;
        ICamera& current_camera();
        void set_camera_mode(CameraMode camera_mode);
        void set_camera_projection_mode(ProjectionMode projection_mode);
        void set_alternate_mode(bool enabled);
        void toggle_alternate_mode();
        void set_alternate_group(uint32_t group, bool enabled);
        bool alternate_group(uint32_t group) const;
        // Tell things that need to be resized that they should resize.
        void resize_elements();
        // Set up keyboard and mouse input for the camera.
        void setup_camera_input();
        void set_show_triggers(bool show);
        void toggle_show_triggers();
        void set_show_hidden_geometry(bool show);
        void toggle_show_hidden_geometry();
        void set_show_water(bool show);
        void set_show_wireframe(bool show);
        uint32_t room_from_pick(const PickResult& pick) const;
        void add_recent_orbit(const PickResult& pick);
        void select_previous_orbit();
        void select_next_orbit();
        void select_pick(const PickResult& pick);

        void register_lua();
        void apply_acceleration_settings();

        graphics::Device& _device;
        Shortcuts _shortcuts;
        std::unique_ptr<graphics::DeviceWindow> _main_window;
        std::unique_ptr<ItemsWindowManager> _items_windows;
        std::unique_ptr<TriggersWindowManager> _triggers_windows;
        std::unique_ptr<RoomsWindowManager> _rooms_windows;
        Level* _level;
        Timer _timer;
        OrbitCamera _camera;
        FreeCamera _free_camera;
        input::Keyboard _keyboard;
        input::Mouse _mouse;
        std::unique_ptr<ViewerUI> _ui;
        CameraMode _camera_mode{ CameraMode::Orbit };
        CameraInput _camera_input;
        std::unique_ptr<ITextureStorage> _texture_storage;
        const graphics::IShaderStorage& _shader_storage;
        UserSettings _settings;
        std::unique_ptr<Picking> _picking;
        PickResult _current_pick;
        WindowResizer _window_resizer;
        TokenStore _token_store;
        AlternateGroupToggler _alternate_group_toggler;
        DirectX::SimpleMath::Vector3 _target;
        ViewMenu _view_menu;
        bool _show_selection{ true };
        SectorHighlight _sector_highlight;
        MenuDetector _menu_detector;

        // Tools:

        Tool _active_tool{ Tool::None };
        std::unique_ptr<Measure> _measure;
        std::unique_ptr<Compass> _compass;
        std::optional<Compass::Axis> _compass_axis;

        // Temporary route objects.
        PickResult _context_pick;
        std::unique_ptr<Route> _route;
        std::unique_ptr<RouteWindowManager> _route_window_manager;
        bool _show_route{ true };

        /// Was the room just changed due to an alternate group or flip being performed?
        bool _was_alternate_select{ false };

        std::unique_ptr<graphics::RenderTarget> _scene_target;
        std::unique_ptr<graphics::Sprite> _scene_sprite;
        bool _scene_changed{ true };
        bool _mouse_changed{ true };
        bool _ui_changed{ true };

        std::vector<PickResult> _recent_orbits;
        std::size_t _recent_orbit_index{ 0u };

        LuaFunctionRegistry _lua_registry;
    };
}

