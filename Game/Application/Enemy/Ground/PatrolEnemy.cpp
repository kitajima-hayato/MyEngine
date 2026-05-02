#include "PatrolEnemy.h"

// ラジアンの定数
constexpr float kFaceRight = 3.14159265f; 
constexpr float kFaceLeft = 0.0f; 

void PatrolEnemy::Initialize()
{
	// ステータスの初期化
	stats = {
		{{1.0f, 1.0f, 1.0f},
		 {0.0f, 0.0f, 0.0f},
		 {0.0f, 0.0f, 0.0f}},
		true,
		{0.0f, 0.0f, 0.0f},
		1,
		1
	};
	// モデルの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	model->SetModel("GamePlay/Enemies/patrolenemy/hat");
	stats.transform.scale = { 0.5f, 0.5f, 0.5f };
	model->SetTransform(stats.transform);

	bodyModel = std::make_unique<Object3D>();
	bodyModel->Initialize();
	bodyModel->SetModel("GamePlay/Enemies/patrolenemy/body");
	bodyTransform = stats.transform;
	bodyModel->SetTransform(bodyTransform);

	// 巡回の初期設定
	direction_ = 1;
}

void PatrolEnemy::Update()
{
	// 生存していない場合は更新処理を行わない
	if (!stats.isAlive) {
		return;
	}
	// 移動と行動の更新
	Move();
	// 行動の更新
	Action();
	// モデルのトランスフォームをステータスに合わせて更新
	model->SetTransform(stats.transform);
	model->Update();

	bodyTransform = stats.transform;
	bodyModel->SetTransform(bodyTransform);
	bodyModel->Update();
}


void PatrolEnemy::Draw()
{
	// 生存している場合のみ描画
	if (stats.isAlive) {
		model->Draw();
		bodyModel->Draw();
	}
}

void PatrolEnemy::Move()
{
	// 巡回の処理
	// 左右の移動上限値 / 左右
	float leftLimit = StartX_ - moveRange_;
	float rightLimit = StartX_ + moveRange_;

	// 現在の位置を取得
	Vector3 pos = stats.transform.translate;

	// 進行方向に移動
	if (pos.x <= leftLimit) {
		direction_ = 1;
	}
	else if (pos.x >= rightLimit) {
		direction_ = -1;
	}
	// 位置を更新
	pos.x += static_cast<float>(direction_) * moveSpeed_;
	stats.transform.translate = pos;
	// 速度を更新
	stats.velocity.x = static_cast<float>(direction_) * moveSpeed_;

	// 向き反転
	if (direction_ > 0) {
		stats.transform.rotate.y  = kFaceRight;
	} else {
		stats.transform.rotate.y = kFaceLeft;
	}

}

void PatrolEnemy::Action()
{
}

