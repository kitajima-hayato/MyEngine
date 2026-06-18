#include "BossStageManager.h"


#include "Game/Application/Map/Map.h"
#include "Game/Application/Block/BlockType.h"
#include "Game/Collision/CollisionManager.h"
#include "Game/Application/Player/Player.h"

void BossStageManager::Initialize(Map* map, Player* player, CollisionManager* collision)
{
	// ポインタを受け取りメンバに保存
	this->map = map;
	this->player = player;
	this->collision = collision;

	// ボスのインスタンスを生成して初期化
	bossEnemy = std::make_unique<BossEnemy>();
	bossEnemy->Initialize();

	// マップをスキャンしてスポーンされたオブジェクトを生成
	ScanAndCreateSpawnedObjects();
}


void BossStageManager::Update()
{
	// ボスエネミーの更新
    bossEnemy->Update();

	// ステージ上にスポーンされたオブジェクトの更新
    for (auto& obj : spawnedObjects) {
		obj->Update();
    }

}

void BossStageManager::Draw()
{
    // ボスエネミーの描画
	bossEnemy->Draw();

	// ステージ上にスポーンされたオブジェクトの描画
    for (auto& obj : spawnedObjects) {
        obj->Draw();
    }
}

void BossStageManager::CheckCollision()
{
	// 当たり判定マネージャーにプレイヤーとボスを登録
    collision->Clear();
	collision->AddCollider(player);
    for(auto& obj : spawnedObjects) {
        if (!obj->IsExpired()) {
			collision->AddCollider(obj.get());
        }
	}
    // 当たり判定の確認
    collision->CheckAllCollisions();

    // 期限切れのオブジェクトを削除する
    spawnedObjects.erase(
        std::remove_if(spawnedObjects.begin(), spawnedObjects.end(),
            [](const auto& obj) { return obj->IsExpired(); }),
		spawnedObjects.end());

}

bool BossStageManager::IsBossDefeated() const
{
	// ボスが倒されたかどうかを返す
    return bossEnemy->IsDefeated();
}


void BossStageManager::ScanAndCreateSpawnedObjects() {

	// マップをスキャンして、スポーンされたオブジェクトを生成する
    for (uint32_t y = 0; y < map->GetHeight(); y++) {
        for (uint32_t x = 0; x < map->GetWidth(); x++) {
            if (map->GetMapChipTypeByIndex(x, y) != BlockType::BossSpawnableBlock) {
                continue;
            }
            Vector3 pos = map->GetMapChipPositionByIndex(x, y);
            auto obj = std::make_unique<BossSpawnedObject>();
            obj->Initialize(pos, bossEnemy.get());
            spawnedObjects.push_back(std::move(obj));
            // タイルはAirに置換（ブロックとして残さない）
            map->SetMapChipTypeByIndex(x, y, BlockType::Air);
        }
    }
}