#include "StageClearScene.h"
#include "Game/Camera/Camera.h"
#include "engine/InsideScene/Framework.h"
#include "engine/Input/Input.h"
#include "ImGuiManager.h"
#include "Game/Application/PlayContext.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>

using Engine::DirectXCommon;
StageClearScene::StageClearScene()
{
}

StageClearScene::~StageClearScene()
{
}

void StageClearScene::Initialize(DirectXCommon* dxCommon)
{
	// カメラの取得と設定
	camera = Framework::GetMainCamera();
	cameraTransform.translate = { 0.0f,-5.0f,4.0f };
	cameraTransform.rotate = { 0.0f,0.0f,0.0f };
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	// @todo: お祝い感のあるパーティクルを降らせる


	backGround_ = std::make_unique<BackGround>();
	backGround_->Initialize();

	// スプライトの初期化
	clearTextSprite = std::make_unique<Sprite>();
	clearTextSprite->Initialize("resources/Scenes/Clear/UI/Texture/StageClear.dds");

	clearTextPos = { 0.0f,0.0f }; 
	clearTextSprite->SetPosition(clearTextPos);
	clearTextSprite->SetSize({ 1280.0f,720.0f });

	// playerObject_の初期化
	playerObject_ = std::make_unique<Object3D>();
	playerObject_->Initialize();
	playerTransform_ = {
		{ 1.0f,1.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 5.0f,-7.0f,20.0f }
	};
	playerObject_->SetTransform(playerTransform_);
	playerObject_->SetModel("GamePlay/Player");


	// OneMore / Select / Title UI
	oneMore_ = std::make_unique<Sprite>();
	oneMore_->Initialize("resources/Scenes/Clear/UI/Texture/NextStage.png");
	oneMore_->SetAnchorPoint({ 0.5f, 0.5f });
	oneMore_->SetPosition({ 375.0f,610.0f });
	oneMore_->SetSize(oneMoreBaseSize_);

	select_ = std::make_unique<Sprite>();
	select_->Initialize("resources/Scenes/Clear/UI/Texture/ClearUI_Select.png");
	select_->SetAnchorPoint({ 0.5f, 0.5f });
	select_->SetPosition({ 975.0f,610.0f });
	select_->SetSize(selectBaseSize_);

	next_ = std::make_unique<Sprite>();
	next_->Initialize("resources/Scenes/Clear/UI/Texture/ClearUI_OneMore.png");
	next_->SetAnchorPoint({ 0.5f, 0.5f });
	next_->SetPosition({ 675.0f,610.0f });
	next_->SetSize(nextBaseSize_);

	// KeyIconUi　/ 左下に配置

	keyIcon_A = std::make_unique<Sprite>();
	keyIcon_A->Initialize("resources/_Common/UI/Texture/inputhints/A.dds");
	keyIcon_A->SetPosition({ 30.0f, 635.0f });
	keyIcon_A->SetSize({ 50.0f, 50.0f });

	keyIcon_D = std::make_unique<Sprite>();
	keyIcon_D->Initialize("resources/_Common/UI/Texture/inputhints/D.dds");
	keyIcon_D->SetPosition({ 80.0f, 635.0f });
	keyIcon_D->SetSize({ 50.0f, 50.0f });

}

