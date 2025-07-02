#include "DebugCamera.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void DebugCamera::Initialize(const Matrix4x4& viewMatrix)
{
    
}
void DebugCamera::Update(const DIMOUSESTATE& mousestate, std::span<const BYTE> key) {
    static Vector3 rotation = {};

    // マウスで回転
    if (input_.IsPress(mousestate.rgbButtons[2]) && input_.IsRelease(key[DIK_LSHIFT])) {
        rotation.y += mousestate.lX * velocity_.x; // yaw
        rotation.x += mousestate.lY * velocity_.y; // pitch
    }

    // pitchに制限をかけない代わりに姿勢が崩れないように回転を行列で制御
    Matrix4x4 rotY = MakeRotateYMatrix(rotation.y);
    Matrix4x4 rotX = MakeRotateXMatrix(rotation.x);
    Matrix4x4 rot = Multiply(rotX, rotY); // Pitch後Yaw回転

    Vector3 forward = TransformMatrix(Vector3{ 0, 0, 1 }, rot); // Z方向に向かって回転
    Vector3 right = TransformMatrix(Vector3{ 1, 0, 0 }, rot);
    Vector3 up = Cross(forward, right);

    // パン（平行移動）
    if (input_.IsPress(mousestate.rgbButtons[2]) && input_.IsPress(key[DIK_LSHIFT])) {
        target = Add(target, Multiply(-mousestate.lX * velocity_.x, right));
        target = Add(target, Multiply(mousestate.lY * velocity_.y, up));
    }

    // ズーム
    if (mousestate.lZ != 0) {
        distance -= mousestate.lZ * velocity_.z;
        distance = std::max(distance, 0.1f);
    }

    // カメラ位置の算出
    Vector3 cameraPos = Add(target, Multiply(-distance, forward));

    // View行列の作成
    viewMatrix_ = MakeLookAtMatrix(cameraPos, target, up);
}