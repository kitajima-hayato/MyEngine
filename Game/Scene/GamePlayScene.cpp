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
	/// オーディオの初期化
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

	SpritesInitialize();

	stageStartEventFlag_ = true;
	player->SetControlEnabled(false);

	isPlayerControlLocked_ = true;

	pauseSystem_ = std::make_unique<PauseSystem>();
	pauseSystem_->Initialize();

	gamePlayHUD_ = std::make_unique<GamePlayHUD>();
	gamePlayHUD_->Initialize();

	startCam_ = std::make_unique<StartCamPhase>();
	startCam_->Bind(camera, &cameraTransform);

	startCam_->Start();

	damageFeedBack_ = std::make_unique<DamageFeedBack>();
	damageFeedBack_->Bind(player.get(), camera, &cameraTransform);
	damageFeedBack_->Initialize();

	ModelParticleManager::GetInstance().Initialize();
}


void GamePlayScene::Update()
{

	if (pauseSystem_->Update()) {
		return;
	}

	// カメラの更新
	camera->Update();
	// 背景の更新
	backGround->Update();
	// スタートカメラの更新
	startCam_->Update(dt);
	stageStartEventFlag_ = startCam_->IsRunning();
	isPlayerControlLocked_ = stageStartEventFlag_;
	player->SetControlEnabled(!isPlayerControlLocked_);

	// マップの更新
	map->Update();
	// プレイヤーの更新
	player->Update();




	// エネミーレイヤーが変更されたらエネミーを再生成
	if (map->ConsumeEnemyLayerDirtyFlag()) {
		GenerateEnemy();
	}


	// 開始演出中はプレイヤーへの入力を受け付けない
	if (isPlayerControlLocked_) {
		player->SetControlEnabled(false);
	} else {
		player->SetControlEnabled(true);
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

	// 当たり判定
	CheckCollision();
	// スプライトの更新
	gamePlayHUD_->Update();


	ModelParticleManager::GetInstance().Update();
	// プレイヤーがゴールに触れていたらシーン遷移
	bool isGoal = player->GetIsGoal();
	if (isGoal) {
		sceneManager->ChangeSceneWithTransition("STAGECLEAR", TransitionType::Normal);
	}



	// ImGuiの描画
	DrawImgui();


}

void GamePlayScene::Draw()
{
	///////////////////
	//  モデルの描画   //
	///////////////////

	backGround->Draw();


	//sceneTransition->Draw();
	/// マップの描画
	map->Draw();

	/// プレイヤーの描画
	player->Draw();

	/// 敵の描画
	for (auto& enemy : enemies) {
		enemy->Draw();
	}

	ParticleManager::GetInstance()->Draw();
	ModelParticleManager::GetInstance().Draw();

	///////////////////
	// スプライトの描画 //
	///////////////////

	// スプライト描画処理
	SpritesDraw();

	damageFeedBack_->Draw();
	gamePlayHUD_->Draw(pauseSystem_->GetPause(), !isPlayerControlLocked_);


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

	/// プレイヤー
	if (player) {
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

void GamePlayScene::UpdateStartCamera(float dt)
{

}



void GamePlayScene::SpritesInitialize()
{

}

void GamePlayScene::SpritesUpdate()
{

}

void GamePlayScene::SpritesDraw()
{
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
	// 読み込んでいるマップデータのキー
	ImGui::Text("SelectedStage:%s", stageKey);
	//==============================
	// Start Camera Intro Tuning UI
	//==============================

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