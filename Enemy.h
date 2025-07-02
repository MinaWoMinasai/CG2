#pragma once
#define NOMINMAX
#include <Windows.h>
#include <numbers>
#include <vector>
#include "Easing.h"
#include <algorithm>
#include "MapChipField.h"
#include "HitBox.h"
#include "Calculation.h"
#include "Model.h"
#include "DebugCamera.h"

class Player;

class Enemy {

public:

	~Enemy();

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="camera">カメラ</param>
	void Initialize(const Vector3& position, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(Renderer renderer, DebugCamera debugcamera);

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// 衝突応答
	void OnCollision(const Player* player);

private:

	// ワールド変換データ
	Transform worldTransform_;
	//// モデル
	Model* model_ = nullptr;
	
	// 速度
	Vector3 velocity_ = {};
	// 経過時間
	float walkTimer_ = 0.0f;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;
	static inline const float kHeight = 1.6f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.02f;
	// 最初の角度
	static inline const float kWalkMotionAngleStart = std::numbers::pi_v<float> *2.0f;
	// 最後の角度
	static inline const float kWalkMotionAngleEnd = std::numbers::pi_v<float>;
	// アニメーションの周期になる時間[秒]
	static inline const float kWalkMotionTime = 2.0f;
};
