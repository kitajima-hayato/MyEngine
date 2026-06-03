#pragma once
#include "engine/math/MyMath.h"
#include <memory>
#include "engine/3d/Object3D.h"
#include "Game/Application/Map/Map.h"
#include "Game/Collision/Collider.h"
#include "Game/Particle/ParticleSystem.h"
/// <summary>
/// プレイヤークラス
/// </summary>



/// <summary>
/// 衝突情報
/// </summary>
struct CollisionMapInfo {
	// 天井衝突
	bool celling = false;
	// 床衝突
	bool landing = false;
	// 壁衝突
	bool hitWall = false;
	// 移動量
	Vector3 move;
};

/// <summary>
/// コーナー
/// </summary>
enum Corner {
	// 右下
	kRightBottom,
	// 左下
	kLeftBottom,
	// 右上
	kRightTop,
	// 左上
	kLeftTop,
	// 要素数
	kNumCorners
};

/// <summary>
/// 衝突タイプ
/// </summary>
enum class CollisionType {
	kTop,
	kBottom,
	kLeft,
	kRight
};

/// プレイヤーのステータス　
struct PlayerStatus {
	//速度パラメーター
	// 加速度
	float kAcceleration = 0.05f;
	// 速度減衰率
	float kAttenuation = 0.2f;
	// 最大速度
	float kMaxSpeed = 0.1f;

	//ダッシュパラメーター
	// ダッシュ速度倍率
	float kDashSpeedScale = 2.0f;
	// ２回押し判定時間
	uint32_t kDashDoubleTapFrame = 15;

	//ジャンプパラメータ
	// 重力加速度
	float kGravity = 0.04f;
	// 最大落下速度
	float kMaxFallSpeed = 0.4f;
	// ジャンプ初速度
	float kJumpPower = 0.5f;
	// 踏みつけ時のジャンプ力
	float kStompJumpPower = 0.3f;

	// チャージ攻撃の速度倍率
	uint32_t kMinChargeFrame = 10;
	uint32_t kMaxChargeFrame = 45;

	// スペシャルダッシュのパラメーター
	float kSpecialDashSpeed = 0.45f;
	float kSpecialDashMinSpeed = 0.25f;
	uint32_t kSpecialDashFrame = 12;

	float kHighJumpPower = 0.65f;
	float kMaxHighJumpPower = 0.85f;

	//当たり判定パラメータ
	// 幅
	float kWidth = 0.8f;
	// 高さ
	float kHeight = 0.8f;
	// 当たり判定の余裕
	float kBlank = 0.0f;
	// 微小値
	float kEpsilon = 0.05f;

	// 攻撃パラメーター
	uint32_t kAttackFrame = 10;
	float kAttackRange = 0.75f;
	float kAttackHeight = 0.55f;
	float kAttackDepth = 0.5f;

	// チャージ攻撃時にどれだけ攻撃範囲が広がるか
	float kAttackChargeRangeBonus = 0.45f;

	//減衰パラメーター
	// 着地時の減衰率
	float kAttenuationLanding = 0.1f;
	// 壁に衝突したときの減衰率
	float kAttenuationWall = 1.0f;

	// jumpブロックに触れたときのジャンプ力
	float kJumpBlockPower = 0.7f;

	//振り向きパラメーター
	// 角度の補間タイム
	float kTurnTime = 0.1f;

	// プレイヤーの体力
	uint32_t kHealth = 3;

	// チャージ攻撃を使用できる停止判定の速度しきい値
	float kAttackStopVelocityThreshold = 0.03f;

};


class Player :public Collider
{
private:
	struct ButtonInputState {
		bool hold = false;
		bool trigger = false;
		bool release = false;
	};

	// プレイヤーの入力状態をまとめる構造体
	struct PlayerInputState {
		// 左右の入力状態
		bool moveRight = false;
		bool moveLeft = false;
		// ダッシュの入力状態
		bool triggerRight = false;
		bool triggerLeft = false;
		// ジャンプの入力状態
		bool jump = false;

