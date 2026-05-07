#include "SideMoveEnemy.h"

void SideMoveEnemy::Initialize()
{
	/// @仮スタッツ
	stats = {
		// scale
		{{1.0f, 1.0f, 1.0f},
		// Rotate
		{0.0f, 0.0f, 0.0f},
		// Translate
		//{4.0f, -3.0f, 20.0f}
		},
		// 生存フラグ
		true,
		// 速度
		{0.03f, 0.1f, 0.0f},
		// 体力
		1,
		// 攻撃力
		1
	};

	// Object3Dの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	// モデルの設定
	model->SetModel("GamePlay/Enemies/normalenemy");
	// トランスフォームの設定
	model->SetTransform(stats.transform);
}

void SideMoveEnemy::Update()
{
	if (stats.isAlive) {
		// 挙動処理
		Move();
		// 攻撃処理
		Action();
		// トランスフォームの反映
		model->SetTransform(stats.transform);
		// モデルの更新
		model->Update();
	}
}

void SideMoveEnemy::Draw()
{
	// 生存していたらモデルを描画する
	if (stats.isAlive) {
		model->Draw();
	}
}

void SideMoveEnemy::Move()
{
	if (!mapQuery) {
		stats.transform.translate.x += stats.velocity.x;
		UpdateFacingDirection();
		return;
	}

	float dir = (stats.velocity.x >= 0.0f) ? 1.0f : -1.0f;

	const Vector3& pos = stats.transform.translate;
	const Vector3& scale = stats.transform.scale;

	float probeX = pos.x + dir * (scale.x * 0.5f + 0.05f);

	float upperY = pos.y + scale.y * 0.25f;
	float lowerY = pos.y - scale.y * 0.25f;

	Vector3 upperProbe{ probeX, upperY, pos.z };
	Vector3 lowerProbe{ probeX, lowerY, pos.z };

	bool hitWall = mapQuery->IsWallAt(upperProbe) || mapQuery->IsWallAt(lowerProbe);

	if (hitWall) {
		stats.velocity.x *= -1.0f;
		UpdateFacingDirection();
		return;
	}

	stats.transform.translate.x += stats.velocity.x;
	UpdateFacingDirection();
}

void SideMoveEnemy::Action()
{
}

void SideMoveEnemy::OnStomped()
{
	// 踏みつけられたら死亡
	stats.isAlive = false;
	stats.health = 0;
}

void SideMoveEnemy::UpdateFacingDirection()
{
	if (stats.velocity.x >= 0.0f) {
		stats.transform.rotate.y = 3.14159265f;
	} else {
		stats.transform.rotate.y = 0.0f;
	}
}
