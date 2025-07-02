#include "DebugCamera.h"
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

void DebugCamera::Initialize(const Matrix4x4& viewMatrix)
{
    
}

void DebugCamera::Update(const DIMOUSESTATE& mousestate, std::span<const BYTE> key) {
   
    ImGui::Begin("Debug Camera");
    ImGui::DragFloat("Distance", &distance, 0.1f, 100.0f);
    ImGui::DragFloat("Theta", &theta, 0.0f, DirectX::XM_2PI);
    ImGui::DragFloat("Phi", &phi, 0.01f, DirectX::XM_PI - 0.01f);
    ImGui::DragFloat3("translate", &target.x, 0.01f);
    ImGui::End();

    if (input_.IsPress(mousestate.rgbButtons[2]) && input_.IsRelease(key[DIK_LSHIFT])) {
        theta += mousestate.lX * velocity_.x;
        phi -= mousestate.lY * velocity_.y;
        phi = std::clamp(phi, 0.01f, DirectX::XM_PI - 0.01f);
    }

    if (input_.IsPress(mousestate.rgbButtons[2]) && input_.IsPress(key[DIK_LSHIFT])) {
        Vector3 right = { cosf(theta), 0, -sinf(theta) };
        Vector3 up = { 0, 1, 0 };
        target = Add(target, Multiply((mousestate.lX * velocity_.x), right));
        target = Add(target, Multiply((mousestate.lY * velocity_.y), up));
    }

    if (mousestate.lZ != 0) {
        distance -= mousestate.lZ * velocity_.z;
        distance = std::max(distance, 0.1f);
    }

    Vector3 cameraPos = {
        distance * sinf(phi) * sinf(theta),
        distance * cosf(phi),
        distance * sinf(phi) * cosf(theta)
    };
    cameraPos = Add(cameraPos, target);

    viewMatrix_ = MakeLookAtMatrix(cameraPos, target, { 0.0f, 1.0f, 0.0f });
}