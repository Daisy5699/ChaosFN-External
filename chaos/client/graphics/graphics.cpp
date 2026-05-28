#include "graphics.hpp"
#include <dwmapi.h>
#include <atomic>
#include <imgui_freetype.h>
#include "misc/style.h"
#include "misc/widgets.h"
#include <client/features/visuals/visuals.hpp>
#include <client/utility/config/config.hpp>
#include <binary/smallestpixel.hpp>

#pragma comment(lib, "dwmapi.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam );

namespace
{
    std::atomic<bool> g_running { true };
    int g_width = 0;
    int g_height = 0;
}

LRESULT CALLBACK window_proc( HWND hWnd , UINT msg , WPARAM wParam , LPARAM lParam )
{
    if ( ImGui_ImplWin32_WndProcHandler( hWnd , msg , wParam , lParam ) )
        return true;

    switch ( msg )
    {
    case WM_SIZE:
        if ( graphics::rendering->m_device != nullptr && wParam != SIZE_MINIMIZED )
        {
            if ( graphics::rendering->m_render_target_view )
            {
                graphics::rendering->m_render_target_view->Release( );
                graphics::rendering->m_render_target_view = nullptr;
            }
            graphics::rendering->m_swapchain->ResizeBuffers( 0 , ( UINT ) LOWORD( lParam ) , ( UINT ) HIWORD( lParam ) , DXGI_FORMAT_UNKNOWN , 0 );
            ID3D11Texture2D * backBuffer = nullptr;
            graphics::rendering->m_swapchain->GetBuffer( 0 , IID_PPV_ARGS( &backBuffer ) );
            if ( backBuffer != nullptr )
            {
                graphics::rendering->m_device->CreateRenderTargetView( backBuffer , nullptr , &graphics::rendering->m_render_target_view );
                backBuffer->Release( );
            }
        }
        return 0;

    case WM_SYSCOMMAND:
        if ( ( wParam & 0xfff0 ) == SC_KEYMENU )
            return 0;
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        return 0;

    default:
        break;
    }
    return DefWindowProc( hWnd , msg , wParam , lParam );
}

bool fullsc( HWND windowHandle )
{
    MONITORINFO monitorInfo = { sizeof( MONITORINFO ) };
    if ( GetMonitorInfo( MonitorFromWindow( windowHandle , MONITOR_DEFAULTTOPRIMARY ) , &monitorInfo ) )
    {
        RECT windowRect;
        if ( GetWindowRect( windowHandle , &windowRect ) )
        {
            return windowRect.left == monitorInfo.rcMonitor.left
                && windowRect.right == monitorInfo.rcMonitor.right
                && windowRect.top == monitorInfo.rcMonitor.top
                && windowRect.bottom == monitorInfo.rcMonitor.bottom;
        }
    }
    return false;
}

void graphics::c_rendering::move_window( )
{
}

