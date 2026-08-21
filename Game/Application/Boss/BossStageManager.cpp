#include "BossStageManager.h"


#include "Game/Application/Map/Map.h"
#include "Game/Application/Block/BlockType.h"
#include "Game/Collision/CollisionManager.h"
#include "Game/Application/Player/Player.h"

#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif

void BossStageManager::Initialize(Map* map, Player* player, CollisionManager* collision)
{
	// ポインタを受け取りメンバに保存
	this->map = map;
	this->player = player;
	this->collision = collision;

	// ボスのインスタンスを生成して初期化
	bossEnemy = std::make_unique<BossEnemy>();
	bossEnemy->Initialize();

    bossHPBar = std::make_unique<BossHPBar>();
    bossHPBar->Initialize();

	// マップをスキャンしてスポーンされたオブジェクトを生成
	ScanAndCreateSpawnedObjects();
}


void BossStageManager::Update()
{
    // 狙い撃ち用にプレイヤーの現在位置をボスへ渡す
    bossEnemy->SetTargetPosition(player->GetTranslate());
    // ボスエネミーの更新
    bossEnemy->Update();
	// HPバーの更新
    bossHPBar->Update(bossEnemy->GetHealth());


    for (const auto& req : bossEnemy->ConsumeShotRequests()) {
        auto proj = std::make_unique<BossProjectile>();
        proj->Initialize(req.origin, req.velocity);
        bossProjectiles.push_back(std::move(proj));
    }

	// ボスの攻撃プロジェクトタイルの更新
        for (auto& proj : bossProjectiles) {
        proj->Update();
        if (proj->IsExpired()) {
            // 着弾点を遊技面(z=0)にスナップしてからオブジェクトを生成する
            Vector3 landPos = proj->GetPosition();
            landPos.z = 0.0f;
            auto obj = std::make_unique<BossSpawnedObject>();
            obj->Initialize(landPos, bossEnemy.get());
            spawnedObjects.push_back(std::move(obj));
        }
    }
    // 期限切れ Projectile 削除
    bossProjectiles.erase(
        std::remove_if(bossProjectiles.begin(), bossProjectiles.end(),
            [](const auto& p) { return p->IsExpired(); }),
        bossProjectiles.end());

    for (auto& obj : spawnedObjects) {
        obj->Update();
    }

}

void BossStageManager::Draw()
{
    // ボスエネミーの描画
	bossEnemy->Draw();

	// ボスの攻撃プロジェクトタイルの描画
	for (auto& proj : bossProjectiles) {
        proj->Draw();
    }

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
    // 突進・叩きつけで当たるようにボス本体も登録する
    collision->AddCollider(bossEnemy.get());
    for (auto& obj : spawnedObjects) {
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


void BossStageManager::DrawHUD()
{
	// HPバーの描画
    bossHPBar->Draw();
}

#ifdef USE_IMGUI
void BossStageManager::DrawImgui()
{
    if (!bossEnemy) return;

    ImGui::Separator();
    ImGui::Text("=== Boss Battle ===");

    uint32_t hp = bossEnemy->GetHealth();
    ImGui::Text("HP: %u / 9   Phase: %d", hp, bossEnemy->GetPhase());

    float timer = bossEnemy->GetShotTimer();
    float interval = bossEnemy->GetShotInterval();
    ImGui::Text("Shot Timer: %.1f / %.1f s", timer, interval);
    ImGui::ProgressBar(timer / interval, ImVec2(-1.0f, 0.0f), "");

    ImGui::Text("Projectiles: %d", static_cast<int>(bossProjectiles.size()));

    int idle = 0, launched = 0;
    for (auto& obj : spawnedObjects) {
        auto s = obj->GetState();
        if (s == BossSpawnedObject::State::kIdel)     idle++;
        else if (s == BossSpawnedObject::State::kLaunched) launched++;
    }
    ImGui::Text("SpawnedObjs: %d  (idle=%d  launched=%d)",
        static_cast<int>(spawnedObjects.size()), idle, launched);
}
#endif


void BossStageManager::ScanAndCreateSpawnedObjects() {

	// マップをスキャンして、スポーンされたオブジェクトを生成する
    for (uint32_t y = 0; y < map->GetHeight(); y++) {
        for (uint32_t x = 0; x < map->GetWidth(); x++) {
            if (map->GetMapChipTypeByIndex(x, y) != BlockType::BossSpawnableBlock) {
                continue;
            }
            Vector3 pos = map->GetMapChipPositionByIndex(x, y);
            // ブロック生成時と同じ座標補正を適用する（Map::CreateBlocks と揃える）
            pos.x += map->GetBlockOffset();
            pos.y -= map->GetBlockOffset();
            pos.z = 0.0f;
            auto obj = std::make_unique<BossSpawnedObject>();
            obj->Initialize(pos, bossEnemy.get());
        }
    }
}