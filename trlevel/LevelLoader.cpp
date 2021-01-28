#include "LevelLoader.h"
#include "Level.h"
#include "LevelLoadUtils.h"

namespace trlevel
{
	LevelLoader::LevelLoader ( const std::string& path )
	: _path (path)
	, _converted ( trview::to_utf16(path) )
	, _level (std::make_unique<Level>())
	{
		auto file = std::make_unique<std::ifstream> ();
		file->exceptions ( std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit );
		file->open ( _converted.c_str (), std::ios::binary );
		_data = std::move ( file );
	}

	bool LevelLoader::is_known_version ( uint32_t version )
	{
		return version == TR1_PC
			|| version == TR2_PC
			|| version == TR3_PC1
			|| version == TR3_PC2
			|| version == TR3_PC3
			|| version == TR4_PC1
			|| version == TR4_PC2
			|| version == TR1_PSX;
	}

	bool LevelLoader::is_tr5 ()
	{
		std::wstring transformed;
		std::transform ( _path.begin (), _path.end (), std::back_inserter ( transformed ), towupper );
		return transformed.find ( L".TRC" ) != _path.npos;
	}

	void LevelLoader::read_palette ()
	{
		_level->_palette = read_vector<tr_colour> ( *_data, 256 );
		if ( _version != LevelVersion::Tomb1 )
		{
			_level->_palette16 = read_vector<tr_colour4> ( *_data, 256 );
		}
	}

	void LevelLoader::read_textiles ()
	{
		if ( _version == LevelVersion::Tomb1 && _target_platform == LevelTarget::PSX )
		{
			uint32_t offsetTextiles;
			_data->seekg ( 8, SEEK_CUR );
			offsetTextiles = read<uint32_t> ( *_data );
			_data->seekg ( offsetTextiles + 8, SEEK_SET );

			_level->_textile4 = read_vector<tr_textile4, uint16_t> ( *_data, 13 );
			_level->_clut = read_vector<tr_clut, uint16_t> ( *_data, 1024 );

			read<uint32_t> ( *_data ); // Padding
		}
		else if ( _version <= LevelVersion::Tomb3 )
		{
			_level->_num_textiles = read<uint32_t> ( *_data );

			for ( uint32_t i = 0; i < _level->_num_textiles; ++i )
			{
				_level->_textile8.emplace_back ( read<tr_textile8> ( *_data ) );
			}

			if ( _version != LevelVersion::Tomb1 )
			{
				for ( uint32_t i = 0; i < _level->_num_textiles; ++i )
				{
					_level->_textile16.emplace_back ( read<tr_textile16> ( *_data ) );
				}
			}
		}
		else // TR4, TR5
		{
			uint16_t num_room_textiles = read<uint16_t>(*_data);
			uint16_t num_obj_textiles = read<uint16_t>(*_data);
			uint16_t num_bump_textiles = read<uint16_t>(*_data);
			_level->_num_textiles = num_room_textiles + num_obj_textiles + num_bump_textiles;
			_level->_textile32 = read_vector_compressed<tr_textile32>(*_data, _level->_num_textiles);
			_level->_textile16 = read_vector_compressed<tr_textile16>(*_data, _level->_num_textiles);
			auto textile32_misc = read_vector_compressed<tr_textile32>(*_data, 2);

			if (_version == LevelVersion::Tomb5)
			{
			    _level->_lara_type = read<uint16_t>(*_data);
			    _level->_weather_type = read<uint16_t>(*_data);
			    _data->seekg(28, std::ios::cur);
			}

			if (_version == LevelVersion::Tomb4)
			{
			    std::vector<uint8_t> level_data = read_compressed(*_data);
			    std::string data(reinterpret_cast<char*>(&level_data[0]), level_data.size());
			    std::istringstream data_stream(data, std::ios::binary);
				_data = std::make_unique<std::istringstream> ( data, std::ios::binary );
			}
			else
			{
				_data->seekg ( 8, SEEK_CUR );
			}
		}
		
	}

