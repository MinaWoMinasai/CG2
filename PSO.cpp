#include "PSO.h"

void PSO::Initialize(Microsoft::WRL::ComPtr<ID3D12Device>& device, Command& command)
{
	root_.Iniitalize();
	root_.Create(device);
	root_.Compile(command);

	inputDesc_.Initialize();

	state_.Initialize();
}

void PSO::Graphics()
{

	graphicsDesc_.pRootSignature = root_.GetSignature().Get();// RootSignature
	graphicsDesc_.InputLayout = inputDesc_.GetLayout();// InputLayout
	graphicsDesc_.VS = { root_.GetVertexShaderBlob()->GetBufferPointer(),
	root_.GetVertexShaderBlob()->GetBufferSize() };// VertexShader
	graphicsDesc_.PS = { root_.GetPixelShaderBlob()->GetBufferPointer(),
	root_.GetPixelShaderBlob()->GetBufferSize() };// pixelShader
	graphicsDesc_.BlendState = state_.GetBlendDesc();// BlendState
	graphicsDesc_.RasterizerState = state_.GetRasterizerDesc();// RasterizerState
	// 書き込むRTVの情報
	graphicsDesc_.NumRenderTargets = 1;
	graphicsDesc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトロポジ(形状)のタイプ、三角形
	graphicsDesc_.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定
	graphicsDesc_.SampleDesc.Count = 1;
	graphicsDesc_.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsDesc_.DepthStencilState = state_.GetDepthStencilDesc();
	graphicsDesc_.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

}

void PSO::Create(Microsoft::WRL::ComPtr<ID3D12Device>& device)
{

	// 実際に生成
	HRESULT hr = device->CreateGraphicsPipelineState(&graphicsDesc_,
		IID_PPV_ARGS(&graphicsState_));
	assert(SUCCEEDED(hr));

}

void PSO::Release()
{

	root_.GetSignatureBlob()->Release();
	if (root_.GetErrorBlob()) {
		root_.GetErrorBlob()->Release();
	}
	root_.GetPixelShaderBlob()->Release();
	root_.GetVertexShaderBlob()->Release();

}
