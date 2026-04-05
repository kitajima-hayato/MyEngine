#include "MyGame.h"
#include "engine/InsideScene/SceneFactory.h"
#include <dxgiformat.h>
#include <memory>
#include "RenderTexture.h"
#include "ModelManager.h"
#include "SkyBox/SkyBox.h"
#include "TextureManager.h"
#include "WinAPI.h"
#include "MyMath.h"
#include "Game/Application/ModelList.h"
#include "engine/InsideScene/Framework.h"
#include "Game/Particle/ModelParticleManager.h"

using namespace Engine;
static std::vector<std::string> LoadTextureManifest(const std::string& manifestPath)
{
	std::ifstream file(manifestPath);
	if (!file) {
		throw std::runtime_error("Failed to open texture manifest: " + manifestPath);
	}

	std::vector<std::string> paths;
	std::string line;
	while (std::getline(file, line)) {
		if (line.empty()) continue;
		if (!line.empty() && line[0] == '#') continue;
		paths.push_back(line);
	}
	return paths;
}
void MyGame::Initialize()
{
	Framework::Initialize();

	// 起動時に全テクスチャを読み込む（最後にWaitは1回だけ）
	const auto paths = LoadTextureManifest("resources/texture_manifest.txt");
	TextureManager::GetInstance()->LoadTextures(paths);

	// シーンマネージャーの初期化
	SceneManager::GetInstance()->Initialize();
	// シーンファクトリーの生成
	sceneFactory_ = make_unique<SceneFactory>();
	SceneManager::GetInstance()->SetSceneFactory(sceneFactory_.get());
	// シーンマネージャーに最初のシーンをセット
	SceneManager::GetInstance()->ChangeScene("TITLE");

	modelList = make_unique<ModelList>();
	modelList->LoadAllModel();

	//スカイボックス
	skyBox = make_unique<SkyBox>();
	skyBox->Initialize(dxCommon.get(), srvManager.get());

	renderTexture = std::make_unique<RenderTexture>();
	renderTexture->Initialize(dxCommon.get(),srvManager.get(),
		WinAPI::kClientWidth,WinAPI::kClientHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 
		Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });
}

void MyGame::Update()
{
#ifdef USE_IMGUI // デバッグ時のみ有効ImGuiの処理
	imGui->Begin(); 
#endif 
	Framework::Update();
	skyBox->Update();
#ifdef USE_IMGUI // デバッグ時のみ有効ImGuiの処理
	imGui->End();
#endif

}

void MyGame::Draw()
{
	if (usePostProcess_) {
		renderTexture->BeginRender();
		srvManager->PreDraw();
		SceneManager::GetInstance()->Draw();
		renderTexture->EndRender();

		dxCommon->PreDraw();
		srvManager->PreDraw();
		renderTexture->Draw();
	} else {
		dxCommon->PreDraw();
		srvManager->PreDraw();
		SceneManager::GetInstance()->Draw();
	}

	Framework::Draw();

#ifdef USE_IMGUI
	// ImGuiの描画
	imGui->Draw();
#endif
	dxCommon->PostDraw();

}


void MyGame::Finalize()
{
	
#pragma region  解放処理
#ifdef USE_IMGUI
	// ImGuiの終了処理
	imGui->Finalize();
#endif
	// CloseHandle(fenceEvent);
	TextureManager::GetInstance()->Finalize();
	// モデルマネージャの終了処理
	ModelManager::GetInstance().Finalize();
	winAPI->Finalize();

#pragma endregion

	Framework::Finalize();
}