void StageClearScene::Update()
{
	// カメラの更新
	camera->Update();
	// 左
	if(Input::GetInstance()->TriggerKey(DIK_D) || Input::GetInstance()->TriggerKey(DIK_LEFT)) {
		int idx = static_cast<int>(selectedItem_);
		idx = (idx - 1 + static_cast<int>(ClearMenuItem::Count)) % static_cast<int>(ClearMenuItem::Count);
		selectedItem_ = static_cast<ClearMenuItem>(idx);
	}
	// 右
	else if (Input::GetInstance()->TriggerKey(DIK_A) || Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		int idx = static_cast<int>(selectedItem_);
		idx = (idx + 1) % static_cast<int>(ClearMenuItem::Count);
		selectedItem_ = static_cast<ClearMenuItem>(idx);
	}

	//  決定（Enter / Space） 
	if (Input::GetInstance()->TriggerKey(DIK_SPACE) ) {
		switch (selectedItem_) {
		case ClearMenuItem::NextStage:
		{
			const std::string& nextStageName = PlayContext::GetInstance().GetSelectedStageKey();
			const size_t dash = nextStageName.find('-');
			if (dash != std::string::npos) {
				const std::string left = nextStageName.substr(0, dash);
				const std::string right = nextStageName.substr(dash + 1);

				auto isAllDigits = [](const std::string& s) {
					return !s.empty() &&
						std::all_of(s.begin(), s.end(),
							[](unsigned char c) { return std::isdigit(c) != 0; });
					};

				if (isAllDigits(left) && isAllDigits(right)) {

					const int world = std::atoi(left.c_str());
					const int stage = std::atoi(right.c_str());


					constexpr int kMaxStage = 8;
					if (stage >= kMaxStage) {
						// もう次が無いので、ステージセレクトに戻す
						SceneManager::GetInstance()->ChangeSceneWithTransition("STAGESELECT", TransitionType::Start);
						break;
					}
					// "1-1" -> "1-2"
					const std::string nextStageKey =
						std::to_string(world) + "-" + std::to_string(stage + 1);

					PlayContext::GetInstance().SetSelectedStage(
						PlayContext::GetInstance().GetSelectedStageId(),
						nextStageKey);
				}
			}


			SceneManager::GetInstance()->ChangeSceneWithTransition("GAMEPLAY", TransitionType::Start);
			break;
		}
		case ClearMenuItem::OneMore:
			SceneManager::GetInstance()->ChangeSceneWithTransition("STAGESELECT", TransitionType::Start);
			break;
		case ClearMenuItem::Select:
			SceneManager::GetInstance()->ChangeSceneWithTransition("GAMEPLAY", TransitionType::Start);
			break;
		default:
			break;
		}
		return; // シーン遷移したら以降の更新を止めたい場合
	}
	oneMore_->SetSize(oneMoreBaseSize_);
	select_->SetSize(selectBaseSize_);
	next_->SetSize(nextBaseSize_);
	
	// 回転させる
	Vector3 rotate = playerObject_->GetRotate();
	rotate.z += -0.1f;
	playerObject_->SetRotate(rotate);

	// 選択中だけ少し大きく
	auto Scale = [](const Vector2& v, float s) { return Vector2{ v.x * s, v.y * s }; };

	switch (selectedItem_) {
	case ClearMenuItem::NextStage:
		oneMore_->SetSize(Scale(oneMoreBaseSize_, selectScale_));
		break;
	case ClearMenuItem::OneMore:
		select_->SetSize(Scale(selectBaseSize_, selectScale_));
		break;
	case ClearMenuItem::Select:
		next_->SetSize(Scale(nextBaseSize_, selectScale_));
		break;
	default:
		break;
	}
	// タイトルへ戻る (1)
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
	// ステージセレクトへ戻る (2)
	else if (Input::GetInstance()->TriggerKey(DIK_2)) {
		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}
	// ゲームプレイへ戻る (3)
	else if (Input::GetInstance()->TriggerKey(DIK_3)) {
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}

	// 各オブジェクトの更新
	playerObject_->Update();
	backGround_->Update();
	// 各スプライトの更新
	clearTextSprite->Update();
	keyIcon_A->Update();
	keyIcon_D->Update();
	oneMore_->Update();
	select_->Update();
	next_->Update();

	// ImGuiの描画
	DrawImgui();
}

void StageClearScene::Draw()
{
	backGround_->Draw();
	playerObject_->Draw();
	clearTextSprite->Draw();

	keyIcon_A->Draw();

	keyIcon_D->Draw();
	oneMore_->Draw();
	select_->Draw();
	next_->Draw();
}

void StageClearScene::Finalize()
{
	// カメラの終了処理
	camera->Finalize();
}

void StageClearScene::DrawImgui()
{
#ifdef USE_IMGUI
	ImGui::Begin("StageClearScene");
	// シーン遷移の説明
	ImGui::Text("Press 1 to go to Title Scene");
	ImGui::Text("Press 2 to go to Stage Select Scene");
	ImGui::Text("Press 3 to go to Game Play Scene");

	// カメラ位置の調整
	ImGui::DragFloat3("CameraPos", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("CameraRot", &cameraTransform.rotate.x, 0.01f);
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	ImGui::Separator();
	
	

	ImGui::End();
#endif
}
