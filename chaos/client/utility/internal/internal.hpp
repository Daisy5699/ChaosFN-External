#pragma once

#include <string>
#include <vector>
#include <cmath>

#undef min
#undef max


namespace internal
{
    struct position_t
    {
        enum e_positions
        {
            left = 0 ,
            top ,
            right ,
            bottom
        } side;

        position_t( e_positions s ) : side( s ) { }

        ImVec2 get_position(
            const ImVec2 & box_pos ,
            const ImVec2 & box_size ,
            const ImVec2 & text_size ,
            float gap = 3.0f ,
            float offset = 0.0f
        ) const
        {
            switch ( side )
            {
            case left:
                return ImVec2(
                    std::round( box_pos.x - gap - text_size.x ) ,
                    std::round( box_pos.y + offset )
                );

            case right:
                return ImVec2(
                    std::round( box_pos.x + box_size.x + gap ) ,
                    std::round( box_pos.y + offset )
                );

            case top:
                return ImVec2(
                    std::round( box_pos.x + ( box_size.x * 0.5f ) - ( text_size.x * 0.5f ) ) ,
                    std::round( box_pos.y - text_size.y - gap - offset )
                );

            case bottom:
                return ImVec2(
                    std::round( box_pos.x + ( box_size.x * 0.5f ) - ( text_size.x * 0.5f ) ) ,
                    std::round( box_pos.y + box_size.y + gap + offset )
                );
            }

            return box_pos;
        }
    };

    struct drawing_t
    {
        void rect(
            const ImVec2 & pos ,
            const ImVec2 & size ,
            ImU32 col ,
            float thickness ,
            float rounding = 0.0f
        )
        {
            ImVec2 rounded_pos( std::round( pos.x ) , std::round( pos.y ) );
            ImVec2 rounded_size( std::round( size.x ) , std::round( size.y ) );

            ImVec2 max(
                rounded_pos.x + rounded_size.x ,
                rounded_pos.y + rounded_size.y
            );

            auto draw = ImGui::GetBackgroundDrawList( );

            float max_rounding =
                std::min( rounded_size.x , rounded_size.y ) / 2.0f;

            rounding = std::min( rounding , max_rounding );

            ImU32 outline = IM_COL32( 0 , 0 , 0 , col >> 24 );

            draw->AddRect(
                rounded_pos ,
                max ,
                outline ,
                rounding ,
                0 ,
                thickness
            );

            draw->AddRect(
                ImVec2( rounded_pos.x - 2.f , rounded_pos.y - 2.f ) ,
                ImVec2( max.x + 2.f , max.y + 2.f ) ,
                outline ,
                rounding ,
                0 ,
                thickness
            );

            draw->AddRect(
                ImVec2( rounded_pos.x - 1.f , rounded_pos.y - 1.f ) ,
                ImVec2( max.x + 1.f , max.y + 1.f ) ,
                col ,
                rounding ,
                0 ,
                thickness
            );
        }

        static void health_bar( const ImVec2 & box_pos , const ImVec2 & box_size , float health , float max_health , const float color [ 4 ] , float gap = 4.0f , float thickness = 3.0f , ImU32 outline_col = IM_COL32( 0 , 0 , 0 , 255 ) )
        {
            auto draw = ImGui::GetBackgroundDrawList( );
            draw->Flags &= ~ImDrawListFlags_AntiAliasedLines;

            float ratio = ( max_health > 0.f ) ? health / max_health : 0.f;
            ratio = std::fmax( 0.f , std::fmin( ratio , 1.f ) );

            internal::position_t left_side( internal::position_t::left );
            ImVec2 bar_base_pos = left_side.get_position( box_pos , box_size , ImVec2( thickness , box_size.y ) , gap );

            float y_min = std::round( box_pos.y );
            float y_max = std::round( box_pos.y + box_size.y );
            float x_hp = std::round( bar_base_pos.x );

            draw->AddRectFilled( ImVec2( x_hp - 1.f , y_min - 2.f ) , ImVec2( x_hp + thickness + 1.f , y_max + 2.f ) , outline_col );
            draw->AddRectFilled( ImVec2( x_hp , y_min - 1.f ) , ImVec2( x_hp + thickness , y_max + 1.f ) , IM_COL32( 130 , 130 , 130 , 150 ) );

            float height = ( y_max - y_min ) * ratio;
            ImVec2 fg_min( x_hp , y_max - height - 1.f );
            ImVec2 fg_max( x_hp + thickness , y_max + 1.f );

            ImVec4 final_color;
            if ( false )
            {
                if ( ratio < 0.2f ) final_color = ImVec4( 235.f / 255.f , 52.f / 255.f , 52.f / 255.f , color [ 3 ] );
                else if ( ratio < 0.4f ) final_color = ImVec4( 235.f / 255.f , 168.f / 255.f , 52.f / 255.f , color [ 3 ] );
                else if ( ratio < 0.7f ) final_color = ImVec4( 192.f / 255.f , 235.f / 255.f , 52.f / 255.f , color [ 3 ] );
                else                     final_color = ImVec4( 94.f / 255.f , 235.f / 255.f , 52.f / 255.f , color [ 3 ] );
            }
            else
            {
                final_color = ImVec4( color [ 0 ] , color [ 1 ] , color [ 2 ] , color [ 3 ] );
            }

            draw->AddRectFilled( fg_min , fg_max , ImGui::ColorConvertFloat4ToU32( final_color ) );
        }

