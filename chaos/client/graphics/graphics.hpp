#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <d3d11.h>
#include <dxgi.h>

namespace graphics
{
	struct fonts_t
	{
		ImFont * smallest_pixel = nullptr;
	};
	inline fonts_t fonts;

	class c_rendering
	{
	public:
		bool create_overlay( HWND target_hwnd );
		void destroy_overlay( );
		void render_loop( );
		void render_menu( );
		void move_window( );
		void call( );
	public:
		HWND m_hwnd = nullptr;
		HWND m_game_hwnd = nullptr;
		ID3D11Device * m_device = nullptr;
		ID3D11DeviceContext * m_context = nullptr;
		IDXGISwapChain * m_swapchain = nullptr;
		ID3D11RenderTargetView * m_render_target_view = nullptr;

	private:
		bool m_show = false;
	};
	inline std::shared_ptr<c_rendering> rendering = std::make_shared<c_rendering>( );
}