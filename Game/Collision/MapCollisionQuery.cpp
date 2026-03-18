#include "MapCollisionQuery.h"

bool MapCollisionQuery::IsWallAt(const Vector3& position) const
{
	// マップが無い場合は衝突無し
	if (!map_) {
		return false;
	}

	// 座標からマップチップのインデックスを取得
	IndexSet indexSet = map_->GetMapChipIndexSetByPosition(position);
	BlockType blockType = map_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	// ブロックタイプがAir以外なら壁とみなす
	return blockType != BlockType::Air;

}
