#include "SideMoveFlyingEnemy.h"

void SideMoveFlyingEnemy::Initialize()
{
	/// @仮スタッツ
	stats = {
		// scale
		{{1.0f, 1.0f, 1.0f},
		// Rotate
		{0.0f, 0.0f, 0.0f},
		// Translate
		//{5.0f, -3.0f, 0.0f},
		},
		// 生存フラグ
		true,
		// 速度
		{0.1f, 0.1f, 0.0f},
		// 体力
		1,
		// 攻撃力
		1
	};
	// タイマーの初期化
	timer = 0.0f;
	// Object3Dの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	// モデルの設定
	model->SetModel("GamePlay/Enemies/tentativeenemy");
	// トランスフォームの設定
	model->SetTransform(stats.transform);
	// baseYの設定
	baseY = stats.transform.translate.y;

}

void SideMoveFlyingEnemy::Update()
{
	// 生存していたら
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

void SideMoveFlyingEnemy::Draw()
{
	// 生存していたら描画する
	if (stats.isAlive) {
		model->Draw();
	}
}

void SideMoveFlyingEnemy::Move()
{
	// ふわふわ浮く挙動
	FloatingEnemyBase::UpdateFloating();
	// 左に流れる処理
	SideMove();
	// 画面外に出たら消す
	if(stats.transform.translate.x < leftBorder_) {
		stats.isAlive = false;
	}

}

void SideMoveFlyingEnemy::Action()
{
}

void SideMoveFlyingEnemy::SetTranslate(const Vector3& translate)
{
	// 位置を設定する際に、baseYも更新するようにオーバーライド
	EnemyBase::SetTranslate(translate);
	baseY = translate.y; 
}

void SideMoveFlyingEnemy::SideMove()
{
	// X軸方向に速度分だけ移動させる
	stats.transform.translate.x -=  stats.velocity.x;
}
