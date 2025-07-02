#pragma once
#define NOMINMAX
#include "Easing.h"
#include <Windows.h>
#include <algorithm>
#include <numbers>
#include <vector>

// 前方宣言
class Player;

class CameraContoroller {
public:

	// 矩形
	struct Rect {
		float left = 0.0f;
		float right = 1.0f;
		float bottom = 0.0f;
		float top = 1.0f;
	};

	// カメラ移動範囲
	Rect movebleArea_ = { 0, 100, 0, 100 };

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Transform& camera);

	void SetTarget(Player* target) { target_ = target; }

	void SetMovebleArea(Rect area) { movebleArea_ = area; }

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void Reset();


private:

	// ビュープロジェクション
	/*Camera* camera_ = nullptr;*/
	Transform camera_;

	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差(オフセット)
	Vector3 targetOffset_ = { 0, 0, -30.0f };

	// カメラの目標座標
	Vector3 targetPos_;
	// 座標補間割合
	static inline const float kInterpolationRate = 0.1f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect kMovebleArea = { 0, 100, 0, 100 };

};

