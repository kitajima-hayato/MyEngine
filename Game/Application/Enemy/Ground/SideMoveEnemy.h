#pragma once
#include "../EnemyBase/EnemyBase.h"
class SideMoveEnemy :
    public EnemyBase
{
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;
	/// <summary>
	///  描画処理
	/// </summary>
	void Draw() override;
	/// <summary>
	/// 動作処理
	/// </summary>
	void Move() override;
	/// <summary>
	/// 攻撃処理
	/// </summary>
	void Action() override;

	/// <summary>
	/// 踏みつけられたら死亡
	/// </summary>
	void OnStomped() override;
};

