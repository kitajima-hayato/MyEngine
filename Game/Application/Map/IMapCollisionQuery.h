#pragma once
#include "MyMath.h"
#include "Game/Application/Block/BlockType.h"
class IMapCollisionQuery
{
public:
	/// <summary>
	/// マップの衝突クエリーインターフェース
	/// </summary>
	virtual ~IMapCollisionQuery() = default;

	/// <summary>
	/// 座標からマップチップのインデックスを取得
	/// </summary>
	virtual bool IsWallAt(const Vector3& position) const = 0;
};