	void LevelLoader::read_rooms () 
	{
		
		if ( _version == LevelVersion::Tomb5 )
		{
			const uint32_t num_tr5_rooms = read<uint32_t> ( *_data );
			for ( auto i = 0u; i < num_tr5_rooms; ++i )
			{
				tr3_room room; 
				read_tr5_room ( room );
				_level->_rooms.push_back ( room );
			}
		}
		else
		{
			const uint32_t num_rooms = read<uint16_t> ( *_data );
			for ( auto i = 0u; i < num_rooms; ++i )
			{
				tr3_room room;
				read_tr1_4_room ( room );
				_level->_rooms.push_back ( room );
			}
		}
	}

	void LevelLoader::read_tr1_4_room ( tr3_room& room )
	{
		room.info = convert_room_info ( read<tr1_4_room_info> ( *_data ) );

		uint32_t NumDataWords = read<uint32_t> ( *_data );

		if ( _version == LevelVersion::Tomb1 && _target_platform == LevelTarget::PSX )
			read<uint16_t> ( *_data ); // Padding
	
		// Read actual room data.
		if ( NumDataWords > 0 )
		{
			if ( _version == LevelVersion::Tomb1 )
			{
				room.data.vertices = convert_vertices ( read_vector<int16_t, tr_room_vertex> ( *_data ) );
			}
			else
			{
				room.data.vertices = read_vector<int16_t, tr3_room_vertex> ( *_data );
			}
			room.data.rectangles = convert_rectangles ( read_vector<int16_t, tr_face4> ( *_data ) );
			room.data.triangles = convert_triangles ( read_vector<int16_t, tr_face3> ( *_data ) );
			room.data.sprites = read_vector<int16_t, tr_room_sprite> ( *_data );

			if ( _target_platform == LevelTarget::PSX )
				for ( auto& rectangle : room.data.rectangles )
				{
					const auto swap = [] ( uint16_t& a, uint16_t& b )
						{
						uint16_t temp = a;
						a = b;
						b = temp;
						};
					swap ( rectangle.vertices [2], rectangle.vertices [3] );
				}
		}

		room.portals = read_vector<uint16_t, tr_room_portal> ( *_data );

		room.num_z_sectors = read<uint16_t> ( *_data );
		room.num_x_sectors = read<uint16_t> ( *_data );
		room.sector_list = read_vector<tr_room_sector> ( *_data, room.num_z_sectors * room.num_x_sectors );

		if ( _version == LevelVersion::Tomb4 )
		{
			room.room_colour = read<uint32_t> ( *_data );
		}
		else
		{
			room.ambient_intensity_1 = read<int16_t> ( *_data );

			if ( _version > LevelVersion::Tomb1 )
			{
				room.ambient_intensity_2 = read<int16_t> ( *_data );
			}
		}

		if ( _version == LevelVersion::Tomb2 )
		{
			room.light_mode = read<int16_t> ( *_data );
		}

		if ( _version == LevelVersion::Tomb1 )
		{
			room.lights =
				_target_platform == LevelTarget::PSX
				? convert_lights ( read_vector<uint16_t, tr_room_light_psx> ( *_data ) )
				: convert_lights ( read_vector<uint16_t, tr_room_light> ( *_data ) );
			
		}
		else if ( _version == LevelVersion::Tomb4 )
		{
			auto lights = read_vector<uint16_t, tr4_room_light> ( *_data );
		}
		else
		{
			room.lights = read_vector<uint16_t, tr3_room_light> ( *_data );
		}

		if ( _version == LevelVersion::Tomb1 )
		{
			room.static_meshes = 
				_target_platform == LevelTarget::PSX
				? convert_room_static_meshes ( read_vector<uint16_t, tr_room_staticmesh_psx> ( *_data ) )
				: convert_room_static_meshes ( read_vector<uint16_t, tr_room_staticmesh> ( *_data ) );
		}
		else
		{
			room.static_meshes = read_vector<uint16_t, tr3_room_staticmesh> ( *_data );
		}

		room.alternate_room = read<int16_t> ( *_data );
		room.flags = read<int16_t> ( *_data );

		if ( _version == LevelVersion::Tomb3 || _version == LevelVersion::Tomb4 )
		{
			room.water_scheme = read<uint8_t> ( *_data );
			room.reverb_info = read<uint8_t> ( *_data );
			room.alternate_group = read<uint8_t> ( *_data );
		}
	}

