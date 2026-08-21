#pragma once
#include "engine/math/MyMath.h"
#include "engine/3d/Object3D.h"
#include <memory>

class BossProjectile
{
public:
    void Initialize(Vector3 origin, Vector3 velocity = {0.0f,0.0f,-0.15f});
    void Update();
    void Draw();

    bool IsExpired() const { return expired_; }
    Vector3 GetPosition() const { return pos_; }

private:
    Vector3 pos_;
	// プレイヤーは画面手前にいるからz-方向に進むようにする
    Vector3 velocity_ = { 0.0f, 0.0f, -0.15f }; 
    std::unique_ptr<Object3D> model_;
    bool expired_ = false;

    // Z=0 より手前に来たら着弾とみなす
    static constexpr float kLandZ_ = 0.0f;
};