#include "ModelParticleManager.h"
#include <numbers>
#include "engine/3d/Object3DCommon.h"
#include "engine/3d/ModelManager.h"
#include "engine/3d/Model.h"
#include "MakeMatrix.h" 
#include "Object3DInstancingCommon.h"

ModelParticleManager& ModelParticleManager::GetInstance()
{
    static ModelParticleManager instance;
    return instance;
}

void ModelParticleManager::Initialize()
{
    model_ = ModelManager::GetInstance().FindModel(modelName_);


    // インスタンス用VBを1個だけ作る
    auto* dx = Object3DCommon::GetInstance()->GetDxCommon();
    instanceResource_ = dx->CreateBufferResource(sizeof(Object3DInstanceData) * kMaxInstances);
    instanceResource_->Map(0, nullptr, reinterpret_cast<void**>(&instanceMapped_));

    instanceVBV_.BufferLocation = instanceResource_->GetGPUVirtualAddress();
    instanceVBV_.SizeInBytes = sizeof(Object3DInstanceData) * kMaxInstances;
    instanceVBV_.StrideInBytes = sizeof(Object3DInstanceData);

    const uint32_t seed = static_cast<uint32_t>(time(nullptr));
    rng_.seed(seed);

    particles_.reserve(kMaxInstances);
}

void ModelParticleManager::EmitBlockDebris(const Vector3& position, const Vector4& color, uint32_t count)
{
#ifdef _DEBUG
    OutputDebugStringA("EmitBlockDebris called\n");
#endif

    if (!model_) {
        model_ = ModelManager::GetInstance().FindModel(modelName_);
    }

    if (!model_ || !instanceMapped_) {
#ifdef _DEBUG
        OutputDebugStringA("EmitBlockDebris failed: model_ or instanceMapped_ is null\n");
#endif
        return;
    }

    std::uniform_real_distribution<float> distSpeed(1.0f, 3.0f);
    std::uniform_real_distribution<float> distAngle(0.0f, 2.0f * std::numbers::pi_v<float>);
    std::uniform_real_distribution<float> distRotation(-5.0f, 5.0f);

    for (uint32_t i = 0; i < count; ++i)
    {
        ModelParticle p{};
        float angle = distAngle(rng_);
        float speed = distSpeed(rng_);

        p.velocity = { std::cos(angle) * speed, distSpeed(rng_), std::sin(angle) * speed };
        p.angularVelocity = { distRotation(rng_), distRotation(rng_), distRotation(rng_) };

        p.transform.scale = { 0.2f, 0.2f, 0.2f };
        p.transform.rotate = { 0.0f, 0.0f, 0.0f };
        p.transform.translate = position;

        p.color = color;

        p.lifeTime = 3.0f;
        p.currentTime = 0.0f;
        p.gravity = -15.0f;

        particles_.push_back(p);
    }
#ifdef _DEBUG
    char buffer[256];
    sprintf_s(
        buffer,
        "EmitBlockDebris pos=(%.2f, %.2f, %.2f) particles=%zu instance=%u\n",
        position.x,
        position.y,
        position.z,
        particles_.size(),
        instanceCount_
    );
    OutputDebugStringA(buffer);
#endif
}

