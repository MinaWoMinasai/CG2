#pragma once
#define NOMINMAX
#include "Calculation.h"
#include "Texture.h"
#include "Resource.h"
#include <Windows.h>
#include <numbers>
#include <vector>
#include "Easing.h"
#include <algorithm>
#include "MapChipField.h"
#include "HitBox.h"
#include <array>
#include <span>
#include "Input.h"
#include "Model.h"
#include "DebugCamera.h"

// 前方宣言
class Enemy;
class MapChipField;

class Player
{
public:

	enum Corner {
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCorner,
	};

	struct CollisionMapInfo {
		// 天井衝突フラグ
		bool ceiling = false;
		// 着地フラグ
		bool landing = false;
		// 壁接触フラグ
		bool hitWall = false;
		// 移動量
		Vector3 velocity = {};
	};

	Vector3 CornerPositon(const Vector3& center, Corner corner);

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move(std::span<const BYTE> key);

	/// <summary>
	/// マップ衝突判定上
	/// </summary>
	/// <param name="info"></param>
	void CollisionTop(CollisionMapInfo& info);

	void CollisionBottom(CollisionMapInfo& info);

	void CollisionRight(CollisionMapInfo& info);
	void CollisionLeft(CollisionMapInfo& info);

	void SwitchLanding(const CollisionMapInfo& info);

	void CollisionWall(const CollisionMapInfo& info);

	/// <summary>
	/// マップ衝突判定
	/// </summary>
	/// <param name="info"></param>
	void Collision(CollisionMapInfo& info);

	void CollisionMove(const CollisionMapInfo& info);

	void CollisionUpdate(const CollisionMapInfo& info);

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, const Vector3& position);

	void Update(std::span<const BYTE> key);

	void Draw(Renderer renderer, DebugCamera debugCamera);

	// ワールド座標を取得
	Vector3 GetWorldPosition();

	// AABBを取得
	AABB GetAABB();

	// 衝突応答
	void OnCollision(const Enemy* enemy);

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// ワールドトランスフォームを取得
	Transform& GetWorldTransform() { return worldTransform_; }

	// デスフラグのgetter
	bool IsDead() const { return isDead; }

private:

	enum class LRDirection {
		kRight,
		kLeft,
	};

	// ワールド変換データ
	Transform worldTransform_;
	
	Vector3 velocity_ = {};

	// 加速度
	static inline const float kAcceleration = 0.02f;
	// 摩擦
	static inline const float kAttenuation = 0.05f;
	// どこを向いているか
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTimeTurn = 1.0f;
	// 最大速度
	static inline const float kLimitRunSpeed = 0.5f;
	// 接地状態フラグ
	bool onGround_ = true;
	// 重力加速度(下方向)
	static inline const float kGravityAcceleration = 0.03f;
	// 最大落下速度(下方向)
	static inline const float kLimitFailSpeed = 3.0f;
	// ジャンプ初速(上方向)
	static inline const float kJumpAcceleration = 0.5f;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 1.6f;
	static inline const float kHeight = 1.6f;

	static inline const float kBlank = 0.05f;

	// 接地時の判定の調整
	static inline const float kLandingAdjust = 0.1f;

	// 着地時の速度減衰率
	static inline const float kAttenuationLanding = 0.1f;
	static inline const float kAttenuationWall = 0.1f;

	// デスフラグ
	bool isDead = false;

	Resource resource;

	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	TransformationMatrix* wvpData = nullptr;

	Input input;

	Model* model_;
};

