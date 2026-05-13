#include "GamePlayHUD.h"
#include "engine/Input/Input.h"

#include "ImGuiManager.h"
#include "Game/Application/Player/Player.h"

void GamePlayHUD::Initialize()
{
	// ポーズアイコン
	pauseSprite_ = MakeSprite("resources/_Common/UI/Texture/inputhints/Esc.png",
		{ 20.0f,10.0f }, { 125.0f,70.0f }, { 1.0f,1.0f,1.0f,1.0f });
	// コントロールUI
	// Dキー
	controlUI_D = MakeSprite("resources/_Common/UI/Texture/inputhints/D.dds",
		{ 200.0f,70.0f }, { 50.0f,50.0f }, { 1.0f,1.0f,1.0f,0.5f });
	// Aキー
	controlUI_A = MakeSprite("resources/_Common/UI/Texture/inputhints/A.dds",
		{ 100.0f,70.0f }, { 50.0f,50.0f }, { 1.0f,1.0f,1.0f,0.5f });
	// Sキー
	controlUI_S = MakeSprite("resources/_Common/UI/Texture/inputhints/S.dds",
		{ 150.0f,70.0f }, { 50.0f,50.0f }, { 1.0f,1.0f,1.0f,0.5f });
	// Wキー
	controlUI_W = MakeSprite("resources/_Common/UI/Texture/inputhints/W.dds",
		{ 150.0f,20.0f }, { 50.0f,50.0f }, { 1.0f,1.0f,1.0f,0.5f });

	UiActive_ = false;
	uiTimer = 0;

	// 右上に配置するために開始座標を計算
	const float totalWidth =
		maxHp_ * heartSize_.x + (maxHp_ - 1) * heartSpacingX_;

	heartStartPos_.x = screenSize_.x - heartMargin_.x - totalWidth;
	heartStartPos_.y = heartMargin_.y;

	// ハート生成
	hpHearts_.clear();
	hpHearts_.reserve(maxHp_);

	for (uint32_t i = 0; i < maxHp_; ++i) {
		Vector2 pos = heartStartPos_;
		pos.x += i * (heartSize_.x + heartSpacingX_);

		auto heart = MakeSprite(heartTexturePath_, pos, heartSize_, heartActiveColor_);
		hpHearts_.push_back(std::move(heart));
	}
	
}

void GamePlayHUD::Update()
{
	// UI
	pauseSprite_->Update();
	controlUI_D->Update();
	controlUI_A->Update();
	controlUI_S->Update();
	controlUI_W->Update();
	// 操作UIの更新
	UpdateControlUI();
	// HP表示の更新
	for (auto& h : hpHearts_) {
		if (h) { h->Update(); }
	}
}

void GamePlayHUD::UpdateControlUI()
{
	// 押されているキーだけ色を濃くする
	// D
	if (Input::GetInstance()->PushKey(DIK_D)) {
		// 色を濃くする
		controlUI_D->SetColor(activeColor_);
		// 少しだけ右にずらす
		controlUI_D->SetPosition({ 200 + pressedOffset_,70.0f });
	} else {
		// 元の薄い色に戻す
		controlUI_D->SetColor(inactiveColor_);
		// 元の位置に戻す
		controlUI_D->SetPosition({ 200.0f,70.0f });
	}
	// A
	 if (Input::GetInstance()->PushKey(DIK_A)) {
        controlUI_A->SetColor(activeColor_);
        controlUI_A->SetPosition({ 100.0f - pressedOffset_, 70.0f });
    } else {
        controlUI_A->SetColor(inactiveColor_);
        controlUI_A->SetPosition({ 100.0f, 70.0f });
    }

    // S
    if (Input::GetInstance()->PushKey(DIK_S)) {
        controlUI_S->SetColor(activeColor_);
        controlUI_S->SetPosition({ 150.0f, 70.0f + pressedOffset_ });
    } else {
        controlUI_S->SetColor(inactiveColor_);
        controlUI_S->SetPosition({ 150.0f, 70.0f });
    }

    // W
    if (Input::GetInstance()->PushKey(DIK_W)) {
        controlUI_W->SetColor(activeColor_);
        controlUI_W->SetPosition({ 150.0f, 20.0f - pressedOffset_ });
    } else {
        controlUI_W->SetColor(inactiveColor_);
        controlUI_W->SetPosition({ 150.0f, 20.0f });
    }
}

