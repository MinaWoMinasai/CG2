#pragma once
#include "Model.h"
#include "Calculation.h"
#include "DebugCamera.h"

class ModelInstance
{
public:
    void InitializeInstanceResources(Model sharedModel, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor);
    void UpdateTransform(const Transform& transform, DebugCamera debugCamera);
    const Transform& GetTransform() const { return transform_; }
    TransformationMatrix* GetWVPData() const { return wvpData_; }

private:
    Model sharedModel_;
    Transform transform_;
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource_;
    TransformationMatrix* wvpData_;
};

