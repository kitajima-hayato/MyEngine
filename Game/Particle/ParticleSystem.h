#pragma once
#include "ParticleManager.h"
using namespace std;
/// <summary>
/// パーティクルエミッタクラス
/// </summary>
class ParticleSystem
{
public:
	// パーティクルシステムの状態
	enum class State {
		Playing, // 再生中
		Stopped, // 停止中
		Paused	 // 一時停止中
	};

	// エミッション設定
	struct EmissionModule {
		bool enabled;        // エミッションの有効・無効
		float rateOverTime; // 時間あたりの発生率
	};

	// メイン設定
	struct MainModule {
		float duration;      // 再生時間
		bool loop;          // ループ再生の有無
		float startLifetime; // パーティクルの寿命
		Vector3 startSpeed;  // パーティクルの初速度
		Vector4 startColor;  // パーティクルの初期色
	};

	/// <summary>
	/// 魔法陣パラメータ
	/// </summary>
	struct MagicCircleParams {
		float radius = 5.0f;        // 半径
		uint32_t particleCount = 32; // 円周上のパーティクル数
		float rotationSpeed = 1.0f;  // 回転速度
		bool multiLayer = false;     // 多重円かどうか
	};

	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	ParticleSystem();
	~ParticleSystem();

	/// <summary>
	/// パーティクルシステム生成
	/// </summary>
	/// <param name="particleName">パーティクルの名前</param>
	/// <param name="texturePath">使用する画像ファイルの名前</param>
	/// <returns></returns>
	static std::unique_ptr<ParticleSystem>Create(const std::string& particleName, const std::string& texturePath);
	/// <summary>
	/// ワンショット再生
	/// </summary>
	/// <param name="particleName">パーティクルの名前</param>
	/// <param name="transform">発生させる場所</param>
	/// <param name="count">発生させる数</param>
	static void PlayOneShot(const std::string& particleName, const Transform& transform, uint32_t count = 10);

	/// <summary>
	/// 再生
	/// </summary>
	void Play();

	/// <summary>
	/// 停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 一時停止
	/// </summary>
	void Pause();


	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// パーティクル発生
	/// </summary>
	void Emit(uint32_t count);




public:	// Getter / Setter


	/// <summary>
	/// Particleの発生位置設定・取得
	/// </summary>
	/// <param name="translate"></param>
	void SetTranslate(const Vector3& translate) { this->transform.translate = translate; }
	Vector3 GetTranslate()const { return transform.translate; }

	/// <summary>
	/// Particleの回転設定・取得
	/// </summary>
	/// <param name="rotate"></param>
	void SetRotate(const Vector3& rotate) { this->transform.rotate = rotate; }
	Vector3 GetRotate()const { return transform.rotate; }

	/// <summary>
	/// Particleのスケール設定・取得
	/// </summary>
	/// <param name="scale"></param>
	void SetScale(const Vector3& scale) { this->transform.scale = scale; }
	Vector3 GetScale()const { return transform.scale; }

	/// <summary>
	/// エミッタのTransform設定・取得
	/// </summary>
	/// <param name="transform">セットするTransform</param>
	void SetTransform(const Transform& transform) { this->transform = transform; }
	Transform GetTransform()const { return transform; }

	/// <summary>
	/// パーティクルシステムの状態取得
	/// </summary>
	/// <returns>状態</returns>
	bool IsPlaying()const { return state == State::Playing; }
	bool IsStopped()const { return state == State::Stopped; }
	bool IsPaused()const { return state == State::Paused; }

	/// <summary>
	/// エミッションレート設定・取得（１秒当たりの発生数）
	/// </summary>
	/// <param name="rate">どのくらいの頻度にするか</param>
	void SetEmissionRate(float rate) {
		emission.rateOverTime = rate;
		emitterData.frequency = 1.0f / rate;
		emitterData.count = static_cast<uint32_t>(rate / 10.0f);
		if (emitterData.count < 1)emitterData.count = 1;
	};
	float GetEmissionRate()const {
		return emission.rateOverTime;
	}

	/// <summary>
	/// ループ設定・取得
	/// </summary>
	/// <param name="loop">ループさせるかのフラグ</param>
	void SetLoop(bool loop) { main.loop = loop; }
	bool GetLoop()const { return main.loop; }

	/// <summary>
	/// 発生させるパーティクルの名前の設定・取得
	/// </summary>
	/// <param name="name"></param>
	void SetParticleName(const string& name) { particleName = name; }
	std::string GetParticleName()const { return particleName; }


	/// <summary>
	/// エフェクトタイプの設定・取得
	/// </summary>
	/// <param name="type">エフェクトのタイプ</param>
	void SetEffectType(ParticleManager::EffectType type) {
		// エフェクトタイプの設定
		effectType = type;
	}
	ParticleManager::EffectType GetEffectType()const { return effectType; }

	/// <summary>
	/// 魔法陣パラメータ設定
	/// </summary>
	/// <param name="params"></param>
	void SetMagicCircleParams(const MagicCircleParams& params) { magicCircleParams = params; }

	/// <summary>
	/// 色のオーバーライド設定
	/// </summary>
	/// <param name="color">変更する色</param>
	void SetColor(const Vector4& color) {
		colorOverride = color;
		useColorOverride = true;
	}
	/// <summary>
	/// 色のオーバーライドをクリア
	/// </summary>
	void ClearColorOverride() {
		useColorOverride = false;
	}
	/// <summary>
	/// 色のオーバーライドが有効かどうか
	/// </summary>
	/// <returns>有効か無効か</returns>
	bool IsColorOverrideEnabled()const { return useColorOverride; }

	/// <summary>
	/// 色のオーバーライド取得
	/// </summary>
	/// <returns>現在のカラーの取得</returns>
	Vector4 GetColorOverride()const { return colorOverride; }

	/// <summary>
	/// エミッション頻度の設定・取得
	/// </summary>
	/// <param name="count"></param>
	void SetBurstCount(uint32_t count) { emitterData.count = (std::max)(1u, count); }

public:	// モジュールへのアクセス
	EmissionModule& GetEmissionModule() { return emission; }
	MainModule& GetMainModule() { return main; }

private:
	/// <summary>
	/// パーティクル発生内部処理
	/// </summary>
	void EmitInternal();

private:
	// パーティクルネーム
	std::string particleName;

	// トランスフォーム
	Transform transform;

	// 状態
	State state = State::Stopped;

	// タイマー
	float currentTime = 0.0f;
	float emissionTimer = 0.0f;

	// モジュール
	EmissionModule emission;
	MainModule main;

	// エミッターデータ（内部用）
	struct EmitterData {
		uint32_t count = 1;
		float frequency = 1.0f;
	}emitterData;

	// １フレームの時間
	const float kDeltaTime = 1.0f / 60.0f;

	ParticleManager::EffectType effectType = ParticleManager::EffectType::Default;
	// 魔法陣エフェクト用のパラメータ
	MagicCircleParams magicCircleParams;

	// 色のオーバーライド
	Vector4 colorOverride = { 1.0f, 1.0f, 1.0f, 1.0f };
	// 色のオーバーライドを使用するかどうか
	bool useColorOverride = false;

	// モデルの向きの補正値
	const float modelYawOffset = -1.570796326f;

	// 移動にかかる時間
	float moveDuration = 0.35f;
	// 向き変更にかかる時間
	float faceDuration = 0.25f;

};
