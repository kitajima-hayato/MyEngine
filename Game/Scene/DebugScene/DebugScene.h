#pragma once
#include "engine/InsideScene/BaseScene.h"
#include "Game/Camera/Camera.h"
#include "MyMath.h"
#include "Game/Application/BackGround.h"
class Object3D;
class DebugScene :
    public BaseScene
{
public:
	/// <summary>
	/// コンストラクタ・デストラクタ
	/// </summary>
	DebugScene();
	~DebugScene();

	/// <summary>
	 /// 初期化処理
	 /// </summary>
	 /// <param name="dxCommon"></param>
	void Initialize(Engine::DirectXCommon* dxCommon) override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update()override;
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw()override;
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// Imgui一括管理 / 描画関数
	/// </summary>
	void DrawImgui();
private:
	/// <summary>
	/// デバッグカメラの更新処理
	/// </summary>
	void UpdateDebugCamera();
private:
	// カメラ
	Camera* camera = nullptr;
	Transform cameraTransform = {};

	// デバッグカメラ設定
	float cameraMoveSpeed_ = 0.15f;
	float cameraShiftSpeed_ = 0.45f;
	float mouseSensitivity_ = 0.01f;

	// デバッグカメラ設定
	bool isDebugCameraActive_ = true;
	bool isFastMoveMode_ = false;

	// オブジェクト
	std::unique_ptr<Object3D> ball = nullptr;
	std::unique_ptr<Object3D> player = nullptr;

	std::unique_ptr<BackGround> backGround = nullptr;
};

