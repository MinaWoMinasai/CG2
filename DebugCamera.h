#pragma once
#include "Calculation.h"
#include <dinput.h>

class DebugCamera
{

public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

private:
	// X,Y,Z軸回りのローカル回転角
	Vector3 rotation_ = {0,0,0};
	// ローカル座標
	Vector3 translation_ = {0,0,-50};
	// ビュー行列
	Matrix4x4 viewMatrix_ = MakeIdentity4x4();
	// 射影行列
	Matrix4x4 projectionMatrix_ = MakeOrthographicMatrix(0.0f, 0.0f, float(kClientWidth), float(kClientHeight), 0.0f, 100.0f);
	// カメラの移動速度
	Vector3 velocity_;


};

