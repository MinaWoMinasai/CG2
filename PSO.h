#pragma once
#include "State.h"
#include "InputDesc.h"
#include "Root.h"

class PSO
{

public:

	void Initialize(Microsoft::WRL::ComPtr<ID3D12Device>& device, Command& command);

	void Graphics();
	void GraphicsLine();

	void Create(Microsoft::WRL::ComPtr<ID3D12Device>& device);
	void CreateLine(Microsoft::WRL::ComPtr<ID3D12Device>& device);

	void Release();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC GetGraphicsDesc() { return graphicsDesc_; }
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetGraphicsState(){ return graphicsState_; }
	State GetState() { return state_; }
	InputDesc GetInputDesc() { return inputDesc_; }
	const Root& GetRoot() { return root_; }
	ID3DBlob* GetSignatureBlob() { return root_.GetSignatureBlob(); }
	ID3DBlob* GetErrorBlob() { return root_.GetErrorBlob(); }
	IDxcBlob* GetVertexShaderBlob() { return root_.GetVertexShaderBlob(); }
	IDxcBlob* GetPixelShaderBlob() { return root_.GetPixelShaderBlob(); }
	Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() { return root_.GetSignature(); }


private:

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc_{};
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDescLine_{};
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsState_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsStateLine_ = nullptr;
	
	State state_;
	InputDesc inputDesc_;
	Root root_;

};

