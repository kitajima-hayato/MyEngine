#pragma once
#include "Audio.h"
#include "engine/InsideScene/BaseScene.h"
#include "Sprite.h"
#include "engine/InsideScene/SceneManager.h"
#include "Game/Particle/ParticleManager.h"
#include "Game/Particle/ParticleSystem.h"
#include "Game/Loader/LevelLoader.h"
#include "Game/Application/BackGround/BackGround.h"


#ifdef USE_IMGUI
#include "ImGuiManager.h"
#endif


/// <summary>
/// タイトルシーン
/// タイトルシーンを管理する
/// </summary>
class Camera;
class Object3D;
class TitleScene :public BaseScene
{
public:
	TitleScene();
	~TitleScene();
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
	///	ロード処理（オーディオ、スプライト）
	/// </summary>
	void LoadAudio();
	void LoadSprite();
	/// <summary>
	/// Imgui一括管理 / 描画関数
	/// </summary>
	void DrawImgui();
private:

	// オーディオ
	// サウンドデータ
	//SoundData soundData;
	//IXAudio2* xaudio2_;


	// パーティクルエミッター
	std::unique_ptr<ParticleSystem> particleEmitter;
	std::unique_ptr<ParticleSystem> particleEmitter2;

	std::unique_ptr<ParticleSystem> presetEffect;


private: // ゲーム要素
	Vector3 speed = { 0.0f,0.0f,0.0f };
	bool isStart = false;
	std::unique_ptr<Object3D> playerObject;
	Transform playerTransform;

	// バックグラウンド
	 std::unique_ptr<BackGround> background;

	 // タイトルスプライト
	 std::unique_ptr<Sprite> titleSprite;
	 std::unique_ptr<Sprite> pressStartSprite;
	 // 点滅
	 const int blinkInterval = 60;
	 const int blinkDuration = 30;

	 /// カメラ
	 Camera* camera = nullptr;
	 Transform cameraTransform;
	

	 // ===== Title demo player particle & jump (add) =====
	 std::unique_ptr<ParticleSystem> titleSmoke_;   // 常時煙（走ってる感）
	 std::unique_ptr<ParticleSystem> titleJumpSpark_; // ジャンプ開始のスパーク（単発）
	 std::unique_ptr<ParticleSystem> titleLandSpark_; // 着地のスパーク（単発）

	 // ジャンプ演出（物理なしの見た目用）
	 float basePlayerY_ = 0.0f;   // ジャンプ前の基準Y
	 bool  isJumping_ = false;
	 int   jumpFrame_ = 0;
	 int   jumpIntervalTimer_ = 0;

	 // 調整用（
	 int   jumpIntervalFrame_ = 180;  // 3秒ごとにジャンプ（60fps）
	 int   jumpDurationFrame_ = 45;   // ジャンプの長さ
	 float jumpHeight_ = 0.8f;        // ジャンプ高さ

	 // 足元位置の補正（煙・スパークを足元へ）
	 float footOffsetY_ = 0.4f;

	 // タイトル画面をロードしなおすまでの距離
	 float resetDistance_ = 135.0f;

};

