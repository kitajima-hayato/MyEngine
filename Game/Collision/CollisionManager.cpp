#include "CollisionManager.h"

void CollisionManager::AddCollider(Collider* collider)
{
    /// リストに登録
	colliders.push_back(collider);
}

void CollisionManager::Clear()
{
    ///登録されているコライダーすべてを削除
	colliders.clear();
}

void CollisionManager::CheckAllCollisions()
{
	for (size_t i = 0; i < colliders.size(); ++i)
	{
		for (size_t j = i + 1; j < colliders.size(); ++j)
		{
			Collider* a = colliders[i];
			Collider* b = colliders[j];

			if (!a || !b) {
				continue;
			}

			// 攻撃判定を先に見る
			bool handledByAttack = false;
			handledByAttack |= CheckAttackCollision(a, b);
			handledByAttack |= CheckAttackCollision(b, a);

			// 攻撃で処理済みなら、通常衝突は行わない
			// これにより、攻撃が当たったのに同時にダメージを受ける事故を防ぐ
			if (handledByAttack) {
				continue;
			}

			// 通常の本体AABB同士の衝突
			if (IsColliding(a, b))
			{
				NotifyCollision(a, b);
			}
		}
	}
}

bool CollisionManager::IsColliding(Collider* a, Collider* b) {

    /// ２つの判定を取る
	return a->GetAABB().Intersects(b->GetAABB());

}

void CollisionManager::NotifyCollision(Collider* a, Collider* b) {
    /// 衝突判定が取れた時通知する
    a->OnCollision(b);
    b->OnCollision(a);
}

bool CollisionManager::CheckAttackCollision(Collider* attacker, Collider* target)
{
	// アタッカーが攻撃判定を持っているか
	if (!attacker || !target) {
		return false;
	}

	// アタッカーが攻撃判定を持っていないなら
	if (!attacker->HasAttackAABB()) {
		return false;
	}

	// ターゲットが攻撃判定を持っている
	const AABB attackAABB = attacker->GetAttackAABB();
	const AABB targetAABB = target->GetAABB();
	// 攻撃判定とターゲットの本体AABBが交差しているか
	if (!attackAABB.Intersects(targetAABB)) {
		return false;
	}

	// 交差しているなら、アタッカーに攻撃が当たったことを通知
	return attacker->OnAttackCollision(target);
}