#include "BossSpawnedObject.h"
#include "BossEnemy.h"


void BossSpawnedObject::Initialize(Vector3 pos, BossEnemy* boss)
{}

bool BossSpawnedObject::OnAttackCollision(Collider* other)
{	
	// 攻撃判定に当たったときの処理
	// アイドル状態以外の時は攻撃判定を受け付けない
	if (state_ != State::kIdel)return false;
	// ボスに向かって発射する
	LaunchTowardBoss();
	return true;
}

void BossSpawnedObject::LaunchTowardBoss()
{
	// ボスの状態を変更
	state_ = State::kLaunched;

	Vector3 dir = boss_->GetPosition() - pos_;
	// 正規化して速度を設定 
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	velocity_ = (dir / len) * 0.2f;
}


void BossSpawnedObject::Update()
{
	if (state_ == State::kLaunched) {
		UpdateLaunched();
	}
}

void BossSpawnedObject::UpdateLaunched()
{
	// 速度に応じて位置を更新
	pos_ += velocity_;
	// ボスに近づきすぎたら消滅
	if (IsAABBOverLap(GetAABB(), boss_->GetAABB())) {
		boss_->TakeDamage(1); // ボスにダメージを与える
		state_ = State::kExpired; // オブジェクトを消滅させる
	}
}



