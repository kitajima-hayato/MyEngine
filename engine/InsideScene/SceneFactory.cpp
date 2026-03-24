#include "SceneFactory.h"
#include "Game/Scene/TitleScene.h"
#include "Game/Scene/GamePlayScene.h"
#include "Game/Scene/StageSelectScene.h"
#include "Game/Scene/StageClearScene.h"
#include "Game/Scene/GameOverScene.h"
#include "Game/Scene/DebugScene/DebugScene.h"
std::unique_ptr<BaseScene> SceneFactory::CreateScene(const std::string& sceneName)
{
    // 次のシーン生成
	if (sceneName == "TITLE") {
		return std::make_unique<TitleScene>();
	}
	else if (sceneName == "GAMEPLAY") {
		return std::make_unique<GamePlayScene>();
	}
	else if (sceneName == "STAGESELECT") {
		return std::make_unique<StageSelectScene>();
	}
	else if (sceneName == "STAGECLEAR") {
		return std::make_unique<StageClearScene>();
	}
	else if (sceneName == "GAMEOVER") {
		return std::make_unique<GameOverScene>();
	}
	// デバッグ用シーン / MyGameから直接切り替えを行う
	else if(sceneName == "DEBUG") {
		return std::make_unique<DebugScene>();
	}
	return nullptr;
}
