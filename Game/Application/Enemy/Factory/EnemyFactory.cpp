#include "EnemyFactory.h"
#include <Logger.h>
#include "../Ground/NormalEnemy.h"
#include "../Flying/FlyingEnemy.h"
#include "../Flying/SideMoveFlyingEnemy.h"
#include "../Ground/SideMoveEnemy.h"

std::unique_ptr<EnemyBase> EnemyFactory::CreateEnemy(const EnemySpawnParams& params)
{
	// 生成するエネミーのインスタンスを格納するためのスマートポインタ
	std::unique_ptr<EnemyBase> enemy = nullptr;

	// EnemyTypeから敵の種類を特定して生成
	switch (params.type) {
		// その場で待機するエネミー
		case EnemyType::NormalEnemy:
			enemy = std::make_unique<NormalEnemy>();
			break;
		// 飛行するエネミー / ふわふわと浮遊する挙動
		case EnemyType::FlyingEnemy:
			enemy = std::make_unique<FlyingEnemy>();
			break;
		// 飛行しながら左に移動するエネミー / ふわふわと浮遊する挙動
		case EnemyType::SideMoveFlyingEnemy:
			enemy = std::make_unique<SideMoveFlyingEnemy>();
			break;
		// 左右に移動するエネミー
		case EnemyType::SideMoveEnemy:
			enemy = std::make_unique<SideMoveEnemy>();
			break;
		default:
			Logger::Log("No EnemyType matched");
			return nullptr; // 対応するエネミーがない場合はnullptrを返す
	}
	// 生成したエネミーの初期化
	enemy->Initialize();
	enemy->SetMapQuery(params.mapQuery);
	enemy->SetTranslate(params.position);
	// 生成したエネミーを返す
	return enemy;
}
