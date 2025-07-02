#include "Player.h"

void Player::Initialize(Model* model, const Vector3& position)
{

	model_ = model;

	worldTransform_.scale = { 1.0f, 1.0f, 1.0f };
	worldTransform_.rotate = { 0.0f, 0.0f, 0.0f };
	worldTransform_.translate = position;
	worldTransform_.rotate.y = std::numbers::pi_v<float>;
}

Vector3 Player::CornerPositon(const Vector3& center, Corner corner) {
	Vector3 offsetTable[kNumCorner] = {
		Vector3{+kWidth / 2.0f, -kHeight / 2.0f, 0}, // 右下
		Vector3{-kWidth / 2.0f, -kHeight / 2.0f, 0}, // 左下
		Vector3{+kWidth / 2.0f, +kHeight / 2.0f, 0}, // 右上
		Vector3{-kWidth / 2.0f, +kHeight / 2.0f, 0}, // 左上
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::Move(std::span<const BYTE> key) {

	// 移動入力
	// 接地状態
	if (onGround_) {
		// 左右移動操作
		if (input.IsPress(key[DIK_RIGHT]) || input.IsPress(key[DIK_LEFT])) {

			// 左右処理
			Vector3 acceleration = {};
			if (input.IsPress(key[DIK_RIGHT])) {

				// 左移動中の右入力
				if (velocity_.x < 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x += kAcceleration;
				if (lrDirection_ != LRDirection::kRight) {
					lrDirection_ = LRDirection::kRight;
					turnFirstRotationY_ = worldTransform_.rotate.y;
					turnTimer_ = kTimeTurn;
				}
			} else if (input.IsPress(key[DIK_LEFT])) {

				// 左移動中の右入力
				if (velocity_.x > 0.0f) {
					velocity_.x *= (1.0f - kAttenuation);
				}
				acceleration.x -= kAcceleration;
				if (lrDirection_ != LRDirection::kLeft) {
					lrDirection_ = LRDirection::kLeft;
					turnFirstRotationY_ = worldTransform_.rotate.y;
					turnTimer_ = kTimeTurn;
				}
			}
			// 加速/減速
			velocity_ += acceleration;
			// 最大速度制限
			velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

		} else {

			// 非入力時は移動減速をかける
			velocity_.x *= (1.0f - kAttenuation);
		}

		// ジャンプ
		if (input.IsPress(key[DIK_UP])) {
			// ジャンプ処理
			velocity_ += Vector3(0, kJumpAcceleration, 0);
		}
		// 空中
	} else {

		// 落下速度
		velocity_ += Vector3(0, -kGravityAcceleration, 0);
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFailSpeed);
	}

	if (turnTimer_ > 0.0f) {

		// 旋回タイマーをカウントダウンする
		turnTimer_ -= 1.0f / 60.0f;

		// 旋回制御
		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = { std::numbers::pi_v<float>, std::numbers::pi_v<float> *2.0f };
		// 状況にあった角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 正規化された時間（0.0～1.0）を計算
		float t = 1.0f - (turnTimer_ / kTimeTurn);

		// イージング補間値を取得
		float easedT = easeInOutCubic(t);

		// 開始角度から終了角度へイージング補間
		float currentRotationY = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * easedT;

		// 自キャラの角度を設定する
		worldTransform_.rotate.y = currentRotationY;
	}
}

void Player::CollisionTop(CollisionMapInfo& info) {

	// 上昇あり？
	if (info.velocity.y <= 0) {
		return;
	}

	std::array<Vector3, kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPositon(worldTransform_.translate + info.velocity, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上のあたり判定を行う
	bool hit = false;
	// (自キャラの)左上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右上の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate);

		if (indexSetNow.yIndex != indexSet.yIndex) {
			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			//float newTopY = positionsNew[kRightTop].y - kHeight / 2.0f;
			info.velocity.y = std::max(0.0f, rect.bottom - worldTransform_.translate.y - (kHeight / 2.0f + kBlank));
			//info.velocity.y = std::max(0.0f, rect.bottom - newTopY + kBlank);

			// 天井に当たったことを記録する
			info.ceiling = true;
		}
	}
}

void Player::CollisionBottom(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPositon(worldTransform_.translate + info.velocity, static_cast<Corner>(i));
	}
	// 下降あり？

	if (info.velocity.y >= 0) {
		return;
	}
	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上のあたり判定を行う
	bool hit = false;
	// 左下の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + Vector3(0, -kLandingAdjust, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + Vector3(0, -kLandingAdjust, 0));
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate);

		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.y = std::min(0.0f, rect.top - worldTransform_.translate.y + (kHeight / 2.0f + kBlank));
			// 天井に当たったことを記録する
			info.landing = true;
		}
	}
}

