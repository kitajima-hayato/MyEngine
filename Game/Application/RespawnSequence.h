#pragma once
#include "engine/math/MyMath.h"
#include <memory>
#include "engine/2d/Sprite.h"

class RespawnSequence
{
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(float dt = 1.0f / 60.0f);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

public:
	/// <summary>
	/// リスポーンシーケンス開始（暗転→真っ暗→明転）
	/// </summary>
	void Start();

	/// <summary>
	/// 実行中か（暗転中 or 明転中）
	/// </summary>
	bool IsRunning() const { return phase_ != Phase::None; }

	/// <summary>
	/// 暗転完了（真っ暗）しているか：この瞬間にワープすると自然
	/// </summary>
	bool IsDarkened() const { return isDarkened_; }

	/// <summary>
	/// 一連の演出が終了したか（明転完了）
	/// </summary>
	bool IsFinished() const { return isFinished_; }

	/// <summary>
	/// 暗転完了フラグを消費する（消費したら false になる）
	/// </summary>
	/// <returns></returns>
	bool ConsumeDarkened();

private:
	enum class Phase {
		None,
		Darken,
		Hold,
		Lighten
	};

private:
	void StartDarken();
	void StartLighten();
	void StartHold();

	/// <summary>
	/// 色補間を1ステップ進める（t:0→1）
	/// </summary>
	void UpdateFade(float dt);

private:
	// 画面暗転用のスプライト
	std::unique_ptr<Sprite> blackOverlay_;

	// フェード色（黒固定でalphaだけ変える想定）
	Vector4 startColor_{ 0.0f, 0.0f, 0.0f, 0.0f };
	Vector4 endColor_{ 0.0f, 0.0f, 0.0f, 1.0f };

	// 色の変化時間
	float colorChangeDuration_ = 0.6f; 
	// 暗転・明転の時間
	float darkenDuration_ = 0.6f;
	float lightenDuration_ = 0.3f;
	// 色の変化の経過時間
	float colorChangeElapsed_ = 0.0f;

	Phase phase_ = Phase::None;

	// 暗転完了フラグ（真っ暗到達）
	bool isDarkened_ = false;

	// 全体完了（明転完了）
	bool isFinished_ = false;


	// 真っ暗保持時間（ここを増やすとリスポーンが遅れる）
	float holdDuration_ = 1.0f;   // 例：0.3秒待つ
	float holdElapsed_ = 0.0f;
	
};