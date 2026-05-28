#pragma once
#include "definitions/definitions.hpp"

class c_driver
{
public:
    INT32        m_process_id = 0;
    ULONGLONG    m_cr3 = 0;
    uintptr_t    m_base = 0;
    std::wstring m_process_name = L"";

private:
    HANDLE m_driver_handle = INVALID_HANDLE_VALUE;

public:
    bool connect_to_driver( )
    {
        return false;
    }

    void disconnect( )
    { }

    INT32 find_process_id_by_name( const wchar_t * name )
    {
        return 0;
    }

    ULONGLONG get_process_cr3( )
    {
        return 0;
    }

    bool read_physical_memory( uintptr_t address , void * buffer , DWORD size )
    {
        return false;
    }

    bool write_physical_memory( uintptr_t address , const void * buffer , DWORD size )
    {
        return false;
    }

    uintptr_t get_module_base_address( )
    {
        return 0;
    }

    uintptr_t get_guarded_region_address( )
    {
        return 0;
    }

    std::wstring get_attached_process_name( ) const
    {
        return m_process_name;
    }

    bool attach_to_process( const std::wstring & name )
    {
        return false;
    }

    template<typename T>
    T read( uint64_t address )
    {
        T value {};
        this->read_physical_memory( address , &value , sizeof( T ) );
        return value;
    }

    template<typename T>
    bool write( uint64_t address , const T & value )
    {
        return this->write_physical_memory( address , &value , sizeof( T ) );
    }

    bool move_mouse( LONG delta_x = 0 , LONG delta_y = 0 , USHORT button_flags = 0 )
    {
        return false;
    }
};

inline std::unique_ptr<c_driver> device = std::make_unique<c_driver>( );