		ButtonInputState actionA;
		ButtonInputState actionB;
		ButtonInputState actionX;
		ButtonInputState actionY;
	};

	struct ChargeButtonState {
		uint32_t frameCount = 0;
		bool charging = false;
	};

public:

	/// <summary>
	/// 向き
	/// </summary>
	enum class Direction {
		kRight,
		kLeft
	};


	// コライダーインターフェイス
	Collider::Type GetType() const override;
	AABB GetAABB() const override;
	void OnCollision(Collider* other) override;

	// 攻撃判定を持っているか
	bool HasAttackAABB() const override { return isAttack_; }

	// 攻撃判定のAABBを取得
	AABB GetAttackAABB() const override;

	// 攻撃判定が当たったときの処理
	bool OnAttackCollision(Collider* other) override;



public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position">初期配置</param>
	void Initialize(Vector3 position);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();


	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// プレイヤーの位置をリスポーン地点に移動させる
	/// </summary>
	/// <param name="pos"></param>
	void Respawn(const Vector3& pos);
	/// <summary>
	/// 死亡演出の開始
	/// </summary>
	/// <param name="cameraPos">演出位置のカメラ座標</param>
	void BeginDeathDemo(const Vector3& cameraPos);

	/// <summary>
	/// 死亡演出中かどうか
	/// </summary>
	/// <returns>死亡演出中かどうか</returns>
	bool IsInDeathDemo() const;

	/// <summary>
	/// 死亡演出が終わったかどうか
	/// </summary>
	/// <returns>死亡演出が終わったかどうか</returns>
	bool IsDeathDemoFinished() const;

	/// <summary>
	/// 落下による死亡を更新
	/// </summary>
	void UpdateDeathDemo();

private:
	/// <summary>
	/// プレイヤーの挙動更新統括 / 判定 / 移動 / その他
	/// </summary>
	void UpdateBehavior();

	/// <summary>
	/// 移動処理
	/// </summary>
	void Move();

	/// <summary>
	/// ジャンプ処理
	/// </summary>
	void Jump();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// ジャンプブロックに接しているか
	/// </summary>
	bool IsOnJumpBlock();

	/// <summary>
	/// マップ衝突チェック
	/// </summary>
	void MapCollision(CollisionMapInfo& collisionInfo);

	/// <summary>
	/// 天井衝突チェック
	///　天井に衝突したら移動量を調整
	///　</summary>
	void CellingCollisionMove(CollisionMapInfo& collisionInfo);

	/// <summary>
	/// 床衝突チェック
	/// 床に衝突したら移動量を調整
	/// </summary>
	void LandingCollisionMove(CollisionMapInfo& collisionInfo);

	/// <summary>
	/// 壁衝突チェック
	/// </summary>
	/// <param name="collisionInfo"></param>
	void WallCollisionMove(CollisionMapInfo& collisionInfo);

	/// <summary>
	/// 衝突マップ情報取得
	/// </summary>
	/// <param name="info"></param>
	void PlayerCollisionMove(const CollisionMapInfo& info);


	/// <summary>
	/// マップとの衝突判定を指定方向に行う共通処理
	/// 移動後の当たり判定ポイントをチェックし、移動条件を満たす場合に移動量を更新する
	/// </summary>
	/// <param name="collisionInfo">衝突判定に使用する情報</param>
	/// <param name="type">判定方向</param>
	/// <param name="checkCorners">判定を取る２つのコーナー</param>
	/// <param name="offset">判定点に加算するオフセット</param>
	/// <param name="moveCondition">方向の判定を行う条件 falseの場合判定をスキップ</param>
	void CollisionMapInfoDirection(
		CollisionMapInfo& collisionInfo,
		CollisionType type,
		const std::array<Corner, 2>& checkCorners,
		const Vector3& offset,
		std::function<bool(const CollisionMapInfo&)>moveCondition);

