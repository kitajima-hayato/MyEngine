#include "RespawnSequence.h"

void RespawnSequence::Initialize()
{
	blackOverlay_ = std::make_unique<Sprite>();
	blackOverlay_->Initialize("resources/Scenes/Debug/BlackDome/Black.dds");

	// 初期は見えない（透明）
	blackOverlay_->SetColor({ 0.0f,0.0f,0.0f,0.0f });
	blackOverlay_->SetPosition({ 0.0f,0.0f });
	blackOverlay_->SetSize({ 1280.0f,720.0f });
}

void RespawnSequence::Update(float dt)
{
	if (!blackOverlay_) { return; }

	if (phase_ == Phase::Darken || phase_ == Phase::Lighten) {
		UpdateFade(dt);
	} else if (phase_ == Phase::Hold) {
		holdElapsed_ += dt;
		if (holdElapsed_ >= holdDuration_) {
			StartLighten();
		}
	}

	blackOverlay_->Update();
}

void RespawnSequence::Draw()
{
	if (!blackOverlay_) { return; }
	blackOverlay_->Draw();
}

void RespawnSequence::Start()
{
	// 状態をリセットして暗転開始
	isFinished_ = false;
	isDarkened_ = false;
	StartDarken();
}

bool RespawnSequence::ConsumeDarkened()
{
	if (!isDarkened_) { return false; }
	isDarkened_ = false;
	return true;
}

void RespawnSequence::StartDarken()
{
	phase_ = Phase::Darken;
	colorChangeDuration_ = darkenDuration_;

	colorChangeElapsed_ = 0.0f;

	// 透明→真っ黒
	startColor_ = { 0.0f, 0.0f, 0.0f, 0.0f };
	endColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };

	if (blackOverlay_) {
		blackOverlay_->SetColor(startColor_);
	}
}

void RespawnSequence::StartLighten()
{
	phase_ = Phase::Lighten;
	colorChangeDuration_ = lightenDuration_;
	colorChangeElapsed_ = 0.0f;

	// 真っ黒→透明
	startColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };
	endColor_ = { 0.0f, 0.0f, 0.0f, 0.0f };

	if (blackOverlay_) {
		blackOverlay_->SetColor(startColor_);
	}
}

void RespawnSequence::StartHold()
{
	phase_ = Phase::Hold;
	holdElapsed_ = 0.0f;

	// 真っ暗を維持（alpha=1）
	if (blackOverlay_) {
		blackOverlay_->SetColor({ 0.0f,0.0f,0.0f,1.0f });
	}
}

void RespawnSequence::UpdateFade(float dt)
{
	colorChangeElapsed_ += dt;

	float t = colorChangeElapsed_ / colorChangeDuration_;
	if (t >= 1.0f) { t = 1.0f; }

	// 線形補間（Lerp）
	Vector4 c;
	c.x = startColor_.x + (endColor_.x - startColor_.x) * t;
	c.y = startColor_.y + (endColor_.y - startColor_.y) * t;
	c.z = startColor_.z + (endColor_.z - startColor_.z) * t;
	c.w = startColor_.w + (endColor_.w - startColor_.w) * t;

	blackOverlay_->SetColor(c);

	// 完了判定
	if (t >= 1.0f) {
		if (phase_ == Phase::Darken) {
			// 真っ暗到達（この瞬間にワープさせる）
			isDarkened_ = true;

			// すぐ明転へ（ワープ後に呼びたいなら、ここで切り替えずGamePlayScene側でStartLighten()を呼ぶ設計もOK）
			StartLighten();
		} else if (phase_ == Phase::Lighten) {
			phase_ = Phase::None;
			isFinished_ = true;
		}
	}
}