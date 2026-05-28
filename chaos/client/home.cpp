#include <client/bootstrap/bootstrap.hpp>

std::int16_t main( std::int8_t argc , char * argv [ ] )
{
	std::ios_base::sync_with_stdio( false );

	ghost::bootstrap->call( );

	std::cin.get( );
}