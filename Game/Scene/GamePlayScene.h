#pragma once
#include "Audio.h"    
#include "engine/InsideScene/BaseScene.h"
#include "engine/math/MyMath.h"
#include "Game/Application/Enemy/Factory/EnemyFactory.h"
#include "Game/Application/BackGround/BackGround.h"
#include "Game/Camera/CameraController.h"
#include "Game/Particle/ParticleSystem.h"
#include "Game/Scene/Pause/PauseSystem.h"
#include "Game/Application/UI/GamePlayHUD.h"
#include "Game/Camera/StartCamPhase.h"
#include "Game/Application/UI/DamageFeedBack.h"
#include "engine/base/DirectXCommon.h"
#include "Game/Collision/MapCollisionQuery.h"
#include "Game/Collision/Collider.h"
#include "Game/Application/RespawnSequence.h"
/// <summary>
/// ゲームプレイシーン
/// ゲームプレイ中のシーンを管理する
/// </summary>

// 前方宣言
class Map;
class Player;
class Object3D;
class CollisionManager;
/// <summary>
/// ゲームプレイシーン : 基底シーンクラスを継承
/// </summary>
class GamePlayScene :public BaseScene
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GamePlayScene();
	/// <summary>
	/// デストラクタ
	/// </summary>
	~GamePlayScene();
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

	/// <summary>
	/// エネミーの初期化
	/// </summary>
	void InitializeEnemy();

	/// <summary>
	/// エネミーの生成
	/// </summary>
	void GenerateEnemy();

	/// <summary>\
	/// 当たり判定
	/// </summary>
	void CheckCollision();

	/// <summary>
	/// スタートカメラの更新
	/// </summary>
	void UpdateStartCamera(float dt);

	/// <summary>
	/// UI/ダメージ/スプライト類の初期化 / 更新 / 描画
	/// </summary>
	void SpritesInitialize();
	void SpritesUpdate();
	void SpritesDraw();

	/// <summary>
	/// AABB同士の当たり判定
	/// </summary>
	bool IsAABBOverlap(const AABB& a, const AABB& b);
	/// <summary>
	/// エネミー同士の重なりを修正
	/// </summary>
	void CorrectEnemyOverlap(EnemyBase* leftEnemy, EnemyBase* rightEnemy);
	/// <summary>
	/// エネミー同士の重なりを修正する処理
	/// </summary>
	void ResolveEnemyVsEnemy();

	/// <summary>
	/// プレイヤーの生存状況統括
	/// </summary>
	void CheckPlayerAlive();
	

private:
	// マップ
	std::unique_ptr<Map> map;
	
	// オーディオ
	// サウンドデータ
	SoundData soundData;
	IXAudio2* xaudio2_;

	// Player
	std::unique_ptr<Player> player;


	/// エネミーファクトリー
	std::vector<std::unique_ptr<EnemyBase>> enemies;

	/// コリジョンマネージャー
	std::unique_ptr<CollisionManager> collision_;


	/// カメラ
	Camera* camera = nullptr;
	Transform cameraTransform;

	

	// 被弾フィードバック
	std::unique_ptr<DamageFeedBack> damageFeedBack_;


	
	float shakeAmp_ = 0.1f;

	


	/// タイトルロゴ
	std::unique_ptr<Object3D> titleLogoObject;
	Transform titleLogoTransform;

	/// バックグラウンド
	std::unique_ptr<BackGround> backGround;



	// フォローカメラ
	std::unique_ptr<CameraController> cameraController_;

	// ポーズシステム
	std::unique_ptr<PauseSystem> pauseSystem_;

	// ステージ開始演出フラグ
	bool stageStartEventFlag_ = true;
	// 固定フレームレート用のデルタタイム
	const float dt = 1.0f / 60.0f;

	// エネミー配置のオフセット
	float enemySpawnOffset_ = 0.5f;
	// エネミー同士の重なり修正用のオフセット
	std::unique_ptr<ParticleSystem>testParticle_;
	// プレイヤーの操作をロックするフラグ
	bool isPlayerControlLocked_ = false;
	// ゲームプレイHUD
	std::unique_ptr<GamePlayHUD> gamePlayHUD_;
	// スタートカメラ
	std::unique_ptr<StartCamPhase> startCam_;
	// スタートカメラの開始位置
	Vector3 baseCameraPos_ = {};
	// ステージキー
	std::string stageKey;
	// マップ衝突クエリ
	MapCollisionQuery mapCollisionQuery;
	// プレイヤーの死亡フラグ
	bool isPlayerDead_ = false;
	// プレイヤーの死因が落下
	bool isPlayerDeathByFall_ = false;

	// リスポーンシーケンス
	std::unique_ptr <RespawnSequence> respawnSequence_;
	// リスポーン中フラグ
	bool isRespawning_ = false;
	bool didWarpOnDark_ = false;
	// リスポーン位置
	const Vector3 respawnPos_ = {  1.5f,1.5f,0.0f  };

	// GameOver前の演出（シェイクしてから遷移）
	bool isGameOverPending_ = false;
	float gameOverTimer_ = 0.0f;

	// 失敗感を出すなら 0.5～0.8 くらいが見やすい
	float gameOverDelay_ = 0.6f;

	// 死亡演出中のカメラ固定
	bool freezeCameraOnDeath_ = false;
	Vector3 frozenCameraPos_ = {};

	// スタートカメラをスキップしたときにプレイヤーが動かないようにするフラグ
	bool skipJustFinishedThisFrame_ = false;
};


