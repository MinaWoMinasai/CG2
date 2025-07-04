#pragma once
#include "Renderer.h"
#include <vector>
#include "Struct.h"
#include "MapChipField.h"
#include <optional>
#include "DebugCamera.h"
#include "Resource.h"
#include "Texture.h"
#include "Model.h"
#include "Player.h"
#include "Descriptor.h"
#include "Enemy.h"
#include "DeathParticle.h"

class GameScene
{

public:

    /// <summary>
    /// デストラクタ
    /// </summary>
    ~GameScene();

    /// <summary>
    /// ブロック生成
    /// </summary>
    void GeneratteBlocks();

    void LoadModel(Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command);

    // そうあたり判定を行う
    void CheakAllCollisions();

    /// <summary>
    /// 初期化
    /// </summary>
    void Initialize(ModelData modelData, Texture texture, Microsoft::WRL::ComPtr<ID3D12Device>& device, Descriptor descriptor, Command command);

    // 頂点初期化

    /// <summary>
    /// 更新
    /// </summary>
    void Update(std::span<const BYTE> key, DebugCamera debugCamera);

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="renderer"></param>
	/// <param name="vbv"></param>
	/// <param name="ibv"></param>
	/// <param name="materialCBV"></param>
	/// <param name="wvpCBV"></param>
	/// <param name="textureSrv"></param>
	/// <param name="lightCBV"></param>
	/// <param name="vertexCount"></param>
	/// <param name="indexCount"></param>
	void Draw(
        DebugCamera debugCamera,
        Renderer renderer, 
        const D3D12_INDEX_BUFFER_VIEW* ibv,
        D3D12_GPU_VIRTUAL_ADDRESS materialCBV,
        D3D12_GPU_DESCRIPTOR_HANDLE textureSrv,
        D3D12_GPU_VIRTUAL_ADDRESS lightCBV,
        UINT vertexCount, UINT indexCount = 0
    );

private:
    // ブロック用のワールドトランスフォーム
    std::vector<std::vector<Transform*>> worldTransformBlocks_;

    std::vector<std::vector<Block>> blocks_;

    // マップチップフィールド
    MapChipField* mapChipField_;

    Microsoft::WRL::ComPtr <ID3D12Resource> vertexResource;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
    TransformationMatrix* wvpData = nullptr;

    // インスタンス
    Player* player;
    const int32_t kEnemyCount = 3;
    std::list<Enemy*> enemies_;
    DeathParticles* deathParticles_ = nullptr;

    // リソース
    Resource resource;

    // モデル
    Model* playerModel;
    Model* enemyModel;

};

