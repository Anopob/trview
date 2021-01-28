#pragma once 

#include <fstream>
#include <memory>
#include "LevelTarget.h"
#include "LevelVersion.h"
#include "trtypes.h"

namespace trlevel
{
	enum LevelMagic : uint32_t
	{
		TR1_PC = 0x20,
		//TR1_PC2 = 0xB020,
		TR2_PC = 0x2D,
		TR3_PC1 = 0xFF080038,
		TR3_PC2 = 0xFF180038,
		TR3_PC3 = 0xFF180034,
		TR4_PC1 = 0x00345254,
		TR4_PC2 = 0x63345254,
		TR1_PSX = 0x56414270,
	};

	class Level;

	class LevelLoader
	{
	public:
		LevelLoader ( const std::string& path );
		std::unique_ptr<Level> to_level ();
	private:
		const std::string& _path;
		std::wstring _converted;
		std::unique_ptr<std::istream> _data;
		//std::istream _data;
		std::unique_ptr<Level> _level;
		LevelVersion _version;
		LevelTarget _target_platform;
		uint32_t _actual_textile_count;
		std::vector<std::pair<uint16_t, uint16_t>> _cache;

		static bool is_known_version ( uint32_t version );
		std::size_t conv4 ( uint16_t tile, uint16_t clut_id );
		void generate_meshes ( const std::vector<uint16_t>& mesh_data );
		void read_tr1_4_room ( tr3_room& room );
		void read_tr5_room ( tr3_room& room );
		bool is_tr5 ();
		void read_palette ();
		void read_textiles ();
		void read_rooms (); 
		void read_floor_data ();
		void read_mesh_data ();
		void read_animations (); 
		void read_mesh_tree ();
		void read_frames ();
		void read_models (); 
		void read_statics ();
		void read_object_textures ();
		void read_sprites ();
		void read_cameras (); 
		void read_sounds ();
		void read_boxes ();
		void read_animated_textures ();
		void read_entities ();
		void read_light_map ();
		void read_ai_objects ();
		void read_cinematics ();
		void read_sound_map ();
		void read_sample_indices ();
	};
}
