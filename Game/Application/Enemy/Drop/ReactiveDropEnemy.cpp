#include "ReactiveDropEnemy.h"
#include "Game/Application/Player/Player.h"

bool ReactiveDropEnemy::ShouldStartDrop()
{
	// プレイヤーの位置を追尾して落下攻撃を開始する条件を定義 / 管理する関数

    // プレイヤーの情報が無い場合は落下攻撃をしない
    if (!player_) {
        return false;
    }
	// プレイヤーとの距離を計算して、一定距離以内なら落下攻撃を開始
	float dx = std::abs(player_->GetTranslate().x - stats.transform.translate.x);
	return dx <= triggerRangeX_;
}
