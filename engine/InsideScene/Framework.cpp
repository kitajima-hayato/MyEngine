#include "Framework.h"
#include "engine/InsideScene/SceneManager.h"
#include "Game/Particle/ModelParticleManager.h"
#include "Game/Application/ModelList.h"
using namespace Engine;

void Framework::Initialize()
{
	//WindowsAPIの初期化
	winAPI = make_unique<WinAPI>();
	winAPI->Initialize();
	// dxCommonの初期化
	dxCommon = make_unique<DirectXCommon>();
	dxCommon->Initialize(winAPI.get());

	// 入力処理のクラスポインタ
	
	Input::GetInstance()->Initialize(winAPI.get());

	// SRVマネージャーの初期化
	srvManager = make_unique<SrvManager>();
	srvManager->Initialize(dxCommon.get());
	// スプライトコモンの初期化
	SpriteCommon::GetInstance()->Initialize(dxCommon.get());
	// テクスチャマネージャーの初期化
	TextureManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get());


#ifdef USE_IMGUI
	// ImGuiの初期化
	imGui = make_unique<ImGuiManager>();
	imGui->Initialize(winAPI.get(), dxCommon.get());
#endif

	// 3Dモデルマネージャの初期化
	ModelManager::GetInstance().Initialize(dxCommon.get());

	ModelList modelList;
	modelList.LoadAllModel();

	// 3Dオブジェクト共通部の初期化
	Object3DCommon::GetInstance()->Initialize(dxCommon.get());

	// モデル共通部の初期化
	modelCommon = make_unique<ModelCommon>();
	modelCommon->Initialize(dxCommon.get());

	// カメラ
	camera = make_unique<Camera>();
	camera->SetRotate({ 0.0f, 0.0f, 0.0f });
	camera->SetTranslate({ 0.0f, 0.0f, -5.0f });

	// 全シーンがアクセス可能なカメラに設定
	mainCamera_ = camera.get();

	Object3DCommon::GetInstance()->SetDefaultCamera(camera.get());

	// パーティクル
	ParticleManager::GetInstance()->Initialize(dxCommon.get(), srvManager.get(), camera.get());

	Object3DInstancingCommon::GetInstance()->Initialize(dxCommon.get());
	
	ModelParticleManager::GetInstance().Initialize();

}

void Framework::Update()
{
#pragma region WindowsAPIのメッセージ処理
	//Windowのメッセージ処理
	if (winAPI->ProcessMessage()) {
		//ゲームループを抜ける
		isEndRequest_ = true;
		return;
	}
#pragma endregion

	Input::GetInstance()->Update();
	// カメラの更新
	camera->Update();

	SceneManager::GetInstance()->Update(dxCommon.get());
	
	// パーティクルの更新
	ParticleManager::GetInstance()->Update();
	

	ModelParticleManager::GetInstance().Update();
}


void Framework::Draw()
{
	
}


void Framework::Finalize()
{
	/// 各種マネージャーの終了処理

	SceneManager::GetInstance()->Finalize();
	SceneManager::DestroyInstance();

	ParticleManager::GetInstance()->DeleteInstance();

	Object3DInstancingCommon::GetInstance()->Finalize();

	ModelParticleManager::GetInstance().Finalize();

	Object3DCommon::GetInstance()->DeleteInstance();
	SpriteCommon::DeleteInstance();

	TextureManager::GetInstance()->DeleteInstance();
	ModelManager::GetInstance().Finalize();

	Audio::GetInstance().DeleteInstance();
	Input::GetInstance()->DeleteInstance();
	
	
	
	mainCamera_ = nullptr;
	winAPI->Finalize();

}

void Framework::Run()
{
	// ゲームの初期化
	Initialize();
	// ゲームのメインループ
	while (true) {
		// ゲームの更新
		Update();
		if (IsEndRequest()) {
			break;
		}
		// ゲームの描画
		Draw();
	}
	// ゲームの終了処理
	Finalize();
}

