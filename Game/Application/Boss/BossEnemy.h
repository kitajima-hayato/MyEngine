#pragma once
#include "Game/Application/Enemy/EnemyBase/EnemyBase.h"
#include "engine/math/MyMath.h"
#include <cstdint>

/// <summary>
/// ボスエネミークラス
/// 敵の基底クラスを継承してHP管理や当たり判定を実装する
/// </summary>
class BossEnemy : public EnemyBase
{
public:
	/// <summary>
	/// 継承した初期化,更新,描画,動作,攻撃,踏まれたときの処理をオーバーライドする
	/// </summary>
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Move() override;
	void Action() override;
	void OnStomped() override;


	/// <summary>
	/// ダメージを受ける
	/// </summary>
	/// <param name="damage">受けるダメージ数</param>
	void TakeDamage(int32_t damage);

	/// <summary>
	/// HPが0になっているか
	/// </summary>
	/// <returns>HPが0ならtrue、それ以外はfalse</returns>
	bool IsDefeated() const { return stats.health == 0; }

	// BossProjectileの発射フラグ
	bool HasPendingShot()const { return pendingShot_; }

	/// <summary>
	/// 発射フラグを消費して、発射元の座標を取得する関数
	/// </summary>
	/// <returns>発射元の座標</returns>
	Vector3 ConsumeShotOrigin();


public:
	/// <summary>
	/// ボスの位置を取得するための関数
	/// </summary>
	Vector3 GetPosition() const { return stats.transform.translate; }

private:

	/// <summary>
	/// 更新フェーズ
	/// </summary>
	void UpdatePhase();

private:
	// ボスの行動フェーズ
	enum class Phase {
		kPhase1,kPhase2,kPhase3,
	};
	Phase phase_ = Phase::kPhase1;
	const uint32_t kMaxHealth_ = 9;

	float shotTimer_ = 0.0f;
	float shotInterval_ = 4.0f;

	bool pendingShot_ = false;

	Vector3 pendingShotOrigin_;

};

