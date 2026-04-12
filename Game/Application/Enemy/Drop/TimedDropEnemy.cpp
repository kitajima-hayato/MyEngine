#include "TimedDropEnemy.h"

bool TimedDropEnemy::ShouldStartDrop()
{
	// 落下の開始条件を定義し管理する関数

	// 経過時間の計測
	intervalTimer_ += 1.0f / 60.0f;

	// 一定時間経過したら落下攻撃を開始
	if (intervalTimer_ >= intervalDuration_) {
		// タイマーのリセット
		intervalTimer_ = 0.0f; 
		return true;
	}
    return false;
}
