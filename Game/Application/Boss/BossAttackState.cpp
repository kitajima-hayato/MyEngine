#include "BossAttackState.h"
#include "BossEnemy.h"
#include <cmath>

void BossAttackSingleShot::Update(BossEnemy& boss)
{
	boss.RequestShotToLand(boss.GetPosition().x + offsetX_, landY_);
	finished = true;
}

void BossAttackRapidFire::Update(BossEnemy& boss)
{
	timer_ -= 1.0f / 60.0f;
	if (timer_ > 0.0f) return;

	// 1発ごとに横と高さを少しずつずらして階段状に並べる
	const float f = static_cast<float>(fired_);
	boss.RequestShotToLand(boss.GetPosition().x + offsetXStep_ * f,
		landYBegin_ + landYStep_ * f);

	fired_++;
	timer_ = interval_;
	if (fired_ >= count_) finished = true;
}

void BossAttackNWay::Update(BossEnemy& boss)
{
	const float bossX = boss.GetPosition().x;
	for (int i = 0; i < wayCount_; i++) {
		// -1.0(左端) 〜 +1.0(右端)
		const float t = (wayCount_ <= 1)
			? 0.0f
			: static_cast<float>(i) / static_cast<float>(wayCount_ - 1) * 2.0f - 1.0f;
		// 端ほど高く（landYCurve_が負なら端ほど低く）並べる
		boss.RequestShotToLand(bossX + t * spanX_ * 0.5f,
			landYCenter_ + landYCurve_ * std::fabs(t));
	}
	finished = true;
}

void BossAttackSweep::Update(BossEnemy& boss)
{
	timer_ -= 1.0f / 60.0f;
	if (timer_ > 0.0f) return;

	// 進行度 0.0〜1.0
	const float t = (count_ <= 1)
		? 0.0f
		: static_cast<float>(fired_) / static_cast<float>(count_ - 1);

	// -1.0 〜 +1.0 に変換（reverse_で向きを反転）
	float side = t * 2.0f - 1.0f;
	if (reverse_) side = -side;

	// 進行に合わせて高さも動かすので斜めに薙ぎ払える
	boss.RequestShotToLand(boss.GetPosition().x + side * spanX_ * 0.5f,
		landYBegin_ + (landYEnd_ - landYBegin_) * t);

	fired_++;
	timer_ = interval_;
	if (fired_ >= count_) finished = true;
}

void BossAttackBurstNWay::Update(BossEnemy& boss)
{
	timer_ -= 1.0f / 60.0f;
	if (timer_ > 0.0f) return;

	const float s = static_cast<float>(firedSet_);
	const float centerX = boss.GetPosition().x + offsetXStep_ * s;
	// セットごとに高さを変えて段違いに並べる
	const float landY = landYBegin_ + landYStep_ * s;

	for (int i = 0; i < wayCount_; i++) {
		const float t = (wayCount_ <= 1)
			? 0.0f
			: static_cast<float>(i) / static_cast<float>(wayCount_ - 1) * 2.0f - 1.0f;
		boss.RequestShotToLand(centerX + t * spanX_ * 0.5f, landY);
	}

	firedSet_++;
	timer_ = interval_;
	if (firedSet_ >= setCount_) finished = true;
}

void BossAttackCharge::Update(BossEnemy& boss)
{
	timer_ += 1.0f / 60.0f;

	// 溜め中：Y回転で予兆を出す（AABBは回転の影響を受けないので当たり判定は変わらない）
	if (timer_ < chargeTime_) {
		Vector3 rot = boss.GetRotate();
		rot.y += 0.25f;
		boss.SetRotate(rot);
		return;
	}

	// 溜め終わり：回転を戻して格子状にばらまく
	boss.SetRotate({ 0.0f, 0.0f, 0.0f });

	const float bossX = boss.GetPosition().x;
	for (int r = 0; r < rows_; r++) {
		const float ry = (rows_ <= 1)
			? 0.0f
			: static_cast<float>(r) / static_cast<float>(rows_ - 1);
		const float landY = landYBottom_ + (landYTop_ - landYBottom_) * ry;

		// 段ごとに半マスずらして市松模様にする
		const float shift = (r % 2 == 0)
			? 0.0f
			: spanX_ / (static_cast<float>(columns_) * 2.0f);

		for (int c = 0; c < columns_; c++) {
			const float t = (columns_ <= 1)
				? 0.0f
				: static_cast<float>(c) / static_cast<float>(columns_ - 1) * 2.0f - 1.0f;
			boss.RequestShotToLand(bossX + t * spanX_ * 0.5f + shift, landY);
		}
	}
	finished = true;
}

