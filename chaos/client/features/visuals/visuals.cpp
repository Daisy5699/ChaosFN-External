#include "visuals.hpp"
#include "client/utility/config/config.hpp"
#include "client/utility/cache/cache.hpp"
#include "client/utility/internal/internal.hpp"
#include <cmath>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

#undef min
#undef max

namespace features
{
	namespace visuals
	{
		// 2D Convex Hull Math Utilities
		inline auto cross_product_2d( const ImVec2 & o , const ImVec2 & a , const ImVec2 & b ) -> float
		{
			return ( a.x - o.x ) * ( b.y - o.y ) - ( a.y - o.y ) * ( b.x - o.x );
		}

		inline auto distance_sq( const ImVec2 & a , const ImVec2 & b ) -> float
		{
			return ( a.x - b.x ) * ( a.x - b.x ) + ( a.y - b.y ) * ( a.y - b.y );
		}

		auto convex_hull( std::vector<ImVec2> & points ) -> std::vector<ImVec2>
		{
			if ( points.size( ) <= 3 )
				return points;

			auto it = std::min_element( points.begin( ) , points.end( ) , [ ] ( const ImVec2 & a , const ImVec2 & b )
				{
					return ( a.y < b.y ) || ( a.y == b.y && a.x < b.x );
				} );

			std::swap( points [ 0 ] , *it );
			ImVec2 p0 = points [ 0 ];

			std::sort( points.begin( ) + 1 , points.end( ) , [ &p0 ] ( const ImVec2 & a , const ImVec2 & b )
				{
					float cross = cross_product_2d( p0 , a , b );
					return ( cross > 0 ) || ( cross == 0 && distance_sq( p0 , a ) < distance_sq( p0 , b ) );
				} );

			std::vector<ImVec2> hull;
			hull.push_back( points [ 0 ] );
			hull.push_back( points [ 1 ] );

			for ( size_t i = 2; i < points.size( ); i++ )
			{
				while ( hull.size( ) > 1 && cross_product_2d( hull [ hull.size( ) - 2 ] , hull.back( ) , points [ i ] ) <= 0 )
				{
					hull.pop_back( );
				}
				hull.push_back( points [ i ] );
			}

			return hull;
		}

