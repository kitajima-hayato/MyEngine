#include "BossProjectile.h"

void BossProjectile::Initialize(Vector3 origin, Vector3 velocity)
{
	// 初期位置と速度を設定
    pos_ = origin;
	velocity_ = velocity;

	// モデルの初期化
    model_ = std::make_unique<Object3D>();
    model_->Initialize();
    model_->SetModel("GamePlay/Player");

	// モデルのトランスフォームを設定
    Transform t;
    t.translate = pos_;
    t.scale = { 0.5f, 0.5f, 0.5f };
    model_->SetTransform(t);
}

void BossProjectile::Update()
{
    if (expired_) return;

    pos_ += velocity_;

    if (pos_.z <= kLandZ_) {
        expired_ = true;
        return;
    }

    Transform t;
    t.translate = pos_;
    t.scale = { 0.5f, 0.5f, 0.5f };
    model_->SetTransform(t);
    model_->Update();
}

void BossProjectile::Draw()
{
    if (!expired_ && model_) model_->Draw();
}