	void LevelLoader::read_tr5_room ( tr3_room& room )
	{
		skip_xela ( *_data );
		uint32_t room_data_size = read<uint32_t> ( *_data );
		const uint32_t room_start = static_cast<uint32_t>( _data->tellg() );
		const uint32_t room_end = room_start + room_data_size;

		const auto header = read<tr5_room_header> ( *_data );

		// Copy useful data from the header to the room.
		room.info = header.info;
		room.num_x_sectors = header.num_x_sectors;
		room.num_z_sectors = header.num_z_sectors;
		room.colour = header.colour;
		room.reverb_info = header.reverb_info;
		room.alternate_group = header.alternate_group;
		room.water_scheme = header.water_scheme;
		room.alternate_room = header.alternate_room;
		room.flags = header.flags;

		// The offsets start measuring from this position, after all the header information.
		const uint32_t data_start = static_cast<uint32_t>(_data->tellg ());

		// Discard lights as they are not currently used:
		skip ( *_data, sizeof ( tr5_room_light ) * header.num_lights );

		_data->seekg ( data_start + header.start_sd_offset, std::ios::beg );
		room.sector_list = read_vector<tr_room_sector> ( *_data, room.num_z_sectors * room.num_x_sectors );
		room.portals = read_vector<uint16_t, tr_room_portal> ( *_data );

		// Separator
		skip ( *_data, 2 );

		_data->seekg ( data_start + header.end_portal_offset, std::ios::beg );
		room.static_meshes = read_vector<tr3_room_staticmesh> ( *_data, header.num_static_meshes );

		_data->seekg ( data_start + header.layer_offset, std::ios::beg );
		auto layers = read_vector<tr5_room_layer> ( *_data, header.num_layers );

		_data->seekg ( data_start + header.poly_offset, std::ios::beg );
		uint16_t vertex_offset = 0;
		for ( const auto& layer : layers )
		{
			auto rects = read_vector<tr4_mesh_face4> ( *_data, layer.num_rectangles );
			for ( auto& rect : rects )
			{
				for ( auto& v : rect.vertices )
				{
					v += vertex_offset;
				}
			}
			std::copy ( rects.begin (), rects.end (), std::back_inserter ( room.data.rectangles ) );

			auto tris = read_vector<tr4_mesh_face3> ( *_data, layer.num_triangles );
			for ( auto& tri : tris )
			{
				for ( auto& v : tri.vertices )
				{
					v += vertex_offset;
				}
			}
			std::copy ( tris.begin (), tris.end (), std::back_inserter ( room.data.triangles ) );

			vertex_offset += layer.num_vertices;
		}

		_data->seekg ( data_start + header.vertices_offset, std::ios::beg );
		for ( const auto& layer : layers )
		{
			auto verts = convert_vertices ( read_vector<tr5_room_vertex> ( *_data, layer.num_vertices ) );
			std::copy ( verts.begin (), verts.end (), std::back_inserter ( room.data.vertices ) );
		}

		_data->seekg ( room_end, std::ios::beg );
	}

	void LevelLoader::read_floor_data ()
	{
		_level->_floor_data = read_vector<uint32_t, uint16_t> ( *_data );
	}

	void LevelLoader::read_mesh_data ()
	{
		_level->_mesh_data = read_vector<uint32_t, uint16_t> ( *_data );
		_level->_mesh_pointers = read_vector<uint32_t, uint32_t> ( *_data );
	}

	void LevelLoader::read_animations ()
	{
		if ( _version == LevelVersion::Tomb4 || _version == LevelVersion::Tomb5)
		{
			auto animations = read_vector<uint32_t, tr4_animation> ( *_data );
		}
		else
		{
			std::vector<tr_animation> animations = read_vector<uint32_t, tr_animation> ( *_data );
		}
		std::vector<tr_state_change> state_changes = read_vector<uint32_t, tr_state_change> ( *_data );
		std::vector<tr_anim_dispatch> anim_dispatches = read_vector<uint32_t, tr_anim_dispatch> ( *_data );
		std::vector<tr_anim_command> anim_commands = read_vector<uint32_t, tr_anim_command> ( *_data );
	}

