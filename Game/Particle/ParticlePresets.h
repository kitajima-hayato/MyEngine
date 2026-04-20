#pragma once
#include "ParticleSystem.h"
#include <functional>
#include <memory>

class ParticlePresets
{
public:
	/// <summary>
	/// プリセット設定構造体
	/// </summary>
	struct PresetConfig {
		std::string name;
		std::string texturePath;
		std::function<void(ParticleSystem&)> configure;
	};

	/// <summary>
	/// 爆発エフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateExplosion(const Vector3& position = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// 煙エフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateSmoke(const Vector3& position = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// スパークエフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateSparks(const Vector3& position = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// タイトルシーンエフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateTitleEffect(const Vector3& position = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// 魔法陣エフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateMagicCircle(const Vector3& position = { 0.0f,0.0f,0.0f });

	/// <summary>
	/// 複雑な魔法陣（多重円）
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateComplexMagicCircle(const Vector3& position = { 0, 0, 0 });

	/// <summary>
	/// 召喚魔法陣（下から上へエネルギーが上がる）
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateSummonCircle(const Vector3& position = { 0, 0, 0 });

	/// <summary>
	/// ジャンプ始まりの足元の砂埃エフェクト
	/// </summary>
	static std::unique_ptr<ParticleSystem>CreateJumpDust(const Vector3& position = { 0, 0, 0 });

	/// <summary>
	/// 着地の足元の砂埃エフェクト
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	static std::unique_ptr<ParticleSystem>CreateLandDust(const Vector3& position = { 0, 0, 0 });

	/// <summary>
	/// ジャンプブロックの上に出る矢印エフェクト
	/// </summary>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクルシステム</returns>
	static std::unique_ptr<ParticleSystem> CreateJumpBlockArrow(const Vector3& position = { 0, 0, 0 });


	/// <summary>
	/// トグルブロックが有効になった瞬間の演出
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateToggleOnBurst(const Vector3& position = { 0, 0, 0 });

	/// <summary>
	/// トグルブロックが無効になった瞬間の演出
	/// </summary>
	static std::unique_ptr<ParticleSystem> CreateToggleOffBurst(const Vector3& position = { 0, 0, 0 });
};

