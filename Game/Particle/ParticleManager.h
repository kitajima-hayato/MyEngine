#pragma once
#include "DirectXCommon.h"
#include "SrvManager.h"
#include "MyMath.h"
#include <random>
#include "Game/Camera/Camera.h"


/// <summary>
/// パーティクルマネージャー
/// </summary>
/// シングルトンクラス

class ParticleManager
{
public:

	// パーティクル構造体
	struct ParticleGroup {		// パーティクルグループ // 使用するテクスチャごとにパーティクルグループとしてまとめる
		MaterialData materialData;			// マテリアルデータ					
		std::list<Particle> particles;		// パーティクルのリスト		
		uint32_t instancingSrvIndex;					// インスタンシングデータ用のSRVインデックス	
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;	// インスタンシングデータ用のリソース
		UINT kNumInstance;					// インスタンス数
		ParticleForGPU* instancingData;		// インスタンシングデータを書き込むためのポインタ
		uint32_t textureSrvIndex;               // テクスチャインデックス
	};

	// エフェクトタイプ列挙型
	enum class EffectType {
		Default,		// 通常エフェクト
		Explosion,		// 爆発
		Smoke,			// 煙
		Spark,			// 火花
		MagicCircle,	// 魔法陣
		JumpDust,		// ジャンプダスト
		LandDust,		// 着地ダスト
		UpArrow,		// 上向きの矢印
	};

	/// <summary>
	/// インスタンス取得
	/// </summary>
	static ParticleManager* GetInstance();
	/// <summary>
	/// インスタンス削除
	/// </summary>
	static void DeleteInstance();

	/// <summary>
	/// 明示的な終了処理
	/// </summary>
	void Finalize();
private:
	// シングルトンパターン
	static ParticleManager* instance;
	// コンストラクタ・デストラクタ・コピー禁止
	ParticleManager() = default;
	// デストラクタ
	~ParticleManager();
	// コピーコンストラクタ・代入演算子を削除
	ParticleManager(ParticleManager&) = delete;
	// 代入演算子
	ParticleManager& operator=(ParticleManager&) = delete;


	// 初期化内部処理
	/// <summary>
	/// ランダムエンジンの初期化
	/// </summary>
	void InitializeRandomEngine();
	/// <summary>
	/// パイプラインの生成
	/// </summary>
	void CreatePipeline();
	/// <summary>
	/// 頂点データの初期化(座標等)
	/// </summary>
	void InitializeVertexData();
	/// <summary>
	/// バッファービューの作成
	/// </summary>
	void CreateVertexBufferView();
	/// <summary>
	/// マテリアルの初期化
	/// </summary>
	void InitializeMaterial();

	// パイプライン生成内部
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignature();
	/// <summary>
	/// グラフィックスパイプラインの設定
	/// </summary>
	void SetGraphicsPipeline();
	/// <summary>
	/// ブレンドモードの設定
	/// </summary>
	/// <param name="desc"></param>
	/// <param name="mode"></param>
	void SetBlendMode(D3D12_BLEND_DESC& desc, BlendMode mode);


	/// <summary>
	/// タイプ別パーティクル生成
	/// </summary>
	/// <param name="randomEngine"></param>
	/// <param name="position"></param>
	/// <param name="type"></param>
	/// <returns></returns>
	Particle MakeParticleByType(std::mt19937& randomEngine, const Vector3& position, EffectType type);

	/// <summary>
	/// タイプ別パーティクル生成（色指定版）（追加）
	/// </summary>
	Particle MakeParticleByTypeWithColor(std::mt19937& randomEngine,const Vector3& position,EffectType type,const Vector4& colorTint);


public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	/// <param name="srvManager"></param>
	/// <param name="camera"></param>
	void Initialize(Engine::DirectXCommon* dxCommon, SrvManager* srvManager, Camera* camera);


	/// <summary>
	///  パーティクルグループの作成
	/// </summary>
	/// <param name="name"></param>
	/// <param name="textureFilePath"></param>
	void CreateParticleGroup(const std::string& name, const std::string textureFilePath);


	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	// 更新内部処理
	/// <summary>
	/// 行列更新
	/// </summary>
	void UpdateMatrix();
	/// <summary>
	/// パーティクル更新
	/// </summary>
	void UpdateParticle();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// パーティクルグループの削除
	/// </summary>
	/// <param name="name"></param>
	void DeleteParticleGroup(const std::string& name);

	/// <summary>
	/// エフェクトの発生
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	/// <param name="count"></param>
	void Emit(const std::string& name, const Vector3& position, uint32_t count);
	/// <summary>
	/// エフェクトの発生(特殊エフェクト)
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	/// <param name="count"></param>
	void EffectEmit(const std::string& name, const Vector3& position, uint32_t count);


	/// <summary>
	/// パーティクル(通常)生成
	/// </summary>
	Particle MakeParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// プリミティブエフェクト生成
	/// </summary>
	Particle MakePrimitiveEffect(std::mt19937& randomEngine, const Vector3& translate);

	/// <summary>
	/// 爆発パーティクル生成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクルを返す</returns>
	Particle MakeExplosionParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// 煙パーティクル生成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクルを返す</returns>
	Particle MakeSmokeParticle(std::mt19937& randomEngine, const Vector3& position);
	
	/// <summary>
	/// 火花パーティクル生成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクルを返す</returns>
	Particle MakeSparkParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// 魔法陣パーティクル生成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">発生位置</param>
	/// <param name="angle">角度</param>
	/// <param name="radius">半径</param>
	/// <returns>パーティクルを返す</returns>
	Particle MakeMagicCircleParticle(std::mt19937& randomEngine,const Vector3& position,float angle,float radius);

