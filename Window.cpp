#include "Window.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

LRESULT Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	{

		if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
			return true;
		}

		// メッセージに応じてゲーム固有の処理を行う
		switch (msg) {
			// ウィンドウが破棄された
		case WM_DESTROY:
			// OSに対して、アプリの終了を伝える
			PostQuitMessage(0);
			return 0;
		}

		// 標準のメッセージ処置を行う
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
}

void Window::Initialize()
{
	// ウィンドウプロシージャ
	wc_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名
	wc_.lpszClassName = L"CG2WindouClass";
	// インタンスバンドル
	wc_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	wc_.hCursor = LoadCursor(nullptr, IDC_ARROW);

	// ウィンドウクラスを登録する
	RegisterClass(&wc_);

	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth_, kClientHeight_ };

	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウの生成
	hwnd_ = CreateWindow(
		wc_.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc_.hInstance,
		nullptr);

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugComtroller = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugComtroller)))) {
		// デバッグレイヤーを有効化する
		debugComtroller->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugComtroller->SetEnableGPUBasedValidation(TRUE);
	}
#endif

	// ウィンドウを表示する
	ShowWindow(hwnd_, SW_SHOW);

}

