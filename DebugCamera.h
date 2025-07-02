#pragma once
#define NOMINMAX
#include "Calculation.h"
#include <dinput.h>
#include "Input.h"
#include "algorithm"
#include <DirectXMath.h>
#include "externals/imgui/imgui.h"

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
	void Update(const DIMOUSESTATE& mousestate, std::span<const BYTE> key);

	Matrix4x4 GetViewMatrix() { return viewMatrix_; }

private:
	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = {0,0,0};
	// ローカル座標
	Vector3 translation_ = {19.0f,12.3f, 0.15f};
	// ビュー行列
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	// 射影行列
	Matrix4x4 projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
	// カメラの移動速度
	Vector3 velocity_ = { 0.005f, 0.005f, 0.005f };
	// カメラ
	Matrix4x4 WorldMatrix_;
	
	Input input_;

	float distance = 50.0f;             // 注視点との距離（ズーム）
	float theta = -3.150f;                // 横回転（経度）
	float phi = 1.56f;                  // 縦回転（緯度）
	Vector3 target = {19.0f, 12.3f, 0.15f};             // 注視点のワールド座標
};

