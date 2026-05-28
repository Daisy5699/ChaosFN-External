#pragma once
#include <imgui.h>

namespace Style
{
    inline ImVec4 Border = ImVec4( 0.090, 0.090, 0.118, 1.0 );
    inline ImVec4 BorderChild = ImVec4( 18 / 255.f, 18 / 255.f, 26 / 255.f, 1.0f );
    inline ImVec4 BorderInner = ImVec4( 0.243, 0.243, 0.282, 1.0 );

    inline ImVec4 WindowBg = ImVec4( 0.153, 0.153, 0.184, 1.0 );
    inline ImVec4 Accent = ImVec4( 206 / 255.f, 115 / 255.f, 136 / 255.f, 1.0f );

    inline ImVec4 MainChildBg = ImVec4( 23 / 255.f, 23 / 255.f, 30 / 255.f, 1.0f );
    inline ImVec4 InnerChildBg = ImVec4( 32 / 255.f, 32 / 255.f, 38 / 255.f, 1.0f );

    inline ImVec4 FrameBg = ImVec4( 0.15f, 0.15f, 0.18f, 1.0f );
    inline ImVec4 FrameBgHovered = ImVec4( 0.18f, 0.18f, 0.22f, 1.0f );
    inline ImVec4 FrameBorder = ImVec4( 0.09f, 0.09f, 0.12f, 1.0f );

    inline ImVec4 TabActiveTop = ImVec4( 0x37 / 255.f, 0x37 / 255.f, 0x40 / 255.f, 1.0f );
    inline ImVec4 TabActiveBot = ImVec4( 0x25 / 255.f, 0x25 / 255.f, 0x2D / 255.f, 1.0f );
    inline ImVec4 TabInactiveTop = ImVec4( 0x31 / 255.f, 0x31 / 255.f, 0x3B / 255.f, 1.0f );
    inline ImVec4 TabInactiveBot = ImVec4( 0x18 / 255.f, 0x18 / 255.f, 0x1C / 255.f, 1.0f );
    inline ImVec4 TabHoveredTop = ImVec4( 0x35 / 255.f, 0x35 / 255.f, 0x3F / 255.f, 1.0f );
    inline ImVec4 TabHoveredBot = ImVec4( 0x20 / 255.f, 0x20 / 255.f, 0x25 / 255.f, 1.0f );

    inline ImU32 TextActive   = IM_COL32( 255, 255, 255, 255 );
    inline ImU32 TextInactive = IM_COL32( 200, 200, 205, 255 );
    inline ImU32 TextHovered  = IM_COL32( 235, 235, 240, 255 );

    inline ImFont* FontRegular = nullptr;
    inline ImFont* FontBold = nullptr;

    inline void Setup( )
    {
        auto& s = ImGui::GetStyle( );
        auto  c = s.Colors;
        c [ ImGuiCol_WindowBg ] = WindowBg;
        c [ ImGuiCol_Text ] = ImVec4( 205 / 255.f, 205 / 255.f, 205 / 255.f, 1.0f );
    }
}