	void LevelLoader::read_mesh_tree ()
	{
		_level->_meshtree = read_vector<uint32_t, uint32_t> ( *_data );
	}

	void LevelLoader::read_frames ()
	{
		_level->_frames = read_vector<uint32_t, uint16_t> ( *_data );
	}

	void LevelLoader::read_models ()
	{
		if ( _version == LevelVersion::Tomb5 )
		{
			_level->_models = convert_models ( read_vector<uint32_t, tr5_model> ( *_data ) );
		}
		else
			_level->_models = 
				_target_platform == LevelTarget::PSX 
				? convert_models ( read_vector<uint32_t, tr_model_psx> (*_data))
				: read_vector<uint32_t, tr_model> ( *_data );
	}

	void LevelLoader::read_statics ()
	{
		auto static_meshes = read_vector<uint32_t, tr_staticmesh> ( *_data );
		for ( const auto& mesh : static_meshes )
		{
			_level->_static_meshes.insert ( { mesh.ID, mesh } );
		}
	}

	void LevelLoader::read_object_textures ()
	{
		if ( _version == LevelVersion::Tomb1 || _version == LevelVersion::Tomb2 || _version == LevelVersion::Tomb3 )
		{
			if ( _target_platform == LevelTarget::PSX )
			{
				auto textures = read_vector<uint32_t, tr_object_texture_psx> ( *_data );
				std::transform (
					textures.begin (),
					textures.end (),
					std::back_inserter ( _level->_object_textures ),
					[this] ( const tr_object_texture_psx& texture )
						{
						uint16_t converted = static_cast<uint16_t>(conv4 ( texture.Tile, texture.Clut ));
						tr_object_texture t
							{
							texture.Attribute,
							converted, 
								{
									{ 0, texture.x0, 0, texture.y0 }, 
									{ 0, texture.x1, 0, texture.y1 },
									{ 0, texture.x3, 0, texture.y3 },
									{ 0, texture.x2, 0, texture.y2 },
								}
							};
						return t;
						} );
			}
			else 
				_level->_object_textures = read_vector<uint32_t, tr_object_texture> ( *_data );
		}
		else if ( _version == LevelVersion::Tomb4 )
		{
			_level->_object_textures = convert_object_textures ( read_vector<uint32_t, tr4_object_texture> ( *_data ) );
		}
		else if ( _version == LevelVersion::Tomb5 )
		{
			_level->_object_textures = convert_object_textures ( read_vector<uint32_t, tr5_object_texture> ( *_data ) );
		}
	}

	std::size_t LevelLoader::conv4 ( uint16_t tile, uint16_t clut_id )
	{
		auto pair = std::make_pair ( tile, clut_id );
		auto found = std::find ( _cache.begin (), _cache.end (), pair );

		if ( found != _cache.end () )
			return std::distance ( _cache.begin (), found );


		// else we need to create a new textile 
		std::size_t new_index = _level->_textile16.size ();
		tr_textile4 t4 = _level->_textile4 [tile];
		tr_clut clut = _level->_clut [clut_id];
		tr_textile16 t16;

		for ( int x = 0; x < 256; x++ )
		{
			for ( int y = 0; y < 256; y++ )
			{
				tr_colorindex4 index = t4.Tile [(y * 256 + x) / 2];
				uint8_t i = (x % 2) ? index.a : index.b;
				tr_rgba5551 col = clut.Colour [i];
				t16.Tile [y * 256 + x] = (col.Alpha << 15) | (col.Red << 10) | (col.Green << 5) | col.Blue;
			}
		}
		_level->_textile16.push_back ( t16 );
		_cache.push_back ( pair );
		return new_index;
	}

