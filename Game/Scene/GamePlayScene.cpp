#include "GamePlayScene.h"
#include "Game/Application/Map/Map.h"
#include "Game/Application/Player/Player.h"
#include "Game/Collision/CollisionManager.h"
#include "Game/Particle/ParticleManager.h"
#include "engine/InsideScene/Framework.h"
#include "Game/Application/PlayContext.h"
#include "Game/Particle/ModelParticleManager.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif

using Engine::DirectXCommon;

GamePlayScene::GamePlayScene()
{
}

GamePlayScene::~GamePlayScene()
{
}

void GamePlayScene::Initialize(DirectXCommon* dxCommon)
{
	// オーディオの初期化
	Audio::GetInstance().Initialize();
	soundData = Audio::GetInstance().LoadWave("resources/_Common/Audio/debug/mokugyo.wav");
	xaudio2_ = Audio::GetInstance().GetXAudio2();
	Audio::GetInstance().SoundPlayWave(xaudio2_, soundData);

	// カメラクラスの生成
	camera = Framework::GetMainCamera();

	const std::string& key = PlayContext::GetInstance().GetSelectedStageKey();

	// マップ
	map = std::make_unique<Map>();

	// ノードに応じたステージを初期化
	stageKey = key.empty() ? "1-1" : key.c_str();
	map->Initialize(stageKey);
	mapCollisionQuery.SetMap(map.get());


	collision_ = std::make_unique<CollisionManager>();

	/// プレイヤーの初期化
	player = std::make_unique<Player>();
	player->Initialize(Vector3{ 2.0f,2.0f,0.0f });
	player->SetMap(map.get());

	InitializeEnemy();

	backGround = std::make_unique<BackGround>();
	backGround->Initialize();

	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize();


	stageStartEventFlag_ = true;
	player->SetControlEnabled(false);
	isPlayerDead_ = player->IsAlive();

	isPlayerControlLocked_ = true;

	pauseSystem_ = std::make_unique<PauseSystem>();
	pauseSystem_->Initialize();

	gamePlayHUD_ = std::make_unique<GamePlayHUD>();
	gamePlayHUD_->Initialize();

	startCam_ = std::make_unique<StartCamPhase>();
	startCam_->Initialize();
	startCam_->Bind(camera, &cameraTransform);

	startCam_->Start();

	baseCameraPos_ = cameraTransform.translate;

	damageFeedBack_ = std::make_unique<DamageFeedBack>();
	damageFeedBack_->Bind(player.get(), camera, &cameraTransform);
	damageFeedBack_->Initialize();

	respawnSequence_ = std::make_unique<RespawnSequence>();
	respawnSequence_->Initialize();

	

	// ゲームオーバー遷移の保留フラグとタイマーをリセット
	isGameOverPending_ = false;
	gameOverTimer_ = 0.0f;

}

void GamePlayScene::CheckPlayerAlive()
{
	// プレイヤーの生存チェック
	isPlayerDead_ = player->IsAlive();
	// プレイヤーの死亡原因が落下かどうか
	isPlayerDeathByFall_ = player->IsDeathByFalling();
}

