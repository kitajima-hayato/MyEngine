#include "ModelList.h"
#include "ModelManager.h"
void ModelList::LoadAllModel()
{
	/// LoadModelList
							   
							   
	/// TitleScene			   
	
	// BackGround
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/fluff");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/flower_pink");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/flower_purple");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/flower_orange");		   
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/mountain_mid");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/mountain_far");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/mountain_front");
	ModelManager::GetInstance().LoadModel("GamePlay/BackGround/sky");

	
							   
	/// StageSelectScene	   
	ModelManager::GetInstance().LoadModel("Scenes/StageSelect/Models/baseground");
	ModelManager::GetInstance().LoadModel("Scenes/StageSelect/Models/node");
	ModelManager::GetInstance().LoadModel("Scenes/StageSelect/Models/route");
							   
	/// GamePlayScene 
	// Player // Block  // Enemy
	ModelManager::GetInstance().LoadModel("GamePlay/Player");

	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/soilblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/grassblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/nullblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/breakblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/moveblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/sandblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/unbreakableblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/goalblock/up");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/goalblock/down");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/damageblock");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/toggleblock_on");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/toggleblock_on_off");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/toggleblock_off");
	ModelManager::GetInstance().LoadModel("GamePlay/Blocks/toggleblock_off_off");
							   
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/tentativeenemy");

	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/flyingenemy/head");
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/flyingenemy/oralarm");
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/normalenemy");
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/dropenemy/reactive");
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/dropenemy/timed");

	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/patrolenemy/hat");
	ModelManager::GetInstance().LoadModel("GamePlay/Enemies/patrolenemy/body");


	// GameOverScene
	ModelManager::GetInstance().LoadModel("Scenes/GameOver/Models/backblack");

	// DebugScene
	ModelManager::GetInstance().LoadModel("Scenes/Debug/BlackDome");
}
