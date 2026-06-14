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
	enum class State {kIdel,kLaunched,kExpired };

	void Initialize(Vector3 pos, BossEnemy* boss);

	void Update();

	void Draw();

	Type GetType() const override { return Type::Enemy; }
	AABB GetAABB() const override;
	void OnCollision(Collider* other) override;
	bool OnAttackCollision(Collider* other) override;

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

