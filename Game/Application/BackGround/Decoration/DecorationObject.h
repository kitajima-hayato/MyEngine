#pragma once
#include <string>
#include "MyMath.h"
#include "engine/3d/Object3D.h"

/// <summary>
/// 
/// </summary>

// 前方宣言
struct DecorationInstanceData;

class DecorationObject
{
public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const DecorationInstanceData& data);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:
	// モデルのパス
	std::string modelPath_;
	// トランスフォーム
	Transform transform_;
	// モデルデーター
	std::unique_ptr<Object3D> object_;
};

