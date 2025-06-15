#include "Input.h"

// キーが押されている状態か
bool IsPressKey(const uint8_t key) {
	if (key) {
		return true;
	}
	return false;
}

// キーが離されている状態か
bool IsReleaseKey(const uint8_t key) {
	if (!key) {
		return true;
	}
	return false;
}

// キーが押された瞬間か
bool IsTriggerKey(const uint8_t key, const uint8_t preKey) {
	if (!preKey && key) {
		return true;
	}
	return false;
}

// キーが離された瞬間か
bool IsMomentReleaseKey(const uint8_t key, const uint8_t prekey) {
	if (prekey && !key) {
		return true;
	}
	return false;
}

// マウスが押されている状態か
bool IsPressMouse(const uint8_t mouse) {
	if (mouse) {
		return true;
	}
	return false;
}

// マウスが離されている状態か
bool IsReleaseMouse(const uint8_t mouse) {
	if (!mouse) {
		return true;
	}
	return false;
}

// マウスが押された瞬間か
bool IsTriggerMouse(const uint8_t mouse, const uint8_t preMouse) {
	if (!preMouse && mouse) {
		return true;
	}
	return false;
}

// マウスが離された瞬間か
bool IsMomentReleaseMouse(const uint8_t mouse, const uint8_t preMouse) {
	if (preMouse && !mouse) {
		return true;
	}
	return false;
}