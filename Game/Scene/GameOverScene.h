#pragma once
#include "engine/InsideScene/BaseScene.h"
#include "Game/Camera/Camera.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif
#include <DirectXCommon.h>

/// <summary>
/// ゲームオーバーシーン
/// </summary>
class GameOverScene : public BaseScene
{
	public:
	GameOverScene();
	~GameOverScene();
	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(Engine::DirectXCommon* dxCommon)override;
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

	// カメラインスタンス
	Camera* camera = nullptr;
	// カメラの配置情報
	Transform cameraTransform = {};
};

