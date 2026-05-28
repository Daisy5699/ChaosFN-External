#pragma once

namespace output
{
    template<typename... Args>
    inline void print( const char * format , Args... args )
    {
        auto now = std::chrono::system_clock::now( );
        std::time_t time = std::chrono::system_clock::to_time_t( now );
        tm local_tm;
        localtime_s( &local_tm , &time );
        std::printf( "\x1b[38;2;162;210;255m[%02d/%02d/%04d %02d:%02d:%02d]\x1b[0m " ,
            local_tm.tm_mon + 1 ,
            local_tm.tm_mday ,
            local_tm.tm_year + 1900 ,
            local_tm.tm_hour ,
            local_tm.tm_min ,
            local_tm.tm_sec );
        std::printf(  "\x1b[38;2;0;119;182m[ghostvisor]\x1b[0m " );
        std::printf( "\x1b[37m" );
        std::printf( format , args... );
        std::printf( "\x1b[0m\n" );
    }
}