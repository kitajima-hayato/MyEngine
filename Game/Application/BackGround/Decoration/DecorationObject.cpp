#include "DecorationObject.h"
#include "DecorationInstanceData.h"

void DecorationObject::Initialize(const DecorationInstanceData& data)
{
	// モデルパスのコピー
	modelPath_ = data.modelPath;
	// 数値の受け渡し
	transform_ = data.transform;

	// モデルの初期化
	object_ = std::make_unique<Object3D>();
	object_->Initialize();
	object_->SetModel(modelPath_);
	object_->SetTransform(transform_);
}


void DecorationObject::Update()
{
	// モデルの更新処理
	if (object_) {
		object_->SetTransform(transform_);
		object_->Update();
	}
}

void DecorationObject::Draw()
{
	// モデルの描画処理
	if (object_) {
		object_->Draw();
	}
}
