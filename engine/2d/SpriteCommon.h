#pragma once
#include "DirectXCommon.h"
/// <summary>
/// スプライト共通クラス
/// </summary>
/// シングルトンクラス
class SpriteCommon
{
public:	
	/// <summary>
	/// インスタンスの取得
	/// </summary>
	static SpriteCommon* GetInstance();
	/// <summary>
	/// インスタンスの削除
	/// </summary>
	static void DeleteInstance();


public:		// メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(Engine::DirectXCommon* dxCommon);

	/// <summary>
	/// 共通描画設定
	/// </summary>
	void DrawSettingCommon();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();
public:		//	Getter,Setter
	/// <summary>
	/// DirectXCommon取得関数
	/// </summary>
	Engine::DirectXCommon* GetDxCommon()const { return dxCommon_; }

public:	// Spriteクラスに向けたラッパー関数(wrapper function)
	/// <summary>
	/// スプライト頂点リソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateSpriteVertexResource();
	/// <summary>
	/// スプライトインデックスリソースの生成
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateSpriteIndexResource();
private:	// メンバ関数
	/// <summary>
	/// ルートシグネチャの作成
	/// </summary>
	void CreateRootSignature();

	/// <summary>
	/// グラフィックスパイプラインの生成
	/// </summary>
	void CreateGraphicsPipeLine();
	/// <summary>
	/// PSO
	/// </summary>
	void InitializePixelShaderOutput();
	

private:	// シングルトン用メンバ変数と関数
	/// <summary>
	/// インスタンス
	/// </summary>
	static SpriteCommon* instance;
	/// <summary>
	/// コンストラクタ・デストラクタ・コピー禁止
	/// </summary>
	SpriteCommon() = default;
	/// <summary>
	/// デストラクタ
	/// </summary>
	~SpriteCommon() = default;
	/// <summary>
	/// コピー禁止
	/// </summary>
	SpriteCommon(SpriteCommon&) = delete;
	/// <summary>
	/// コピー禁止
	/// </summary>
	SpriteCommon& operator=(SpriteCommon&) = delete;

private:	// メンバ変数
	// 絶対にnew,deleteしない
	Engine::DirectXCommon* dxCommon_;
	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	// 入力レイアウトの記述を設定
	D3D12_INPUT_LAYOUT_DESC inputLayoutDescs{};
	// 頂点シェーダーのコンパイル結果を格納するBlob
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = nullptr;
	// ピクセルシェーダーのコンパイル結果を格納するBlob
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = nullptr;
	// ブレンドステートの設定
	D3D12_BLEND_DESC blendDesc{};
	// ラスタライザーステートの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// ルートシグネチャ
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature = nullptr;
	// パイプラインステートの作成
	Microsoft::WRL::ComPtr <ID3D12PipelineState> graphicsPipelineState = nullptr;
	// 入力要素の定義配列を初期化
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};


private:	// メンバ変数
	Microsoft::WRL::ComPtr<ID3D12Resource>vertexResource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource>indexResource = nullptr;

};

