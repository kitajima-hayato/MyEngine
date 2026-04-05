#pragma once
#include "ModelCommon.h"
#include "Math.h"
#include "TextureManager.h"
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <wrl.h>
#include <d3d12.h>
#include <numbers>
#include "MakeMatrix.h"
#include "MyMath.h"
using namespace std;
/// <summary>
/// 3Dモデルクラス
/// </summary>
class Model
{
public: // メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="directory path">モデルのファイル名</param>
	/// <param name="filename">モデルの名前</param>
	void Initialize(ModelCommon* modelCommon,const string& directorypath,const string&filename);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// OBJファイルの読み込み
	/// </summary>
	/// <param name="directoryPath"></param>
	/// <param name="filename"></param>
	/// <returns></returns>
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);

	/// <summary>
	/// インスタンシング描画
	/// </summary>
	void DrawInstanced(uint32_t instanceCount);
public: // Getter / Setter
	/// <summary>
	/// モデルデータの取得
	/// </summary>
	ModelData GetModelData()const { return modelData; }
	/// <summary>
	/// 頂点リソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource()const { return vertexResource; }
	/// <summary>
	/// 頂点バッファビューの取得
	/// </summary>
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView()const { return vertexBufferView; }
	/// <summary>
	/// マテリアルルソースの取得
	/// </summary>
	Microsoft::WRL::ComPtr<ID3D12Resource> GetMaterialResource()const { return materialResource; }
	/// <summary>
	/// マテリアルデータの取得
	/// </summary>
	Material* GetMaterialData()const { return materialData; }
private: // メンバ関数
	/// <summary>
	/// モデルデータの読み込み
	/// </summary>
	MaterialData LoadMaterialTempLateFile(const std::string& directoryPath, const std::string& filename);
	/// <summary>
	/// 頂点リソースデータの作成
	/// </summary>
	void CreateVertexResourceData();
	/// <summary>
	/// マテリアルリソースの作成
	/// </summary>
	void CreateMaterialResource();

	

private:	// メンバ変数
	// ModelCommon
	ModelCommon* modelCommon = nullptr;
	// モデルデータ
	ModelData modelData;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource;
	// バッファリソースの使い道を補足するバッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// バッファリソース内のデータを指すポインタ
	VertexData* vertexData = nullptr;
	// バッファリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource;
	// マテリアルデータ
	Material* materialData = nullptr;

	std::string cubeMapPath = "resources/_Common/Textures/debug/rostock_laage_airport_4k.dds";

};

