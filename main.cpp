#define DIRECTINPUT_VERSION 0x0800
#include "Engine.h"
#include "GameScene.h"

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

	struct Vertex {
		Vector3 position;
		Vector4 color;
	};

	D3D12_INPUT_ELEMENT_DESC gridInputLayout[] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,   0, 0,
	  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
	  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = gridInputLayout;
	inputLayoutDesc.NumElements = _countof(gridInputLayout);

	CompileShader compileShader;

	IDxcBlob* vertexShaderBlob = compileShader.Initialize(L"GridVS.hlsl",
		L"vs_6_0", command.GetDxcUtils(), command.GetDxcCompiler(), command.GetIncludeHandler());
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = compileShader.Initialize(L"GridPS.hlsl",
		L"ps_6_0", command.GetDxcUtils(), command.GetDxcCompiler(), command.GetIncludeHandler());
	assert(pixelShaderBlob != nullptr);

	State state;
	state.Initialize();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsDesc{};
	graphicsDesc.pRootSignature = pso.GetRootSignature().Get();// RootSignature
	graphicsDesc.InputLayout = inputLayoutDesc;// InputLayout
	graphicsDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() };// VertexShader
	graphicsDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() };// pixelShader
	graphicsDesc.BlendState = state.GetBlendDesc();// BlendState
	graphicsDesc.RasterizerState = state.GetRasterizerDesc();// RasterizerState
	// 書き込むRTVの情報
	graphicsDesc.NumRenderTargets = 1;
	graphicsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトロポジ(形状)のタイプ、三角形
	graphicsDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	// どのように画面に色を打ち込むかの設定
	graphicsDesc.SampleDesc.Count = 1;
	graphicsDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsDesc.DepthStencilState = state.GetDepthStencilDesc();
	graphicsDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsDesc,
		IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));
	std::vector<Vertex> gridLines;
	const int gridHalfSize = 50;      // -50 〜 +50
	const float spacing = 1.0f;

	for (int i = -gridHalfSize; i <= gridHalfSize; ++i) {
		Vector4 lineColor;

		// 区切り線ごとの色分け（X軸固定 = Z方向の線）
		if (i == 0) {
			lineColor = { 1.0f, 0.0f, 0.0f, 1.0f }; // X軸（赤）
		} else if (i % 10 == 0) {
			lineColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 区切り（白）
		} else {
			lineColor = { 0.5f, 0.5f, 0.5f, 1.0f }; // 通常（グレー）
		}

		float x = i * spacing;
		float zMin = -gridHalfSize * spacing;
		float zMax = gridHalfSize * spacing;

		// Z方向の線（Xを固定）
		gridLines.push_back({ { x, 0.0f, zMin }, lineColor });
		gridLines.push_back({ { x, 0.0f, zMax }, lineColor });
	}

	for (int i = -gridHalfSize; i <= gridHalfSize; ++i) {
		Vector4 lineColor;

		// 区切り線ごとの色分け（Z軸固定 = X方向の線）
		if (i == 0) {
			lineColor = { 0.0f, 0.0f, 1.0f, 1.0f }; // Z軸（青）
		} else if (i % 10 == 0) {
			lineColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 区切り（白）
		} else {
			lineColor = { 0.5f, 0.5f, 0.5f, 1.0f }; // 通常（グレー）
		}

		float z = i * spacing;
		float xMin = -gridHalfSize * spacing;
		float xMax = gridHalfSize * spacing;

		// X方向の線（Zを固定）
		gridLines.push_back({ { xMin, 0.0f, z }, lineColor });
		gridLines.push_back({ { xMax, 0.0f, z }, lineColor });
	}
	// グリッド用の頂点リソースを作成
	UINT vertexBufferSize = static_cast<UINT>(sizeof(Vertex) * gridLines.size());

	// 1. リソース作成
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraBuffer;

	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer((sizeof(Matrix4x4) + 0xFF) & ~0xFF);

	hr = device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&cameraBuffer)
	);
	assert(SUCCEEDED(hr)); // 必ず成功チェック

	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&bufferDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	);

	// 2. 転送
	void* mapped = nullptr;
	vertexBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, gridLines.data(), vertexBufferSize);
	vertexBuffer->Unmap(0, nullptr);

	// 3. ビュー作成
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vbView.SizeInBytes = vertexBufferSize;
	vbView.StrideInBytes = sizeof(Vertex);

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
	ID3D12CommandList* commandLists[] = { command.GetList().Get() };
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

	Renderer renderer;
	renderer.SetDSVHandle(descriptor.GetDsvHeap()->GetCPUDescriptorHandleForHeapStart());
	renderer.SetSwapChainResources({ view.GetSwapChainResource()[0], view.GetSwapChainResource()[1] });
	renderer.SetGraphicsPipelineState(pso.GetGraphicsState().Get());
	renderer.SetSRVHeap(descriptor.GetSrvHeap().Get());
	renderer.SetRTVHandles({ view.GetRtvHandles()[0], view.GetRtvHandles()[1] });
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

	GameScene* gameScene = new GameScene;
	gameScene->LoadModel(device, descriptor, command);
	gameScene->Initialize(modelData, texture, device, descriptor, command);

	MSG msg{};
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

			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
			Matrix4x4 projectionMatrix = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
			wvpData->WVP = worldViewProjectionMatrix;
			wvpData->World = worldMatrix;
			
			gameScene->Update(input.GetKey(), debugCamera);

			// 開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム固有の処理に置き換える
			ImGui::ShowDemoWindow();

			// ImGuiの内部コマンドを生成する
			ImGui::Render();

			Matrix4x4 view = debugCamera.GetViewMatrix();
			Matrix4x4 proj = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			Matrix4x4 vp = Multiply(view, proj);

			// CBVに書き込む
			void* mapped = nullptr;
			cameraBuffer->Map(0, nullptr, &mapped);
			memcpy(mapped, &vp, sizeof(vp));
			cameraBuffer->Unmap(0, nullptr);

			UINT backBufferIndex = swapChain.GetList()->GetCurrentBackBufferIndex();
			renderer.BeginFrame(backBufferIndex);

			ID3D12DescriptorHeap* heaps[] = { descriptor.GetSrvHeap().Get() };
			command.GetList()->SetDescriptorHeaps(1, heaps);
			command.GetList()->SetGraphicsRootSignature(pso.GetRootSignature().Get());
			command.GetList()->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
			command.GetList()->SetGraphicsRootConstantBufferView(1, cameraBuffer->GetGPUVirtualAddress());

			// グリッドの描画
			command.GetList()->IASetVertexBuffers(0, 1, &vbView);
			command.GetList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
			command.GetList()->DrawInstanced(static_cast<UINT>(gridLines.size()), 1, 0, 0);

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
				debugCamera,
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
	
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();
	
	CloseHandle(command.GetFenceEvent());

	pso.Release();

	CloseWindow(hwnd);

	CoUninitialize();

	return 0;
}