	/// <summary>
	/// コーナー位置取得
	/// </summary>
	Vector3 CornerPosition(const Vector3& center, Corner corner);

	/// <summary>
	/// 当たり判定ポイントチェック
	/// </summary>
	/// <param name="posList"></param>
	/// <param name="type"></param>
	/// <param name="collisionInfo"></param>
	bool CheckCollisionPoints(const std::array<Vector3, 2>& posList, CollisionType type, CollisionMapInfo& collisionInfo);


	/// <summary>
	/// 当たり判定ブロックテーブル
	/// </summary>
	/// <param name="type">指定のブロックは判定を取るか</param>
	/// <returns>当たっているかいないか</returns>
	bool IsHitBlockTable(BlockType type);

	/// <summary>
	/// ゴールブロック当たり判定テーブル
	/// </summary>
	/// <param name="type">ゴール判定のあるブロック</param>
	/// <returns>ゴール判定のあるブロックかどうか</returns>
	bool IsHitGoalBlockTable(BlockType type);

	/// <summary>
	/// 壊せるブロック当たり判定テーブル
	/// </summary>
	/// <param name="type">プレイヤーにダメージを与えるブロック</param>
	/// <returns>ダメージの判定があるブロックか</returns>
	bool IsHitBlockBreakableTable(BlockType type);

	/// <summary>
	/// ダメージブロック当たり判定テーブル
	/// </summary>
	bool IsHitBlockDamageTable(BlockType type);

	/// <summary>
	/// デバッグ用プレイヤー死亡時リセット
	/// </summary>
	void DebugPlayerReset();

	/// <summary>
	/// エネミーに当たった場合の衝突処理
	/// </summary>
	void TakeDamage();

	/// <summary>
	///	点滅更新処理
	/// </summary>
	void FlashingUpdate();

	/// <summary>
	/// 向きを変更したときにプレイヤーのモデルを回転させる
	/// </summary>
	void PlayerTurn();

	/// <summary>
	/// ダッシュ中に発生するエフェクトの更新
	/// </summary>
	void UpdateDashEffect();

	/// <summary>
	/// 通常移動のエフェクトの更新
	/// </summary>
	void UpdateMoveEffect();

	/// <summary>
	///	エネミーを踏みつけたときの処理
	/// </summary>
	/// <param name="enemy"></param>
	void StompEnemy(Collider* enemy);

	/// <summary>
	/// エネミーに攻撃が当たったときの処理
	/// </summary>
	void DefeatEnemy(Collider* enemy,bool bouncePlayer);

	/// <summary>
	/// 敵の死亡位置にパーティクルの発生
	/// </summary>
	void EmitEnemyDefeatParticle(const Vector3& position);

	/// <summary>
	/// 攻撃で敵を倒す処理
	/// </summary>
	/// <param name="enemy"></param>
	void AttackEnemy(Collider* enemy);

	/// <summary>
	/// ダメージブロックに触れているかの判定
	/// </summary>
	/// <returns></returns>
	bool IsTouchingDamageBlock();

	/// <summary>
	/// ダメージブロックに触れているかの追跡とダメージ処理
	/// </summary>
	/// <returns></returns>
	bool IsTouchingHazardSpike()const;

	/// <summary>
	/// ジャンプ開始のパーティクルの発生
	/// </summary>
	void EmitJumpParticle();

	/// <summary>
	/// 着地時のパーティクルの発生
	/// </summary>
	void EmitLandParticle();

	/// <summary>
	/// 足元の発生位置
	/// </summary>
	/// <returns>パーティクルを発生させる足元の位置</returns>
	Vector3 GetFootParticlePos() const;

	/// <summary>
	/// プレイヤーの入力状態を更新
	/// </summary>
	void UpdateInput();

	/// <summary>
	/// チャージ項目の処理
	/// </summary>
	void UpdateChargeActions();

	/// <summary>
	/// スペシャルダッシュの処理
	/// </summary>
	void StartSpecialDash(float chargeRate);