	void LevelLoader::read_sprites ()
	{
		if ( _version == LevelVersion::Tomb4 || _version == LevelVersion::Tomb5 )
		{
			_data->seekg ( 3, std::ios::cur );
			if ( _version == LevelVersion::Tomb5 )
			{
				skip ( *_data, 1 );
			}
		}

		if ( _target_platform == LevelTarget::PSX )
		{
			auto textures = read_vector <uint32_t, tr_sprite_texture_psx> ( *_data );
			_level->_sprite_textures.reserve ( textures.size () );
			std::transform (
				textures.begin (),
				textures.end (),
				std::back_inserter ( _level->_sprite_textures ),
				[this] ( const tr_sprite_texture_psx& texture )
					{
					auto converted = static_cast<uint16_t>(conv4 ( texture.Tile, texture.Clut ));
					tr_sprite_texture t { converted, texture.u0, texture.v0, 256, 256, texture.LeftSide, texture.TopSide, texture.RightSide, texture.BottomSide };
					return t;
					} );
		}
		else
			_level->_sprite_textures = read_vector<uint32_t, tr_sprite_texture> ( *_data );
		
		_level->_sprite_sequences = read_vector<uint32_t, tr_sprite_sequence> ( *_data );
	}

	void LevelLoader::read_cameras ()
	{
		std::vector<tr_camera> cameras = read_vector<uint32_t, tr_camera> ( *_data );
		if ( _version == LevelVersion::Tomb4 || _version == LevelVersion::Tomb5 )
		{
			std::vector<tr4_flyby_camera> flyby_cameras = read_vector<uint32_t, tr4_flyby_camera> ( *_data );
		}
	}

	void LevelLoader::read_sounds ()
	{
		std::vector<tr_sound_source> sound_sources = read_vector<uint32_t, tr_sound_source> ( *_data );
	}

	void LevelLoader::read_boxes ()
	{
		uint32_t num_boxes = 0;
		if ( _version == LevelVersion::Tomb1 )
		{
			std::vector<tr_box> boxes = read_vector<uint32_t, tr_box> ( *_data );
			num_boxes = static_cast<uint32_t>(boxes.size ());
		}
		else
		{
			std::vector<tr2_box> boxes = read_vector<uint32_t, tr2_box> ( *_data );
			num_boxes = static_cast<uint32_t>(boxes.size ());
		}
		std::vector<uint16_t> overlaps = read_vector<uint32_t, uint16_t> ( *_data );

		if ( _version == LevelVersion::Tomb1 )
		{
			std::vector<int16_t> zones = read_vector<int16_t> ( *_data, num_boxes * 6 );
		}
		else
		{
			std::vector<int16_t> zones = read_vector<int16_t> ( *_data, num_boxes * 10 );
		}

	}

	void LevelLoader::read_animated_textures ()
	{
		std::vector<uint16_t> animated_textures = read_vector<uint32_t, uint16_t> ( *_data );
		if ( _version == LevelVersion::Tomb4 || _version == LevelVersion::Tomb5 )
		{
			// Animated textures uv count - not yet used:
			skip ( *_data, 1 );

			_data->seekg ( 3, std::ios::cur );
			if ( _version == LevelVersion::Tomb5 )
			{
				skip ( *_data, 1 );
			}
		}
	}

	void LevelLoader::read_entities ()
	{
		if ( _version == LevelVersion::Tomb1 )
		{
			_level->_entities = convert_entities ( read_vector<uint32_t, tr_entity> ( *_data ) );
		}
		else
		{
			// TR4 entity is in here, OCB is not set but goes into intensity2 (convert later).
			_level->_entities = read_vector<uint32_t, tr2_entity> ( *_data );
		}
	}

	void LevelLoader::read_light_map ()
	{
		std::vector<uint8_t> light_map = read_vector<uint8_t> ( *_data, 32 * 256 );
	}

