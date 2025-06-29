#define DIRECTINPUT_VERSION 0x0800
#include "Input.h"
#include "debugCamera.h"
#include "Audio.h"
#include "Window.h"
#include "LogWrite.h"
#include "CompileShader.h"
#include "LoadFile.h"
#include "Dump.h"
#include "Texture.h"
#include "DebugLayer.h"
#include "Renderer.h"
#include "useAdapterDevice.h"
#include "Command.h"
#include "Descriptor.h"
#include "InputDesc.h"
#include "Root.h"
#include "SwapChain.h"
#include "State.h"
#include "PSO.h"
#include "Resource.h"
#include "WindowScreenSize.h"
#include "View.h"
#include "GameScene.h"

struct D3DResouceLeakCheaker {
	~D3DResouceLeakCheaker()
	{
		// リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};

class ResourceObject {
public:
	ResourceObject(Microsoft::WRL::ComPtr<ID3D12Resource>& resource)
		:resource_(resource)
	{
	}
	~ResourceObject() {
		if (resource_) {
			resource_->Release();
		}
	}
	Microsoft::WRL::ComPtr<ID3D12Resource> Get() { return resource_; };

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
};

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	D3DResouceLeakCheaker leakCheck;
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;

	// COMの初期化を行う
	CoInitializeEx(0, COINIT_MULTITHREADED);

	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);

	// 誰も捕捉しなかった場合に(Unheadled)、捕捉する関数を登録
	// mein関数始まってすぐに登録する
	Dump dump;
	SetUnhandledExceptionFilter(dump.Export);

	// ログの初期化
	LogWrite log;
	log.Initialize();
	log.Log(log.GetLogStream(), "test");

	Window window;
	window.Initialize();

	WNDCLASS wc{};
	wc = window.GetWindowClass();

