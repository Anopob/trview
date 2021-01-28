#include "Level.h"
#include "LevelLoadException.h"

namespace trlevel
{
    namespace
    {
        const float PiMul2 = 6.283185307179586476925286766559f;
        const int16_t Lara = 0;
        const int16_t LaraSkinTR3 = 315;
        const int16_t LaraSkinPostTR3 = 8;
    }

    // Get the version of the game that the level was built for.
    // Returns: The level version.
    LevelVersion Level::get_version () const
    {
        return _version;
    }

    // Get the platform (PC, PSX, etc.) that this level was built for.
    // Returns: The platform.
    LevelTarget Level::get_target_platform () const
    {
        return _target_platform;
    }

    tr_colour Level::get_palette_entry8(uint32_t index) const
    {
        return _palette[index];
    }

    tr_colour4 Level::get_palette_entry_16(uint32_t index) const
    {
        return _palette16[index];
    }

    tr_colour4 Level::get_palette_entry(uint32_t index) const
    {
        if (index < _palette16.size())
        {
            return get_palette_entry_16(index);
        }
        else if ( index < _palette.size () )
        {
            return convert_to_colour4 ( get_palette_entry8 ( index ) );
        }
        return { 0 };
    }

    tr_colour4 Level::get_palette_entry(uint32_t index8, uint32_t index16) const
    {
        if (index16 < _palette16.size())
        {
            return get_palette_entry_16(index16);
        }
        return convert_to_colour4(get_palette_entry8(index8));
    }

    uint32_t Level::num_textiles() const
    {
        return _num_textiles;
    }

    tr_textile4 Level::get_textile4 ( uint32_t index ) const
    {
        return _textile4 [index];
    }

    tr_clut Level::get_clut ( uint32_t index ) const
    {
        return _clut [index];
    }

    tr_textile8 Level::get_textile8(uint32_t index) const
    {
        return _textile8[index];
    }

    tr_textile16 Level::get_textile16(uint32_t index) const
    {
        return _textile16[index];
    }

    std::vector<uint32_t> Level::get_textile(uint32_t index) const
    {
        std::vector<uint32_t> results;
        results.reserve(256 * 256);

        if (index < _textile32.size())
        {
            const auto& textile = _textile32[index];
            std::transform(textile.Tile,
                textile.Tile + sizeof(textile.Tile) / sizeof(uint32_t),
                std::back_inserter(results),
                convert_textile32);
        }
        else if (index < _textile16.size())
        {
            const auto& textile = _textile16[index];
            std::transform(textile.Tile, 
                textile.Tile + sizeof(textile.Tile) / sizeof(uint16_t),  
                std::back_inserter(results),
                convert_textile16);
        }
        else 
        {
            const auto& textile = _textile8[index];
            std::transform(textile.Tile,
                textile.Tile + sizeof(textile.Tile) / sizeof(uint8_t),
                std::back_inserter(results),
                [&](uint8_t entry_index)
            {
                // The first entry in the 8 bit palette is the transparent colour, so just return 
                // fully transparent instead of replacing it later.
                if (entry_index == 0)
                {
                    return 0x00000000u;
                }
                auto entry = get_palette_entry(entry_index);
                uint32_t value = 0xff000000 | entry.Blue << 16 | entry.Green << 8 | entry.Red;
                return value;
            });
        }

        return results;
    }

    uint32_t Level::num_rooms() const
    {
        return static_cast<uint32_t>(_rooms.size ());
    }

    tr3_room Level::get_room(uint32_t index) const
    {
        return _rooms[index];
    }

    uint32_t Level::num_object_textures() const
    {
        return static_cast<uint32_t>(_object_textures.size());
    }

    tr_object_texture Level::get_object_texture(uint32_t index) const
    {
        return _object_textures[index];
    }

    uint32_t Level::num_floor_data() const
    {
        return static_cast<uint32_t>(_floor_data.size ());
    }

    uint16_t Level::get_floor_data(uint32_t index) const
    {
        return _floor_data[index];
    }

    std::vector<std::uint16_t> 
    Level::get_floor_data_all() const
    {
        return _floor_data;
    }

    uint32_t Level::num_entities() const
    {
        return static_cast<uint32_t>(_entities.size());
    }

    tr2_entity Level::get_entity(uint32_t index) const 
    {
        return _entities[index];
    }

    uint32_t Level::num_models() const
    {
        return static_cast<uint32_t>(_models.size());
    }

    tr_model Level::get_model(uint32_t index) const
    {
        return _models[index];
    }

    bool Level::get_model_by_id(uint32_t id, tr_model& output) const 
    {
        for (const auto& model : _models)
        {
            if (model.ID == id)
            {
                output = model;
                return true;
            }
        }
        return false;
    }

