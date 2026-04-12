#pragma once
#include "../EnemyBase/DropEnemyBase.h"

/// <summary>
/// プレイヤーの位置を追尾して落下攻撃をしてくるエネミーのクラス
/// 落下挙動は DropEnemyBase クラスで共通化し、追尾の管理をこのクラスで行う
/// </summary>
class ReactiveDropEnemy : public DropEnemyBase
{
private:
	/// <summary>
	/// 落下攻撃開始の条件 / プレイヤーの位置を追尾して開始する
	/// </summary>
	/// <returns>落下攻撃を開始するかどうか</returns>
	bool ShouldStartDrop() override;

	/// <summary>
	/// モデル名の取得 
	/// </summary>
	const char* GetModelName() const override { return "GamePlay/Enemies/tentativeenemy"; }

private:
	// 攻撃を開始するためのプレイヤーとの距離
	float triggerRangeX_ = 1.5f;
};

