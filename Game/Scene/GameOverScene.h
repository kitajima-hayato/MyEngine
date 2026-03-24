#pragma once
#include "engine/InsideScene/BaseScene.h"
#include "Game/Camera/Camera.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif
#include <DirectXCommon.h>
#include "engine/3d/Object3D.h"
#include "Game/Application/BackGround.h"
#include "engine/2d/Sprite.h"

/// <summary>
/// ゲームオーバーシーン
/// </summary>
class GameOverScene : public BaseScene
{
	enum class ClearMenuItem {
		OneMore = 0,
		Select,
		Title,
		Count
	};
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
	/// <summary>
	/// デバッグカメラの更新処理
	/// </summary>
	void UpdateDebugCamera();
private:

	// デバッグカメラ設定
	float cameraMoveSpeed_ = 0.15f;
	float cameraShiftSpeed_ = 0.45f;
	float mouseSensitivity_ = 0.01f;

	// デバッグカメラ設定
	bool isDebugCameraActive_ = true;
	bool isFastMoveMode_ = false;

	// カメラインスタンス
	Camera* camera = nullptr;
	// カメラの配置情報
	Transform cameraTransform = {};

	std::unique_ptr<BackGround> backGround = nullptr;
	std::unique_ptr<Object3D> backBlack = nullptr;
	Transform backBlackTransform;

	std::unique_ptr<Object3D> PlayerModel = nullptr;
	Transform playerModelTransform;

	// ゲームオーバーUI
	std::unique_ptr<Sprite> gameOverUI = nullptr;
	Vector2 gameOverUIPosition = { 0.0f,0.0f };


	std::unique_ptr<Sprite> oneMore_;
	Vector2 oneMorePos;
	std::unique_ptr<Sprite> select_;
	Vector2 selectPos;
	std::unique_ptr<Sprite> title_;
	Vector2 titlePos;
	// 選択しているアイテム
	ClearMenuItem selectedItem_ = ClearMenuItem::OneMore;
	// 選択時の拡縮編集用Vector2
	Vector2 oneMoreBaseSize_ = { 200.0f, 90.0f };
	Vector2 selectBaseSize_ = { 200.0f, 90.0f };
	Vector2 titleBaseSize_ = { 200.0f, 90.0f };
	// 選択時に８％程大きくする
	float selectScale_ = 1.08f;


	// kyeUI
	std::unique_ptr<Sprite> keyIcon_A;
	std::unique_ptr<Sprite> keyIcon_D;

	// enter
	std::unique_ptr<Sprite> keyIcon_Enter;
	Vector2 keyIcon_Enter_Pos = { 200.0f,650.0f };
};

