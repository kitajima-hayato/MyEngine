#include "BossEnemy.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

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
	// 移動処理
	Move();
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
	// 攻撃中は攻撃ステートに位置を任せる
	if (currentAttack_) return;

	moveTimer_ += 1.0f / 60.0f;

	// 移動の中心をプレイヤーのXへ緩やかに寄せる
	homeX_ += (targetPos_.x - homeX_) * kFollowRate_;

	// サインカーブで左右に往復しつつ、倍の速さで上下に揺れる
	stats.transform.translate.x = homeX_ + std::sin(moveTimer_ * kMoveSpeed_) * kMoveRangeX_;
	stats.transform.translate.y = kHomeY_ + std::sin(moveTimer_ * kMoveSpeed_ * 2.0f) * kBobRangeY_;
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
	// 攻撃中：ステートに任せ、終わったらクールダウンへ
	if (currentAttack_) {
		currentAttack_->Update(*this);
		if (currentAttack_->IsFinished()) {
			currentAttack_.reset();
			shotTimer_ = 0.0f;
			// 突進などでボスが動いた場合、移動の中心を現在地に合わせて
			// 通常移動へ戻る瞬間のワープを防ぐ
			homeX_ = stats.transform.translate.x;
		}
		return;
	}

	// クールダウン中：時間が満ちたら次の攻撃を選ぶ
	shotTimer_ += 1.0f / 60.0f;   // 60fps 固定 dt
	if (shotTimer_ >= shotInterval_) {
		shotTimer_ = 0.0f;
		UpdatePhase();	// HPに応じてフェーズと間隔を更新
		currentAttack_ = CreateAttack();
	}
}

void BossEnemy::RequestShotToLand(float landX, float landY)
{
	// 着弾地点から弾速を逆算して発射する
	const Vector3 from = stats.transform.translate;
	const float kVz = 0.15f;

	// z が 0 になるまでのフレーム数から、必要なX/Y速度を逆算する
	const float frames = (from.z > kVz) ? (from.z / kVz) : 1.0f;

	float vx = (landX - from.x) / frames;
	float vy = (landY - from.y) / frames;

	// 速すぎると避けられないので上限で丸める
	vx = std::clamp(vx, -kMaxShotVX_, kMaxShotVX_);
	vy = std::clamp(vy, -kMaxShotVY_, kMaxShotVY_);

	RequestShot(from, Vector3{ vx, vy, -kVz });
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





void BossEnemy::RequestShot(const Vector3& origin, const Vector3& velocity)
{
	// 発射リクエストをキューに追加
	shotRequests_.push_back({ origin, velocity });
}

std::vector<BossEnemy::ShotRequest> BossEnemy::ConsumeShotRequests()
{
	// 発射リクエストを返し、キューをクリア
	std::vector<ShotRequest> out = std::move(shotRequests_);
	shotRequests_.clear();
	return out;
}

std::unique_ptr<BossAttackState> BossEnemy::CreateAttack()
{
	switch (phase_) {
	case Phase::kPhase1:	// ゆるやか
	{
		const int r = rand() % 4;
		if (r == 0) return std::make_unique<BossAttackSingleShot>(0.0f, kLandYLow);
		if (r == 1) return std::make_unique<BossAttackRapidFire>(2, 0.8f, 2.0f, kLandYLow, 2.0f);
		if (r == 2) return std::make_unique<BossAttackNWay>(2, 6.0f, kLandYMid, -2.0f);
		return std::make_unique<BossAttackSweep>(5, 0.25f, 10.0f, kLandYLow, kLandYHigh);
	}
	case Phase::kPhase2:	// 手数が増える
	{
		const int r = rand() % 6;
		if (r == 0) return std::make_unique<BossAttackAimed>(1, 0.0f, 1.5f);
		if (r == 1) return std::make_unique<BossAttackRapidFire>(3, 0.5f, -2.0f, kLandYHigh, -2.0f);
		if (r == 2) return std::make_unique<BossAttackNWay>(3, 8.0f, kLandYMid, 2.0f);
		if (r == 3) return std::make_unique<BossAttackSweep>(7, 0.18f, 12.0f, kLandYHigh, kLandYLow);
		if (r == 4) return std::make_unique<BossAttackRush>(0.9f, 0.45f, 0.12f);
		return std::make_unique<BossAttackBurstNWay>(2, 3, 8.0f, 0.5f, kLandYLow, 3.0f, 1.5f);
	}
	case Phase::kPhase3:	// 激化
	default:
	{
		const int r = rand() % 7;
		if (r == 0) return std::make_unique<BossAttackCharge>(1.2f, 3, 2, 12.0f, kLandYLow, kLandYMid);
		if (r == 1) return std::make_unique<BossAttackBurstNWay>(3, 3, 10.0f, 0.5f, kLandYLow, 2.5f, 1.0f);
		if (r == 2) return std::make_unique<BossAttackSweep>(7, 0.22f, 16.0f, kLandYLow, kLandYHigh, true);
		if (r == 3) return std::make_unique<BossAttackAimed>(3, 0.4f, 2.0f);
		if (r == 4) return std::make_unique<BossAttackRush>(0.7f, 0.6f, 0.15f);
		if (r == 5) return std::make_unique<BossAttackSlam>(0.7f, 0.5f, 6.0f, 0.03f, 0.4f);
		return std::make_unique<BossAttackRapidFire>(5, 0.3f, 1.5f, kLandYMid, 1.5f);
	}
	}
}