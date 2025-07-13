#include "GameScene.h"
#include "Calculation.h"

GameScene::~GameScene()
{
	worldTransformBlocks_.clear();
	delete mapChipField_;
	delete playerModel;
	delete player;
	delete enemyModel;
	for (Enemy*& enemy : enemies_) {
		delete enemy;
	}
	delete deathParticles_;
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

void GameScene::LoadModel(Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command)
{
	// モデルの初期化と読み込み
	playerModel = new Model;
	playerModel->Initialize(device, descriptor);
	playerModel->Load(command, "player2.obj", "player.png", 3);
}

void GameScene::CheakAllCollisions()
{

	// 自キャラと敵キャラのあたり判定

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player->GetAABB();

	// 自キャラと敵弾すべてのあたり判定
	for (Enemy* enemy : enemies_) {
		// 敵弾の座標
		aabb2 = enemy->GetAABB();

		// AABB同士の当たり判定
		if (IsCollision(aabb1, aabb2)) {

			if (!player->IsDead()) {
				// 自キャラの座標を取得
				const Vector3& deathParticlesPosition = player->GetWorldPosition();

				// 自キャラの位置にデスパーティクルを生成
				deathParticles_->SetPos(deathParticlesPosition);
			}
			// 自キャラの衝突時コールバックを呼び出す
			player->OnCollision(enemy);
			// 敵キャラの衝突時コールバックを呼び出す
			enemy->OnCollision(player);

		}
	}
}

void GameScene::Initialize(ModelData modelData, Texture texture, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command)
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

	player = new Player;
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(3, 12);
	player->Initialize(playerModel, { playerPosition });
	player->SetMapChipField(mapChipField_);

	// 敵の生成と初期化
	for (int32_t i = 0; i < kEnemyCount; i++) {
		Enemy* newEnemy = new Enemy();
		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(20, 13 + i * 2);
		newEnemy->Initialize(enemyPosition, device, descriptor, command);
		enemies_.push_back(newEnemy);
	}

	deathParticles_ = new DeathParticles;
	deathParticles_->Initialize(playerPosition, device, descriptor, command);

}

void GameScene::Update(std::span<const BYTE> key, DebugCamera debugCamera)
{

	// プレイヤーの更新
	if (player->IsDead()) {
		// デスパーティクルの更新
		deathParticles_->Update();
	} else {
		player->Update(key);
	}

	// 敵の更新
	for (Enemy*& enemy : enemies_) {
		enemy->Update();
	}

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
	// あたり判定を行う
	CheakAllCollisions();

}

void GameScene::Draw(
	DebugCamera debugCamera,
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

	// 敵の描画
	for (Enemy*& enemy : enemies_) {
		enemy->Draw(renderer, debugCamera);
	}

	if (player->IsDead()) {
		// デスパーティクルを描画
		deathParticles_->Draw(renderer, debugCamera);
	} else {
		// プレイヤーの描画
		player->Draw(renderer, debugCamera);
	}

}