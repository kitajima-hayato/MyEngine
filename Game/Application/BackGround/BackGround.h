#pragma once
#include "engine/3d/Object3D.h"
#include "engine/math/MyMath.h"
#include <memory>
#include "Game/Application/BackGround/Decoration/DecorationField.h"
/// <summary>
/// 背景クラス
/// 各ステージの背景を管理する
/// </summary>

class BackGround
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// Imgui一括管理 / 描画関数
	/// </summary>
	void DrawImgui();
private:
	// タイトル背景
	struct BgItem {
		std::string debugName;              // ImGui用ラベル
		std::string modelName;              // 読み込むモデル名
		Transform transform{};              // 初期Transform & 現在Transform
		std::unique_ptr<Object3D> object;   // 実体
	};

	std::vector<BgItem>items_;

	// items_ の追加ヘルパ（Initializeを短くする用）
	void AddItem(const char* debugName, const char* modelName, const Transform& transform);


	DecorationField flowerField_;
};