	/// <summary>
	/// ジャンプダストパーティクル生成
	/// </summary>
	Particle MakeJumpDustParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// 着地ダストパーティクル生成
	/// </summary>
	/// <param name="randomEngine">乱数</param>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクル情報</returns>
	Particle MakeLandDustParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// 上向きの矢印パーティクル生成
	/// </summary>
	/// <param name="randomEngine">ランダムエンジン</param>
	/// <param name="position">発生位置</param>
	/// <returns>パーティクル情報</returns>
	Particle MakeUpArrowParticle(std::mt19937& randomEngine, const Vector3& position);

	/// <summary>
	/// Ringエフェクト
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	Particle MakeRingEffect(const Vector3& position);
	/// <summary>
	/// Cylinderエフェクト
	/// </summary>
	/// <param name="position"></param>
	/// <returns></returns>
	Particle MakeCylinderEffect(const Vector3& position);

	

	/// <summary>
	/// Ring
	/// </summary>
	void CreateRingVertex();
	/// <summary>
	/// RingEffectの描画
	/// </summary>
	void DrawRing();

	/// <summary>
	/// Cylinder
	/// </summary>
	void CreateCylinderVertex();
	/// <summary>
	/// CylinderEffectの描画
	/// </summary>
	void DrawCylinder();

	/// <summary>
	/// ImGui描画
	/// </summary>
	void DrawImgui();

	/// <summary>
	/// カメラのポインタをセットする
	/// </summary>
	/// <param name="cameraPtr_"></param>
	void SetCamera(Camera* cameraPtr_) { camera = cameraPtr_; }

	/// <summary>
	/// パーティクルグループが存在するか
	/// </summary>
	/// <param name="name">存在を確認するグループの名前</param>
	/// <returns>存在すればtrue / 存在しなければfalse</returns>
	bool HasParticleGroup(const std::string& name) const { return particleGroups.contains(name); }

	/// <summary>
	/// パーティクルグループが存在しなければ作成する
	/// </summary>
	/// <param name="name"></param>
	/// <param name="textureFilePath"></param>
	void EnsureParticleGroup(const std::string& name, const std::string& textureFilePath);
	
	/// <summary>
	/// エフェクトの発生(特殊エフェクト)
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	/// <param name="count"></param>
	/// <param name="effectType"></param>
	void EmitWithEffectType(const std::string& name, const Vector3& position, uint32_t count, EffectType effectType);

	/// <summary>
	/// 魔法陣エフェクトの発生
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	/// <param name="radius"></param>
	void EmitMagicCircle(const std::string& name, const Vector3& position,uint32_t count, float radius);

	/// <summary>
	/// 複雑な魔法陣エフェクトの発生
	/// </summary>
	/// <param name="name"></param>
	/// <param name="position"></param>
	void EmitComplexMagicCircle(const std::string& name,const Vector3& position);

	/// <summary>
	/// エフェクトの発生(特殊エフェクト) / 色指定バージョン
	/// </summary>
	/// <param name="name">パーティクルグループ名</param>
	/// <param name="position">発生位置</param>
	/// <param name="count">発生数</param>
	/// <param name="effectType">エフェクトタイプ</param>
	/// <param name="color">色の調整</param>
	void EmitWithEffectTypeAndColor(const std::string& name, const Vector3& position, uint32_t count, EffectType effectType, const Vector4& color);

private:
	// DirectXCommon
	Engine::DirectXCommon* dxCommon;
	// SRVマネージャー
	SrvManager* srvManager;
	// 頂点データ
	VertexData* vertexData = nullptr;
	// カメラ
	Camera* camera = nullptr;

	// ランダムエンジン
	std::mt19937 randomEngine;


	// ルートパラメーター
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	// サンプラー
	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	// ルートシグネチャ
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	// インプットレイアウト
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3];
	// パイプラインステート

	// インプットレイアウト
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};

	// ブレンド
	BlendMode blendMode;
	BlendMode currentBlendMode;
	D3D12_BLEND_DESC blendDesc{};
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// グラフィックスパイプライン
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc;
	// シェーダーバイナリ
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob;

	// パーティクルの発生上限
	static const int kMaxParticle = 1000;

	// 頂点バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = nullptr;

	// パーティクルデータ
	// パーティクルのモデルデータ
	ModelData modelData;
	// バッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	// マテリアルリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = nullptr;
	// マテリアルデータ
	Material* materialData = nullptr;
	// パーティクルグループ / グループ名をキーにしてパーティクルグループを管理
	std::unordered_map<std::string, ParticleGroup> particleGroups;

	// カメラ行列
	Matrix4x4 cameraMatrix;
	// ワールドビュープロジェクション行列
	Matrix4x4 viewMatrix;
	// プロジェクション行列
	Matrix4x4 projectionMatrix;
	// ビューポート行列
	Matrix4x4 backToFrontMatrix;
	// ビルボード行列
	Matrix4x4 billboardMatrix;
	bool useBillboard = false;

	// パーティクルのリスト
	std::list<Particle>particles;

	// Δtを定義６０fos固定
	const float kDeltaTime = 1.0f / 60.0f;

	// リングエフェクト
	Microsoft::WRL::ComPtr<ID3D12Resource> ringVertexBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW ringVertexBufferView{};
	uint32_t ringVertexCount = 0;

	// UV座標変更
	Vector2 uvOffset = { 0.0f, 0.0f };
	Vector2 uvScrollSpeed = { 0.0f, 0.0f };  // 水平方向に流す場合


};

