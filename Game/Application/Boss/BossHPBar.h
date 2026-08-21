#pragma once
#include "engine/2d/Sprite.h"
#include <memory>
#include <array>
#include <cstdint>

class BossHPBar
{
public:
    /// <summary>
	/// 初期化
    /// </summary>
    void Initialize();
    /// <summary>
    /// 更新
    /// </summary>
    /// <param name="hp">0-9</param>
    void Update(uint32_t hp);
    /// <summary>
    /// 描画
    /// </summary>
    void Draw();

private:
	// HPバーの位置とサイズ
    static constexpr float kPhaseW_ = 200.0f; 
    static constexpr float kBarH_ = 26.0f; 
    static constexpr float kGap_ = 4.0f;   
    static constexpr float kTotalW_ = kPhaseW_ * 3.0f + kGap_ * 2.0f;
    static constexpr float kBarX_ = (1280.0f - kTotalW_) * 0.5f;  
    static constexpr float kBarY_ = 20.0f;

	// HPバーのスプライト
	// 背景
    std::unique_ptr<Sprite> bg_;
	// HPゲージの塗りつぶし
    std::array<std::unique_ptr<Sprite>, 3> fill_; 
	// HPゲージの区切り線
    std::array<std::unique_ptr<Sprite>, 2> sep_;  
};