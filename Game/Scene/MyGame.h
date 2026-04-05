#pragma once
#include"externals/DirectXTex/d3dx12.h"
#include "engine/InsideScene/Framework.h"
#include "engine/3d/SkyBox/SkyBox.h"
/// <summary>
/// ゲーム全体 : フレームワーククラス継承
/// </summary>
class MyGame :public Framework
{
public:	// メンバ関数
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update()override;
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw()override;
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;


private:	// メンバ変数
	
	// スカイボックス
	std::unique_ptr<SkyBox> skyBox = nullptr;

	// レンダーテクスチャ
	bool usePostProcess_ = false;

	bool didInstancingWarmup_ = false;
};

