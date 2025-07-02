#pragma once
#include "Renderer.h"
#include "Calculation.h"
#include "Command.h"
#include "resource.h"
#include "Texture.h"
#include "LoadFile.h"
#include "Descriptor.h"

class Model
{

public:
	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor);

	/// <summary>
	/// モデル読み込み
	/// </summary>
	void Load(Command command, const std::string& filename, const std::string& textureName, const uint32_t& index);

	/// <summary>
	/// モデル描画
	/// </summary>
	void Update();

	/// <summary>
	/// モデル描画
	/// </summary>
	void Draw(Renderer renderer, const Transform& transform, const Matrix4x4& cameraMatrix);

private:

	LoadFile loadFile_;
	ModelData modelData_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSkydome;
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSkydome;
	// WVP用のリソースを作る。Matrix4x41つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	Renderer renderer_;
	TransformationMatrix* wvpData = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU;
	Resource resource;
	Texture texture;
	Microsoft::WRL::ComPtr<ID3D12Device> device_;
	Descriptor descriptor_;
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource;
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
};

