#pragma once
#include "Game/Application/Enemy/EnemyBase/EnemyBase.h"
#include "engine/math/MyMath.h"
#include "BossAttackState.h"
#include <vector>
#include <memory>
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

public:
	
	// 弾の発射リクエスト
	struct ShotRequest {
		Vector3 origin;
		Vector3 velocity;
	};

	/// <summary>
	/// 弾の発射リクエストを行う
	/// </summary>
	/// <param name="origin"></param>
	/// <param name="velocity"></param>
	void RequestShot(const Vector3& origin, const Vector3& velocity);

	/// <summary>
	/// 弾の発射リクエストを取得する
	/// </summary>
	/// <returns></returns>
	std::vector<ShotRequest> ConsumeShotRequests();

private:
	
	/// <summary>
	/// ボスの攻撃を生成する関数(ボスのHP状況に応じてそのフェーズの攻撃をランダムに１つ選択)
	/// </summary>
	/// <returns></returns>
	std::unique_ptr<BossAttackState>CreateAttack();

	// 実行中の攻撃ステート
	std::unique_ptr<BossAttackState> currentAttack_;	

	// 発射リクエストのキュー
	std::vector<ShotRequest> shotRequests_;


public:
	/// <summary>
	/// ボスの位置を取得するための関数
	/// </summary>
	Vector3 GetPosition() const { return stats.transform.translate; }

	// GetAABB() の下あたりに追記
	uint32_t GetHealth()       const { return stats.health; }
	int      GetPhase()        const { return static_cast<int>(phase_) + 1; } // 1-3
	float    GetShotTimer()    const { return shotTimer_; }
	float    GetShotInterval() const { return shotInterval_; }

	// 狙い撃ち用のターゲット座標（プレイヤー位置）を設定/取得
	void SetTargetPosition(const Vector3& pos) { targetPos_ = pos; }
	Vector3 GetTargetPosition() const { return targetPos_; }

	/// <summary>
	/// ステージ上の (landX, landY) に着弾するよう弾速を逆算して発射する
	/// </summary>
	void RequestShotToLand(float landX, float landY);

public: // 定数
	// 着弾させる高さの目安（ステージに合わせて調整する）
	static constexpr float kLandYLow = 2.0f;
	static constexpr float kLandYMid = 5.0f;
	static constexpr float kLandYHigh = 8.0f;

	// 弾速の上限（避けられる余地を残すための制限）
	static constexpr float kMaxShotVX_ = 0.25f;
	static constexpr float kMaxShotVY_ = 0.25f;

	// 突進で到達する手前側のZ座標（プレイヤーの居る面を通り越す）
	static constexpr float kRushEndZ = -1.0f;
	// 叩きつけで到達する地面のY座標
	static constexpr float kSlamGroundY = 1.5f;

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

	// 狙い撃ちのターゲット座標
	Vector3 targetPos_ = { 0.0f, 0.0f, 0.0f };

	// 左右移動のパラメータ
	float moveTimer_ = 0.0f;
	// 移動の中心Xの初期値
	static constexpr float kHomeX_ = 20.0f;
	// 移動の中心X（プレイヤーに追従して動く）
	float homeX_ = kHomeX_;
	// 移動の中心Y
	static constexpr float kHomeY_ = 5.0f;
	// 左右の振れ幅
	static constexpr float kMoveRangeX_ = 4.0f;
	// 上下の揺れ幅
	static constexpr float kBobRangeY_ = 0.5f;
	// 揺れの速さ
	static constexpr float kMoveSpeed_ = 1.2f;
	// プレイヤーXへの追従の速さ
	static constexpr float kFollowRate_ = 0.03f;

	

};

