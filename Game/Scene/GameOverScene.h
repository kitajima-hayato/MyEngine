#pragma once
#include "engine/InsideScene/BaseScene.h"
#include "Game/Camera/Camera.h"
#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif
#include <DirectXCommon.h>
#include "engine/3d/Object3D.h"
#include "Game/Application/BackGround/BackGround.h"
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


	void StartPlayerDropIntro(const Transform& finalTr);

	void UpdatePlayerDropIntro(float dt);

	void StartPlayerShrinkSpin();
	void UpdatePlayerShrinkSpin(float dt);
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




	Transform playerFinalTr_;
	Transform playerAnimTr_;

	bool isDropIntroRunning_ = false;
	int bounceCount_ = 0;

	float vy_ = 0.0f;
	float gravity_ = -35.0f;        // 落ちる速さ（調整）
	float restitution_ = 0.45f;     // 反発（卓球っぽくするなら 0.4～0.6）
	float spawnHeight_ = 8.0f;      // どれだけ上から落とすか

	float settleEps_ = 0.05f;       // 収束判定
	float rotBlendT_ = 0.0f;        // 回転補間用

	// バウンドの最大回数（これ以上は跳ねないで止まる）
	int maxBounces_ = 3;

	float bounceDamping_ = 0.75f; // 0.6～0.85くらいで調整

	enum class PlayerOutroState {
		DropBounce,   // 落下→バウンド中
		ShrinkSpin,   // 回転しながら縮小
		Done          // 完了（非表示）
	};

	PlayerOutroState playerOutroState_ = PlayerOutroState::DropBounce;

	// 縮小しながら回転するパラメータ
	float shrinkSpeed_ = 0.5f;    
	float minScale_ = 0.03f;      
	float spinSpeedY_ = 8.0f;     
	float spinSpeedX_ = 2.5f;     
	bool isPlayerVisible_ = true; 


	bool loopPlayerIntro_ = true;

	float loopDelaySec_ = 0.0f;      
	float loopDelayTimer_ = 0.0f;
};

