#pragma once
#include "MyMath.h"
struct AABB
{
	Vector3 min;
	Vector3 max;
	
	bool Intersects(const AABB& other)const {
		return	
			(min.x <= other.max.x && max.x >= other.min.x) &&
			(min.y <= other.max.y && max.y >= other.min.y) &&
			(min.z <= other.max.z && max.z >= other.min.z);
	}
};

class Collider
{
public:
	enum class Type
	{
		Player,
		Enemy,
		Static,
	};

	virtual ~Collider() = default;

	virtual Type GetType() const = 0;
	virtual AABB GetAABB() const = 0;

	virtual void OnCollision(Collider* other) = 0;

	// 攻撃判定を持っているか
	virtual bool HasAttackAABB() const { return false; }

	// 攻撃判定用AABB
	virtual AABB GetAttackAABB() const { return GetAABB(); }

	// 攻撃判定が当たったときの処理
	// 処理したなら true
	virtual bool OnAttackCollision(Collider* other) { return false; }
};