bool graphics::c_rendering::create_overlay( HWND target_hwnd )
{
    m_game_hwnd = target_hwnd;

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof( WNDCLASSEXA );
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = GetModuleHandleA( nullptr );
    wc.lpszClassName = "OverlayClass";
    RegisterClassExA( &wc );

    RECT desktop;
    GetWindowRect( GetDesktopWindow( ) , &desktop );

    m_hwnd = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW ,
        "OverlayClass" , "Overlay" , WS_POPUP ,
        desktop.left , desktop.top ,
        desktop.right - desktop.left ,
        desktop.bottom - desktop.top ,
        nullptr , nullptr , GetModuleHandleA( nullptr ) , nullptr );

    if ( !m_hwnd )
        return false;

    SetLayeredWindowAttributes( m_hwnd , RGB( 0 , 0 , 0 ) , 255 , LWA_ALPHA );
    MARGINS margin = { -1,-1,-1,-1 };
    DwmExtendFrameIntoClientArea( m_hwnd , &margin );
    ShowWindow( m_hwnd , SW_SHOW );
    UpdateWindow( m_hwnd );

    DXGI_SWAP_CHAIN_DESC sd {};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = m_hwnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureArray [ 1 ] = { D3D_FEATURE_LEVEL_11_0 };
    if ( FAILED( D3D11CreateDeviceAndSwapChain( nullptr , D3D_DRIVER_TYPE_HARDWARE , nullptr , 0 ,
        featureArray , 1 , D3D11_SDK_VERSION ,
        &sd , &m_swapchain , &m_device , &featureLevel , &m_context ) ) )
    {
        UnregisterClassA( wc.lpszClassName , wc.hInstance );
        return false;
    }

    ID3D11Texture2D * backBuffer = nullptr;
    m_swapchain->GetBuffer( 0 , IID_PPV_ARGS( &backBuffer ) );
    m_device->CreateRenderTargetView( backBuffer , nullptr , &m_render_target_view );
    backBuffer->Release( );

    IMGUI_CHECKVERSION( );
    ImGui::CreateContext( );
    ImGuiIO & io = ImGui::GetIO( );
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark( );

    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 3;
    cfg.PixelSnapH = true;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    cfg.FontDataOwnedByAtlas = false;

    ImFontConfig font_cfg;
    font_cfg.PixelSnapH = true;
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 1;
    font_cfg.RasterizerMultiply = 1.05f;
    font_cfg.FontBuilderFlags = 0;

    Style::FontRegular = io.Fonts->AddFontFromFileTTF( "E:\\Windows\\Fonts\\Bahnschrift.ttf" , 13.0f , &font_cfg );
    Style::FontBold = io.Fonts->AddFontFromFileTTF( "E:\\Windows\\Fonts\\Bahnschrift.ttf" , 13.0f , &font_cfg );
    fonts.smallest_pixel = io.Fonts->AddFontFromMemoryTTF( binary_smallestpixel , sizeof( binary_smallestpixel ) , 10.0f , &cfg , io.Fonts->GetGlyphRangesJapanese( ) );


    ImGui_ImplWin32_Init( m_hwnd );
    ImGui_ImplDX11_Init( m_device , m_context );

    RECT r;
    GetClientRect( m_hwnd , &r );
    g_width = r.right - r.left;
    g_height = r.bottom - r.top;

    g_running = true;

    return true;
}

void graphics::c_rendering::destroy_overlay( )
{
    g_running = false;

    ImGui_ImplDX11_Shutdown( );
    ImGui_ImplWin32_Shutdown( );
    ImGui::DestroyContext( );

    if ( m_render_target_view ) { m_render_target_view->Release( ); m_render_target_view = nullptr; }
    if ( m_swapchain ) { m_swapchain->Release( ); m_swapchain = nullptr; }
    if ( m_context ) { m_context->Release( ); m_context = nullptr; }
    if ( m_device ) { m_device->Release( ); m_device = nullptr; }

    if ( m_hwnd )
    {
        DestroyWindow( m_hwnd );
        m_hwnd = nullptr;
    }
    UnregisterClassA( "OverlayClass" , GetModuleHandleA( nullptr ) );
}

