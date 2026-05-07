#include "FlyingEnemy.h"

void FlyingEnemy::Initialize()
{
	/// @仮スタッツ
	stats = {
		// scale
		{{0.5f, 0.5f, 0.5f},
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
	
	timer = 0.0f;
	

	/// Object3Dの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	/// モデルの設定
	model->SetModel("GamePlay/Enemies/flyingenemy/head");
	/// トランスフォームの設定
	model->SetTransform(stats.transform);

	bodyModel = std::make_unique<Object3D>();
	bodyModel->Initialize();
	bodyModel->SetModel("GamePlay/Enemies/flyingenemy/oralarm");
	bodyModel->SetTransform(stats.transform);

	baseY = stats.transform.translate.y;
}


void FlyingEnemy::Update()
{
	if (stats.isAlive) {
		/// 挙動処理
		Move();
		/// 攻撃処理
		Action();
		/// トランスフォームの反映
		/// トランスフォームの反映
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

		/// モデルの更新
		model->Update();
		bodyModel->Update();
	}
}

void FlyingEnemy::Draw()
{
	if (stats.isAlive) {
		/// モデルの描画
		model->Draw();
		bodyModel->Draw();
	}
}

void FlyingEnemy::Move()
{ 
	FloatingEnemyBase::UpdateFloating();
}

void FlyingEnemy::Action()
{
	
}

void FlyingEnemy::SetTranslate(const Vector3& translate)
{
	EnemyBase::SetTranslate(translate);
	baseY = translate.y;     
}

