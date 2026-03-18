#pragma once
#include "Game/Application/Map/IMapCollisionQuery.h"
#include "Game/Application/Map/Map.h"


class MapCollisionQuery :
    public IMapCollisionQuery
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	MapCollisionQuery() = default;
	/// <summary>
	/// マップ衝突クエリー
	/// </summary>
	/// <param name="map"></param>
	explicit MapCollisionQuery(Map* map) : map_(map) {}

	/// <summary>
	/// 座標からマップチップのインデックスを取得
	/// </summary>
	/// <param name="position">調べる座標</param>
	/// <returns>ブロックがあるか</returns>
	bool IsWallAt(const Vector3& position)const override;

	/// <summary>
	/// マップの設定
	/// </summary>
	/// <param name="map">マップのポインタ</param>
	void SetMap(Map* map) { map_ = map; }

private:
	Map* map_ = nullptr;
};

