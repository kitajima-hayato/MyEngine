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

private:

	void LaunchTowardBoss();

	void UpdateLaunched();

	bool IsAABBOverLap(const AABB& a, const AABB& b) const;

private:

	Vector3 pos_;
	Vector3 velocity_;
	State state_ = State::kIdel;
	BossEnemy* boss_ = nullptr;
	std::unique_ptr<Object3D> model_;

};

