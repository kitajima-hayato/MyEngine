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
	// 頭モデル
	model->SetModel("GamePlay/Enemies/flyingenemy/head");
	// トランスフォームの設定
	model->SetTransform(stats.transform);

	// 脚パーツモデル
	bodyModel = std::make_unique<Object3D>();
	bodyModel->Initialize();
	bodyModel->SetModel("GamePlay/Enemies/flyingenemy/oralarm");
	bodyModel->SetTransform(stats.transform);
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

		// 頭は通常のTransform
		model->SetTransform(stats.transform);

		// 脚パーツだけ別Transformにする
		auto legTransform = stats.transform;

		// sinで左右にゆらゆら動かす
		float swing = std::sin(timer * legSwingSpeed_);

		// 少し左右に振る
		legTransform.rotate.z += swing * legSwingRot_;

		// 位置も少しだけずらす
		legTransform.translate.x += swing * legSwingMoveX_;

		// 上下にも少し揺らす
		legTransform.translate.y -= std::abs(swing) * legSwingMoveY_;

		bodyModel->SetTransform(legTransform);

		// モデルの更新
		model->Update();
		bodyModel->Update();
	}
}

void SideMoveFlyingEnemy::Draw()
{
	// 生存していたら描画する
	if (stats.isAlive) {
		model->Draw();
		bodyModel->Draw();
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
