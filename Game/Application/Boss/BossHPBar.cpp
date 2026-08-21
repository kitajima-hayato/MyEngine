#include "BossHPBar.h"
#include <algorithm>

static const char* kWhite = "resources/_Common/UI/Texture/utility/white.png";

// uint32_t の型を引数として使うとき 3u や 8u となっているのは
// 明示的に符号無し32bit整数として扱うための指示。これをサフィックスと呼ぶ

// スプライトを作成する補助関数
static std::unique_ptr<Sprite> MakeRect(Vector2 pos, Vector2 sz, Vector4 col)
{
	// 白い四角形のスプライトを作成する
    auto s = std::make_unique<Sprite>();
    s->Initialize(kWhite);
    s->SetPosition(pos);
    s->SetSize(sz);
    s->SetColor(col);
    s->Update();
    return s;
}

void BossHPBar::Initialize()
{
	// HPバーの背景の四角形
    float totalW = kPhaseW_ * 3.0f + kGap_ * 2.0f;
    // 背景の情報入力
    bg_ = MakeRect({ kBarX_ - 4.0f, kBarY_ - 4.0f },
        { totalW + 8.0f, kBarH_ + 8.0f },
        { 0.0f, 0.0f, 0.0f, 0.8f });
    // PhaseごとのHPゲージの塗りつぶし
    const Vector4 kColors[3] = {
        {0.9f, 0.2f, 0.2f, 1.0f},
        {1.0f, 0.6f, 0.1f, 1.0f},
        {0.3f, 0.9f, 0.3f, 1.0f},
    };
	// HPゲージの塗りつぶしの情報入力
    for (int i = 0; i < 3; ++i) {
        float x = kBarX_ + i * (kPhaseW_ + kGap_);
        fill_[i] = MakeRect({ x, kBarY_ }, { kPhaseW_, kBarH_ }, kColors[i]);
    }

    // 区切り線
    for (int i = 0; i < 2; ++i) {
        float x = kBarX_ + (i + 1) * kPhaseW_ + i * kGap_;
        sep_[i] = MakeRect({ x, kBarY_ - 2.0f }, { kGap_, kBarH_ + 4.0f },
            { 1.0f, 1.0f, 1.0f, 0.6f });
    }
}

void BossHPBar::Update(uint32_t hp)
{
	// HPの最大値は9に制限
    hp = std::min<uint32_t>(hp, 9u);

	// HPを3つのPhaseに分ける
    uint32_t hp3 = std::min<uint32_t>(hp, 3u);
    uint32_t hp2 = std::min<uint32_t>(hp, 6u) - hp3;
    uint32_t hp1 = hp - std::min<uint32_t>(hp, 6u);
    const uint32_t hps[3] = { hp3, hp2, hp1 };

	// HPゲージの塗りつぶしのサイズを更新
    for (int i = 0; i < 3; ++i) {
        fill_[i]->SetSize({ kPhaseW_ * (hps[i] / 3.0f), kBarH_ });
        fill_[i]->Update();
    }
    // 背景と区切り線の更新
    bg_->Update();
    for (auto& s : sep_) s->Update();
}

void BossHPBar::Draw()
{
	// 描画
    bg_->Draw();
    for (auto& f : fill_) f->Draw();
    for (auto& s : sep_) s->Draw();
}