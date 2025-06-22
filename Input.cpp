#include "Input.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void Input::Initialize(const WNDCLASS& wc, const HWND& hwnd)
{
	IDirectInput8* directInput = nullptr;
	HRESULT hr = DirectInput8Create(
		wc.hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr
	);
	assert(SUCCEEDED(hr));
	
	hr = directInput->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard); // 標準形式
	assert(SUCCEEDED(hr));

	// 排他制御レベルのセット
	hr = keyboard_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
	
	hr = directInput->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetDataFormat(&c_dfDIMouse); // 標準形式
	assert(SUCCEEDED(hr));

	// 排他制御レベルのセット
	hr = mouse_->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));

}

void Input::BeforeFrameData()
{
	// 前のフレームのキー状態を保存
	memcpy(preKey_, key_, sizeof(key_));
	keyboard_->Acquire();
	keyboard_->GetDeviceState(sizeof(key_), key_);

	// マウス情報の取得
	memcpy(&preMouseState_, &mouseState_, sizeof(DIMOUSESTATE));
	mouse_->Acquire();
	mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
}

bool Input::IsPress(const uint8_t key)
{
	if (key) {
		return true;
	}
	return false;
}

bool Input::IsRelease(const uint8_t key)
{
	if (!key) {
		return true;
	}
	return false;
}

bool Input::IsTrigger(const uint8_t key, const uint8_t preKey)
{
	if (!preKey && key) {
		return true;
	}
	return false;
}

bool Input::IsMomentRelease(const uint8_t key, const uint8_t prekey)
{
	if (prekey && !key) {
		return true;
	}
	return false;
}