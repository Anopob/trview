#pragma once

#include <SimpleMath.h>
#include <trview.app/Geometry/IRenderable.h>
#include <trview.app/Geometry/Mesh.h>
#include <trview.common/Colour.h>

namespace trview
{
    /// A waypoint is an entry in a route.
    class Waypoint final : public IRenderable
    {
    public:
        /// Defines the type of waypoint.
        enum class Type
        {
            /// The waypoint is to a position in the world.
            Position,
            /// The waypoint is to an entity in the world.
            Entity,
            /// The waypoint is to a trigger in the world.
            Trigger
        };

        /// Create a new waypoint of position type.
        /// @param mesh The waypoint mesh.
        /// @param position The position of the waypoint in the world.
        /// @param room The room that the waypoint is in.
        explicit Waypoint(Mesh* mesh, const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& normal, uint32_t room);

        /// Create a new waypoint.
        /// @param mesh The waypoint mesh.
        /// @param position The position of the waypoint in the world.
        /// @param room The room that waypoint is in.
        /// @param type The type of waypoint.
        /// @param index The index of the entity or trigger being referenced if this is a non-position type.
        /// @param route_colour The colour of the route.
        explicit Waypoint(Mesh* mesh, const DirectX::SimpleMath::Vector3& position, const DirectX::SimpleMath::Vector3& normal, uint32_t room, Type type, uint32_t index, const Colour& route_colour);

        /// Destructor for waypoint.
        virtual ~Waypoint() = default;

        /// Render the waypoint in the 3D view.
        /// @param device The device to use to render the waypoint.
        /// @param camera The current camera being used for rendering.
        /// @param texture_storage The current texture storage instance.
        /// @param colour The colour to render this object.
        virtual void render(const graphics::Device& device, const ICamera& camera, const ILevelTextureStorage& texture_storage, const DirectX::SimpleMath::Color& colour) override;

        /// Get the transparent triangles that are contained in this object.
        /// @param transparency The transparency buffer to add triangles to.
        /// @param camera The current camera being used for rendering.
        /// @param colour The colour to render the triangles.
        virtual void get_transparent_triangles(TransparencyBuffer& transparency, const ICamera& camera, const DirectX::SimpleMath::Color& colour) override;

        /// Get the position of the waypoint in the 3D view.
        DirectX::SimpleMath::Vector3 position() const;

        DirectX::SimpleMath::Vector3 blob_position() const;

        /// Get the type of the waypoint.
        Type type() const;

        /// Get whether the waypoint has an attached save file.
        bool has_save() const;

        /// Gets the index of the entity or trigger that the waypoint refers to.
        uint32_t index() const;

        /// Gets the room that the waypoint is in.
        uint32_t room() const;

        /// Get any notes associated with the waypoint.
        std::wstring notes() const;

        /// Get the contents of the attached save file.
        std::vector<uint8_t> save_file() const;

        /// Set the notes associated with the waypoint.
        /// @param notes The notes to save.
        void set_notes(const std::wstring& notes);

        /// Set the route colour for the waypoint blob.
        /// @param colour The colour of the route.
        void set_route_colour(const Colour& colour);

        /// Set the contents of the attached save file.
        void set_save_file(const std::vector<uint8_t>& data);

        virtual bool visible() const override;
        virtual void set_visible(bool value) override;
    private:
        DirectX::SimpleMath::Matrix calculate_waypoint_rotation() const;

        std::wstring                 _notes;
        std::vector<uint8_t>         _save_data;
        DirectX::SimpleMath::Vector3 _position;
        DirectX::SimpleMath::Vector3 _normal;
        Mesh*                        _mesh;
        Type                         _type;
        uint32_t                     _index;
        uint32_t                     _room;
        Colour                       _route_colour;
        bool                         _visible{ true };
    };

    Waypoint::Type waypoint_type_from_string(const std::string& value);
    std::wstring waypoint_type_to_string(Waypoint::Type type);
}