void GamePlayScene::Update()
{
	// ポーズ中はポーズシステムの更新のみ行う / ポーズ以外入力不可
	if (pauseSystem_->Update()) {
		return;
	}

	// カメラの更新
	camera->Update();
	// 背景の更新
	backGround->Update();

	const bool wasStatCamRunning = startCam_->IsRunning();
	// スタートカメラの更新
	startCam_->Update(dt);
	// このフレームで開始演出が終了したか
	skipJustFinishedThisFrame_ = wasStatCamRunning && !startCam_->IsRunning();
	bool isGoal = player->GetIsGoal();
	stageStartEventFlag_ = startCam_->IsRunning();

	// ステージ開始演出中  ゴールしていたらプレイヤーの操作を受け付けない
	isPlayerControlLocked_ = stageStartEventFlag_ || isGoal || skipJustFinishedThisFrame_;
	player->SetControlEnabled(!isPlayerControlLocked_);

	// マップの更新
	map->Update();
	// プレイヤーの更新
	player->Update();




	// エネミーレイヤーが変更されたらエネミーを再生成 
	// @TODO　エネミーの生成をクラス化
	if (map->ConsumeEnemyLayerDirtyFlag()) {
		GenerateEnemy();
	}



	// フォローカメラ
	if (!startCam_->IsRunning())
	{
		// baseCamPos_ を入力として使う（前フレームのシェイクを混ぜない）
		cameraController_->SetCameraPosition(baseCameraPos_);
		cameraController_->SetTargetPosition(player->GetTranslate());
		cameraController_->Update(dt);

		// フォロー結果を「基準」として保存
		baseCameraPos_ = cameraController_->GetCameraPosition();

	} else {
		// 開始演出中は基準位置もカメラ位置も開始演出の位置にする
		baseCameraPos_ = cameraTransform.translate;

	}
	cameraTransform.translate = baseCameraPos_;
	camera->SetTranslate(baseCameraPos_);

	damageFeedBack_->SetBaseCameraPos(baseCameraPos_);
	damageFeedBack_->Update(dt);


	// 敵の更新
	for (auto& enemy : enemies) {
		if (!enemy) continue;
		enemy->Update();
	}

	// 敵同士の重なり解消
	ResolveEnemyVsEnemy();

	// 当たり判定
	CheckCollision();


	// プレイヤーの生存確認
	CheckPlayerAlive();
	if (isRespawning_) {
		// 復帰演出中は入力を止める
		player->SetControlEnabled(false);

		// フェード更新
		respawnSequence_->Update(dt);

		// 真っ暗になった瞬間にワープ（1回だけ）
		if (!didWarpOnDark_ && respawnSequence_->ConsumeDarkened()) {
			didWarpOnDark_ = true;
			player->Respawn(respawnPos_);
		}

		// 明転が終わったら復帰完了
		if (respawnSequence_->IsFinished()) {
			isRespawning_ = false;
			didWarpOnDark_ = false;
			player->SetControlEnabled(true);
		}

		return;
	}

	// プレイヤーが死んでいたらゲームオーバーシーンへ
	if (!isPlayerDead_) {
		if (!isGameOverPending_) {
			isGameOverPending_ = true;

			// 失敗感のシェイク
			damageFeedBack_->StartShake(0.7f, 0.75f, false);

			// 死亡演出（こちら向いて少し上にジャンプして落ちる）
			player->BeginDeathDemo(camera->GetTranslate());
		}

		// 演出が終わったらゲームオーバーへ
		if (player->IsDeathDemoFinished()) {
			sceneManager->ChangeSceneWithTransition("GAMEOVER");
		}

		return;
	}
	// 落下フラグを消費して、復帰演出を開始する（HPが残っている場合）
	if (!isRespawning_ && player->ConsumeDeathByFalling()) {
		if (player->IsAlive()) {
			isRespawning_ = true;
			didWarpOnDark_ = false;
			respawnSequence_->Start();
			player->SetControlEnabled(false);
			return;
		} else {
			sceneManager->ChangeSceneWithTransition("GAMEOVER");
		}
	}

	
	// プレイヤーがゴールに触れていたらシーン遷移

	if (isGoal) {
		// ゴールしたら操作を受け付けない
		sceneManager->ChangeSceneWithTransition("STAGECLEAR", TransitionType::Normal);
	}

	// スプライトの更新
	gamePlayHUD_->Update();


	// ImGuiの描画
	DrawImgui();

}

void GamePlayScene::Draw()
{
	///////////////////
	//  モデルの描画   //
	///////////////////

	// 背景の描画
	backGround->Draw();

	// マップの描画
	map->Draw();

	// プレイヤーの描画
	player->Draw();

	// 敵の描画
	for (auto& enemy : enemies) {
		enemy->Draw();
	}

	ParticleManager::GetInstance()->Draw();
	

	///////////////////
	// スプライトの描画 //
	///////////////////


	damageFeedBack_->Draw();
	gamePlayHUD_->Draw(pauseSystem_->GetPause(), !isPlayerControlLocked_);

	startCam_->DrawUI();

	if (respawnSequence_) {
		respawnSequence_->Draw();
	}
	pauseSystem_->Draw();

}

void GamePlayScene::InitializeEnemy()
{

	// csvを読み込み敵の配置情報を取得
	GenerateEnemy();

}

void GamePlayScene::GenerateEnemy()
{
	// エネミーを一旦クリア
	enemies.clear();

	const EnemyLayerData& enemyLayerData = map->GetEnemyLayerData();
	const auto& enemyData = enemyLayerData.enemyData;

	const uint32_t mapHeight = static_cast<uint32_t>(enemyData.size());
	for (uint32_t y = 0; y < mapHeight; y++) {
		const uint32_t mapWidth = static_cast<uint32_t>(enemyData[y].size());
		for (uint32_t x = 0; x < mapWidth; x++) {
			EnemyType type = enemyData[y][x];
			// 敵の種類がNoneならスキップ
			if (EnemyType::None == type) {
				continue;
			}

			// 敵の生成
			// EnemyTypeから敵の種類を特定して生成
			std::string enemyId;
			switch (type) {
			case EnemyType::NormalEnemy:
				enemyId = "NormalEnemy";
				break;
			case EnemyType::FlyingEnemy:
				enemyId = "FlyingEnemy";
				break;
			case EnemyType::SideMoveFlyingEnemy:
				enemyId = "SideMoveFlyingEnemy";
				break;
			case EnemyType::SideMoveEnemy:
				enemyId = "SideMoveEnemy";
				break;
			default:
				continue;
			}

			// ファクトリーでエネミーを生成
			auto enemy = EnemyFactory::CreateEnemy(enemyId);
			// 生成失敗チェック
			if (!enemy) continue;
			// エネミーの初期化
			enemy->Initialize();
			enemy->SetMapQuery(&mapCollisionQuery);
			// マップ上の位置にセット
			Vector3 enemyPos = map->GetMapChipPositionByIndex(x, y);
			// オフセット
			enemyPos.x += enemySpawnOffset_;
			enemyPos.y -= enemySpawnOffset_;
			enemy->SetTranslate(enemyPos);
			// エネミーリストに追加
			enemies.push_back(std::move(enemy));
		}
	}
}

