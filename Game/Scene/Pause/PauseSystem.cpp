#include "PauseSystem.h"
#include "engine/Input/Input.h"

#include "engine/InsideScene/SceneManager.h"


void PauseSystem::Initialize()
{
	pauseUI_ = std::make_unique<PauseUI>();
	pauseUI_->Initialize();
}

bool PauseSystem::Update()
{
	Input* input = Input::GetInstance();

	constexpr int kControllerNo = 0;

	// Escキー / コントローラーStartボタンでポーズの切り替え
	if (input->TriggerKey(DIK_ESCAPE)|| 
		input->TriggerButton(kControllerNo,ControllerButtonType::Start)) 
	{
		isPause_ = !isPause_;
	}
	if (isPause_) {
		// ポーズ画面
		pauseUI_->Update();
		// 続けるが選ばれて決定したらポーズ解除
		if (pauseUI_->PauseReleaseRequested()) {
			isPause_ = false;
		}

		switch (pauseUI_->ConsumeAction())
		{
			
			case PauseUI::Action::Retry:
				// リトライ処理
				SceneManager::GetInstance()->ChangeSceneWithTransition("GAMEPLAY", TransitionType::Start);
				break;
			case PauseUI::Action::StageSelect:
				// ステージセレクトへ
				SceneManager::GetInstance()->ChangeSceneWithTransition("STAGESELECT", TransitionType::Start);
				break;
			case PauseUI::Action::Title:
				// タイトルへ
				SceneManager::GetInstance()->ChangeSceneWithTransition("TITLE", TransitionType::Start);
				break;
		}

		// ポーズ中にもimguiを表示する
		DrawImgui();
		// ゲームの更新を行わない
		return true;
	}
	
	return isPause_;
}

void PauseSystem::Draw()
{
	if (isPause_) {
		// ポーズ画面
		pauseUI_->Draw();
	}
}

void PauseSystem::DrawImgui()
{
	if (isPause_) {
		pauseUI_->DrawImGui();
	}
}
