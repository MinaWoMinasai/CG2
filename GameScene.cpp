#include "GameScene.h"
#include "Calculation.h"

GameScene::~GameScene()
{
	worldTransformBlocks_.clear();
	delete mapChipField_;
}

void GameScene::GeneratteBlocks()
{

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数を設定(縦設定のブロック数)
	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {

		// 1列の要素数を設定(横方向のブロック数)
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {
			
			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {

				Transform* worldTransform = new Transform();
				worldTransform->scale = { 1.0f, 1.0f, 1.0f };
				worldTransform->rotate = { 0.0f, 0.0f, 0.0f };
				worldTransform->translate = mapChipField_->GetMapChipPositionByIndex(j, i);

				worldTransformBlocks_[i][j] = worldTransform;
			} else {
				worldTransformBlocks_[i][j] = nullptr;
			}
		}
	}
}

void GameScene::Initialize(ModelData modelData, Texture texture, Microsoft::WRL::ComPtr<ID3D12Device>& device)
{
	// 頂点バッファビュー作成
	vertexBufferView = resource.CreateVBV(modelData, texture, device, vertexResource);

	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("resources/blocks.csv");

	// ブロック生成
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	blocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		blocks_[i].resize(numBlockHorizontal);

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {

				Block block{};
				block.transform.scale = { 1.0f, 1.0f, 1.0f };
				block.transform.rotate = { 0.0f, 0.0f, 0.0f };
				block.transform.translate = mapChipField_->GetMapChipPositionByIndex(j, i);

				// ブロックごとにWVPリソース作成
				resource.CreateWVP(texture, device, block.wvpResource, block.wvpData);

				blocks_[i][j] = block;
			}
		}
	}
}
void GameScene::Update(DebugCamera debugCamera)
{
	Matrix4x4 viewMatrix = debugCamera.GetViewMatrix();
	Matrix4x4 projectionMatrix = MakePerspectiveForMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);

	for (auto& row : blocks_) {
		for (auto& block : row) {
			if (!block.wvpData)
				continue;

			Matrix4x4 worldMatrix = MakeAffineMatrix(block.transform.scale, block.transform.rotate, block.transform.translate);
			Matrix4x4 wvpMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

			block.wvpData->WVP = wvpMatrix;
			block.wvpData->World = worldMatrix;
		}
	}
}

void GameScene::Draw(
	Renderer renderer,
	const D3D12_INDEX_BUFFER_VIEW* ibv,
	D3D12_GPU_VIRTUAL_ADDRESS materialCBV,
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrv,
	D3D12_GPU_VIRTUAL_ADDRESS lightCBV,
	UINT vertexCount, UINT indexCount)
{
	for (const auto& row : blocks_) {
		for (const auto& block : row) {
			if (!block.wvpResource)
				continue;

			renderer.DrawModel(
				vertexBufferView,
				ibv,
				materialCBV,
				block.wvpResource->GetGPUVirtualAddress(),  // 個別のWVP
				textureSrv,
				lightCBV,
				vertexCount,
				indexCount
			);
		}
	}
}