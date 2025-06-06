#include "DebugCamera.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void DebugCamera::Initialize()
{

}

void DebugCamera::Update()
{

	// ワールド行列を作成する
	Matrix4x4 WorldMatrix = MakeAffineMatrix({ 1.0f, 1.0f, 1.0f }, translation_, rotation_);

	//if()

}
