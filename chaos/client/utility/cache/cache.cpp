#include "cache.hpp"
#include "dependency/voyager/voyager.hpp"
#include <thread>
#include <chrono>
#include <client/utility/config/config.hpp>

auto c_cache::data( ) -> void
{
	for ( ;; )
	{
		sdk::u_world * uworld_ptr = sdk::u_world::get_frontend( );
		if ( !uworld_ptr )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}

		uworld.store( uworld_ptr , std::memory_order_release );

		uintptr_t uworld_addr = reinterpret_cast< uintptr_t >( uworld_ptr );
		sdk::u_game_instance * gi = device->read<sdk::u_game_instance *>( uworld_addr + 0x240 );
		if ( !gi )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}
		game_instance.store( gi , std::memory_order_release );

		uintptr_t gi_addr = reinterpret_cast< uintptr_t >( gi );
		uintptr_t local_players = device->read<uintptr_t>( gi_addr + 0x38 );
		if ( !local_players )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}

		sdk::u_local_player * lp = device->read<sdk::u_local_player *>( local_players );
		if ( !lp )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}
		local_player.store( lp , std::memory_order_release );

		uintptr_t lp_addr = reinterpret_cast< uintptr_t >( lp );
		sdk::a_player_controller * pc = device->read<sdk::a_player_controller *>( lp_addr + 0x30 );
		if ( !pc )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}
		player_controller.store( pc , std::memory_order_release );

		uintptr_t pc_addr = reinterpret_cast< uintptr_t >( pc );
		acknowledged_pawn.store( device->read<sdk::a_character *>( pc_addr + 0x358 ) , std::memory_order_release );

		sdk::a_game_state_base * gs = device->read<sdk::a_game_state_base *>( uworld_addr + 0x1C8 );
		if ( !gs )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}
		game_state.store( gs , std::memory_order_release );

		uintptr_t gs_addr = reinterpret_cast< uintptr_t >( gs );
		player_array.store( device->read<uintptr_t>( gs_addr + 0x2C8 ) , std::memory_order_release );
		player_array_size.store( device->read<int>( gs_addr + 0x2C8 + sizeof( uintptr_t ) ) , std::memory_order_release );

		std::this_thread::sleep_for( std::chrono::milliseconds( 50 ) );
	}
}

math::f_vector c_cache::read_bone( uintptr_t mesh , int bone_id )
{
	if ( !mesh )
		return math::f_vector( 0 , 0 , 0 );

	uintptr_t bone_array = device->read<uintptr_t>( mesh + 0x628 );
	if ( !bone_array )
		bone_array = device->read<uintptr_t>( mesh + 0x600 );
	if ( !bone_array )
		return math::f_vector( 0 , 0 , 0 );

	math::f_transform bone = device->read<math::f_transform>( bone_array + ( bone_id * sizeof( math::f_transform ) ) );
	math::f_transform component_to_world = device->read<math::f_transform>( mesh + 0x1E0 );

	math::f_matrix bone_matrix = bone.to_matrix_with_scale( );
	math::f_matrix ctw_matrix = component_to_world.to_matrix_with_scale( );
	math::f_matrix result = math::matrix_multiplication( bone_matrix , ctw_matrix );

	return math::f_vector( result.m [ 3 ][ 0 ] , result.m [ 3 ][ 1 ] , result.m [ 3 ][ 2 ] );
}

