#pragma once
#define NOMINMAX
#include <Windows.h>
#include <numbers>
#include <vector>
#include "Easing.h"
#include <algorithm>
#include "MapChipField.h"


struct AABB {
	Vector3 min;
	Vector3 max;
};

// 直方体と直方体の当たり判定
bool IsCollision(const AABB& aabb1, const AABB& aabb2);
