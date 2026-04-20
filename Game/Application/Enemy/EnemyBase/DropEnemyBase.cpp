#include "DropEnemyBase.h"
#include <cmath>

void DropEnemyBase::Initialize()
{
	stats = {
		.transform = { 
			{1.0f, 1.0f, 1.0f}, 
			{0.0f, 0.0f, 0.0f}, 
			{0.0f, startY_, 0.0f} },
		.isAlive = true,
		.velocity = { 0.0f, 0.0f, 0.0f },
		.health = 1,
		.attackPower = 1,
	};
	// モデルの初期化
	model = std::make_unique<Object3D>();
	model->Initialize();
	// 継承先で各自モデルを指定する
	model->SetModel(GetModelName());
	model->SetTransform(stats.transform);

	// 初期状態は待機状態
	state_ = DropState::Waiting;
	// クールダウンタイマーをリセット
	coolDownTimer_ = 0.0f;
}

void DropEnemyBase::Draw()
{
	if (stats.isAlive) {
		model->Draw();
	}
}

void DropEnemyBase::Action()
{
	// 共通処理は無いためここでは何もしない
}

void DropEnemyBase::SetTranslate(const Vector3& translate)
{
	EnemyBase::SetTranslate(translate);
	// 初期位置のY座標を設定
	startX_ = translate.x;
	startY_ = translate.y;
	targetY_ = startY_ - dropDistance_;
}

void DropEnemyBase::Update()
{
	// 死亡している場合は処理しない
	if (!stats.isAlive) return;


	Move();

	Action();

	model->SetTransform(stats.transform);
	model->Update();

}

void DropEnemyBase::Move()
{
	switch (state_)
	{
	case DropEnemyBase::DropState::Waiting:
		// 待機状態ではY座標を固定して速度も0にする
		stats.velocity = { 0.0f, 0.0f, 0.0f };
		// 待機中の見た目
		stats.transform.scale = { 1.0f, 1.0f, 1.0f };

		// XYの値を初期位置に
		stats.transform.translate.x = startX_; 
		stats.transform.translate.y = startY_;

		if (ShouldStartDrop()) {
			// 落下攻撃開始の条件を満たしたら落下状態に遷移
			state_ = DropState::Warning;
			// 警告タイマーをリセット
			warningTimer_ = 0.0f; 
		}
		break;
	case DropEnemyBase::DropState::Warning:
		UpdateWarning();
		break;
	case DropEnemyBase::DropState::Falling:
		UpdateFalling();
		break;
	case DropEnemyBase::DropState::Cooldown:
		UpdateCooldown();
		break;
	case DropEnemyBase::DropState::Returning:
		UpdateReturning();
		break;
	default:
		break;
	}
}

void DropEnemyBase::UpdateWarning()
{
	// 60FPS
	warningTimer_ += 1.0f / 60.0f;
	// 移動量を0にして待機する
	stats.velocity = { 0.0f, 0.0f, 0.0f };

	// 予兆の見た目
	stats.transform.scale = { 1.1f, 0.9f, 1.0f };

	// 警告時間中は左右に小刻みに揺れる演出
	float shake = std::sin(warningTimer_ * warningShakeFrequency_) * warningShakeAmplitude_;
	stats.transform.translate.x = startX_ + shake;
	
	// 警告演出が終了したら落下状態に遷移
	if (warningTimer_ >= warningTime_) {
		// タイマーのリセット
		warningTimer_ = 0.0f; 
		// 元に戻す
		stats.transform.scale = { 1.0f, 1.0f, 1.0f };
		// ぶるぶる震える演出をリセット
		stats.transform.translate.x = startX_;
		// 落下速度を初期値に設定
		currentFallSpeed_ = initialFallSpeed_;
		// 警告時間が終了したら落下状態に遷移
		state_ = DropState::Falling;
	}
}

void DropEnemyBase::UpdateFalling()
{
	// 落下速度を加速させる
	currentFallSpeed_ += fallAcceleration_;
	if (currentFallSpeed_ > maxFallSpeed_) {
		currentFallSpeed_ = maxFallSpeed_;
	}

	// 落下速度分Y座標を減少させる
	stats.transform.translate.y -= currentFallSpeed_;
	stats.velocity = { 0.0f, -currentFallSpeed_, 0.0f };
	// 落下中の見た目
	stats.transform.scale = { 0.9f, 1.15f, 1.0f };

	if (HasReachedGround()) {
		// Y座標を地面の位置に固定して速度も0にする
		stats.transform.translate.y = targetY_; 
		// 地面に到達したらクールダウン状態に遷移
		state_ = DropState::Cooldown;

		// クールダウンタイマーをリセット
		coolDownTimer_ = 0.0f;

		// 落下速度のリセット
		currentFallSpeed_ = 0.0f; 

		// 落下速度をリセット
		stats.velocity = { 0.0f, 0.0f, 0.0f };
	}
}

void DropEnemyBase::UpdateCooldown()
{
	// 60FPS
	coolDownTimer_ += 1.0f / 60.0f;
	// 移動量を0にして待機する
	stats.velocity = { 0.0f, 0.0f, 0.0f };
	// クールダウン中の見た目
	stats.transform.scale = { 1.0f, 1.0f, 1.0f };

	if (coolDownTimer_ >= coolDownTime_) {
		// クールダウンが終了したら元の位置へ戻る状態に遷移
		state_ = DropState::Returning;
	}
}

void DropEnemyBase::UpdateReturning()
{
	// 戻る速度分Y座標を増加させる
	stats.transform.translate.y += returnSpeed_;
	// 元の位置に戻る速度を設定
	stats.velocity = { 0.0f, returnSpeed_, 0.0f };
	// 元の位置に戻る見た目
	stats.transform.scale = { 1.05f, 0.95f, 1.0f };

	// Y座標が初期位置に戻ったら
	if(stats.transform.translate.y >= startY_) {
		// Y座標を元の位置に固定して速度も0にする
		stats.transform.translate.y = startY_;
		// Xの値も初期位置に
		stats.transform.translate.x = startX_;
		// 速度をリセット
		stats.velocity = { 0.0f, 0.0f, 0.0f };
		// 元の位置に戻ったら待機状態に遷移
		state_ = DropState::Waiting;
	}
}
