#pragma once
#include <cstdint>
#include "Sprite.h"
#include <memory>
#include <vector>
#include <MyMath.h>
class PauseUI
{
	enum class Slot : uint32_t {
		// 上 (W)
		Continue = 0,
		// 右 (D)
		Retry = 1,
		// 左 (A)
		StageSelect = 2,
		// 下 (S)
		Title = 3,
		Count
	};
	enum class UiAnimState { Idle, Decide };
public:
	enum class Action {
		None,
		Continue,
		Retry,
		StageSelect,
		Title
	};
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
	/// スプライト読み込み
	/// </summary>
	void LoadSprite();

	/// <summary>
	/// ImGuiの描画
	/// </summary>
	void DrawImGui();

	/// <summary>
	/// ポーズの解除要求
	/// </summary>
	bool PauseReleaseRequested();

	/// <summary>
	/// 決定入力処理
	/// </summary>
	void HandleDecideInput();

	/// <summary>
	/// 方向入力処理
	/// </summary>
	void HandleDirectionInput();

	void ApplySelectedStyle(Sprite* sp,
		const Vector2& basePos,
		const Vector2& baseSize,
		bool selected,
		const Vector2& outwardDir);

	float GetBounceExtra();



public:	// Getter / Setter
	/// <summary>
	/// 選択中のスロット取得
	/// </summary>
	Slot GetSelectedSlot() const { return selectedSlot_; }

	Action ConsumeAction();

private:
	// メニュー項目数
	Slot selectedSlot_ = Slot::Continue;

	Action pendingAction_ = Action::None;

	// ポーズ解除要求フラグ
	bool requestPauseRelease_ = false;

	std::vector<std::unique_ptr<Sprite>> sprites_;

	// スロットUI
	std::unique_ptr<Sprite> slotUIUp_;
	std::unique_ptr<Sprite> slotUIDown_;
	std::unique_ptr<Sprite> slotUILeft_;
	std::unique_ptr<Sprite> slotUIRight_;

	// 選択中のスロットのテキストスプライト
	std::unique_ptr<Sprite> slotTextUIUp_;
	std::unique_ptr<Sprite> slotTextUIDown_;
	std::unique_ptr<Sprite> slotTextUILeft_;
	std::unique_ptr<Sprite> slotTextUIRight_;

	// Space
	std::unique_ptr<Sprite> decideKeyUI_;

	
	// 選択時のスロットのカラー
	// PauseUI.h
	const Vector4 normalSlotColor_{ 1.0f, 1.0f, 1.0f, 1.0f };
	const Vector4 selectedSlotColor_{ 0.05f, 1.0f, 1.0f, 1.0f }; 

	// 基準（非選択）時の位置とサイズを保存しておく
	Vector2 basePosUp_, basePosRight_, basePosDown_, basePosLeft_;
	Vector2 baseSizeUp_, baseSizeRight_, baseSizeDown_, baseSizeLeft_;

	UiAnimState uiAnimState_ = UiAnimState::Idle;

	Slot prevSelectedSlot_ = Slot::Continue;
	// バウンス用
	float bounceT_ = 1.0f;       
	float bounceDuration_ = 0.10f; 

	

};

