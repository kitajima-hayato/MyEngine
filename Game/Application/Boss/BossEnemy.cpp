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
	model->SetModel("GamePlay/BossEnemy");
	model->SetTransform(stats.transform);
}

void BossEnemy::Update()
{
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
{}

void BossEnemy::Action()
{}

void BossEnemy::OnStomped()
{}

void BossEnemy::TakeDamage(int32_t damage)
{
	// ダメージを受ける処理 / HPが0以下にならないようにする
	if (stats.health <= static_cast<uint32_t>(damage)) {
		stats.health = 0;
	} else {
		stats.health -= static_cast<uint32_t>(damage);
	}
}

Vector3 BossEnemy::ConsumeShotOrigin()
{
	return Vector3();
}

void BossEnemy::UpdatePhase()
{}
