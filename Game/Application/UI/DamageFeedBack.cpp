#include "DamageFeedBack.h"


static float Clamp01(float x) {
	return std::clamp(x, 0.0f, 1.0f);
}

void DamageFeedBack::Bind(Player* player, Camera* camera, Transform* cameraTransform)
{
	// 各種ポインタの受け渡し
	player_ = player;
	camera_ = camera;
	cameraTransform_ = cameraTransform;
}

bool DamageFeedBack::IsActive() const
{
	// スプライトのα値を基に処理を動かす
	const float a = hitSprite_->GetColor().w;
	return shakeActive_ || (a > 0.0f);
}

void DamageFeedBack::Reset()
{
	// 諸々のフラグの初期化
	wasHitPrev_ = false;
	shakeActive_ = false;
	shakeTimer_ = 0.0f;
	baseCamPos_ = {};
	// スプライトが見えないように色をセット
	hitSprite_->SetColor(flashColorOff_);
}

void DamageFeedBack::StartShake(float timeSec, float amp, bool withFlash)
{
	if (timeSec > 0.0f) {
		shakeTime_ = timeSec;
	}
	if (amp > 0.0f) {
		shakeAmp_ = amp;
	}

	// シェイク開始
	shakeActive_ = true;
	shakeTimer_ = 0.0f;

	// フラッシュも欲しい場合だけ
	if (withFlash && hitSprite_) {
		hitSprite_->SetColor(flashColorOn_);
	}
}

void DamageFeedBack::BeginHit()
{
	// カメラのシェイク
	shakeActive_ = true;
	shakeTimer_ = 0.0f;

	// ダメージエフェクト
	hitSprite_->SetColor(flashColorOn_);
}

void DamageFeedBack::UpdateFlash()
{
	Vector4 currentColor = hitSprite_->GetColor();
	if (currentColor.w > 0.0f) {
		// 減算スピードは 0.01f ～ 0.05f くらいで調整
		currentColor.w = (std::max)(0.0f, currentColor.w - fadeSpeed_);
		hitSprite_->SetColor(currentColor);
	}
}

void DamageFeedBack::UpdateShake(float dt)
{
	if (!shakeActive_)return;
	if (!camera_ || !cameraTransform_) return;

	shakeTimer_ += dt;

	// 減衰
	float u = Clamp01(1.0f - (shakeTimer_ / shakeTime_));

	// ２軸(x,y)のシェイク
	float sx = std::sin(shakeTimer_ * 80.0f);
	float sy = std::sin(shakeTimer_ * 100.0f);

	Vector3 offset = { sx * shakeAmp_ * u, sy * shakeAmp_ * u, 0.0f };

	cameraTransform_->translate = baseCamPos_ + offset;
	camera_->SetTranslate(cameraTransform_->translate);

	// シェイク終了
	if (shakeTimer_ >= shakeTime_) {
		shakeTimer_ = 0.0f;
		shakeActive_ = false;
		// 最終的には元の位置に戻す
		cameraTransform_->translate = baseCamPos_;
		camera_->SetTranslate(baseCamPos_);
	}

}


void DamageFeedBack::Initialize()
{
	// ダメージ用スプライトの初期化
	hitSprite_ = std::make_unique<Sprite>();
	hitSprite_->Initialize(texturePath_);
	hitSprite_->SetSize({ 1280.0f,720.0f });
	hitSprite_->SetPosition({ 0.0f,0.0f });
	hitSprite_->SetColor(flashColorOff_);
	// 初期値にしておく
	Reset();
}

void DamageFeedBack::Update(float dt)
{
	// 各種情報が取得できていない場合は処理を通さない
	if (!player_ || !camera_ || !cameraTransform_)return;
	// エネミーに当たっているか
	const bool hitNow = player_->GetHitEnemy();
	// 多重ヒットにならないように
	if (hitNow && !wasHitPrev_) {
		BeginHit();
	}
	// フレームのヒット情報の記録
	wasHitPrev_ = hitNow;

	// ヒットスプライトの更新
	hitSprite_->Update();

	// フラッシュ減衰
	UpdateFlash();

	// シェイク更新
	UpdateShake(dt);
}

void DamageFeedBack::Draw()
{
	if (IsActive()) {
		hitSprite_->Draw();
	}
}