	void LevelLoader::read_ai_objects ()
	{
		if ( _version == LevelVersion::Tomb4 || _version == LevelVersion::Tomb5 )
		{
			std::vector<tr4_ai_object> ai_objects = read_vector<uint32_t, tr4_ai_object> ( *_data );
			std::transform ( ai_objects.begin (), ai_objects.end (), std::back_inserter ( _level->_entities ),
				[] ( const auto& ai_object )
				{
					tr2_entity entity {};
					entity.TypeID = ai_object.type_id;
					entity.Room = ai_object.room;
					entity.x = ai_object.x;
					entity.y = ai_object.y;
					entity.z = ai_object.z;
					entity.Angle = ai_object.angle;
					entity.Intensity1 = 0;
					entity.Intensity2 = ai_object.ocb;
					entity.Flags = ai_object.flags;
					return entity;
				} );
		}
	}

	void LevelLoader::read_cinematics ()
	{
		if ( _version == LevelVersion::Tomb1 || _version == LevelVersion::Tomb2 || _version == LevelVersion::Tomb3 )
		{
			std::vector<tr_cinematic_frame> cinematic_frames = read_vector<uint16_t, tr_cinematic_frame> ( *_data );
		}
	}

	void LevelLoader::read_sound_map ()
	{
		std::vector<uint8_t> demo_data = read_vector<uint16_t, uint8_t> ( *_data );

		if ( _version == LevelVersion::Tomb1 )
		{
			std::vector<int16_t> sound_map = read_vector<int16_t> ( *_data, 256 );
		}
		else if ( _version == LevelVersion::Tomb2 || _version == LevelVersion::Tomb3 )
		{
			std::vector<int16_t> sound_map = read_vector<int16_t> ( *_data, 370 );
		}
		else if ( _version == LevelVersion::Tomb4 )
		{
			if ( demo_data.size () == 2048 )
			{
				std::vector<int16_t> sound_map = read_vector<int16_t> ( *_data, 1024 );
			}
			else
			{
				std::vector<int16_t> sound_map = read_vector<int16_t> ( *_data, 370 );
			}
		}
		else
		{
			std::vector<int16_t> sound_map = read_vector<int16_t> ( *_data, 450 );
		}

		std::vector<tr3_sound_details> sound_details = read_vector<uint32_t, tr3_sound_details> ( *_data );

		if ( _version == LevelVersion::Tomb1 )
		{
			std::vector<uint8_t> sound_data = read_vector<int32_t, uint8_t> ( *_data );
		}

	}

	void LevelLoader::read_sample_indices ()
	{
		std::vector<uint32_t> sample_indices = read_vector<uint32_t, uint32_t> ( *_data );
	}

	void LevelLoader::generate_meshes ( const std::vector<uint16_t>& mesh_data )
	{
		std::string data ( reinterpret_cast<const char*>(&mesh_data [0]), mesh_data.size () * sizeof ( uint16_t ) );
		std::istringstream stream ( data, std::ios::binary );
		stream.exceptions ( std::istream::failbit | std::istream::badbit | std::istream::eofbit );
		for ( auto pointer : _level->_mesh_pointers )
		{
			// Does the map already contain this mesh? If so, don't bother reading it again.
			auto found = _level->_meshes.find ( pointer );
			if ( found != _level->_meshes.end () )
			{
				continue;
			}

			stream.seekg ( pointer, std::ios::beg );

			tr_mesh mesh;
			mesh.centre = read<tr_vertex> ( stream );
			mesh.coll_radius = read<int32_t> ( stream );

			if ( _target_platform == LevelTarget::PSX )
			{
				int16_t vertices_count = read<int16_t> ( stream );
				int16_t normals_count = vertices_count;
				vertices_count = static_cast<int16_t>(std::abs ( vertices_count ));

				mesh.vertices = convert_vertices ( read_vector<tr_vertex_psx, uint16_t> ( stream, vertices_count) );
				for ( int i = 0; i < vertices_count; ++i )
				{
					if ( normals_count > 0 )
					{
						tr_vertex_psx norm;
						norm = read<tr_vertex_psx> ( stream );
						norm.w = 1;
						mesh.normals.push_back ( { norm.x, norm.y, norm.z } );
					}
					else
					{
						mesh.lights.push_back(read<int16_t> ( stream )); // intensity
						mesh.normals.push_back ( { 0, 0, 0 } );
					}
				}
				mesh.textured_rectangles = convert_rectangles ( read_vector<int16_t, tr_face4> ( stream ) );
				mesh.textured_triangles = convert_triangles ( read_vector<int16_t, tr_face3> ( stream ) );
			}
			else
			{
				mesh.vertices = read_vector<int16_t, tr_vertex> ( stream );
				int16_t normals = read<int16_t> ( stream );
				if ( normals > 0 )
				{
					mesh.normals = read_vector<tr_vertex> ( stream, normals );
				}
				else
				{
					mesh.lights = read_vector<int16_t> ( stream, abs ( normals ) );
				}

				if ( _version < LevelVersion::Tomb4 )
				{
					mesh.textured_rectangles = convert_rectangles ( read_vector<int16_t, tr_face4> ( stream ) );
					mesh.textured_triangles = convert_triangles ( read_vector<int16_t, tr_face3> ( stream ) );
					mesh.coloured_rectangles = read_vector<int16_t, tr_face4> ( stream );
					mesh.coloured_triangles = read_vector<int16_t, tr_face3> ( stream );
				}
				else
				{
					mesh.textured_rectangles = read_vector<int16_t, tr4_mesh_face4> ( stream );
					mesh.textured_triangles = read_vector<int16_t, tr4_mesh_face3> ( stream );
				}
			}
			_level->_meshes.insert ( { pointer, mesh } );
		}
	}


