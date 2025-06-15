#pragma once
#include <format>
#include "Input.h"

// キーが押されている状態か
bool IsPressKey(const uint8_t key);
// キーが離されている状態か
bool IsReleaseKey(const uint8_t key);

// キーが押された瞬間か
bool IsTriggerKey(const uint8_t key, const uint8_t preKey);

// キーが離された瞬間か
bool IsMomentReleaseKey(const uint8_t key, const uint8_t prekey);

// マウスが押されている状態か
bool IsPressMouse(const uint8_t mouse);

// マウスが離されている状態か
bool IsReleaseMouse(const uint8_t mouse);

// マウスが押された瞬間か
bool IsTriggerMouse(const uint8_t mouse, const uint8_t preMouse);

// マウスが離された瞬間か
bool IsMomentReleaseMouse(const uint8_t mouse, const uint8_t preMouse);

