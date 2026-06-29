#include "BossSpawnedObject.h"
#include "BossEnemy.h"


void BossSpawnedObject::Initialize(Vector3 pos, BossEnemy* boss)
{
	// 初期位置とボスのポインタを保存
	pos_ = pos;
	boss_ = boss;

	// モデルの初期化
	model_ = std::make_unique<Object3D>();
	model_->Initialize();
	// @todo: モデルは代用なのでボスモデルの作成を
	model_->SetModel("GamePlay/Player");

	Transform transform;
	transform.translate = pos_;
	transform.scale = { 1.0f,1.0f,1.0f };
	model_->SetTransform(transform);
}

AABB BossSpawnedObject::GetAABB() const
{
	// AABBは中心から0.5fの大きさの立方体とする
	Vector3 halfSize = { 0.5f, 0.5f, 0.5f }; 
	return { pos_ - halfSize, pos_ + halfSize };
}

void BossSpawnedObject::OnCollision(Collider* other)
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
	// 状態に応じた更新処理
	if (state_ == State::kLaunched) {
		UpdateLaunched();
	}

	// モデルの位置を更新
	if (model_) {
		Transform transform;
		transform.translate = pos_;
		transform.scale = { 1.0f,1.0f,1.0f };
		model_->SetTransform(transform);
		model_->Update();
	}
}

void BossSpawnedObject::Draw()
{
	// ボスモデルの描画
	if (model_) {
		model_->Draw();
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

bool BossSpawnedObject::IsAABBOverLap(const AABB& a, const AABB& b) const
{
	// AABB同士の重なり判定
	return a.Intersects(b);
}



