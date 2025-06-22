#include "DebugCamera.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void DebugCamera::Initialize(const Matrix4x4& viewMatrix)
{

}

void DebugCamera::Update(const DIMOUSESTATE& mousestate)
{

	// 左クリックで回転
	if (input_.IsPress(mousestate.rgbButtons[0])) {

		rotation_.x += mousestate.lY * velocity_.y;
		rotation_.y += mousestate.lX * velocity_.x;

	}

	// 移動ベクトルを角度分だけ回転させる
	Matrix4x4 matRot = MakeRotateXMatrix(rotation_.x);
	matRot = Multiply(matRot, MakeRotateYMatrix(rotation_.y));

	// 中クリックで左右移動
	if (input_.IsPress(mousestate.rgbButtons[2])) {

		// 移動ベクトル
		Vector3 velocity = { mousestate.lX * velocity_.x, mousestate.lY * velocity_.y, 0.0f };

		// 移動ベクトルを角度分だけ回転
		Vector3 rotatedVelocity = TransformNormal(velocity, matRot);
		// 座標を加算
		translation_.x += rotatedVelocity.x;
		translation_.y += rotatedVelocity.y;
		translation_.z += rotatedVelocity.z;

	}

	// ホイール前後で前後移動
	if (mousestate.lZ != 0) {

		// 移動ベクトル
		Vector3 velocity = { 0.0f, 0.0f,  mousestate.lZ * velocity_.z };
		// 移動ベクトルを角度分だけ回転
		Vector3 rotatedVelocity = TransformNormal(velocity, matRot);
		// 座標を加算
		translation_.x += rotatedVelocity.x;
		translation_.y += rotatedVelocity.y;
		translation_.z += rotatedVelocity.z;
	}

	// ワールド行列を作成する
	WorldMatrix_ = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, rotation_, translation_);
	// ビュー行列を更新
	viewMatrix_ = Inverse(WorldMatrix_);
}