#pragma once

#include <memory>

#include "engine/InsideScene/BaseScene.h"
#include "Game/Camera/Camera.h"
#include "MyMath.h"
#include "Game/Application/BackGround/BackGround.h"

class Object3D;

class DebugScene :
	public BaseScene
{
private:
	enum class DiceRollDirection {
		None,
		Forward,
		Back,
		Left,
		Right
	};

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
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize() override;

	/// <summary>
	/// Imgui一括管理 / 描画関数
	/// </summary>
	void DrawImgui();

private:
	/// <summary>
	/// デバッグカメラの更新処理
	/// </summary>
	void UpdateDebugCamera();

	// サイコロ操作
	void UpdateDice();
	void StartDiceRoll(DiceRollDirection direction);
	void UpdateDiceRoll();
	void ApplyDiceFaceRoll(DiceRollDirection direction);

	// 補間
	float EaseOutQuad(float t);
	float Lerp(float start, float end, float t);

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

private:
	// サイコロ制御
	bool isDiceRolling_ = false;
	float diceRollTimer_ = 0.0f;
	float diceRollDuration_ = 0.20f;

	// サイコロの1マス移動量
	float diceMoveDistance_ = 5.0f;

	DiceRollDirection currentDiceRollDirection_ = DiceRollDirection::None;

	Transform diceStartTransform_{};
	Transform diceTargetTransform_{};

	// サイコロの面情報
	int diceTop_ = 1;
	int diceBottom_ = 6;
	int diceFront_ = 2;
	int diceBack_ = 5;
	int diceLeft_ = 4;
	int diceRight_ = 3;
};