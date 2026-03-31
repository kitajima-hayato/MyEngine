#include "DecorationField.h"
#include "DecorationPlacer.h"
#include "DecorationFactory.h"
#include "DecorationObject.h"

void DecorationField::Generate(const DecorationSpawnSetting& setting)
{
	// DecorationPlacerを使って配置データのリストを作成
	objects_.clear();

	// 
	DecorationPlacer placer;

	// 配置データのリストを作成
	std::vector<DecorationInstanceData> layout = 
		placer.CreateLayout(setting);

	for (const auto& data : layout) {
		auto object = DecorationFactory::Create(data);
		if (object) {
			objects_.push_back(std::move(object));
		}
	}
}

void DecorationField::Update()
{
	// 生成したオブジェクトを一括更新
	for (auto& object : objects_) {
		object->Update();
	}
}

void DecorationField::Draw()
{
	// 生成したオブジェクトを一括描画
	for (auto& object : objects_) {
		object->Draw();
	}
}