void GamePlayHUD::UpdatePauseIcon()
{
}

void GamePlayHUD::Draw(bool isPaused, bool showControls)
{
	// ポーズアイコンを表示
	if (!isPaused) {
		pauseSprite_->Draw();
	}
	
	// 操作系UI表示
	if (showControls) {
		controlUI_D->Draw();
		controlUI_A->Draw();
		controlUI_S->Draw();
		controlUI_W->Draw();
	}
	
	// HP表示（Playerがセットされている前提）
	if (player_) {
		const uint32_t hp = player_->GetHealth();

		for (uint32_t i = 0; i < hpHearts_.size(); ++i) {
			if (!hpHearts_[i]) continue;

			if (i < hp) {
				hpHearts_[i]->SetColor(heartActiveColor_);
			} else {
				hpHearts_[i]->SetColor(heartInactiveColor_);
			}
			hpHearts_[i]->Draw();
		}
	}
}

void GamePlayHUD::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("GamePlay HUD");
	ImGui::SeparatorText("Pause Icon");
	Vector2 pausePos = pauseSprite_->GetPosition();
	Vector2 pauseSize = pauseSprite_->GetSize();
	ImGui::DragFloat2("Pause Position", &pausePos.x, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat2("Pause Size", &pauseSize.x, 1.0f, 0.0f, 500.0f);
	pauseSprite_->SetPosition(pausePos);
	pauseSprite_->SetSize(pauseSize);
	ImGui::SeparatorText("Control UI Positions and Sizes");
	Vector2 dPos = controlUI_D->GetPosition();
	Vector2 aPos = controlUI_A->GetPosition();
	Vector2 sPos = controlUI_S->GetPosition();
	Vector2 wPos = controlUI_W->GetPosition();
	Vector2 dSize = controlUI_D->GetSize();
	Vector2 aSize = controlUI_A->GetSize();
	Vector2 sSize = controlUI_S->GetSize();
	Vector2 wSize = controlUI_W->GetSize();
	ImGui::DragFloat2("D Position", &dPos.x, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat2("D Size", &dSize.x, 1.0f, 0.0f, 500.0f);
	ImGui::DragFloat2("A Position", &aPos.x, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat2("A Size", &aSize.x, 1.0f, 0.0f, 500.0f);
	ImGui::DragFloat2("S Position", &sPos.x, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat2("S Size", &sSize.x, 1.0f, 0.0f, 500.0f);
	ImGui::DragFloat2("W Position", &wPos.x, 1.0f, 0.0f, 1280.0f);
	ImGui::DragFloat2("W Size", &wSize.x, 1.0f, 0.0, 500.0f);
	controlUI_D->SetPosition(dPos);
	controlUI_D->SetSize(dSize);
	controlUI_A->SetPosition(aPos);
	controlUI_A->SetSize(aSize);
	controlUI_S->SetPosition(sPos);
	controlUI_S->SetSize(sSize);
	controlUI_W->SetPosition(wPos);
	controlUI_W->SetSize(wSize);
	ImGui::End();
#endif
}

std::unique_ptr<Sprite> GamePlayHUD::MakeSprite(const std::string& path, const Vector2& pos, const Vector2& size, const Vector4 color)
{
	// スプライト生成
	auto sprite = std::make_unique<Sprite>();
	sprite->Initialize(path);
	sprite->SetPosition(pos);
	sprite->SetSize(size);
	sprite->SetColor(color);
	return sprite;
}
