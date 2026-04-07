#pragma once
#include <memory>
#include "engine/2d/Sprite.h"
#include "engine/math/MyMath.h"

class Player;

class GamePlayHUD
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="dt">ゲーム内時間</param>
	void Update();

	/// <summary>
	/// 操作UIの更新
	/// </summary>
	void UpdateControlUI();

	/// <summary>
	/// ポーズアイコンの更新
	/// </summary>
	void UpdatePauseIcon();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="isPaused">ポーズ中か</param>
	/// <param name="showControls">操作系UIが見える状態か</param>
	void Draw(bool isPaused,bool showControls);


	/// <summary>
	/// ImGuiの描画
	/// </summary>
	void DrawImGui();

	/// <summary>
	/// スプライトの生成ヘルパ
	/// </summary>
	std::unique_ptr<Sprite> MakeSprite(const std::string& path, const Vector2& pos, const Vector2& size, const Vector4 color);

	/// <summary>
	/// プレイヤーのポインタをセット
	/// </summary>
	void SetPlayer(Player* player) { player_ = player; }

	/// <summary>
	/// 
	/// </summary>
	void SetScreenSize(float w, float h) { screenSize_ = { w, h }; }
private:
	// プレイヤーのポインタ
	Player* player_ = nullptr;

	// 操作UI
	// DキーUI
	std::unique_ptr<Sprite> controlUI_D;
	Vector2 controlUI_DPos_ = { 100.0f, 600.0f };
	Vector4 controlUIColorD_ = { 1.0f,1.0f,1.0f,0.5f };
	// AキーUI
	std::unique_ptr<Sprite>controlUI_A;
	Vector2 controlUIAPos_ = { 50.0f,600.0f };
	Vector4 controlUIColorA_ = { 1.0f,1.0f,1.0f,0.5f };
	// WキーUI
	std::unique_ptr<Sprite>controlUI_W;
	Vector2 controlUIWPos_ = { 75.0f,550.0f };
	Vector4 controlUIColorW_ = { 1.0f,1.0f,1.0f,0.5f };
	// SキーUI
	std::unique_ptr<Sprite>controlUI_S;
	Vector2 controlUISPos_ = { 75.0f,650.0f };
	Vector4 controlUIColorS_ = { 1.0f,1.0f,1.0f,0.5f };

	// ポーズ案内(esc)
	std::unique_ptr<Sprite> pauseSprite_;

	// UIアクティブフラグ
	bool UiActive_ = false;
	int32_t uiTimer = 0;

	// 非アクティブカラー / 押されていない時は半透明
	Vector4 inactiveColor_ = { 1.0f,1.0f,1.0f,0.5f };
	// アクティブカラー / 押されている時は不透明
	Vector4 activeColor_ = { 1.0f,1.0f,1.0f,1.0f };

	// 押された時のオフセット量
	float pressedOffset_ = 5.0f;

	// HP表示用のハートスプライト
	std::vector<std::unique_ptr<Sprite>> hpHearts_;
	uint32_t maxHp_ = 3;
	// ハートの配置開始位置とサイズ、間隔
	Vector2 heartStartPos_ = { 20.0f, 90.0f };
	Vector2 heartSize_ = { 60.0f, 60.0f };
	float heartSpacingX_ = 8.0f;
	// ハートの色
	Vector4 heartActiveColor_ = { 1,1,1,1 };
	Vector4 heartInactiveColor_ = { 1,1,1,0.2f }; 

	// ハートのテクスチャパス
	std::string heartTexturePath_ = "resources/GamePlay/HP/HPUI.png";

	// 画面サイズ（UI配置の基準）
	Vector2 screenSize_ = { 1280.0f, 720.0f }; 
	Vector2 heartMargin_ = { 20.0f, 20.0f };   

	
};

