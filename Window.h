#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

const int32_t kClientWidth_ = 1280;
const int32_t kClientHeight_ = 720;

class Window
{

public:

	// ウィンドウプロシージャ
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	WNDCLASS GetWindowClass() { return wc_; };
	HWND GetHwnd() { return hwnd_; }

private:
	HWND hwnd_ = nullptr;
	WNDCLASS wc_{};
	
};

