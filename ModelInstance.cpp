#include "ModelInstance.h"

void ModelInstance::InitializeInstanceResources(Model sharedModel, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor)
{
   
    sharedModel_ = sharedModel;
    Resource resource;
    Texture dummyTexture; // Textureインターフェースは必要（中身使わない）

    // WVPリソース作成（TransformMatrixが1つ分）
    resource.CreateWVP(dummyTexture, device, wvpResource_, wvpData_);
}

void ModelInstance::UpdateTransform(const Transform& transform, DebugCamera debugCamera)
{
    transform_ = transform;

    Matrix4x4 worldMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
    Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
    Matrix4x4 projectionMatrix = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
    Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
    wvpData_->WVP = worldViewProjectionMatrix;
    wvpData_->World = worldMatrix;

}
