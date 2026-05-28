#include "bootstrap.hpp"

namespace ghost
{
	auto c_bootstrap::call( ) -> void
	{
		console::set_title( "ghostvisor - grounded 1" );
		console::set_font( L"Terminal" );
		console::set_ansi( );

		output::print( "setting up communications between driver" );
		if ( !device->attach_to_process( L"FortniteClient-Win64-Shipping.exe" ) )
		{
			output::print( "failed to setup communication between driver" );
		}
		output::print( "communication established between driver" );

		output::print( "process id: %d" , device->m_process_id );
		output::print( "base address: <%llx>" , device->m_base );
		output::print( "cr3: <%llx>" , device->m_cr3 );

		sdk::u_world * frontend = sdk::u_world::get_frontend( );
		sdk::u_game_instance * game_instance = frontend->owning_game_instance;
		sdk::u_local_player * local_players = game_instance->local_players [ 0 ];

		output::print( "GWorld: <%llx>" , frontend );
		output::print( "OwningGameInstance: <%llx>" , game_instance );
		output::print( "LocalPlayer: <%llx>" , local_players );

		features::exploits::call( );
		graphics::rendering->call( );

		header->get_pipe( );
		header->check_dependency( );
	}
}