    uint32_t Level::num_static_meshes() const
    {
        return static_cast<uint32_t>(_static_meshes.size());
    }

    tr_staticmesh Level::get_static_mesh(uint32_t mesh_id) const
    {
        return _static_meshes.find(mesh_id)->second;
    }

    uint32_t Level::num_mesh_pointers() const
    {
        return static_cast<uint32_t>(_mesh_pointers.size());
    }

    tr_mesh Level::get_mesh_by_pointer(uint32_t mesh_pointer) const
    {
        auto index = _mesh_pointers[mesh_pointer];
        return _meshes.find(index)->second;
    }

    std::vector<tr_meshtree_node> Level::get_meshtree(uint32_t starting_index, uint32_t node_count) const
    {
        uint32_t index = starting_index;
        std::vector<tr_meshtree_node> nodes;
        for (uint32_t i = 0; i < node_count; ++i)
        {
            tr_meshtree_node node;
            node.Flags = _meshtree[index++];
            node.Offset_X = static_cast<int32_t>(_meshtree[index++]);
            node.Offset_Y = static_cast<int32_t>(_meshtree[index++]);
            node.Offset_Z = static_cast<int32_t>(_meshtree[index++]);
            nodes.push_back(node);
        }
        return nodes;
    }

    tr2_frame Level::get_frame(uint32_t frame_offset, uint32_t mesh_count) const
    {
        uint32_t offset = frame_offset;
        tr2_frame frame;
        frame.bb1x = _frames[offset++];
        frame.bb1y = _frames[offset++];
        frame.bb1z = _frames[offset++];
        frame.bb2x = _frames[offset++];
        frame.bb2y = _frames[offset++];
        frame.bb2z = _frames[offset++];
        frame.offsetx = _frames[offset++];
        frame.offsety = _frames[offset++];
        frame.offsetz = _frames[offset++];

        // Tomb Raider I has the mesh count in the frame structure - all other tombs
        // already know based on the number of meshes.
        if (_version == LevelVersion::Tomb1)
        {
            mesh_count = _frames[offset++];
        }

        for (uint32_t i = 0; i < mesh_count; ++i)
        {
            tr2_frame_rotation rotation;

            uint16_t next = 0;
            uint16_t data = 0;
            uint16_t mode = 0;

            // Tomb Raider I has reversed words and always uses the two word format.
            if ( _version == LevelVersion::Tomb1 )
            {
                next = _frames[offset++];
                data = _frames[offset++];
            }
            else
            {
                data = _frames[offset++];
                mode = data & 0xC000;
                if (!mode)
                {
                    next = _frames[offset++];
                }
            }

            if (mode)
            {
                float angle = 0;
                if ( _version >= LevelVersion::Tomb4 )
                {
                    angle = (data & 0x0fff) * PiMul2 / 4096.0f;
                }
                else
                {
                    angle = (data & 0x03ff) * PiMul2 / 1024.0f;
                }

                if (mode == 0x4000)
                {
                    rotation.x = angle;
                }
                else if (mode == 0x8000)
                {
                    rotation.y = angle;
                }
                else if (mode == 0xC000)
                {
                    rotation.z = angle;
                }
            }
            else
            {
                rotation.x = ((data & 0x3ff0) >> 4) * PiMul2 / 1024.0f;
                rotation.y = ((((data & 0x000f) << 6)) | ((next & 0xfc00) >> 10)) * PiMul2 / 1024.0f;
                rotation.z = (next & 0x03ff) * PiMul2 / 1024.0f;
            }
            frame.values.push_back(rotation);
        }
        return frame;
    }

    bool Level::get_sprite_sequence_by_id(int32_t sprite_sequence_id, tr_sprite_sequence& output) const
    {
        auto found_sequence = std::find_if(_sprite_sequences.begin(), _sprite_sequences.end(), [=](const auto& sequence)
        {
            return sequence.SpriteID == sprite_sequence_id; 
        });

        if (found_sequence == _sprite_sequences.end())
        {
            return false;
        }

        output = *found_sequence;
        return true;
    }

    tr_sprite_texture Level::get_sprite_texture(uint32_t index) const
    {
        return _sprite_textures[index];
    }

    bool Level::find_first_entity_by_type(int16_t type, tr2_entity& entity) const
    {
        auto found = std::find_if(_entities.begin(), _entities.end(), [type](const auto& e) { return e.TypeID == type; });
        if (found == _entities.end())
        {
            return false;
        }
        entity = *found;
        return true;
    }

    int16_t Level::get_mesh_from_type_id(int16_t type) const
    {
        if (type != 0 || _version < LevelVersion::Tomb3 )
        {
            return type;
        }

        if ( _version > LevelVersion::Tomb3 )
        {
            return LaraSkinPostTR3;
        }
        return LaraSkinTR3;
    }
}
