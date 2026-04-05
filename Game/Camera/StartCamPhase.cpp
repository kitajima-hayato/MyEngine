#include "StartCamPhase.h"
#include "Input.h"
#include <cassert>
#include "Sprite.h"

void StartCamPhase::Initialize()
{
	// UIの初期化
	skipUI_ = std::make_unique<Sprite>();
	skipUI_->Initialize(textureFilePath_);
	skipUI_->SetPosition({ 0.0f,0.0f });
}

void StartCamPhase::DrawUI()
{
	// UIの描画 / 演出中のみ描画する
	if (phase_ != Phase::None) {
		skipUI_->Draw();
	}
}

void StartCamPhase::Bind(Camera* camera, Transform* cameraTransform)
{
	// カメラ情報のバインド
	camera_ = camera;
	cameraTransform_ = cameraTransform;
}

void StartCamPhase::Start()
{
	// 中身があるかどうか
	assert(camera_ != nullptr);

	// 固定地点の保存
	introFixedY_ = camTargetPos_.y;
	introFixedZ_ = camTargetPos_.z;

	// 開始地点保存
	startPos_ = { camTargetPos_.x,introFixedY_,camTargetPos_.z };

	// カメラを初期位置へ
	cameraTransform_->translate = startPos_;
	cameraTransform_->rotate = { 0.0f,0.0f,0.0f };
	// カメラに反映
	camera_->SetTranslate(cameraTransform_->translate);
	camera_->SetRotate(cameraTransform_->rotate);

	// 開始の演出状態
	phase_ = Phase::MoveToLeft;
	timer_ = 0.0f;



}

void StartCamPhase::Skip()
{
	// 即座にターゲットへ
	introFixedY_ = camTargetPos_.y;
	introFixedZ_ = camTargetPos_.z;
	// 開始地点保存
	startPos_ = { camTargetPos_.x,introFixedY_,introFixedZ_ };
	cameraTransform_->translate = startPos_;
	// 演出状態を終了
	phase_ = Phase::None;
	timer_ = 0.0f;
}


void StartCamPhase::Update(float dt)
{
	// カメラの演出状態がNoneの場合は何もしない
	if (phase_ == Phase::None) {
		return;
	}else{ 
		skipUI_->Update(); 
	}
	
	SkipUIBlink(dt);

	const Vector3 leftPos = { introLeftX_,introFixedY_,introFixedZ_ };
	const Vector3 rightPos = { introRightX_,introFixedY_,introFixedZ_ };
	const Vector3 startPos = { camTargetPos_.x,introFixedY_,introFixedZ_ };

#pragma region 各カメラの動き
	switch (phase_)
	{
	case StartCamPhase::Phase::None:
		break;
	case StartCamPhase::Phase::MoveToLeft:
	{
		timer_ += dt;
		const float t = EaseOutCubic(timer_ / introMoveDur_);
		cameraTransform_->translate = Lerp(startPos_, Vector3{ introLeftX_,introFixedY_,introFixedZ_ }, t);
		if (timer_ >= introMoveDur_) {
			phase_ = Phase::PanToRight;
			timer_ = 0.0f;
			cameraTransform_->translate = leftPos;
		}
		// スペースキーでスキップ
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Skip();
		}
	}
	break;
	case StartCamPhase::Phase::PanToRight:
	{
		timer_ += dt;
		const float t = EaseOutCubic(timer_ / introPanDur_);
		cameraTransform_->translate = Lerp(leftPos, rightPos, t);
		if (timer_ >= introPanDur_) {
			phase_ = Phase::Hold;
			timer_ = 0.0f;
			cameraTransform_->translate = rightPos;
		}
		// スペースキーでスキップ
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Skip();
		}
	}
	break;
	case StartCamPhase::Phase::Hold:
	{
		timer_ += dt;
		cameraTransform_->translate = rightPos;

		if (timer_ >= introHoldDur_) {
			phase_ = Phase::ReturnToStart;
			timer_ = 0.0f;
		}
		// スペースキーでスキップ
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Skip();
		}
	}
	break;
	case StartCamPhase::Phase::ReturnToStart:
	{

		timer_ += dt;

		float u = (introReturnDur_ > 0.0f) ? (timer_ / introReturnDur_) : 1.0f;
		u = std::clamp(u, 0.0f, 1.0f);

		float t = EaseOutBack(u);
		t = std::clamp(t, 0.0f, 1.0f);
		cameraTransform_->translate = Lerp(rightPos, startPos, t);

		if (timer_ >= introReturnDur_) {
			phase_ = Phase::None;
			timer_ = 0.0f;
			cameraTransform_->translate = startPos;
		}
		// スペースキーでスキップ
		if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
			Skip();
		}
	}

	break;
	default:

		break;
	}
#pragma endregion
	camera_->SetTranslate(cameraTransform_->translate);
}


bool StartCamPhase::IsRunning() const
{
	// Noneじゃなければtrueを返す
	return phase_ != Phase::None;
}

void StartCamPhase::DrawImgui()
{
}

void StartCamPhase::SkipUIBlink(float dt)
{
	// 進行度の反映
	blinkTimer_ += dt;
	// sin波
	float alpha = (std::sin(blinkTimer_ * blinkSpeed_ + 1.0f) * 1.0f);

	// スプライトに色の反映
	skipUI_->SetColor({ 1.0f,1.0f,1.0f,alpha });
}
