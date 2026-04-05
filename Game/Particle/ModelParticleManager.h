#pragma once
#include "MyMath.h"
#include <memory>
#include <vector>
#include <random>
#include "engine/3d/Object3D.h"


struct ModelParticle {
	Transform transform;		// 変換情報
	Vector3 velocity;			// 速度
	Vector4 color;				// 色
	Vector3 angularVelocity;	// 角速度
	float lifeTime;				// 寿命
	float currentTime;			// 経過時間
	float gravity;				// 重力影響度
};

/// <summary>
/// モデルパーティクルマネージャークラス
/// </summary>
class ModelParticleManager 
{
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	static ModelParticleManager& GetInstance();



	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();



	/// <summary>
	/// ブロックの破片パーティクルを発生させる
	/// </summary>
	/// <param name="position">発生位置</param>
	/// <param name="color">破片の色</param>
	/// <param name="count">何個出すか</param>
	void EmitBlockDebris(const Vector3& position, const Vector4& color, uint32_t count = 10);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// インスタンシング描画のウォームアップ（最初の1回は重いので、先に呼んでおく）
	/// </summary>
	void WarmupDrawOnce();
private:
	// コンストラクタ
	ModelParticleManager() = default;
	// デストラクタ
	~ModelParticleManager() = default;
	// コピーコンストラクタ禁止
	ModelParticleManager(const ModelParticleManager&) = delete;
	// 代入演算子禁止
	ModelParticleManager& operator=(const ModelParticleManager&) = delete;
private:
	// パーティクルリスト
	std::vector<ModelParticle> particles_;

	// モデルの名前
	std::string modelName_ = "GamePlay/Blocks/breakblock";

	Model* model_ = nullptr;

	static constexpr uint32_t kMaxInstances = 2048;

	Microsoft::WRL::ComPtr<ID3D12Resource> instanceResource_;
	Object3DInstanceData* instanceMapped_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW instanceVBV_{};

	uint32_t instanceCount_ = 0;

	std::mt19937 rng_;
};

