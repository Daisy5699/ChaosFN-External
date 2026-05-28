#pragma once

namespace console
{
	inline auto set_title( std::string title ) -> void
	{
		SetConsoleTitleA( title.c_str( ) );
	}

	inline auto set_ansi( ) -> void
	{
		auto console_handle = GetStdHandle( STD_OUTPUT_HANDLE );
		if ( console_handle == INVALID_HANDLE_VALUE )
			return;

		DWORD mode = 0;
		if ( !GetConsoleMode( console_handle , &mode ) )
			return;

		mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		if ( !SetConsoleMode( console_handle , mode ) )
			return;
	}

	inline auto set_font( const wchar_t* font_name ) -> void
	{
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof( CONSOLE_FONT_INFOEX );
		cfi.nFont = 0;
		cfi.dwFontSize.X = 8;
		cfi.dwFontSize.Y = 12;
		cfi.FontFamily = FF_DONTCARE;
		cfi.FontWeight = FW_NORMAL;
		wcscpy( cfi.FaceName , font_name );
		SetCurrentConsoleFontEx( GetStdHandle( STD_OUTPUT_HANDLE ) , FALSE , &cfi );
	}
}