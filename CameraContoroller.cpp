#include "CameraContoroller.h"
#include "Player.h"

void CameraContoroller::Initialize(const Transform& camera) {

	// 引数の内容をメンバ変数に記録
	camera_ = camera;

}

void CameraContoroller::Reset() {

	// 追従対象のワールドトランスフォームを参照
	const Transform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットからカメラの座標を計算
	camera_.translate = targetWorldTransform.translate + targetOffset_;

}

void CameraContoroller::Update() {

	// 追従対象のワールドトランスフォームを参照
	const Transform& targetWorldTrnasform = target_->GetWorldTransform();
	//追従対象とオフセットからカメラの目標座標を計算
	targetPos_ = targetWorldTrnasform.translate + targetOffset_;

	// 座標補間によりゆったり追従
	camera_.translate = Lerp(camera_.translate, targetPos_, kInterpolationRate);

	// 移動範囲制限
	//camera_->translation_.x = std::clamp(camera_->translation_.x, movebleArea_.left, movebleArea_.right);
	//camera_->translation_.y = std::clamp(camera_->translation_.y, movebleArea_.bottom, movebleArea_.top);

	camera_.translate.x = std::max(camera_.translate.x, movebleArea_.left + kMovebleArea.left);
	camera_.translate.x = std::min(camera_.translate.x, movebleArea_.right + kMovebleArea.right);
	camera_.translate.y = std::max(camera_.translate.y, movebleArea_.bottom + kMovebleArea.bottom);
	camera_.translate.y = std::min(camera_.translate.y, movebleArea_.top + kMovebleArea.top);

	// 行列を更新する

	//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	//Matrix4x4 viewMatrix = ;
	//Matrix4x4 projectionMatrix = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
	//Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	//wvpData->WVP = worldViewProjectionMatrix;
	//wvpData->World = worldMatrix;

	//camera_.UpdateMatrix();
}
