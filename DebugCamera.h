#pragma once
#include "Calculation.h"
#include <dinput.h>
#include "Input.h"

class DebugCamera
{

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const Matrix4x4& viewMatrix);

	/// <summary>
	/// 更新
	/// </summary>
	void Update(const DIMOUSESTATE& mousestate);

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }

private:
	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = {0,0,0};
	// ローカル座標
	Vector3 translation_ = {0,0,-5.0f};
	// ビュー行列
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	// 射影行列
	Matrix4x4 projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
	// カメラの移動速度
	Vector3 velocity_ = { 0.005f, 0.005f, 0.005f };
	// カメラ
	Matrix4x4 WorldMatrix_;
	
	Input input_;

};

