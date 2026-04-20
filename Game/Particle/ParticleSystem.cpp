#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
	: currentTime(0.0f)
	, emissionTimer(0.0f)
	, state(State::Stopped)
{
	// SRT 初期化
	transform.translate = { 0.0f, 0.0f, 0.0f };
	transform.rotate = { 0.0f, 0.0f, 0.0f };
	transform.scale = { 1.0f, 1.0f, 1.0f };

	// エミッターデータ初期化
	emitterData.count = 1;
	emitterData.frequency = 1.0f;

	// エミッションモジュール初期化
	emission.enabled = true;
	emission.rateOverTime = 10.0f; // 1秒あたり10個発

	// メインモジュール初期化
	main.duration = 5.0f;         // 再生時間5秒
	main.loop = true;            // ループ再生
	main.startLifetime = 1.0f;   // パーティクルの寿命3秒
	main.startSpeed = { 0.0f, 0.0f, 0.0f };
	main.startColor = { 1.0f, 1.0f, 1.0f, 1.0f }; // 白色
}

ParticleSystem::~ParticleSystem()
{

}

std::unique_ptr<ParticleSystem> ParticleSystem::Create(
	const std::string& particleName,
	const std::string& texturePath,
	BlendMode blendMode)
{
	// パーティクルグループの自動生成
	auto manager = ParticleManager::GetInstance();
	if (!manager->HasParticleGroup(particleName)) {
		manager->CreateParticleGroup(particleName, texturePath, blendMode);
	}
	// パーティクルシステムの生成
	auto particleSystem = std::make_unique<ParticleSystem>();
	particleSystem->SetParticleName(particleName);

	return particleSystem;
}

void ParticleSystem::PlayOneShot(const std::string& particleName, const Transform& transform, uint32_t count)
{
	ParticleManager::GetInstance()->Emit(particleName, transform.translate, count);
}

void ParticleSystem::Play()
{
	// 再生状態に設定
	state = State::Playing;
	currentTime = 0.0f;
	emissionTimer = 0.0f;
}

void ParticleSystem::Stop()
{
	// 停止状態に設定
	state = State::Stopped;
	currentTime = 0.0f;
	emissionTimer = 0.0f;
}

void ParticleSystem::Pause()
{
	// 一時停止状態に設定
	state = State::Paused;
}


void ParticleSystem::Update()
{
	// 停止中または一時停止中は更新しない
	if (state != State::Playing) return;

	// パーティクルの名前が設定されていない場合は更新しない
	if (particleName.empty()) return;

	// エミッションが無効なら更新しない
	if (!emission.enabled) return;

	// 経過時間を加算
	currentTime += kDeltaTime;
	emissionTimer += kDeltaTime;

	// ループしない場合、再生時間を超えたら停止
	if (!main.loop && currentTime >= main.duration) {
		Stop();
		return;
	}

	// エミッション頻度に基づいて発生
	if (emissionTimer >= emitterData.frequency) {
		EmitInternal();
		emissionTimer = 0.0f; // タイマーリセット
	}
}



void ParticleSystem::Emit(uint32_t count)
{
	// パーティクルを発生させる
	ParticleManager::GetInstance()->Emit(particleName, transform.translate, count);
}



void ParticleSystem::EmitInternal()
{
	if (effectType == ParticleManager::EffectType::MagicCircle) {
		if (magicCircleParams.multiLayer) {
			// 複雑な魔法陣
			ParticleManager::GetInstance()->EmitComplexMagicCircle(
				particleName,
				transform.translate
			);
		} else {
			// シンプルな魔法陣
			ParticleManager::GetInstance()->EmitMagicCircle(
				particleName,
				transform.translate,
				magicCircleParams.particleCount,
				magicCircleParams.radius
			);
		}
	} else {
		// 通常のエフェクト
			// 色のオーバーライドがある場合は色付きで発生（修正）
		if (useColorOverride) {
			ParticleManager::GetInstance()->EmitWithEffectTypeAndColor(
				particleName,
				transform.translate,
				emitterData.count,
				effectType,
				colorOverride  
			);
		} else {
			ParticleManager::GetInstance()->EmitWithEffectType(
				particleName,
				transform.translate,
				emitterData.count,
				effectType
			);
		}
	}
}

