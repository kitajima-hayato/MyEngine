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
	Input* input = Input::GetInstance();

	constexpr int kControllerNo = 0;
	constexpr float kStickThreshold = 0.6f;

	camera->Update();

	//==================================================
	// 左右入力
	//==================================================

	const StickState stick = input->GetLeftStickState(kControllerNo);
	const StickState prevStick = input->GetController()->GetLeftStickStatePrevious(kControllerNo);

	const bool stickLeft =
		prevStick.x >= -kStickThreshold &&
		stick.x < -kStickThreshold;

	const bool stickRight =
		prevStick.x <= kStickThreshold &&
		stick.x > kStickThreshold;

	const bool inputLeft =
		input->TriggerKey(DIK_A) ||
		input->TriggerKey(DIK_LEFT) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadLEFT) ||
		stickLeft;

	const bool inputRight =
		input->TriggerKey(DIK_D) ||
		input->TriggerKey(DIK_RIGHT) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadRIGHT) ||
		stickRight;

	// 左 = 前の項目、右 = 次の項目
	int move = 0;
	if (inputLeft) {
		move = -1;
	} else if (inputRight) {
		move = +1;
	}

	if (move != 0) {
		const int count = static_cast<int>(ClearMenuItem::Count);
		int idx = static_cast<int>(selectedItem_);
		idx = (idx + move + count) % count;
		selectedItem_ = static_cast<ClearMenuItem>(idx);
	}

	//==================================================
	// 決定
	//==================================================

	const bool decide =
		input->TriggerKey(DIK_SPACE) ||
		input->TriggerKey(DIK_RETURN) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::A);

	if (decide) {
		switch (selectedItem_) {
		case ClearMenuItem::NextStage:
		{
			// 左の「次のステージ」
			const std::string& currentStageKey =
				PlayContext::GetInstance().GetSelectedStageKey();

			const size_t dash = currentStageKey.find('-');

			if (dash != std::string::npos) {
				const std::string left = currentStageKey.substr(0, dash);
				const std::string right = currentStageKey.substr(dash + 1);

				auto isAllDigits = [](const std::string& s) {
					return !s.empty() &&
						std::all_of(
							s.begin(),
							s.end(),
							[](unsigned char c) {
								return std::isdigit(c) != 0;
							}
						);
					};

				if (isAllDigits(left) && isAllDigits(right)) {
					const int world = std::atoi(left.c_str());
					const int stage = std::atoi(right.c_str());

					constexpr int kMaxStage = 8;

					if (stage >= kMaxStage) {
						SceneManager::GetInstance()->ChangeSceneWithTransition(
							"STAGESELECT",
							TransitionType::Start
						);
						return;
					}

					const std::string nextStageKey =
						std::to_string(world) + "-" + std::to_string(stage + 1);

					PlayContext::GetInstance().SetSelectedStage(
						PlayContext::GetInstance().GetSelectedStageId(),
						nextStageKey
					);
				}
			}

			SceneManager::GetInstance()->ChangeSceneWithTransition(
				"GAMEPLAY",
				TransitionType::Start
			);
			return;
		}

		case ClearMenuItem::OneMore:
			// 中央の「もう一度」
			SceneManager::GetInstance()->ChangeSceneWithTransition(
				"GAMEPLAY",
				TransitionType::Start
			);
			return;

		case ClearMenuItem::Select:
			// 右の「セレクト画面」
			SceneManager::GetInstance()->ChangeSceneWithTransition(
				"STAGESELECT",
				TransitionType::Start
			);
			return;

		default:
			break;
		}
	}

	//==================================================
	// 選択中UI
	//==================================================

	oneMore_->SetSize(oneMoreBaseSize_);
	next_->SetSize(nextBaseSize_);
	select_->SetSize(selectBaseSize_);

	auto Scale = [](const Vector2& v, float s) {
		return Vector2{ v.x * s, v.y * s };
		};

	switch (selectedItem_) {
	case ClearMenuItem::NextStage:
		// 左の「次のステージ」
		oneMore_->SetSize(Scale(oneMoreBaseSize_, selectScale_));
		break;

	case ClearMenuItem::OneMore:
		// 中央の「もう一度」
		next_->SetSize(Scale(nextBaseSize_, selectScale_));
		break;

	case ClearMenuItem::Select:
		// 右の「セレクト画面」
		select_->SetSize(Scale(selectBaseSize_, selectScale_));
		break;

	default:
		break;
	}

	//==================================================
	// 通常更新
	//==================================================

	Vector3 rotate = playerObject_->GetRotate();
	rotate.z += -0.1f;
	playerObject_->SetRotate(rotate);

	playerObject_->Update();
	backGround_->Update();

	clearTextSprite->Update();
	keyIcon_A->Update();
	keyIcon_D->Update();
	oneMore_->Update();
	next_->Update();
	select_->Update();

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