void graphics::c_rendering::render_menu( )
{
    const char * watermark = "chaos-fortnite";
    ImVec2 wm_size = ImGui::CalcTextSize( watermark );
    ImVec2 wm_pos = ImVec2( 10.0f , 10.0f );
    ImGui::GetForegroundDrawList()->AddText( wm_pos , IM_COL32( 255 , 255 , 255 , 200 ) , watermark );

    if ( this->m_show )
    {
        ShowCursor( false );

        ImGuiIO & io = ImGui::GetIO( );
        ImDrawList * draw_list = ImGui::GetForegroundDrawList( );

        ImVec2 cursor_pos = io.MousePos;
        float radius = 5.0f;
        draw_list->AddCircleFilled( cursor_pos , radius , ImColor( Style::Accent ) );
        draw_list->AddCircle( cursor_pos , radius , IM_COL32( 40 , 20 , 25 , 60 ) , 0 , 1.0f );

        ImGui::SetNextWindowSizeConstraints( ImVec2( 700 , 475 ) , ImVec2( FLT_MAX , FLT_MAX ) );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize , 0.0f );

        ImGui::Begin( "Window" , nullptr , ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize );
        ImGui::PopStyleVar( );
        {
            ImVec2 pos = ImGui::GetWindowPos( );
            ImVec2 sz = ImGui::GetWindowSize( );
            ImDrawList * dl = ImGui::GetWindowDrawList( );

            Widgets::DrawBorder( dl , pos , sz , ImColor( Style::Border ) );
            Widgets::DrawBorder( dl , pos , sz , ImColor( Style::BorderInner ) , 2 );

            ImVec2 ws = sz;

            ImGui::SetCursorPos( ImVec2( 8 , 8 ) );
            ImGui::PushStyleColor( ImGuiCol_ChildBg , Style::MainChildBg );
            ImGui::BeginChild( "Main" , ImVec2( ws.x - 16 , ws.y - 16 ) , true );
            {
                ImVec2 cpos = ImGui::GetWindowPos( );
                ImVec2 csize = ImGui::GetWindowSize( );
                ImDrawList * cdl = ImGui::GetWindowDrawList( );

                ImVec2 a = ImVec2( cpos.x + 2.5f , cpos.y + 2.5f );
                ImVec2 b = ImVec2( cpos.x + csize.x - 2.0f , cpos.y + 4 );
                cdl->AddRectFilled( a , b , ImColor( Style::Accent ) );

                ImVec2 cs = csize;

                ImGui::SetCursorPos( ImVec2( 12 , 12 ) );
                ImGui::PushStyleColor( ImGuiCol_ChildBg , Style::InnerChildBg );
                ImGui::BeginChild( "Inner" , ImVec2( cs.x - 24 , cs.y - 24 ) , true );
                {
                    static int tab = 0;
                    const char * labels [ 5 ] = { "combat", "visuals", "exploits", "misc", "config" };



                    ImVec2 ip = ImGui::GetWindowPos( );
                    ImVec2 is = ImGui::GetWindowSize( );
                    ImDrawList * dl2 = ImGui::GetWindowDrawList( );

                    float left = ip.x;
                    float right = ip.x + is.x;
                    float top = ip.y;
                    float w = ( right - left ) / 6.0f;
                    float h = 28.0f;

                    for ( int i = 0; i < 5; ++i )
                    {
                        float x0 = left + w * i;
                        float x1 = left + w * ( i + 1 );

                        bool hovered = false;
                        ImGui::SetCursorScreenPos( ImVec2( x0 , top ) );
                        ImGui::InvisibleButton( labels [ i ] , ImVec2( w , h ) );
                        hovered = ImGui::IsItemHovered( );
                        if ( ImGui::IsItemClicked( ) ) tab = i;

                        ImU32 tcol , bcol;
                        if ( i == tab )
                        {
                            tcol = IM_COL32( 0x37 , 0x37 , 0x40 , 255 );
                            bcol = IM_COL32( 0x25 , 0x25 , 0x2D , 255 );
                        }
                        else
                        {
                            tcol = IM_COL32( 0x31 , 0x31 , 0x3B , 255 );
                            bcol = IM_COL32( 0x18 , 0x18 , 0x1C , 255 );

                            if ( hovered )
                            {
                                tcol = IM_COL32( 0x35 , 0x35 , 0x3F , 255 );
                                bcol = IM_COL32( 0x20 , 0x20 , 0x25 , 255 );
                            }
                        }

                        ImVec2 ra( x0 , top ) , rb( x1 , top + h );
                        dl2->AddRectFilledMultiColor( ra , rb , tcol , tcol , bcol , bcol );

                        dl2->AddLine( ImVec2( x0 , top ) , ImVec2( x0 , rb.y ) , ImColor( Style::BorderInner ) );
                        dl2->AddLine( ImVec2( x1 , top ) , ImVec2( x1 , rb.y ) , ImColor( Style::BorderInner ) );
                        dl2->AddLine( ImVec2( x0 , top ) , ImVec2( x1 , top ) , ImColor( Style::BorderInner ) );
                        dl2->AddLine( ImVec2( x0 , rb.y ) , ImVec2( x1 , rb.y ) , ImColor( Style::BorderInner ) );

                        if ( i == tab )
                            dl2->AddRectFilled( ImVec2( x0 , rb.y - 1 ) , ImVec2( x1 , rb.y ) , ImColor( Style::Accent ) );

                        ImVec2 ts = ImGui::CalcTextSize( labels [ i ] );
                        ImVec2 tc( x0 + ( w - ts.x ) * 0.5f , top + ( h - ts.y ) * 0.5f - 1.0f );
                        ImU32 text_col;
                        if ( i == tab ) text_col = IM_COL32( 255 , 255 , 255 , 255 );
                        else if ( hovered ) text_col = IM_COL32( 235 , 235 , 240 , 255 );
                        else text_col = IM_COL32( 200 , 200 , 205 , 255 );

                        dl2->AddText( tc , text_col , labels [ i ] );
                    }

                    ImGui::Dummy( ImVec2( 0 , h - 25.0f ) );

                    ImVec2 avail = ImGui::GetContentRegionAvail( );

                    if ( tab == 0 )
                    {
                        if ( Widgets::BeginTag( "LeftChild" , "legit" , ImVec2( avail.x * 0.5f - 6 , avail.y ) ) )
                        {
                            if ( ImGui::IsWindowHovered( ) )
                                ImGui::GetCurrentWindow( )->Flags |= ImGuiWindowFlags_NoMove;

                            static int subtab = 0;
                            const char * sublabels [ 1 ] = { "aimbot" };

                            ImVec2 start = ImGui::GetCursorScreenPos( );
                            float width = ImGui::GetWindowSize( ).x;

                            Widgets::SubTab( sublabels , 1 , subtab , start , width );
                            ImGui::Dummy( ImVec2( 0 , 6 ) );
                            ImGui::Indent( 44.0f );

                            if ( subtab == 0 )
                            {
                                static bool aimbot_enabled;
                                Widgets::Checkbox( "enable" , &aimbot_enabled );
                                if ( aimbot_enabled )
                                {
                                    static float fov = 2.5f;
                                    ImGui::SliderFloat( "fov" , &fov , 0.0f , 10.0f , "%.1f" );
                                    static int aim_types = 0;
                                    std::vector<const char *> targeting_modes = { "closest", "fov" };
                                    Widgets::Dropdown( "target type" , "target type" , targeting_modes , &aim_types );
                                }


                            }

                            ImGui::Unindent( 44.0f );
                        }
                        Widgets::EndTag( );

                        ImGui::SameLine( );

                        if ( Widgets::BeginTag( "RightChild" , "other" , ImVec2( avail.x * 0.5f - 6 , avail.y ) , ImColor( Style::BorderChild ) ) )
                        {
                            if ( ImGui::IsWindowHovered( ) )
                                ImGui::GetCurrentWindow( )->Flags |= ImGuiWindowFlags_NoMove;

                            static int subtab = 0;
                            const char * sublabels [ 2 ] = { "silent aim", "triggerbot" };

                            ImVec2 start = ImGui::GetCursorScreenPos( );
                            float width = ImGui::GetWindowSize( ).x;

                            Widgets::SubTab( sublabels , 2 , subtab , start , width );
                            ImGui::Dummy( ImVec2( 0 , 6 ) );
                            ImGui::Indent( 44.0f );

                            if ( subtab == 0 )
                            {
                                static bool silent;
                                Widgets::Checkbox( "enable" , &silent );
                            }

                            ImGui::Unindent( 44.0f );
                        }
                        Widgets::EndTag( );
                    }
                    else if ( tab == 1 )
                    {
                        if ( Widgets::BeginTag( "LeftChild" , "visuals" , ImVec2( avail.x * 0.5f - 6 , avail.y ) ) )
                        {
                            if ( ImGui::IsWindowHovered( ) )
                                ImGui::GetCurrentWindow( )->Flags |= ImGuiWindowFlags_NoMove;

                            static int subtab = 0;
                            const char * sublabels [ 2 ] = { "players", "items" };

                            ImVec2 start = ImGui::GetCursorScreenPos( );
                            float width = ImGui::GetWindowSize( ).x;

                            Widgets::SubTab( sublabels , 1 , subtab , start , width );
                            ImGui::Dummy( ImVec2( 0 , 6 ) );
                            ImGui::Indent( 44.0f );

                            if ( subtab == 0 )
                            {
                                Widgets::Checkbox( "enable" , &config::visuals::enable );
                                Widgets::Checkbox( "bounding box" , &config::visuals::box );
                                Widgets::Checkbox( "name" , &config::visuals::name );
                                Widgets::Checkbox( "platform" , &config::visuals::platform );
                                Widgets::Checkbox( "gender" , &config::visuals::gender );
                                Widgets::Checkbox( "china hat" , &config::visuals::china_hat );
                                Widgets::Checkbox( "skeleton" , &config::visuals::skeleton );
                                Widgets::Checkbox( "rank" , &config::visuals::rank );

                                ImGui::SliderFloat( "text y" , &config::visuals::text_y , 0.0f , 30.0f );
                            }

                            ImGui::Unindent( 44.0f );
                        }
                        Widgets::EndTag( );

                        ImGui::SameLine( );

                        if ( Widgets::BeginTag( "RightChild" , "other" , ImVec2( avail.x * 0.5f - 6 , avail.y ) , ImColor( Style::BorderChild ) ) )
                        {
                            if ( ImGui::IsWindowHovered( ) )
                                ImGui::GetCurrentWindow( )->Flags |= ImGuiWindowFlags_NoMove;

                            static int subtab = 0;
                            const char * sublabels [ 2 ] = { "world", "chams" };

                            ImVec2 start = ImGui::GetCursorScreenPos( );
                            float width = ImGui::GetWindowSize( ).x;

                            Widgets::SubTab( sublabels , 2 , subtab , start , width );
                            ImGui::Dummy( ImVec2( 0 , 6 ) );
                            ImGui::Indent( 44.0f );

                            if ( subtab == 0 )
                            {
                            
                            }

                            ImGui::Unindent( 44.0f );
                        }
                        Widgets::EndTag( );
                    }
                    

                    Widgets::RenderTags( );
                }
                ImGui::EndChild( );
                ImGui::PopStyleColor( );
            }
            ImGui::EndChild( );
            ImGui::PopStyleColor( );
        }
        ImGui::End( );

    }
}