		auto call( ) -> void
		{
			static bool init = false;
			if ( !init )
			{
				std::thread( [ ] ( ) { cache->data( ); } ).detach( );
				std::thread( [ ] ( ) { cache->entities( ); } ).detach( );
				init = true;
			}

			if ( !config::visuals::enable )
				return;

			sdk::u_world * uworld_ptr = cache->uworld.load( std::memory_order_acquire );
			sdk::a_player_controller * pc_ptr = cache->player_controller.load( std::memory_order_acquire );

			uintptr_t uworld_addr = reinterpret_cast< uintptr_t >( uworld_ptr );
			uintptr_t player_controller = reinterpret_cast< uintptr_t >( pc_ptr );

			if ( !uworld_addr || !player_controller )
				return;

			std::vector<actor_t> actors;
			{
				std::lock_guard<std::mutex> lock( cache->actor_mutex );
				actors = cache->actor_list;
			}

			if ( actors.empty( ) )
				return;

			uintptr_t location_pointer = device->read<uintptr_t>( uworld_addr + 0x178 );
			uintptr_t rotation_pointer = device->read<uintptr_t>( uworld_addr + 0x188 );

			struct fn_rot {
				double a;
				double b;
				double c;
			};

			fn_rot fnrot {};
			fnrot.a = device->read<double>( rotation_pointer );
			fnrot.b = device->read<double>( rotation_pointer + 0x20 );
			fnrot.c = device->read<double>( rotation_pointer + 0x1D0 );

			math::f_minimal_view_info pov;
			pov.location = device->read<math::f_vector>( location_pointer );

			pov.rotation.pitch = asin( fnrot.c ) * ( 180.0 / 3.14159265358979323846 );
			pov.rotation.yaw = ( ( ( atan2( fnrot.a * -1.0 , fnrot.b ) * ( 180.0 / 3.14159265358979323846 ) ) * -1.0 ) * -1.0 );
			pov.rotation.roll = 0.0;

			/*
			 sdk::a_player_camera_manager * camera_manager = pc_ptr->player_camera_manager;
			if ( camera_manager )
			{
				uintptr_t camera_manager_addr = reinterpret_cast< uintptr_t >( camera_manager );

				// Non-blocking single write/read snapshot pass per frame execution loop
				device->write<float>( camera_manager_addr + 0x1610 , 120.f );
				pov.fov = device->read<float>( camera_manager_addr + 0x1610 );
			}
			else
			{
				pov.fov = 90.f;
			}
			*/

			sdk::a_player_camera_manager * camera_manager = pc_ptr->player_camera_manager;
			if ( camera_manager )
			{
				uintptr_t camera_manager_addr = reinterpret_cast< uintptr_t >( camera_manager );

				// Read the game's actual FOV without overriding or changing it
				pov.fov = device->read<float>( camera_manager_addr + 0x1610 );
			}
			else
			{
				pov.fov = 90.f;
			}

			ImGuiIO & io = ImGui::GetIO( );
			int width = static_cast< int >( io.DisplaySize.x );
			int height = static_cast< int >( io.DisplaySize.y );

			if ( width <= 0 || height <= 0 )
				return;

			ImDrawList * draw_list = ImGui::GetForegroundDrawList( );

			for ( const auto & actor : actors )
			{
				if ( !actor.m_bones_valid )
					continue;

				math::f_vector head_3d = actor.m_head_pos;
				math::f_vector foot_3d = actor.m_foot_pos;

				math::f_vector head_top_3d = math::f_vector( head_3d.x , head_3d.y , head_3d.z + 10.f );

				math::f_vector_2d head_box , foot_screen;

				if ( !math::world_to_screen( head_top_3d , head_box , pov , width , height ) )
					continue;

				if ( !math::world_to_screen( foot_3d , foot_screen , pov , width , height ) )
					continue;

				float box_height = std::abs( static_cast< float >( head_box.y - foot_screen.y ) ) * 1.1f;
				float box_width = box_height * 0.6f;

				if ( box_height < 5.f || box_height > 2000.f )
					continue;

				ImVec2 c1 = ImVec2( static_cast< float >( head_box.x ) - ( box_width / 2.f ) , static_cast< float >( head_box.y ) );
				ImVec2 c2 = ImVec2( box_width , box_height );

				if ( config::visuals::box )
				{
					internal::drawing_t drawing;
					drawing.rect( c1 , c2 , ImGui::ColorConvertFloat4ToU32( config::visuals::box_color ) , 1.0f , 0.0f );
				}

				if ( config::visuals::skeleton && !actor.m_skeleton_lines.empty( ) )
				{
					auto color = ImGui::ColorConvertFloat4ToU32( config::visuals::skeleton_color );
					for ( const auto & line : actor.m_skeleton_lines )
					{
						math::f_vector_2d screen1 , screen2;
						if ( math::world_to_screen( line.first , screen1 , pov , width , height ) && math::world_to_screen( line.second , screen2 , pov , width , height ) )
						{
							draw_list->AddLine( ImVec2( static_cast< float >( screen1.x ) , static_cast< float >( screen1.y ) ) , 
								ImVec2( static_cast< float >( screen2.x ) ,
									static_cast< float >( screen2.y ) ) , IM_COL32_BLACK , 3.0f );
							draw_list->AddLine( ImVec2( static_cast< float >( screen1.x ) , static_cast< float >( screen1.y ) ) ,
								ImVec2( static_cast< float >( screen2.x ) , static_cast< float >( screen2.y ) ) , color , 1.0f );
						}
					}
				}

				if ( config::visuals::name )
				{
					internal::drawing_t drawing;
					drawing.text( ImGui::GetBackgroundDrawList( ) , c1 , c2 , "unknown" , internal::position_t::e_positions::top , ImGui::ColorConvertFloat4ToU32( config::visuals::name_color ) , IM_COL32_BLACK );
				}

				if ( config::visuals::platform )
				{
					internal::drawing_t drawing;
					auto color = ImGui::ColorConvertFloat4ToU32( config::visuals::platform_color );

					sdk::a_fort_player_state ps( actor.m_player_state );
					sdk::e_common_platform_type platform = ( sdk::e_common_platform_type ) ps.platform;
					std::string str = "unknown";

					switch ( platform )
					{
					case sdk::e_common_platform_type::PC: str = "pc"; break;
					case sdk::e_common_platform_type::PS4: str = "ps4"; break;
					case sdk::e_common_platform_type::PS5: str = "ps5"; break;
					case sdk::e_common_platform_type::XBox: str = "xbox"; break;
					case sdk::e_common_platform_type::XSX: str = "xbox series x"; break;
					case sdk::e_common_platform_type::Mac: str = "mac"; break;
					case sdk::e_common_platform_type::Switch: str = "switch"; break;
					case sdk::e_common_platform_type::Switch2: str = "switch 2"; break;
					default: str = "unknown platform"; break;
					}

					drawing.text( ImGui::GetBackgroundDrawList( ) , c1 , c2 , str.c_str( ) , internal::position_t::e_positions::right , color , IM_COL32_BLACK , 3.2f );
				}

				if ( config::visuals::china_hat )
				{
					const float hat_radius = 24.0f;
					const float hat_height = 12.0f;
					const int segments = 24;

					math::f_vector head_world = actor.m_head_pos;
					head_world.z += 10.f;

					std::vector<math::f_vector> vertices3d;
					std::vector<math::f_vector_2d> vertices2d;

					vertices3d.push_back( head_world + math::f_vector( 0.f , 0.f , hat_height ) );

					for ( int i = 0; i <= segments; i++ )
					{
						float angle = ( i * 2.0f * 3.14159f ) / segments;
						vertices3d.push_back( head_world + math::f_vector(
							hat_radius * cosf( angle ) ,
							hat_radius * sinf( angle ) ,
							0.f
						) );
					}

					for ( int ring = 1; ring <= 3; ring++ )
					{
						float ring_radius = hat_radius * ( ring / 4.0f );
						float ring_height = hat_height * ( 1.0f - ring / 4.0f );

						for ( int i = 0; i <= segments; i++ )
						{
							float angle = ( i * 2.0f * 3.14159f ) / segments;
							vertices3d.push_back( head_world + math::f_vector(
								ring_radius * cosf( angle ) ,
								ring_radius * sinf( angle ) ,
								ring_height
							) );
						}
					}

					bool all_visible = true;
					for ( auto & v : vertices3d )
					{
						math::f_vector_2d screen;
						if ( !math::world_to_screen( v , screen , pov , width , height ) )
						{
							all_visible = false;
							break;
						}
						vertices2d.push_back( screen );
					}

					if ( all_visible )
					{
						auto color = ImGui::ColorConvertFloat4ToU32( config::visuals::china_hat_color );

						for ( int i = 1; i <= segments; i++ )
						{
							draw_list->AddLine(
								ImVec2( static_cast< float >( vertices2d [ 0 ].x ) , static_cast< float >( vertices2d [ 0 ].y ) ) ,
								ImVec2( static_cast< float >( vertices2d [ i ].x ) , static_cast< float >( vertices2d [ i ].y ) ) ,
								color , 0.8f
							);
						}

						for ( int i = 1; i <= segments; i++ )
						{
							int next = ( i == segments ) ? 1 : i + 1;
							draw_list->AddLine(
								ImVec2( static_cast< float >( vertices2d [ i ].x ) , static_cast< float >( vertices2d [ i ].y ) ) ,
								ImVec2( static_cast< float >( vertices2d [ next ].x ) , static_cast< float >( vertices2d [ next ].y ) ) ,
								color , 1.0f
							);
						}

						for ( int ring = 1; ring <= 3; ring++ )
						{
							int ring_start = 1 + ring * ( segments + 1 );
							for ( int i = 0; i < segments; i++ )
							{
								int curr = ring_start + i;
								int next = ring_start + ( ( i + 1 ) % ( segments + 1 ) );
								draw_list->AddLine(
									ImVec2( static_cast< float >( vertices2d [ curr ].x ) , static_cast< float >( vertices2d [ curr ].y ) ) ,
									ImVec2( static_cast< float >( vertices2d [ next ].x ) , static_cast< float >( vertices2d [ next ].y ) ) ,
									color , 0.6f
								);
							}
						}

						for ( int i = 0; i < segments; i += 2 )
						{
							for ( int ring = 0; ring < 3; ring++ )
							{
								int curr = 1 + ring * ( segments + 1 ) + i;
								int next = 1 + ( ring + 1 ) * ( segments + 1 ) + i;
								draw_list->AddLine(
									ImVec2( static_cast< float >( vertices2d [ curr ].x ) , static_cast< float >( vertices2d [ curr ].y ) ) ,
									ImVec2( static_cast< float >( vertices2d [ next ].x ) , static_cast< float >( vertices2d [ next ].y ) ) ,
									color , 0.5f
								);
							}
						}
					}
				}

				if ( config::visuals::gender )
				{
					internal::drawing_t drawing;
					auto color = ImGui::ColorConvertFloat4ToU32( config::visuals::gender_color );

					sdk::a_fort_player_pawn fpp( actor.m_pawn );
					sdk::e_fort_custom_gender cgender = ( sdk::e_fort_custom_gender ) fpp.character_gender;
					std::string str = "unknown";

					switch ( cgender )
					{
					case sdk::e_fort_custom_gender::male:   str = "male"; break;
					case sdk::e_fort_custom_gender::female: str = "female"; break;
					case sdk::e_fort_custom_gender::both: str = "transgender"; break;
					case sdk::e_fort_custom_gender::invalid: str = "invaid"; break;
					case sdk::e_fort_custom_gender::e_fort_custom_gender_max: str = "gender max"; break;
					default: str = "unknown"; break;
					}

					drawing.text( ImGui::GetBackgroundDrawList( ) , c1 , c2 , str.c_str( ) , internal::position_t::e_positions::right , color , IM_COL32_BLACK , 3.2 , 9.43 );
				}

				if ( config::visuals::rank )
				{
					internal::drawing_t drawing;
					auto color = ImGui::ColorConvertFloat4ToU32( config::visuals::rank_color );

					uintptr_t player_state_addr = device->read<uintptr_t>( actor.m_pawn.address + offsets::Pawn::PlayerState );
					if ( player_state_addr )
					{
						uintptr_t habanero_component = device->read<uintptr_t>( player_state_addr + 0x948 );
						if ( habanero_component )
						{
							int rprogress_value = device->read<int>( habanero_component + 0xd8 );
							sdk::e_ranked_type rprogress = ( sdk::e_ranked_type ) rprogress_value;

							std::string str = "unknown";

							switch ( rprogress )
							{
							case sdk::e_ranked_type::none:     str = "unranked"; break;
							case sdk::e_ranked_type::bronze:   str = "bronze";   break;
							case sdk::e_ranked_type::silver:   str = "silver";   break;
							case sdk::e_ranked_type::gold:     str = "gold";     break;
							case sdk::e_ranked_type::platinum: str = "platinum"; break;
							case sdk::e_ranked_type::diamond:  str = "diamond";  break;
							case sdk::e_ranked_type::elite:    str = "elite";    break;
							case sdk::e_ranked_type::unreal:   str = "unreal";   break;
							default:                           str = "unknown rank"; break;
							}

							drawing.text( ImGui::GetBackgroundDrawList( ) , c1 , c2 , str.c_str( ) , internal::position_t::e_positions::right , color , IM_COL32_BLACK , 3.2f , config::visuals::text_y );
						}
					}
				}
			}
		}
	}
}