void GamePlayScene::CheckCollision()
{

	collision_->Clear();

	/// プレイヤー（死亡演出中は当たり判定から外す）
	if (player && !player->IsInDeathDemo()) {
		collision_->AddCollider(player.get());
	}

	/// エネミー全種
	for (auto& enemy : enemies) {
		if (!enemy)continue;
		// 生存していたら当たり判定をとる
		if (enemy->IsAlive()) {
			collision_->AddCollider(enemy.get());
		}

	}
	// 衝突判定実行
	collision_->CheckAllCollisions();

}



bool GamePlayScene::IsAABBOverlap(const AABB& a, const AABB& b)
{
	return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
		(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
		(a.min.z <= b.max.z && a.max.z >= b.min.z);
}
void GamePlayScene::CorrectEnemyOverlap(EnemyBase* enemyA, EnemyBase* enemyB)
{
	// どちらかがnullptrなら処理しない
	if (!enemyA || !enemyB) {
		return;
	}

	AABB a = enemyA->GetAABB();
	AABB b = enemyB->GetAABB();
	// AABB同士が重なっていないなら処理しない
	if (!IsAABBOverlap(a, b)) {
		return;
	}
	// 重なっている場合、x軸方向に修正する
	Vector3 posA = enemyA->GetTranslate();
	Vector3 posB = enemyB->GetTranslate();

	// x軸方向の重なり量
	float overlapLeft = b.max.x - a.min.x;
	float overlapRight = a.max.x - b.min.x;
	float correctionX = (overlapLeft < overlapRight) ? overlapLeft : -overlapRight;

	// 半分ずつ離す
	float push = correctionX * 0.5f;

	posA.x += push;
	posB.x -= push;

	enemyA->SetTranslate(posA);
	enemyB->SetTranslate(posB);

	// 互いに反転
	Vector3 velA = enemyA->GetVelocity();
	Vector3 velB = enemyB->GetVelocity();

	velA.x *= -1.0f;
	velB.x *= -1.0f;

	enemyA->SetVelocity(velA);
	enemyB->SetVelocity(velB);
}
void GamePlayScene::ResolveEnemyVsEnemy()
{
	const size_t count = enemies.size();

	for (size_t i = 0; i < count; ++i) {
		if (!enemies[i] || !enemies[i]->IsAlive()) {
			continue;
		}

		for (size_t j = i + 1; j < count; ++j) {
			if (!enemies[j] || !enemies[j]->IsAlive()) {
				continue;
			}

			CorrectEnemyOverlap(enemies[i].get(), enemies[j].get());
		}
	}
}



void GamePlayScene::Finalize()
{
	map->Finalize();

	/// オーディオの終了処理
	Audio::GetInstance().SoundUnload(&soundData);
	// カメラの終了処理
	camera->Finalize();


}


void GamePlayScene::DrawImgui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Camera Settings / GamePlayScene");
	if (ImGui::Button("Test Shake")) {
		damageFeedBack_->StartShake(0.6f, 0.35f, false);
	}

	// 読み込んでいるマップデータのキー
	ImGui::Text("SelectedStage:%s", stageKey);

	// カメラの配置 / 回転修正
	cameraTransform.translate = camera->GetTranslate();
	ImGui::DragFloat3("Camera Position", &cameraTransform.translate.x, 0.1f, -10000.0f, 10000.0f);

	cameraTransform.rotate = camera->GetRotate();
	ImGui::DragFloat3("Camera Rotation", &cameraTransform.rotate.x, 0.1f, -180.0f, 180.0f);



	if (ImGui::Button("Vertical Camera")) {
		cameraTransform.translate = { 8.0f,20.0f,0.0f };
		cameraTransform.rotate = { 1.6f,0.0f,0.0f };
	}
	ImGui::End();

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
#endif

}