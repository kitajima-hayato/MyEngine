#include "BossProjectile.h"

void BossProjectile::Initialize(Vector3 origin)
{
    pos_ = origin;

    model_ = std::make_unique<Object3D>();
    model_->Initialize();
    model_->SetModel("GamePlay/Player"); 

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