void ModelParticleManager::Update()
{
    if (!instanceMapped_) {
        instanceCount_ = 0;
        return;
    }
    const float dt = 1.0f / 60.0f;

    for (auto it = particles_.begin(); it != particles_.end();)
    {
        if (it->currentTime >= it->lifeTime) {
            it = particles_.erase(it);
            continue;
        }

        it->transform.translate += it->velocity * dt;
        it->velocity.y += it->gravity * dt;

        it->transform.rotate.x += it->angularVelocity.x * dt;
        it->transform.rotate.y += it->angularVelocity.y * dt;
        it->transform.rotate.z += it->angularVelocity.z * dt;

        it->currentTime += dt;
        ++it;
    }

    Camera* cam = Object3DCommon::GetInstance()->GetDefaultCamera();
    const Matrix4x4& vp = cam->GetViewProjectionMatrix();

    instanceCount_ = 0;
    for (const auto& p : particles_)
    {
        if (instanceCount_ >= kMaxInstances) { break; }

        Matrix4x4 world = MakeAffineMatrix(p.transform.scale, p.transform.rotate, p.transform.translate);
        Matrix4x4 wvp = Multiply(world, vp);

        Object3DInstanceData& dst = instanceMapped_[instanceCount_++];

        // world 行列を Vector4 行に分解
        dst.world0 = { world.m[0][0], world.m[0][1], world.m[0][2], world.m[0][3] };
        dst.world1 = { world.m[1][0], world.m[1][1], world.m[1][2], world.m[1][3] };
        dst.world2 = { world.m[2][0], world.m[2][1], world.m[2][2], world.m[2][3] };
        dst.world3 = { world.m[3][0], world.m[3][1], world.m[3][2], world.m[3][3] };

        dst.wvp0 = { wvp.m[0][0], wvp.m[0][1], wvp.m[0][2], wvp.m[0][3] };
        dst.wvp1 = { wvp.m[1][0], wvp.m[1][1], wvp.m[1][2], wvp.m[1][3] };
        dst.wvp2 = { wvp.m[2][0], wvp.m[2][1], wvp.m[2][2], wvp.m[2][3] };
        dst.wvp3 = { wvp.m[3][0], wvp.m[3][1], wvp.m[3][2], wvp.m[3][3] };

        dst.color = p.color;
    }
}

void ModelParticleManager::Draw()
{
#ifdef _DEBUG
    if (instanceCount_ > 0) {
        OutputDebugStringA("ModelParticleManager Draw instance\n");
    }
#endif

    if (!model_ || instanceCount_ == 0) { return; }

    Object3DInstancingCommon::GetInstance()->DrawSettingCommon();

    auto* cmd = Object3DInstancingCommon::GetInstance()->GetDxCommon()->GetCommandList();
    cmd->IASetVertexBuffers(1, 1, &instanceVBV_);

    model_->DrawInstanced(instanceCount_);
}

void ModelParticleManager::Finalize()
{
    particles_.clear();

    if (instanceResource_) {
        instanceResource_->Unmap(0, nullptr);
        instanceMapped_ = nullptr;
        instanceResource_.Reset();
    }
    model_ = nullptr;
    instanceCount_ = 0;
}

void ModelParticleManager::Reset()
{
    particles_.clear();
    instanceCount_ = 0;

    if (!model_) {
        model_ = ModelManager::GetInstance().FindModel(modelName_);
    }
}

void ModelParticleManager::WarmupDrawOnce()
{
	//このメソッドは最初の一回は重いので、先に呼んでおくためのもの　空回し
    if (!model_ || !instanceMapped_) { return; }

    // 画面外に1インスタンスだけ詰める / 画面外に発生させる
    // 最初の一回は重いので先に呼んでおく
    Transform t{};
    t.scale = { 0.2f, 0.2f, 0.2f };
    t.rotate = { 0.0f, 0.0f, 0.0f };
    t.translate = { 100000.0f, 100000.0f, 100000.0f };

    Camera* cam = Object3DCommon::GetInstance()->GetDefaultCamera();
    const Matrix4x4& vp = cam->GetViewProjectionMatrix();

    Matrix4x4 world = MakeAffineMatrix(t.scale, t.rotate, t.translate);
    Matrix4x4 wvp = Multiply(world, vp);

    instanceCount_ = 1;
    Object3DInstanceData& dst = instanceMapped_[0];

    dst.world0 = { world.m[0][0], world.m[0][1], world.m[0][2], world.m[0][3] };
    dst.world1 = { world.m[1][0], world.m[1][1], world.m[1][2], world.m[1][3] };
    dst.world2 = { world.m[2][0], world.m[2][1], world.m[2][2], world.m[2][3] };
    dst.world3 = { world.m[3][0], world.m[3][1], world.m[3][2], world.m[3][3] };

    dst.wvp0 = { wvp.m[0][0], wvp.m[0][1], wvp.m[0][2], wvp.m[0][3] };
    dst.wvp1 = { wvp.m[1][0], wvp.m[1][1], wvp.m[1][2], wvp.m[1][3] };
    dst.wvp2 = { wvp.m[2][0], wvp.m[2][1], wvp.m[2][2], wvp.m[2][3] };
    dst.wvp3 = { wvp.m[3][0], wvp.m[3][1], wvp.m[3][2], wvp.m[3][3] };

    dst.color = { 1.0f, 1.0f, 1.0f, 1.0f };

    Draw();

    instanceCount_ = 0;
}