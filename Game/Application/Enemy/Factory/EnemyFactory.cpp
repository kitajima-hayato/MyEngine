#include "EnemyFactory.h"
#include <Logger.h>
#include "../Ground/SideMoveEnemy.h"
#include "../Ground/NormalEnemy.h"
#include "../Ground/PatrolEnemy.h"
#include "../Flying/FlyingEnemy.h"
#include "../Flying/SideMoveFlyingEnemy.h"
#include "../Drop/TimedDropEnemy.h"
#include "../Drop/ReactiveDropEnemy.h"


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
		// 巡回する敵
	case EnemyType::PatrolEnemy:
		enemy = std::make_unique<PatrolEnemy>();
		break;
		// 一定時間ごとに落下攻撃をするエネミー
	case EnemyType::TimedDropEnemy:
		enemy = std::make_unique<TimedDropEnemy>();
		break;
		// プレイヤーの位置を追尾して落下攻撃をするエネミー
	case EnemyType::ReactiveDropEnemy: {
		auto  reactive = std::make_unique<ReactiveDropEnemy>();
		// プレイヤーの情報をセットする / 落下攻撃の条件でプレイヤーの位置を参照するため
		reactive->SetPlayer(params.player);
		enemy = std::move(reactive);
		break;
	}
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