        static void text(
            ImDrawList * draw ,
            const ImVec2 & box_pos ,
            const ImVec2 & box_size ,
            const std::string & text ,
            const position_t & side ,
            ImU32 color ,
            ImU32 outline_color ,
            float gap = 3.0f ,
            float offset = 0.0f
        )
        {
            ImVec2 text_size =
                ImGui::CalcTextSize( text.c_str( ) );

            ImVec2 pos =
                side.get_position(
                    box_pos ,
                    box_size ,
                    text_size ,
                    gap ,
                    offset
                );

            for ( int x = -1; x <= 1; x++ )
            {
                for ( int y = -1; y <= 1; y++ )
                {
                    if ( x || y )
                    {
                        draw->AddText(
                            pos + ImVec2( ( float ) x , ( float ) y ) ,
                            outline_color ,
                            text.c_str( )
                        );
                    }
                }
            }

            draw->AddText(
                pos ,
                color ,
                text.c_str( )
            );
        }
    };

    // Helper context for automatically tracking stacked text offsets around an ESP box
    struct esp_context_t
    {
        ImDrawList* draw;
        ImVec2 box_pos;
        ImVec2 box_size;
        float offsets[ 4 ];

        esp_context_t( ImDrawList* d , const ImVec2& pos , const ImVec2& size )
            : draw( d ) , box_pos( pos ) , box_size( size )
        {
            offsets[ 0 ] = 0.0f;
            offsets[ 1 ] = 0.0f;
            offsets[ 2 ] = 0.0f;
            offsets[ 3 ] = 0.0f;
        }

        void text(
            const std::string& text_str ,
            const position_t& side ,
            ImU32 color ,
            ImU32 outline_color ,
            float gap = 3.0f
        )
        {
            ImVec2 text_size = ImGui::CalcTextSize( text_str.c_str( ) );

            float& current_offset = offsets[ side.side ];
            ImVec2 pos = side.get_position( box_pos , box_size , text_size , gap , current_offset );

            // Update offset for next item on this side
            current_offset += text_size.y + 1.0f; // Add a tiny 1px padding between stacked texts

            for ( int x = -1; x <= 1; x++ )
            {
                for ( int y = -1; y <= 1; y++ )
                {
                    if ( x || y )
                    {
                        draw->AddText(
                            pos + ImVec2( ( float )x , ( float )y ) ,
                            outline_color ,
                            text_str.c_str( )
                        );
                    }
                }
            }

            draw->AddText( pos , color , text_str.c_str( ) );
        }
    };

    inline std::vector<position_t> esides =
    {
        position_t( position_t::left ),
        position_t( position_t::top ),
        position_t( position_t::right ),
        position_t( position_t::bottom ),
        position_t( position_t::left )
    };

    namespace colors
    {
        inline ImU32 float_to_imvec4( const float color [ 4 ] )
        {
            return ImGui::ColorConvertFloat4ToU32(
                ImVec4(
                    color [ 0 ] ,
                    color [ 1 ] ,
                    color [ 2 ] ,
                    color [ 3 ]
                )
            );
        }

        inline ImU32 float_to_imvec4(
            const float color [ 3 ] ,
            float alpha = 1.0f
        )
        {
            return ImGui::ColorConvertFloat4ToU32(
                ImVec4(
                    color [ 0 ] ,
                    color [ 1 ] ,
                    color [ 2 ] ,
                    alpha
                )
            );
        }

        inline float * imvec4_to_float( const ImVec4 & color )
        {
            static float values [ 4 ];

            values [ 0 ] = color.x;
            values [ 1 ] = color.y;
            values [ 2 ] = color.z;
            values [ 3 ] = color.w;

            return values;
        }
    }
}
