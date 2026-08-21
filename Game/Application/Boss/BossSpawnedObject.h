#pragma once
#include "Game/Collision/Collider.h"
#include "engine/3d/Object3D.h"

#include <memory>

// 前方宣言
class BossEnemy;


/// <summary>
/// ボススポーンオブジェクトクラス
/// </summary>
class BossSpawnedObject : public Collider
{

public:
	//　ボスの行動状態
	enum class State {kIdel,kLaunched,kExpired };

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pos">初期位置</param>
	/// <param name="boss">ボスのポインタ</param>
	void Initialize(Vector3 pos, BossEnemy* boss);

	/// <summary>
	/// 更新 / 描画
	/// </summary>
	void Update();
	void Draw();

	// Colliderのオーバーライド
	Type GetType() const override { return Type::Enemy; }
	AABB GetAABB() const override;
	void OnCollision(Collider* other) override;
	bool OnAttackCollision(Collider* other) override;


	/// <summary>
	/// 期限切れかどうかを返す
	/// </summary>
	bool IsExpired() const { return state_ == State::kExpired; }

	/// <summary>
	/// 状態を取得する
	/// </summary>
	/// <returns>現在の状態</returns>
	State GetState() const { return state_; }

	

	// Colliderのオーバーライド
	bool IsHarmfulToPlayer() const override { return false; }
private:

	// ボスの方向に向かって発射する
	void LaunchTowardBoss();

	// 発射後の更新
	void UpdateLaunched();

	// AABB同士の衝突判定
	bool IsAABBOverLap(const AABB& a, const AABB& b) const;

	// もうすぐ消滅する状態か
	bool IsVanishing()const { return state_ == State::kIdel && idleFrame_ > kVanishWarnFrame_; }

	/// <summary>
	/// 待機状態の更新
	/// </summary>
	void UpdateIdle();

private:
	// 位置、速度、状態、ボスへのポインタ、モデル
	Vector3 pos_;
	Vector3 velocity_;
	State state_ = State::kIdel;
	BossEnemy* boss_ = nullptr;
	std::unique_ptr<Object3D> model_;

	// 発射後の経過フレーム（保険の寿命管理用）
	int lifeFrame_ = 0;
	// 10秒当たらなければ消滅
	static constexpr int   kMaxLifeFrame_ = 600; 
	static constexpr float kLaunchSpeed_ = 0.2f;


	// 待機中の落下速度
	float fallSpeed_ = 0.0f;
	// 待機中の経過フレーム
	int idleFrame_ = 0;

	// 当たり判定の半径（見た目より大きめに取って打ち返しやすくする）
	static constexpr float kHalfSize_ = 0.9f;
	// 待機中の落下の加速度と最大落下速度
	static constexpr float kFallGravity_ = 0.004f;
	static constexpr float kMaxFallSpeed_ = 0.06f;
	// これ以上は落ちない高さ（地上の自機が攻撃を当てられる高さ）
	static constexpr float kRestY_ = 2.0f;

	// 打ち返されないまま消えるまでのフレーム数（10秒）
	static constexpr int kMaxIdleFrame_ = 600;
	// 消滅予告を始めるフレーム数（残り2秒から点滅＆縮小）
	static constexpr int kVanishWarnFrame_ = 480;
	// 点滅の周期（フレーム）
	static constexpr int kBlinkCycle_ = 5;
	// 消滅直前まで縮む最小スケール
	static constexpr float kVanishMinScale_ = 0.4f;
};

