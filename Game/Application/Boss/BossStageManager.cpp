#include "BossStageManager.h"


#include "Game/Application/Map/Map.h"
#include "Game/Application/Block/BlockType.h"

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


void BossStageManager::ScanAndCreateSpawnedObjects() {
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