	/// <summary>
	/// ハイジャンプの処理
	/// </summary>
	void StartHighJump(float chargeRate);

	/// <summary>
	/// チャージ攻撃の処理
	/// </summary>
	void StartAttack(float chargeRate);

	/// <summary>
	/// スペシャルダッシュ中のブロック破壊処理
	/// </summary>
	bool TryBreakBlockBySpecialDash(const CollisionMapInfo& collisionInfo);


	/// <summary>
	/// チャージ攻撃の当たり判定と処理
	/// </summary>
	void UpdateAttack();

	/// <summary>
	/// チャージ攻撃が使用できる状態かどうか
	/// </summary>
	bool CanChargeAttack() const;

	/// <summary>
	/// チャージ攻撃の使用中の処理
	/// </summary>
	void UpdateAttackChargeAction();

	/// <summary>
	/// チャージ率からスペシャルダッシュの速度を計算
	/// </summary>
	float CalcChargeRate(const ChargeButtonState& charge) const;

	/// <summary>
	/// チャージ攻撃の使用中のエフェクトの更新
	/// </summary>
	void UpdateChargeEffects();

	/// <summary>
	/// 単体のチャージエフェクト更新
	/// </summary>
	/// <param name="effect"></param>
	/// <param name="charge"></param>
	/// <param name="wasCharging"></param>
	/// <param name="position"></param>
	/// <param name="minEmissionRate"></param>
	/// <param name="maxEmissionRate"></param>
	void UpdateChargeEffect(ParticleSystem* effect,
		const ChargeButtonState& charge,
		bool& wasCharging,
		const Vector3& position,
		float minEmissionRate,
		float maxEmissionRate);

public:	/// Setter / Getter
	// 死亡判定の高さを設定
	void SetDeathHeight(float deathHeight) { deathHeight_ = deathHeight; }

	// マップのセット
	void SetMap(Map* map) { map_ = map; }

	// プレイヤーの位置情報取得
	const Vector3 GetTranslate()const { return playerModel_->GetTranslate(); }

	// プレイヤーが敵にヒットしているかどうか
	bool GetHitEnemy()const { return isEnemyHit_; }

	// ゴールしているかどうか
	bool GetIsGoal()const { return isGoal_; }

	// 操作の有効/無効
	void SetControlEnabled(bool enabled) { controlEnabled_ = enabled; }
	bool IsControlEnabled() const { return controlEnabled_; }

	// プレイヤーの体力取得
	uint32_t GetHealth() const { return status_.kHealth; }

	// プレイヤーの生存状態を取得
	bool IsAlive() const { return status_.kHealth > 0; }

	// プレイヤーが落下して死亡したかの取得
	bool IsDeathByFalling() const { return isDeathByFalling_; }

	// 落下死フラグを「消費」する（trueなら内部でfalseに戻す）
	bool ConsumeDeathByFalling();

	// プレイヤーが攻撃中かどうか
	bool IsAttacking() const { return isAttack_; }

private:	// メンバ変数
	// プレイヤーステータス
	PlayerStatus status_;
	// 速度
	Vector3 velocity_ = {};

	// 死亡判定の高さ / 画面外に出たら死亡判定とする
	float deathHeight_;

	// プレイヤーのモデル
	std::unique_ptr<Object3D> playerModel_ = nullptr;

	// 死亡フラグ
	bool isDead_ = false;

	// マップ
	Map* map_ = nullptr;

	// ゴールフラグ
	bool isGoal_ = false;

	// ジャンプ
	bool onGround_ = true;

	// ダッシュ
	bool isDash_ = false;

	// 向き / +1:右 -1:左 0:なし
	int32_t dashDirection_ = 0;

	// ダブルタップ判定用タイマー
	uint32_t rightTapTimer_ = 0;
	uint32_t leftTapTimer_ = 0;

	// エネミーに当たったかどうか
	bool isEnemyHit_ = false;

