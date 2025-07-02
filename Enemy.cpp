#include "Enemy.h"
#include "cmath"

Enemy::~Enemy()
{
	delete model_;
}

void Enemy::Initialize(const Vector3& position, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command) {

	model_ = new Model;
	model_->Initialize(device, descriptor);
	model_->Load(command, "player2.obj", "player.png", 4);
	// ワールドトランスフォームの初期化
	worldTransform_ = { { 1.0f, 1.0f, 1.0f }, {0.0f,0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	worldTransform_.translate = position;
	worldTransform_.rotate.y = std::numbers::pi_v<float> * 2.0f;
	velocity_ = { -kWalkSpeed, 0, 0 };
	walkTimer_ = 0.0f;
	
}

void Enemy::Update() {

	// 移動
	worldTransform_.translate += velocity_;

	// タイマーを加算
	walkTimer_ += 1.0f / 60.0f;

	// 回転アニメーション
	float param = std::sin(std::numbers::pi_v<float> *2.0f * (walkTimer_ / kWalkMotionTime));
	float radian = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;

	worldTransform_.rotate.z = std::sin(radian);

}

void Enemy::Draw(Renderer renderer, DebugCamera debugcamera) {
	model_->Draw(renderer, worldTransform_, debugcamera.GetViewMatrix());
}

Vector3 Enemy::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate.x;
	worldPos.y = worldTransform_.translate.y;
	worldPos.z = worldTransform_.translate.z;

	return worldPos;
}

AABB Enemy::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Enemy::OnCollision(const Player* player) { (void)player; }
