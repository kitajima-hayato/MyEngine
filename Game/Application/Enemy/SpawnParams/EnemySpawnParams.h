#pragma once
#include "engine/math/MyMath.h"
#include "Game/Application/Map/IMapCollisionQuery.h"
#include "Game/Application/Enemy/Types/EnemyType.h"

/// エネミーの生成パラメーター
struct EnemySpawnParams {
	// 生成するエネミーの種類
	EnemyType type;
	// 生成位置
	Vector3 position;
	// マップ衝突クエリー
	IMapCollisionQuery* mapQuery = nullptr;
};