	// 点滅用フレームカウント
	uint32_t flashingFrameCount_ = 0;
	// 現状の点滅回数
	uint32_t flashingCount_ = 0;
	// 点滅持続フレーム数
	uint32_t maxFlashingFlame_ = 120;
	// 何フレームごとに点滅するか
	uint32_t flashingIntervalFrame_ = 10;
	// 可視フラグ
	bool isVisible_ = true;

	// プレイヤーの一周
	float playerTurnAround_ = 6.3f;

	// プレイヤーの向き
	Direction direction_ = Direction::kRight;
	float turnSpeed_ = 0.2f;
	float targetYaw_ = 0.0f;


	// 操作の有効/無効
	bool controlEnabled_ = true;

	// 通常移動のパーティクル
	std::unique_ptr <ParticleSystem>moveEffect_;

	// ダッシュエフェクト用パーティクル
	// 足元の煙
	std::unique_ptr<ParticleSystem>dashSmokeEffect_;
	// ダッシュ開始時の少し派手目用
	std::unique_ptr<ParticleSystem>dashStartEffect_;
	// 踏みつけパーティクルエフェクト
	std::unique_ptr<ParticleSystem>stompEffect_;

	// パーティクルを出すためのダッシュ状態の追跡
	bool wasDashing_ = false;
	uint32_t dashEffectCounter_ = 0;
	// パーティクルの発生位置をずらす
	float particleSpawnPosOffset_ = 0.4f;

	// ダメージブロックに触れているかの追跡
	bool wasTouchingDamageBlock_ = false;

	// ダメージブロックに触れてからのフレーム数を追跡
	uint32_t damageTouchFrames_ = 0;
	uint32_t damageTickIntervalFrames_ = 30;

	// 落下して死亡したのフラグ
	bool isDeathByFalling_ = false;

	bool isDying_ = false;
	bool deathDemoFinished_ = false;
	float deathDemoTimer_ = 0.0f;
	float deathDemoMinDuration_ = 0.2f;

	Vector3 deathVel_ = { 0,0,0 };
	float deathJumpSpeed_ = 12.0f;
	float deathGravity_ = 32.0f;

	Vector3 deathFaceRot_ = { 0,0,0 };

	// 通常移動のパーティクルが再生中か
	bool wasMoving_ = false;

	// ジャンプの開始と着地時のパーティクル
	std::unique_ptr<ParticleSystem> jumpEffect_;
	std::unique_ptr<ParticleSystem> landEffect_;
	// ジャンプブロックに触れたときのエフェクト
	std::unique_ptr<ParticleSystem> jumpBlockArrowEffect_;
	bool wasOnJumpBlock_ = false;

	// プレイヤーの入力状態
	PlayerInputState inputState_ = {};

	// チャージ攻撃の状態
	ChargeButtonState attackCharge_;
	bool isAttack_ = false;
	uint32_t attackTimer_ = 0;
	float attackChargeRate_ = 0.0f;

	// スペシャルダッシュの状態
	ChargeButtonState specialDashCharge_;
	bool isSpecialDash_ = false;
	uint32_t specialDashTimer_ = 0;
	int32_t specialDashDirection_ = 0;
	float specialDashSpeed_ = 0.0f;
	float kSpecialDashRollScale = 1.2f;

	// ハイジャンプのチャージ状態
	ChargeButtonState highJumpCharge_;


	// チャージ攻撃のチャージエフェクト
	std::unique_ptr<ParticleSystem> attackChargeEffect_;
	// 特殊ダッシュのチャージエフェクト
	std::unique_ptr<ParticleSystem> specialDashChargeEffect_;
	// ハイジャンプのチャージエフェクト
	std::unique_ptr<ParticleSystem> highJumpChargeEffect_;

	// チャージエフェクト再生状態
	bool wasAttackCharging_ = false;
	bool wasSpecialDashCharging_ = false;
	bool wasHighJumpCharging_ = false;


	
};

