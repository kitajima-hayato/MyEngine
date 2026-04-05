#pragma once
#include "Math.h"
#include <string>
#include <d3d12.h>
#include <vector>
#include <wrl.h>
#include "Model.h"
#include "Game/Camera/Camera.h"

// リング状の頂点データ構造体
struct RingVertex {
	Vector4 position;
	Vector3 normal;
	Vector2 uv;
};
/// <summary>
/// 3Dオブジェクトクラス
/// </summary>
class Object3D
{

public:	// メンバ関数
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	///  レベルローダーモデル生成
	/// </summary>
	void Create(Model* model);
	/// <summary>
	/// デストラクタ
	/// </summary>
	~Object3D();

public:	// Getter/Setter
	/// <summary>
	/// カメラのセット
	/// </summary>
	void SetCamera(Camera* camera) { this->camera = camera; }
	/// <summary>
	/// Model 型のポインタをメンバ変数に設定します。
	/// </summary>
	void SetModel(Model* model) { this->model = model; }
	/// <summary>
	/// モデルの読み込みと設定
	/// </summary>
	void SetModel(const std::string& filePath);
	/// <summary>
	/// スケールの設定
	/// </summary>
	void SetScale(const Vector3& scale) { transform.scale = scale; }
	/// <summary>
	/// 回転の設定
	/// </summary>
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }
	/// <summary>
	/// 平行移動の設定
	/// </summary>
	void SetTranslate(const Vector3& translate) { transform.translate = translate; }
	/// <summary>
	/// トランスフォームの設定
	/// </summary>
	void SetTransform(const Transform& transform) { this->transform = transform; }

	/// <summary>
	/// スケールの取得
	/// </summary>
	const Vector3 GetScale()const { return transform.scale; }
	/// <summary>
	/// 回転の取得
	/// </summary>
	const Vector3 GetRotate()const { return transform.rotate; }
	/// <summary>
	/// 平行移動の取得
	/// </summary>
	const Vector3 GetTranslate()const { return transform.translate; }
	/// <summary>
	/// トランスフォームの取得
	/// </summary>
	const Transform& GetTransform()const { return transform; }
private: // メンバ関数
	/// <summary>
	/// 座標変換行列データの作成
	/// </summary>
	void CreateTransformationMatrixData();
	/// <summary>
	/// 平行光源リソースの作成
	/// </summary>
	void CreateDirectionalLightResource();
	/// <summary>
	/// カメラリソースの作成
	/// </summary>
	void CreateCameraResource();


private:// メンバ変数
	// カメラ
	Camera* camera = nullptr;
	// モデルデータ
	Model* model = nullptr;
	// バッファリソース / 座標変換行列リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource;
	// バッファリソース内のデータを指すポインタ
	TransformationMatrix* wvpData = nullptr;
	// バッファリソース / 平行光源リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource;
	// バッファリソース内のデータを指すポインタ
	DirectionalLight* directionalLightData = nullptr;

	// Transform
	Transform transform;


	Microsoft::WRL::ComPtr<ID3D12Resource>vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource>indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	D3D12_INDEX_BUFFER_VIEW ibView{};
	uint32_t indexCount = 0;

	// カメラリソース
	Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource;
	CameraForGPU* cameraForGpuData = nullptr;
};