void graphics::c_rendering::render_loop( )
{
    MSG msg = { NULL };
    while ( msg.message != WM_QUIT && g_running )
    {
        UpdateWindow( m_hwnd );
        ShowWindow( m_hwnd , SW_SHOW );

        if ( PeekMessageA( &msg , NULL , 0 , 0 , PM_REMOVE ) ) {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }

        if ( !g_running ) break;

        ImGuiIO& io = ImGui::GetIO( );
        io.DeltaTime = 1.0f / 60.0f;

        POINT p;
        GetCursorPos( &p );
        io.MousePos.x = static_cast< float >( p.x );
        io.MousePos.y = static_cast< float >( p.y );
        io.MouseDown [ 0 ] = ( GetAsyncKeyState( VK_LBUTTON ) & 0x8000 ) != 0;

        bool insert_now = ( GetAsyncKeyState( VK_INSERT ) & 0x8000 ) != 0;
        static bool insert_was_down = false;
        if ( insert_now && !insert_was_down ) {
            this->m_show = !this->m_show;
            SetWindowLongA( m_hwnd, GWL_EXSTYLE, this->m_show
                ? ( WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW )
                : ( WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW ) );
        }
        insert_was_down = insert_now;

        ImGui_ImplDX11_NewFrame( );
        ImGui_ImplWin32_NewFrame( );
        ImGui::NewFrame( );

        this->render_menu( );

        ImGui::PushFont( Style::FontRegular);
        features::visuals::call( );
        ImGui::PopFont( );

        ImGui::EndFrame( );
        ImGui::Render( );

        const float clear [ 4 ] = { 0.f, 0.f, 0.f, 0.f };
        m_context->OMSetRenderTargets( 1 , &m_render_target_view , nullptr );
        m_context->ClearRenderTargetView( m_render_target_view , clear );
        ImGui_ImplDX11_RenderDrawData( ImGui::GetDrawData( ) );

        m_swapchain->Present( 0 , 0 );
    }
}

void graphics::c_rendering::call( )
{
    if ( !rendering->create_overlay( nullptr ) )
        return;

    rendering->render_loop( );
    rendering->destroy_overlay( );
}