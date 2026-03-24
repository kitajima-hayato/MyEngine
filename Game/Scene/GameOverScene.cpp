#include "GameOverScene.h"
#include "engine/InsideScene/Framework.h"

GameOverScene::GameOverScene()
{
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::Initialize(Engine::DirectXCommon* dxCommon)
{
	// カメラの取得
	camera = Framework::GetMainCamera();
}

void GameOverScene::Update()
{
	// カメラの更新
	camera->Update();
	// ImGuiの描画
	DrawImgui();
}

void GameOverScene::Draw()
{
}

void GameOverScene::Finalize()
{
	// カメラの終了処理
	camera->Finalize();
}

void GameOverScene::DrawImgui()
{
#ifdef USE_IMGUI
	// カメラの調整
	// カメラの配置情報の変更
	ImGui::Begin("Camera Transform");
	cameraTransform = { {0.0f,0.0f,0.0f}, camera->GetRotate(),camera->GetTranslate() };
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	
	// カメラの配置情報の反映
	camera->SetRotate(cameraTransform.rotate);
	camera->SetTranslate(cameraTransform.translate);
	ImGui::End();
#endif
}
