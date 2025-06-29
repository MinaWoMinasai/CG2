#pragma once
#include <d3d12.h>
#include <span>
#include <wrl.h>
#include <cassert>
#include "LogWrite.h"
#include "CompileShader.h"
#include "Command.h"

class Root
{
public:

	void Iniitalize();

	void Create(Microsoft::WRL::ComPtr<ID3D12Device>& device);

	void Compile(Command& command);

	D3D12_ROOT_SIGNATURE_DESC GetDescriptionSignature() { return descriptionSignature_; }
	std::span<const D3D12_ROOT_PARAMETER> GetParameters() const { return Parameters_; }
	std::span<const D3D12_DESCRIPTOR_RANGE> GetDescriptorRange() const { return descriptorRange_; }
	ID3DBlob* GetSignatureBlob() { return signatureBlob_; }
	ID3DBlob* GetErrorBlob() { return errorBlob_; }
	IDxcBlob* GetVertexShaderBlob() { return vertexShaderBlob_; }
	IDxcBlob* GetPixelShaderBlob() { return pixelShaderBlob_; }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetSignature() { return signature_; }


private:
	D3D12_ROOT_SIGNATURE_DESC descriptionSignature_{};
	D3D12_ROOT_PARAMETER Parameters_[4]{};
	D3D12_DESCRIPTOR_RANGE descriptorRange_[1] = {};
	D3D12_STATIC_SAMPLER_DESC staticSamplers_[1] = {};
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> signature_ = nullptr;
	LogWrite log;



	CompileShader compileShader;

	// Shaderをコンパイルする
	IDxcBlob* vertexShaderBlob_;
	IDxcBlob* pixelShaderBlob_;

};