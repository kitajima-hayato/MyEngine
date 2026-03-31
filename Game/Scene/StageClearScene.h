#pragma once
#include "engine/InsideScene/BaseScene.h"
#include "MyMath.h"
#include "Game/Application/BackGround/BackGround.h"
#include <memory>
#include "MyMath.h"
#include "Sprite.h"
/// <summary>
/// ステージクリアシーン
/// ステージをクリアしたときに表示されるシーン
/// </summary>
class Object3D;
class Camera;
class StageClearScene :
    public BaseScene
{
	enum class ClearMenuItem {
		NextStage = 0,
		OneMore,
		Select,
		Count
	};

public:
	StageClearScene();
	~StageClearScene();

	/// <summary>
	/// 初期化処理
	/// </summary>
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
	/// Imgui一括管理
	/// </summary>
	void DrawImgui();

private:
	/// カメラ
	Camera* camera = nullptr;
	Transform cameraTransform;
	// 背景
	std::unique_ptr<BackGround> backGround_;
	// プレイヤーオブジェクト
	std::unique_ptr<Object3D>playerObject_;
	Transform playerTransform_;


	// クリアテキスト
	std::unique_ptr<Sprite> clearTextSprite;
	Vector2 clearTextPos = {};

	// OneMore / Select / Title UI

	std::unique_ptr<Sprite> oneMore_;
	Vector2 oneMorePos;
	std::unique_ptr<Sprite> select_;
	Vector2 selectPos;
	std::unique_ptr<Sprite> title_;
	Vector2 titlePos;
	// 選択しているアイテム
	ClearMenuItem selectedItem_ = ClearMenuItem::NextStage;
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

