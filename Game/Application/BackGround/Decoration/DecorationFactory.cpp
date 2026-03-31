#include "DecorationFactory.h"
#include "DecorationObject.h"

std::unique_ptr<DecorationObject> DecorationFactory::Create(const DecorationInstanceData& data)
{
	// DecorationObjectのインスタンスを生成
	auto object = std::make_unique<DecorationObject>();
	// 受け取ったデータで初期化
	object->Initialize(data);
	// 生成したオブジェクトを返す
	return object;
}
