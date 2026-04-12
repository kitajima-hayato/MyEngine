#pragma once
#include "engine/math/MyMath.h"
#include "Game/Application/Map/IMapCollisionQuery.h"
#include "Game/Application/Enemy/Types/EnemyType.h"

// 前方宣言
class Player;
/// エネミーの生成パラメーター
struct EnemySpawnParams {
	// 生成するエネミーの種類
	EnemyType type;
	// 生成位置
	Vector3 position;
	// マップ衝突クエリー
	IMapCollisionQuery* mapQuery = nullptr;
	// プレイヤーの情報
	Player* player = nullptr;
};