	std::unique_ptr<Level> LevelLoader::to_level ()
	{
		uint32_t version = read <uint32_t> ( *_data );

		if ( !is_known_version ( version ) )
			version = read <uint32_t> ( *_data );

		switch ( version )
		{
		case TR1_PC:
			_target_platform = LevelTarget::PC;
			_version = LevelVersion::Tomb1;
			break;
		case TR2_PC:
			_target_platform = LevelTarget::PC;
			_version = LevelVersion::Tomb2;
			break;
		case TR3_PC1:
		case TR3_PC2:
		case TR3_PC3:
			_target_platform = LevelTarget::PC;
			_version = LevelVersion::Tomb3;
			break;
		case TR4_PC1:
		case TR4_PC2:
			_target_platform = LevelTarget::PC;
			_version = is_tr5 () ? LevelVersion::Tomb5 : LevelVersion::Tomb4;
			break;
		case TR1_PSX:
			_target_platform = LevelTarget::PSX;
			_version = LevelVersion::Tomb1;
			break;
		}

		switch ( _target_platform )
		{
		case LevelTarget::PSX:
			{
			read_textiles ();
			read_rooms ();
			read_floor_data ();
			read_mesh_data ();
			read_animations ();
			read_mesh_tree ();
			read_frames ();
			read_models ();
			read_statics ();
			read_object_textures ();
			read_sprites ();
			read_cameras ();
			read_sounds ();
			read_boxes ();
			read_animated_textures ();
			read_entities ();
			_level->_num_textiles = static_cast<uint32_t>(_level->_textile16.size ());
			break;
			}
		case LevelTarget::PC:
			if ( _version == LevelVersion::Tomb2 || _version == LevelVersion::Tomb3 )
				read_palette ();
			read_textiles ();
			read<uint32_t> ( *_data ); // read unused value
			read_rooms ();
			read_floor_data ();
			read_mesh_data ();
			read_animations ();
			read_mesh_tree ();
			read_frames ();
			read_models ();
			read_statics ();
			if ( _version <= LevelVersion::Tomb2 )
				read_object_textures ();
			read_sprites ();
			read_cameras ();
			read_sounds ();
			read_boxes ();
			read_animated_textures ();
			if ( _version >= LevelVersion::Tomb3 )
				read_object_textures ();
			read_entities ();
			read_light_map ();
			if ( _version == LevelVersion::Tomb1 )
				read_palette ();
			read_cinematics ();
			break;
			read_sound_map ();
			read_sample_indices ();
			break;
		}
		_level->_target_platform = _target_platform;
		_level->_version = _version;
		generate_meshes (_level->_mesh_data);
		return std::move ( _level );
	}


}