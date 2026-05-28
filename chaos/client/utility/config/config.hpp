#pragma once
#include <imgui.h>

namespace config
{
	namespace visuals
	{
		inline bool enable = false;

		inline bool box = false;
		inline int box_type = 0;
		inline bool box_fill = false;

		inline bool skeleton = false;

		inline bool name = false;
		inline bool distance = false;
		inline bool platform = false;
		inline bool gender = false;
		inline bool rank = false;
		inline bool teamid = false;
	
		inline bool china_hat = false;
		inline float text_y = 3.2f;
		inline ImVec4 china_hat_color = ImVec4( 196.f / 255.f , 59.f / 255.f , 68.f / 255.f , 1.f );
		inline ImVec4 box_color = ImVec4( 196.f / 255.f , 59.f / 255.f , 68.f / 255.f , 1.f );
		inline ImVec4 skeleton_color = ImVec4( 196.f / 255.f , 59.f / 255.f , 68.f / 255.f , 1.f );
		inline ImVec4 name_color = ImVec4( 59.f / 255.f , 98.f / 255.f , 196.f / 255.f , 1.f );
		inline ImVec4 distance_color = ImVec4( 96.f / 255.f , 59.f / 255.f , 196.f / 255.f , 1.f );   // purple
		inline ImVec4 platform_color = ImVec4( 59.f / 255.f , 196.f / 255.f , 173.f / 255.f , 1.f ); // cyan/teal
		inline ImVec4 gender_color = ImVec4( 196.f / 255.f , 127.f / 255.f , 59.f / 255.f , 1.f );  // orange
		inline ImVec4 rank_color = ImVec4( 196.f / 255.f , 196.f / 255.f , 59.f / 255.f , 1.f );    // yellow
		inline ImVec4 teamid_color = ImVec4( 59.f / 255.f , 196.f / 255.f , 91.f / 255.f , 1.f );   // green
	}

	namespace exploits
	{
		inline bool enable = true;
		inline bool walkspeed = false;
		inline float walkspeed_value = 1.0f;
		inline bool big_player = false;
	}
}