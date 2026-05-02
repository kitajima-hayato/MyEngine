#pragma once
#include "../EnemyBase/DropEnemyBase.h"

/// <summary>
/// 一定時間後に落下攻撃をしてくるエネミーのクラス
/// 落下挙動は DropEnemyBase クラスで共通化し、タイマーの管理をこのクラスで行う
/// </summary>
class TimedDropEnemy : public DropEnemyBase
{
private:
	/// <summary>
	/// 落下攻撃開始の条件 / 一定時間経過で開始する
	/// </summary>
	/// <returns>落下攻撃を開始するかどうか</returns>
	bool ShouldStartDrop() override;

	/// <summary>
	/// モデル名の取得 
	/// </summary>
	const char* GetModelName() const override { return "GamePlay/Enemies/dropenemy/timed"; }

private:
	// 落下開始までの計測タイマー
	float intervalTimer_ = 0.0f;
	// 落下開始までの時間
	float intervalDuration_ = 2.0f;
};