void Player::CollisionRight(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPositon(worldTransform_.translate + info.velocity, static_cast<Corner>(i));
	}

	// 右移動あり？
	if (info.velocity.x <= 0.0f) {
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上のあたり判定を行う
	bool hit = false;
	// 右上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate);
		//// めり込み先ブロックの範囲矩形
		//
		if (indexSetNow.xIndex != indexSet.xIndex) {
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			// info.move.x = std::min(0.0f, rect.left - (worldTransform_.translate.x + (kWidth / 2.0f + kBlank)));
			info.velocity.x = std::max(0.0f, rect.left - worldTransform_.translate.x - (kWidth / 2.0f + kBlank));
			// 壁に当たったことを記録する
			// DebugText::GetInstance()->ConsolePrintf("landing");
			info.hitWall = true;
		}
	}
}

void Player::CollisionLeft(CollisionMapInfo& info) {

	std::array<Vector3, kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPositon(worldTransform_.translate + info.velocity, static_cast<Corner>(i));
	}

	// 左移動あり？
	if (info.velocity.x >= 0.0f) {
		return;
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上のあたり判定を行う
	bool hit = false;
	// 左上の判定
	MapChipField::IndexSet indexSet;
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 左下の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {

		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translate);

		if (indexSetNow.xIndex != indexSet.xIndex) {
			//  めり込みを排除する方向に移動量を設定する

			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.velocity.x = std::min(0.0f, rect.right - worldTransform_.translate.x + (kWidth / 2.0f + kBlank));
			// 壁に当たったことを記録する
			// DebugText::GetInstance()->ConsolePrintf("landing");
			info.hitWall = true;
		}
	}
}

void Player::SwitchLanding(const CollisionMapInfo& info) {
	// 自キャラが接地状態？
	if (onGround_) {

		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}

		std::array<Vector3, kNumCorner> positionsNew;

		for (uint32_t i = 0; i < positionsNew.size(); ++i) {
			positionsNew[i] = CornerPositon(worldTransform_.translate + info.velocity, static_cast<Corner>(i));
		}
		MapChipType mapChipType;
		// 真上のあたり判定を行う
		bool hit = false;
		// 左下の判定
		MapChipField::IndexSet indexSet;
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (mapChipType == MapChipType::kBlock) {
			hit = true;
		}
		// 右下の判定
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (mapChipType == MapChipType::kBlock) {
			hit = true;
		}

		// 落下開始
		if (!hit) {
			// 空中状態に切り替える
			onGround_ = false;
		}

	} else {

		// 着地フラグ
		if (info.landing) {
			// 着地状態に切り替える(落下を止める)
			onGround_ = true;
			// 着地時にX座標を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);
			// Y速度をゼロにする
			velocity_.y = 0.0f;
		}
	}
}

void Player::CollisionWall(const CollisionMapInfo& info) {

	if (info.hitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

void Player::Collision(CollisionMapInfo& info) {
	CollisionBottom(info);
	CollisionTop(info);
	CollisionLeft(info);
	CollisionRight(info);
}

void Player::CollisionMove(const CollisionMapInfo& info) {
	// 移動
	worldTransform_.translate += info.velocity;
}

void Player::CollisionUpdate(const CollisionMapInfo& info) {
	// 天井に当たった？
	if (info.ceiling) {
		velocity_.y = 0.0f;
	}
}

void Player::Update(std::span<const BYTE> key) {


	ImGui::Begin("Player");
	ImGui::DragFloat3("scale", &worldTransform_.scale.x, 0.5f);
	ImGui::DragFloat3("rotate", &worldTransform_.rotate.x, 0.5f);
	ImGui::DragFloat3("transform", &worldTransform_.translate.x, 0.5f);
	ImGui::DragFloat3("velocity", &velocity_.x, 0.1f);
	ImGui::End();

	Move(key);

	// 衝突判定を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.velocity = velocity_;

	// マップ衝突チェック
	Collision(collisionMapInfo);

	CollisionUpdate(collisionMapInfo);
	CollisionWall(collisionMapInfo);
	SwitchLanding(collisionMapInfo);
	CollisionMove(collisionMapInfo);

}

void Player::Draw(Renderer renderer, DebugCamera debugCamera) {

	model_->Draw(renderer, worldTransform_, debugCamera.GetViewMatrix());
}

Vector3 Player::GetWorldPosition() {

	// ワールド座標を入れる変数
	Vector3 worldPos;
	// ワールド行列の平行移動成分を取得(ワールド座標)
	worldPos.x = worldTransform_.translate.x;
	worldPos.y = worldTransform_.translate.y;
	worldPos.z = worldTransform_.translate.z;

	return worldPos;
}

AABB Player::GetAABB() {
	Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = { worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f };
	aabb.max = { worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f };

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	// デスフラグを立てる
	isDead = true;
}