auto c_cache::entities( ) -> void
{
	for ( ;; )
	{
		sdk::u_world * uworld_ptr = uworld.load( std::memory_order_acquire );
		sdk::a_character * ack_pawn = acknowledged_pawn.load( std::memory_order_acquire );

		if ( !uworld_ptr )
		{
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
			continue;
		}

		std::vector<actor_t> previous_list;
		{
			std::lock_guard<std::mutex> lock( actor_mutex );
			previous_list = actor_list;
		}

		temporary_actor_list.clear( );
		uintptr_t ack_pawn_addr = reinterpret_cast< uintptr_t >( ack_pawn );

		sdk::t_array<sdk::u_level *> levels_list = uworld_ptr->levels;

		if ( levels_list.data && levels_list.count > 0 && levels_list.count < 1000 )
		{
			for ( int i = 0; i < levels_list.count; ++i )
			{
				sdk::u_level * level = device->read<sdk::u_level *>( levels_list.data + ( i * sizeof( sdk::u_level * ) ) );
				if ( !level ) continue;

				sdk::t_array<sdk::a_actor *> actors_list = level->actors;
				if ( !actors_list.data || actors_list.count <= 0 || actors_list.count > 10000 ) continue;

				for ( int j = 0; j < actors_list.count; ++j )
				{
					sdk::a_actor * actor = device->read<sdk::a_actor *>( actors_list.data + ( j * sizeof( sdk::a_actor * ) ) );				
					if ( !actor ) continue;

					uintptr_t pawn_addr = reinterpret_cast< uintptr_t >( actor );
				//	if ( ack_pawn_addr && pawn_addr == ack_pawn_addr ) continue;

					uintptr_t mesh = device->read<uintptr_t>( pawn_addr + offsets::Character::Mesh );
					if ( !mesh ) continue;

					uintptr_t player_state_ptr = device->read<uintptr_t>( pawn_addr + offsets::Pawn::PlayerState );
				//	if ( !player_state_ptr ) continue;

					sdk::a_player_state player_state {};
					player_state.address = player_state_ptr;

					actor_t cached_actor {};
					cached_actor.m_pawn.address = pawn_addr;
					cached_actor.m_mesh.address = mesh;
					cached_actor.m_player_state = player_state;

					math::f_vector head = read_bone( mesh , static_cast< int >( sdk::e_bone_ids::head ) );
					math::f_vector foot = read_bone( mesh , static_cast< int >( sdk::e_bone_ids::left_foot ) );

					if ( head.length( ) > 0.0 && foot.length( ) > 0.0 )
					{
						cached_actor.m_head_pos = head;
						cached_actor.m_foot_pos = foot;

						if ( config::visuals::skeleton )
						{
							uintptr_t bone_array = device->read<uintptr_t>( mesh + 0x628 );
							if ( !bone_array ) bone_array = device->read<uintptr_t>( mesh + 0x600 );

							if ( bone_array )
							{
								math::f_transform component_to_world = device->read<math::f_transform>( mesh + 0x1E0 );
								math::f_matrix ctw_matrix = component_to_world.to_matrix_with_scale( );

								auto get_bone = [ & ] ( sdk::e_bone_ids id ) -> math::f_vector
									{
										math::f_transform bone = device->read<math::f_transform>( bone_array + ( static_cast< int >( id ) * sizeof( math::f_transform ) ) );
										math::f_matrix bone_matrix = bone.to_matrix_with_scale( );
										math::f_matrix result = math::matrix_multiplication( bone_matrix , ctw_matrix );
										return math::f_vector( result.m [ 3 ][ 0 ] , result.m [ 3 ][ 1 ] , result.m [ 3 ][ 2 ] );
									};

								cached_actor.m_skeleton_lines.clear( );
								auto add_line = [ & ] ( sdk::e_bone_ids b1 , sdk::e_bone_ids b2 )
									{
										math::f_vector pos1 = get_bone( b1 );
										math::f_vector pos2 = get_bone( b2 );
										if ( pos1.length( ) > 1.0 && pos2.length( ) > 1.0 )
										{
											cached_actor.m_skeleton_lines.push_back( { pos1, pos2 } );
										}
									};

								add_line( sdk::e_bone_ids::head , sdk::e_bone_ids::neck );
								add_line( sdk::e_bone_ids::neck , sdk::e_bone_ids::chest );
								add_line( sdk::e_bone_ids::chest , sdk::e_bone_ids::pelvis );

								add_line( sdk::e_bone_ids::chest , sdk::e_bone_ids::left_shoulder );
								add_line( sdk::e_bone_ids::left_shoulder , sdk::e_bone_ids::left_arm );
								add_line( sdk::e_bone_ids::left_arm , sdk::e_bone_ids::left_elbow );

								add_line( sdk::e_bone_ids::chest , sdk::e_bone_ids::right_shoulder );
								add_line( sdk::e_bone_ids::right_shoulder , sdk::e_bone_ids::right_arm );
								add_line( sdk::e_bone_ids::right_arm , sdk::e_bone_ids::right_elbow );

								add_line( sdk::e_bone_ids::pelvis , sdk::e_bone_ids::left_hip );
								add_line( sdk::e_bone_ids::left_hip , sdk::e_bone_ids::left_knee );
								add_line( sdk::e_bone_ids::left_knee , sdk::e_bone_ids::left_foot );

								add_line( sdk::e_bone_ids::pelvis , sdk::e_bone_ids::right_hip );
								add_line( sdk::e_bone_ids::right_hip , sdk::e_bone_ids::right_knee );
								add_line( sdk::e_bone_ids::right_knee , sdk::e_bone_ids::right_foot );
							}
						}

						cached_actor.m_bones_valid = true;
					}
					else
					{
						cached_actor.m_bones_valid = false;
						for ( const auto & prev : previous_list )
						{
							if ( prev.m_pawn.address == pawn_addr && prev.m_bones_valid )
							{
								cached_actor.m_head_pos = prev.m_head_pos;
								cached_actor.m_foot_pos = prev.m_foot_pos;
								cached_actor.m_skeleton_lines = prev.m_skeleton_lines;
								cached_actor.m_bones_valid = true;
								break;
							}
						}
					}

					temporary_actor_list.push_back( cached_actor );
				}
			}
		}

		if ( !temporary_actor_list.empty( ) )
		{
			std::lock_guard<std::mutex> lock( actor_mutex );
			actor_list.swap( temporary_actor_list );
		}

		std::this_thread::sleep_for( std::chrono::milliseconds( 30 ) );
	}
}