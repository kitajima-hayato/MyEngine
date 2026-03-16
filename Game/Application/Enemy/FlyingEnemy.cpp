#include "FlyingEnemy.h"

void FlyingEnemy::Initialize()
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
	
	timer = 0.0f;
	

	/// Object3Dの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	/// モデルの設定
	model->SetModel("GamePlay/Enemies/tentativeenemy");
	/// トランスフォームの設定
	model->SetTransform(stats.transform);

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
		model->SetTransform(stats.transform);
		/// モデルの更新
		model->Update();
	}
}

void FlyingEnemy::Draw()
{
	if (stats.isAlive) {
		/// モデルの描画
		model->Draw();
	}
}

void FlyingEnemy::Move()
{ 
	// Y軸方向に浮遊する挙動を実装
	// ふり幅と速度を調整して、自然な浮遊感を出す
	const float amplitude = 2.0f;
	const float frequency = 0.01f;

	timer += frequency;
	// timerの値を元に、正弦波を使ってY軸方向のオフセットを計算
	float offsetY = std::sin(timer * 2.0f * 3.14159265f) * amplitude;
	// baseYにオフセットを加えて、Y軸方向の位置を更新
	stats.transform.translate.y = baseY + offsetY;
}

void FlyingEnemy::Action()
{
	
}

void FlyingEnemy::SetTranslate(const Vector3& translate)
{
	EnemyBase::SetTranslate(translate);
	baseY = translate.y;      
	timer = 0.0f;             
}

