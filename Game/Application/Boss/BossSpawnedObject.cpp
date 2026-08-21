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
	// 判定は見た目より大きめに取り、打ち返しの成功率を上げる
	Vector3 halfSize = { kHalfSize_, kHalfSize_, kHalfSize_ };
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
	lifeFrame_ = 0;

	Vector3 dir = boss_->GetPosition() - pos_;
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	// ゼロ除算を避ける
	if (len > 0.0001f) {
		velocity_ = (dir / len) * kLaunchSpeed_;
	}
}


void BossSpawnedObject::Update()
{
	// 状態に応じた更新処理
	if (state_ == State::kIdel) {
		UpdateIdle();
	} else if (state_ == State::kLaunched) {
		UpdateLaunched();
	}

	// モデルの位置とスケールを更新
	if (model_) {
		// 消滅が近いほど小さくして、消えることを見た目で伝える
		float scale = 1.0f;
		if (IsVanishing()) {
			const float t = static_cast<float>(idleFrame_ - kVanishWarnFrame_) /
				static_cast<float>(kMaxIdleFrame_ - kVanishWarnFrame_);
			scale = 1.0f - (1.0f - kVanishMinScale_) * t;
		}

		Transform transform;
		transform.translate = pos_;
		transform.scale = { scale, scale, scale };
		model_->SetTransform(transform);
		model_->Update();
	}
}

void BossSpawnedObject::Draw()
{
	if (model_ == nullptr)return;
	// 消滅予告中は点滅させる（描画を間引く）
	if (IsVanishing() && ((idleFrame_ / kBlinkCycle_) % 2) == 1) {
		return;
	}
	// モデルの描画
	model_->Draw();
}

void BossSpawnedObject::UpdateLaunched()
{
	// ボスが動くので毎フレーム狙い直す（ホーミング）
	Vector3 dir = boss_->GetPosition() - pos_;
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
	if (len > 0.0001f) {
		velocity_ = (dir / len) * kLaunchSpeed_;
	}

	// 速度に応じて位置を更新
	pos_ += velocity_;

	// ボスに当たったらダメージを与えて消滅
	if (IsAABBOverLap(GetAABB(), boss_->GetAABB())) {
		boss_->TakeDamage(1);
		state_ = State::kExpired;
		return;
	}

	// 保険：一定時間当たらなければ消滅させる
	lifeFrame_++;
	if (lifeFrame_ > kMaxLifeFrame_) {
		state_ = State::kExpired;
	}
}

bool BossSpawnedObject::IsAABBOverLap(const AABB& a, const AABB& b) const
{
	// AABB同士の重なり判定
	return a.Intersects(b);
}



void BossSpawnedObject::UpdateIdle()
{
	// 着弾後はゆっくり落下させる
	// 高い位置に湧いても、待てば自機の届く高さまで降りてくるようにする
	if (pos_.y > kRestY_) {
		fallSpeed_ += kFallGravity_;
		if (fallSpeed_ > kMaxFallSpeed_) {
			fallSpeed_ = kMaxFallSpeed_;
		}
		pos_.y -= fallSpeed_;

		if (pos_.y < kRestY_) {
			pos_.y = kRestY_;
			fallSpeed_ = 0.0f;
		}
	}

	// 打ち返されないまま10秒経ったら消滅させる
	idleFrame_++;
	if (idleFrame_ > kMaxIdleFrame_) {
		state_ = State::kExpired;
	}
}