void BossAttackAimed::Update(BossEnemy& boss)
{
	timer_ -= 1.0f / 60.0f;
	if (timer_ > 0.0f) return;

	// プレイヤーの少し上を狙うので、真上から降ってくるように見える
	const Vector3 to = boss.GetTargetPosition();
	boss.RequestShotToLand(to.x, to.y + aimOffsetY_);

	fired_++;
	timer_ = interval_;
	if (fired_ >= count_) finished = true;
}

void BossAttackRush::Update(BossEnemy& boss)
{
	Vector3 pos = boss.GetPosition();

	// 最初のフレームで元のZを覚えておく
	if (!homeSaved_) {
		homeZ_ = pos.z;
		homeSaved_ = true;
	}

	switch (step_) {
	case Step::kTelegraph:
	{
		timer_ += 1.0f / 60.0f;

		// プレイヤーのXへ寄せて狙いを定める
		pos.x += (boss.GetTargetPosition().x - pos.x) * 0.15f;
		boss.SetTranslate(pos);

		// 回転で「これから突っ込む」ことを知らせる
		Vector3 rot = boss.GetRotate();
		rot.z += 0.3f;
		boss.SetRotate(rot);

		if (timer_ >= telegraphTime_) {
			step_ = Step::kRush;
		}
		break;
	}
	case Step::kRush:
	{
		// プレイヤーの居る手前の面へ突っ込む
		pos.z -= rushSpeed_;
		boss.SetTranslate(pos);

		if (pos.z <= BossEnemy::kRushEndZ) {
			step_ = Step::kReturn;
		}
		break;
	}
	case Step::kReturn:
	default:
	{
		// 奥へ戻る
		pos.z += returnSpeed_;
		boss.SetRotate({ 0.0f, 0.0f, 0.0f });

		if (pos.z >= homeZ_) {
			pos.z = homeZ_;
			boss.SetTranslate(pos);
			finished = true;
			return;
		}
		boss.SetTranslate(pos);
		break;
	}
	}
}

void BossAttackSlam::Update(BossEnemy& boss)
{
	Vector3 pos = boss.GetPosition();

	// 最初のフレームで元の位置を覚えておく
	if (!homeSaved_) {
		homePos_ = pos;
		homeSaved_ = true;
	}

	const Vector3 target = boss.GetTargetPosition();

	switch (step_) {
	case Step::kApproach:
	{
		timer_ += 1.0f / 60.0f;

		// プレイヤーと同じX/Zへ、Yはプレイヤーより少し高い位置へ寄せる
		const float rate = 0.12f;
		pos.x += (target.x - pos.x) * rate;
		pos.y += ((target.y + heightAbove_) - pos.y) * rate;
		pos.z += (0.0f - pos.z) * rate;
		boss.SetTranslate(pos);

		if (timer_ >= approachTime_) {
			timer_ = 0.0f;
			step_ = Step::kAim;
		}
		break;
	}
	case Step::kAim:
	{
		timer_ += 1.0f / 60.0f;

		// 真上で静止して狙いを定める（この間にプレイヤーは逃げられる）
		Vector3 rot = boss.GetRotate();
		rot.y += 0.4f;
		boss.SetRotate(rot);

		if (timer_ >= aimTime_) {
			step_ = Step::kFall;
		}
		break;
	}
	case Step::kFall:
	{
		// 加速しながら一気に落とす
		fallSpeed_ += fallAccel_;
		pos.y -= fallSpeed_;

		if (pos.y <= BossEnemy::kSlamGroundY) {
			pos.y = BossEnemy::kSlamGroundY;
			boss.SetRotate({ 0.0f, 0.0f, 0.0f });
			timer_ = 0.0f;
			step_ = Step::kRecover;
		}
		boss.SetTranslate(pos);
		break;
	}
	case Step::kRecover:
	default:
	{
		// 着地後は少し硬直させて、反撃の隙を作る
		timer_ += 1.0f / 60.0f;
		if (timer_ < recoverTime_) break;

		// 元の位置へ戻る
		const float rate = 0.08f;
		pos.x += (homePos_.x - pos.x) * rate;
		pos.y += (homePos_.y - pos.y) * rate;
		pos.z += (homePos_.z - pos.z) * rate;
		boss.SetTranslate(pos);

		// 十分近づいたら元の位置に合わせて終了
		if (std::fabs(pos.z - homePos_.z) < 0.3f) {
			boss.SetTranslate(homePos_);
			finished = true;
		}
		break;
	}
	}
}