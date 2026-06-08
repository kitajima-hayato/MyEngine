#include "ParticlePresets.h"

std::unique_ptr<ParticleSystem> ParticlePresets::CreateExplosion(const Vector3& position)
{
	// パーティクルシステムの生成
	auto particleSystem = ParticleSystem::Create("Explosion", "resources/Particle/circle.dds");
	// 位置の設定
	particleSystem->SetTranslate(position);
	// エミッション設定 / 描画される粒子の数
	particleSystem->SetEmissionRate(50.0f);
	// メイン設定 / ループ再生の有無、寿命など
	particleSystem->SetLoop(false);
	particleSystem->GetMainModule().duration = 0.5f;
	particleSystem->GetMainModule().startLifetime = 1.0f;

	// 爆発タイプを設定
	particleSystem->SetEffectType(ParticleManager::EffectType::Explosion);
	return particleSystem;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateSmoke(const Vector3& position)
{
	auto particleSystem = ParticleSystem::Create(
		"Smoke", "resources/Effects/Particles/Textures/smoke2.png",BlendMode::kBlendModeNormal);
	// 位置の設定
	particleSystem->SetTranslate(position);
	// エミッション設定 / 描画される粒子の数
	particleSystem->SetEmissionRate(15.0f);
	// メイン設定 / ループ再生の有無、寿命など
	particleSystem->SetLoop(true);
	particleSystem->GetMainModule().duration = 0.2f;
	particleSystem->GetMainModule().startLifetime = 0.8f;

	// 煙タイプを設定
	particleSystem->SetEffectType(ParticleManager::EffectType::Smoke);
	return particleSystem;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateSparks(const Vector3& position)
{
	auto particleSystem = ParticleSystem::Create(
		"Sparks", "resources/Effects/Particles/Textures/star.png",BlendMode::kBlendModeNormal);
	// 位置の設定
	particleSystem->SetTranslate(position);
	// エミッション設定 / 描画される粒子の数
	particleSystem->SetEmissionRate(1.0f);
	// メイン設定 / ループ再生の有無、寿命など
	particleSystem->SetLoop(false);
	particleSystem->GetMainModule().duration = 0.15f;
	particleSystem->GetMainModule().startLifetime = 0.25f;
	// 白デフォルト
	particleSystem->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	// スパークタイプを設定
	particleSystem->SetEffectType(ParticleManager::EffectType::Spark);
	return particleSystem;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateTitleEffect(const Vector3& position)
{
	auto particleSystem = ParticleSystem::Create("TitleEffect", 
		"resources/Effects/Particles/Textures/circle.dds");
	// 位置の設定
	particleSystem->SetTranslate(position);
	// エミッション設定 / 描画される粒子の数
	particleSystem->SetEmissionRate(5.0f);
	// メイン設定 / ループ再生の有無
	particleSystem->SetLoop(true);

	// タイトルシーンエフェクトタイプを設定
	particleSystem->SetEffectType(ParticleManager::EffectType::Default);
	return particleSystem;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateMagicCircle(const Vector3& position)
{
	auto ps = ParticleSystem::Create("MagicCircle", 
		"resources/Effects/Particles/Textures/circle.dds");
	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::MagicCircle);

	// 魔法陣パラメータ設定
	ParticleSystem::MagicCircleParams params;
	params.radius = 5.0f;
	params.particleCount = 32;
	params.rotationSpeed = 1.0f;
	params.multiLayer = false;
	ps->SetMagicCircleParams(params);

	ps->SetLoop(true);
	ps->SetEmissionRate(10.0f);  // ゆっくり追加

	return ps;
}


std::unique_ptr<ParticleSystem> ParticlePresets::CreateComplexMagicCircle(const Vector3& position)
{
	auto ps = ParticleSystem::Create("ComplexMagicCircle", 
		"resources/Effects/Particles/Textures/circle.dds");
	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::MagicCircle);

	// 多重円の設定
	ParticleSystem::MagicCircleParams params;
	params.radius = 6.0f;
	params.particleCount = 40;
	params.rotationSpeed = 0.5f;
	params.multiLayer = true;
	ps->SetMagicCircleParams(params);

	ps->SetLoop(true);
	ps->SetEmissionRate(5.0f);

	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateSummonCircle(const Vector3& position)
{
	auto ps = ParticleSystem::Create("SummonCircle", 
		"resources/Effects/Particles/Textures/circle.dds");
	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::MagicCircle);

	ParticleSystem::MagicCircleParams params;
	params.radius = 4.0f;
	params.particleCount = 24;
	params.rotationSpeed = 2.0f;  // 速く回転
	params.multiLayer = true;
	ps->SetMagicCircleParams(params);

	ps->SetLoop(false);
	ps->GetMainModule().duration = 2.0f;  // 2秒間だけ

	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateJumpDust(const Vector3& position)
{
	auto ps = ParticleSystem::Create("JumpDust",
		"resources/Effects/Particles/Textures/smoke2.png", BlendMode::kBlendModeNormal);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::JumpDust);

	ps->SetLoop(false);

	// 短い間だけ、1回だけ出すイメージにする
	ps->SetEmissionRate(10.0f);       
	ps->SetBurstCount(10);             

	ps->GetMainModule().duration = 0.12f;
	ps->GetMainModule().startLifetime = 0.25f;

	// 砂色（少し薄め）
	//ps->SetColor(Vector4(0.78f, 0.70f, 0.52f, 0.80f));
	ps->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateLandDust(const Vector3& position)
{
	auto ps = ParticleSystem::Create("LandDust",
		"resources/Effects/Particles/Textures/smoke2.png",BlendMode::kBlendModeNormal);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::LandDust);

	ps->SetLoop(false);

	ps->SetEmissionRate(20.0f);
	ps->SetBurstCount(6);              

	ps->GetMainModule().duration = 0.12f;
	ps->GetMainModule().startLifetime = 0.20f;

	// 色は最後の微調整
	ps->SetColor(Vector4(1.0f, 1.0f, 1.0f, 0.95f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateJumpBlockArrow(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"JumpBlockArrow",
		"resources/Effects/Particles/Textures/upArrow.png", BlendMode::kBlendModeAdd);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::UpArrow);

	// jumpBlock 上にいる間だけ見せたいのでループ
	ps->SetLoop(true);

	// ほどよい頻度
	ps->SetEmissionRate(12.0f);

	// 色を少し強調
	ps->SetColor(Vector4(1.8f, 0.45f, 0.45f, 1.0f));
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateToggleOnBurst(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"ToggleOnBurst",
		"resources/Effects/Particles/Textures/square.png");

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::Spark);

	ps->SetLoop(false);
	ps->SetEmissionRate(35.0f);
	ps->SetBurstCount(8);

	ps->GetMainModule().duration = 0.10f;
	ps->GetMainModule().startLifetime = 0.20f;

	// 少し緑寄り
	ps->SetColor(Vector4(0.45f, 1.2f, 0.45f, 1.0f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateToggleOffBurst(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"ToggleOffBurst",
		"resources/Effects/Particles/Textures/wind.dds");

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::Smoke);

	ps->SetLoop(false);
	ps->SetEmissionRate(20.0f);
	ps->SetBurstCount(6);

	ps->GetMainModule().duration = 0.10f;
	ps->GetMainModule().startLifetime = 0.18f;

	// 少し赤寄り
	ps->SetColor(Vector4(1.1f, 0.45f, 0.45f, 0.85f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateFireSparkCharge(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"FireSparkCharge",
		"resources/Effects/Particles/Textures/square.png",
		BlendMode::kBlendModeAdd
	);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::FireSpark);

	// 長押し中に出続ける
	ps->SetLoop(true);

	// チャージ中の発生量
	ps->SetEmissionRate(45.0f);
	ps->SetBurstCount(4);

	ps->GetMainModule().duration = 1.0f;
	ps->GetMainModule().startLifetime = 0.15f;

	ps->SetColor(Vector4(1.4f, 0.75f, 0.25f, 1.0f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateFireSparkBurst(const Vector3& position)
{
	// チャージが完了して放つ火花のバーストエフェクト
	auto ps = ParticleSystem::Create(
		"FireSparkBurst",
		"resources/Effects/Particles/Textures/square.png",
		BlendMode::kBlendModeAdd
	);
	// 発生位置の設定
	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::FireSpark);

	// 一瞬だけ出す
	ps->SetLoop(false);

	// 一瞬に多めに出す
	ps->SetEmissionRate(120.0f);
	ps->SetBurstCount(14);
	// 寿命は短め
	ps->GetMainModule().duration = 0.10f;
	ps->GetMainModule().startLifetime = 0.14f;
	// 色はチャージ中よりさらに強調
	ps->SetColor(Vector4(1.6f, 0.8f, 0.25f, 1.0f));

	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateSpecialDashBurst(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"SpecialDashBurst",
		"resources/Effects/Particles/Textures/circle.dds",
		BlendMode::kBlendModeAdd
	);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::SpecialDashBurst);

	// 発動時だけ
	ps->SetLoop(false);

	// 一瞬に多めに出す
	ps->SetEmissionRate(90.0f);
	ps->SetBurstCount(16);

	ps->GetMainModule().duration = 0.12f;
	ps->GetMainModule().startLifetime = 0.20f;

	// オレンジ寄り
	ps->SetColor(Vector4(1.5f, 0.75f, 0.25f, 1.0f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateAttackChargeEnergy(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"AttackChargeEnergy",
		"resources/Effects/Particles/Textures/circle.dds",
		BlendMode::kBlendModeAdd
	);

	ps->SetTranslate(position);
	ps->SetEffectType(ParticleManager::EffectType::AttackChargeEnergy);

	// チャージ中に出続ける
	ps->SetLoop(true);

	// 発生量
	ps->SetEmissionRate(35.0f);
	ps->SetBurstCount(3);

	ps->GetMainModule().duration = 1.0f;
	ps->GetMainModule().startLifetime = 0.25f;

	// 黄色寄りのエネルギー
	ps->SetColor(Vector4(1.2f, 1.0f, 0.55f, 1.0f));

	ps->Pause();
	return ps;
}

std::unique_ptr<ParticleSystem> ParticlePresets::CreateAttackReleaseEnergy(const Vector3& position)
{
	auto ps = ParticleSystem::Create(
		"AttackReleaseEnergy",
		"resources/Effects/Particles/Textures/circle.png",
		BlendMode::kBlendModeAdd
	);

	ps->SetTranslate(position);

	// 初期値は右向き。Player側で向きに応じてRight/Leftを切り替える
	ps->SetEffectType(ParticleManager::EffectType::AttackReleaseEnergyRight);

	// 攻撃発動時だけ出す
	ps->SetLoop(false);

	// 衝撃波の弧を見せるため少し多めに出す
	ps->SetEmissionRate(220.0f);
	ps->SetBurstCount(36);

	// 短く強く
	ps->GetMainModule().duration = 0.08f;
	ps->GetMainModule().startLifetime = 0.18f;

	// 青白い衝撃波
	ps->SetColor(Vector4(0.75f, 1.0f, 1.45f, 1.0f));

	ps->Pause();
	return ps;
}