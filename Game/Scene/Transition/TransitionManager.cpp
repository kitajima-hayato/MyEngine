#include "TransitionManager.h"

void TransitionManager::Initialize()
{
	phase_ = TransitionPhase::None;
	loadingComplete_ = false;
	transitionEffect_.reset();
}

void TransitionManager::Update(float deltaTime)
{
	if (!transitionEffect_ || phase_ == TransitionPhase::None) {
		return;
	}

	// Loading中は演出を進めない
	if (phase_ != TransitionPhase::Loading) {
		transitionEffect_->Update(deltaTime);
	}

	// フェーズの遷移処理
	switch (phase_)
	{
	case TransitionPhase::FadeOut:
		// 中間地点に到達したらLoading状態へ
		if (transitionEffect_->IsHalfway()) {
			phase_ = TransitionPhase::Loading;
		}
		break;
	case TransitionPhase::Loading:
		// ロードの完了を待つ
		if (loadingComplete_) {
			phase_ = TransitionPhase::FadeIn;
			loadingComplete_ = false;
		}
		break;
	case TransitionPhase::FadeIn:
		if (transitionEffect_->IsComplete()) {
			phase_ = TransitionPhase::None;
			transitionEffect_.reset();
		}
		break;
	default:
		break;
	}

}

void TransitionManager::Draw()
{
	// 遷移エフェクトが
	if (transitionEffect_ && phase_ != TransitionPhase::None) {
		transitionEffect_->Draw();
	}
}

void TransitionManager::StartTransition(TransitionType type)
{
	if (transitionEffect_) {
		transitionEffect_->Start();
		phase_ = TransitionPhase::FadeOut;
		loadingComplete_ = false;
	}
}

void TransitionManager::SetTransitionEffect(std::unique_ptr<ITransitionEffect> effect)
{
	transitionEffect_ = std::move(effect);
}

void TransitionManager::OnLoadingComplete()
{
	loadingComplete_ = true;
}
