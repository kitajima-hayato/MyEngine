#include "BackGround.h"
#include "Game/Application/BackGround/Decoration/DecorationSpawnSetting.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif

void BackGround::Initialize()
{
	items_.clear();
	items_.reserve(7);

	// AddItem("Imguiで表示する名前", "設定するモデルのファイルパス", モデルの配置座標);

	AddItem("skyBack", "GamePlay/BackGround/sky", Transform{
		{ 400.0f,100.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 7.5f,0.0f,250.0f }
		});
	AddItem("titleBackGround3", "GamePlay/BackGround/mountain_far", Transform{
		{ 30.0f,50.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 7.5f,-4.0f,200.0f }
		});

	AddItem("titleBackGround3_2", "GamePlay/BackGround/mountain_far", Transform{
		{ 30.0f,50.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 170.0f,-4.0f,200.0f }
		});

	AddItem("titleBackGround2", "GamePlay/BackGround/mountain_mid", Transform{
		{ 30.0f,60.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 7.5f,-4.0f,150.0f }
		});

	AddItem("titleBackGround2_2", "GamePlay/BackGround/mountain_mid", Transform{
		{ 30.0f,60.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 170.0f,-4.0f,150.0f }
		});

	AddItem("titleBackGround2_3", "GamePlay/BackGround/mountain_mid", Transform{
		{ 30.0f,60.0f,1.0f },
		{ 0.0f,0.0f,0.0f },
		{ 350.0f,-4.0f,150.0f }
		});
	
	AddItem("soil", "GamePlay/Blocks/grassblock", Transform{
		{ 300.0f,1.0f,2.0f },
		{ 0.0f,0.0f,0.0f },
		{ 0.0f,-8.0f,20.0f }
		});

	// 花
	/*AddItem("flower_white_front_L", "GamePlay/BackGround/fluff", Transform{
	  { 2.0f, 2.0f, 2.0f }, { 0.0f, 0.15f, 0.0f }, { 4.1f, -7.7f, 21.0f }
		});
	AddItem("flower_pink_front_L", "GamePlay/BackGround/flower_pink", Transform{
	  { 2.0f, 2.0f, 2.0f }, { 0.0f,-0.10f, 0.0f }, { 6.2f, -8.0f, 21.7f }
		});

	AddItem("flower_orange_front_R", "GamePlay/BackGround/flower_orange", Transform{
	  { 1.5f, 1.5f, 1.5f }, { 0.0f, 0.20f, 0.0f }, { 8.2f, -7.8f, 21.0f }
		});
	AddItem("flower_blue_front_R", "GamePlay/BackGround/flower_purple", Transform{
	  { 2.0f, 2.0f, 2.0f }, { 0.0f,-0.20f, 0.0f }, { 10.6f, -8.0f, 21.2f }
		});

	AddItem("flower_pink_mid", "GamePlay/BackGround/flower_pink", Transform{
	  { 2.0f, 2.0f, 2.0f }, { 0.0f, 0.10f, 0.0f }, { 9.4f, -8.0f, 23.3f }
		});
	AddItem("flower_blue_mid", "GamePlay/BackGround/flower_purple", Transform{
	  { 2.0f, 2.0f, 2.0f }, { 0.0f,-0.15f, 0.0f }, { 13.4f, -8.0f, 21.8f }
		});*/

	DecorationSpawnSetting setting;

	setting.modelPaths = { 
		"GamePlay/BackGround/fluff",
		"GamePlay/BackGround/flower_pink",
		"GamePlay/BackGround/flower_orange", 
		"GamePlay/BackGround/flower_purple",
		"GamePlay/BackGround/flower_pink"
	};
	setting.areaMin = { 0.0f, -7.5f, 20.0f };
	setting.areaMax = { 150.0f, -7.5f, 20.0f };

	setting.objectCount = 100;
	setting.spawnRangeInterval = 0.8f;

	setting.minScale = { 0.5f, 0.5f, 1.0f };
	setting.maxScale = { 1.5f, 1.5f, 1.0f };

	setting.minRotation = { 0.0f, -0.1f, 0.0f };
	setting.maxRotation = { 0.0f, 0.1f, 0.0f };

	setting.maxTrialCount = 100;

	// 群れの設定値
	// 群れの数
	setting.clusterCountMin = 2;
	setting.clusterCountMax = 4;
	// 1群れ当たりの花の数
	setting.clusterSizeMin = 4;
	setting.clusterSizeMax = 8;
	// 群れの広がり半径
	setting.clusterRadiusMin = 1.5f;
	setting.clusterRadiusMax = 3.0f;
	// 単独で生える花の割合
	setting.singleSpawnRatio = 0.1f;
	setting.spawnRangeInterval = 0.4f;

	setting.fixY = true;
	setting.fixZ = true;

	flowerField_.Generate(setting);
	
}

void BackGround::Update()
{
	// 各オブジェクトの更新
	for (auto& item : items_) {
		item.object->SetTransform(item.transform);
		item.object->Update();
	}
	// 花の自動配置
	flowerField_.Update();

	// Imguiの描画
	DrawImgui();
}

void BackGround::Draw()
{
	// 各オブジェクトの描画	
	for (auto& item : items_) {
		item.object->Draw();
	}

	flowerField_.Draw();
}

void BackGround::DrawImgui()
{
#ifdef USE_IMGUI
	ImGui::Begin("BackGround Transform");

	for (auto& item : items_) {
		std::string sScale = item.debugName + " Scale";
		std::string sRotate = item.debugName + " Rotate";
		std::string sTrans = item.debugName + " Translate";

		ImGui::DragFloat3(sScale.c_str(), &item.transform.scale.x, 0.1f);
		ImGui::DragFloat3(sRotate.c_str(), &item.transform.rotate.x, 0.1f);
		ImGui::DragFloat3(sTrans.c_str(), &item.transform.translate.x, 0.1f);

		// 変更を即時反映
		item.object->SetTransform(item.transform);
	}

	ImGui::End();
#endif
}

void BackGround::AddItem(const char* debugName, const char* modelName, const Transform& transform)
{
	BgItem item;
	item.debugName = debugName;
	item.modelName = modelName;
	item.transform = transform;

	item.object = std::make_unique<Object3D>();
	item.object->Initialize();
	item.object->SetModel(modelName);
	item.object->SetTransform(transform);

	items_.push_back(std::move(item));
}