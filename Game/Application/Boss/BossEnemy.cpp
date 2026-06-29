#include "BossEnemy.h"

void BossEnemy::Initialize()
{
	// HPを最大値に設定
	stats.health = kMaxHealth_;
	// ステージ奥に配置
	stats.transform.translate = Vector3{ 20.0f,5.0f,10.0f };

	// @todo: 要調整
	stats.transform.scale = Vector3{ 3.0f,3.0f,3.0f };

	model = std::make_unique<Object3D>();
	model->Initialize();
	model->SetModel("GamePlay/Player");
	model->SetTransform(stats.transform);
}

void BossEnemy::Update()
{
	// 行動処理
	Action();
	// モデルの更新
	model->SetTransform(stats.transform);
	model->Update();
}

void BossEnemy::Draw()
{
	// モデルの描画
	model->Draw();
}
void BossEnemy::Move()
{
	
}



void BossEnemy::OnStomped()
{
	
}

void BossEnemy::TakeDamage(int32_t damage)
{
	// ダメージを受ける処理 / HPが0以下にならないようにする
	if (stats.health <= static_cast<uint32_t>(damage)) {
		stats.health = 0;
	} else {
		stats.health -= static_cast<uint32_t>(damage);
	}
}


void BossEnemy::Action()
{
	shotTimer_ += 1.0f / 60.0f;   // 60fps 固定 dt
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0.0f;
		pendingShot_ = true;
		pendingShotOrigin_ = stats.transform.translate;
		UpdatePhase();
	}
}

void BossEnemy::UpdatePhase()
{
	if (stats.health > 6) {
		phase_ = Phase::kPhase1;
		shotInterval_ = 4.0f;
	} else if (stats.health > 3) {
		phase_ = Phase::kPhase2;
		shotInterval_ = 2.5f;
	} else {
		phase_ = Phase::kPhase3;
		shotInterval_ = 1.5f;
	}
}

Vector3 BossEnemy::ConsumeShotOrigin()
{
	pendingShot_ = false;
	return pendingShotOrigin_;
}