	HWND hwnd;
	hwnd = window.GetHwnd();

	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));

	// アダプターとデバイスを初期化
	UseAdapterDevice useAdapterDevice;
	useAdapterDevice.Initialize(dxgiFactory);
	useAdapterDevice.Create(device);

	// キーの初期化
	Input input;
	input.Initialize(wc, hwnd);

	// デバッグレイヤー
	DebugLayer::EnableDebugLayer(device.Get());
	
	// コマンドリスト
	Command command;
	command.Initialize(device);
	
	SwapChain swapChain;
	swapChain.Initialize(kClientWidth, kClientHeight);
    swapChain.Create(dxgiFactory, command.GetQueue(), hwnd);
	
	Descriptor descriptor;
	descriptor.Initialize(device);

	Texture texture;

	View view;
	view.CreateDSV(texture, kClientWidth, kClientHeight, swapChain, descriptor, device);
	view.CreateSRV(swapChain, descriptor, device);

	PSO pso;
	pso.Initialize(device, command);
	pso.Graphics();
	pso.Create(device);

	LoadFile loadFile;

	// モデル読み込み
	ModelData modelData = loadFile.Obj("resources", "block.obj");
	ModelData modelSkydome = loadFile.Obj("resources", "skydome2.obj");

	// リソースを作成する
	Resource resource;

	// 天球リソース
	Microsoft::WRL::ComPtr <ID3D12Resource> vertexResourceSkydome;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSkydome = resource.CreateVBV(modelSkydome, texture, device, vertexResourceSkydome);

	// マテリアル用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = resource.CreateMaterial(texture, device);

	// WVP用のリソースを作る。Matrix4x41つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	// データを書き込む	
	TransformationMatrix* wvpData = nullptr;
	resource.CreateWVP(texture, device, wvpResource, wvpData);

	Transform transform{ {1.0f, 1.0f,1.0f}, {0.0f, 0.0f,0.0f}, {0.0f,0.0f,0.0f} };

	// 平行光源用のリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = resource.CreatedirectionalLight(texture, device);
	
	WindowScreenSize windowScreenSize;
	windowScreenSize.Initialize(kClientWidth, kClientHeight);

	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = texture.Load("resources/block.png");
	const DirectX::TexMetadata metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = texture.CreateResource(device, metadata);
    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = texture.UploadData(textureResource, mipImages, device, command.GetList());
	DirectX::ScratchImage mipImagesS = texture.Load("resources/uvChecker.png");
	const DirectX::TexMetadata metadataS = mipImagesS.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResourceS = texture.CreateResource(device, metadataS);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResourceS = texture.UploadData(textureResourceS, mipImagesS, device, command.GetList());
	// コマンドリストの内容を確定させるすべてのコマンドを積んでからCloseすること
	hr = command.GetList()->Close();
	assert(SUCCEEDED(hr));

	// GPUにコマンドリストの実行を行わせる
	ID3D12CommandList* commandLists[] = { command.GetList().Get()};
	command.GetQueue()->ExecuteCommandLists(1, commandLists);

	//GPUとOSに画面の交換を行うよう通知する
	swapChain.GetList()->Present(1, 0);
	// command.GetFence()の値を更新
	command.SetFenceValue(command.GetFenceValue() + 1);
	// GPUがここまでたどり着いたときに、Fenceの値を指定して値に代入するようにSignalを送る
	command.GetQueue()->Signal(command.GetFence().Get(), command.GetFenceValue());
	// Fenceの値が指定したSignal値にたどり着いているか確認する
	// GetCompletedValueの初期値はFence作成時に渡した初期値
	if (command.GetFence()->GetCompletedValue() < command.GetFenceValue()) {
		// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
		command.GetFence()->SetEventOnCompletion(command.GetFenceValue(), command.GetFenceEvent());
		// イベント待つ
		WaitForSingleObject(command.GetFenceEvent(), INFINITE);
	}
	// 次のフレーム用のコマンドリストを準備
	hr = command.GetAllocator()->Reset();
	assert(SUCCEEDED(hr));
	hr = command.GetList()->Reset(command.GetAllocator().Get(), nullptr);
	assert(SUCCEEDED(hr));

	// Imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device.Get(),
		swapChain.GetDesc().BufferCount,
		view.GetRtvDesc().Format,
		descriptor.GetSrvHeap().Get(),
		descriptor.GetSrvHeap()->GetCPUDescriptorHandleForHeapStart(),
		descriptor.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart());

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadataS.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadataS.mipLevels);

	// DescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	const uint32_t descriptorSizeRTV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	const uint32_t descriptorSizeDSV = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	descriptor.GetCPUHandle(descriptor.GetRtvHeap(), descriptorSizeRTV, 0);

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = descriptor.GetSrvHeap()->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = descriptor.GetSrvHeap()->GetGPUDescriptorHandleForHeapStart();

	// SRVを作成するDescriptorHeapの場所を決める
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPUS = descriptor.GetCPUHandle(descriptor.GetSrvHeap(), descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPUS = descriptor.GetGPUHandle(descriptor.GetSrvHeap(), descriptorSizeSRV, 2);

	// 先端はImGuiが使っているのでその次を使う
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// SRVの作成
	device->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);
	device->CreateShaderResourceView(textureResourceS.Get(), &srvDesc2, textureSrvHandleCPUS);


	DebugCamera debugCamera;

	Audio audio;
	audio.Initialize();
	audio.LoadAudio(L"bgm", L"resources/BGM.mp3");
	audio.playAudio(L"bgm");
	audio.LoadAudio(L"bgm2", L"resources/Alarm01.wav");
	audio.playAudio(L"bgm2");

	Renderer renderer;
	renderer.SetDSVHandle(descriptor.GetDsvHeap()->GetCPUDescriptorHandleForHeapStart());
	renderer.SetSwapChainResources({ view.GetSwapChainResource()[0], view.GetSwapChainResource()[1] });
	renderer.SetGraphicsPipelineState(pso.GetGraphicsState().Get());
	renderer.SetSRVHeap(descriptor.GetSrvHeap().Get());
	renderer.SetRTVHandles({view.GetRtvHandles()[0], view.GetRtvHandles()[1]});
	renderer.SetRootSignature(pso.GetRootSignature().Get());
	renderer.SetViewport(windowScreenSize.GetViewport());
	renderer.SetScissorRect(windowScreenSize.GetSissorRect());
	renderer.SetFenceValue(command.GetFenceValue());
	renderer.SetSwapChain(swapChain.GetList());

	// 例：Rendererの初期化（最初に一度だけ）
	renderer.Initialize(
		device.Get(),
		command.GetList().Get(),
		command.GetQueue().Get(),
		command.GetFence().Get(),
		command.GetFenceEvent(),
		command.GetAllocator().Get(),
		2 // バックバッファ数
	);

	MSG msg{};

	GameScene* gameScene = new GameScene;

	gameScene->Initialize(modelData, texture, device);

	// ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {

		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// ゲームの処理

			// 前のフレームのキー状態を保存
			input.BeforeFrameData();

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			// デバッグカメラ
			debugCamera.Update(input.GetMouseState(), input.GetKey());

			// 三角形の位置などを変えられるようにする
			ImGui::DragFloat3("scale", &transform.scale.x, 0.1f);
			ImGui::DragFloat3("rotate", &transform.rotate.x, 0.1f);
			ImGui::DragFloat3("translate", &transform.translate.x, 1.0f);
			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			ImGui::ShowDemoWindow();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
			Matrix4x4 projectionMatrix = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;

			gameScene->Update(debugCamera);

			UINT backBufferIndex = swapChain.GetList()->GetCurrentBackBufferIndex();
			renderer.BeginFrame(backBufferIndex);

			renderer.DrawModel(
				vertexBufferViewSkydome,                // VBV
				nullptr,                          // IBV（インデックスなしの例）
				materialResource->GetGPUVirtualAddress(),  // Material CBV
				wvpResource->GetGPUVirtualAddress(),       // WVP CBV
				textureSrvHandleGPUS, // SRV
				directionalLightResource->GetGPUVirtualAddress(),            // Light CBV
				static_cast<UINT>(modelSkydome.vertices.size())
			);

			gameScene->Draw(
				renderer,
				nullptr,                          // IBV（インデックスなしの例）
				materialResource->GetGPUVirtualAddress(),  // Material CBV
				textureSrvHandleGPU, // SRV
				directionalLightResource->GetGPUVirtualAddress(),            // Light CBV
				static_cast<UINT>(modelData.vertices.size())
			);

			// 実際のcommandListのImGuiの描画コマンドを組む
			renderer.DrawImGui();

			renderer.EndFrame();

		}
	}

	delete gameScene;

	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CoUninitialize();

	CloseHandle(command.GetFenceEvent());
	
	pso.Release();

	CloseWindow(hwnd);

	CoUninitialize();

	return 0;
}