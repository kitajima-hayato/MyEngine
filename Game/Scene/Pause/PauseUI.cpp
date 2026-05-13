#include "PauseUI.h"
#include "engine/Input/Input.h"
#include "ImGuiManager.h"


static float Clamp01(float x) {
	if (x < 0.0f) return 0.0f;
	if (x > 1.0f) return 1.0f;
	return x;
}

float PauseUI::GetBounceExtra()
{
	if (bounceT_ >= bounceDuration_) {
		return 0.0f;
	}

	// 60fps想定で進める（dtがあるなら bounceT_ += dt; に置き換え）
	bounceT_ += 1.0f / 60.0f;

	float t = Clamp01(bounceT_ / bounceDuration_); // 0→1
	float s = std::sinf(3.1415926f * t);           // 0→1→0

	// 1.10 → 1.15 → 1.10 にしたいので上乗せ最大 +0.05
	return 0.05f * s;
}

PauseUI::Action PauseUI::ConsumeAction()
{
	PauseUI::Action action = pendingAction_;
	pendingAction_ = PauseUI::Action::None;
	return action;
}



void PauseUI::Initialize()
{
	// 選択インデックス初期化
	selectedSlot_ = Slot::Continue;
	// ポーズ解除要求フラグの初期化
	requestPauseRelease_ = false;

	// スプライト読み込み
	LoadSprite();
}

void PauseUI::Update()
{
	// 入力処理
	HandleDirectionInput();
	// 選択が変わった瞬間だけバウンス開始
	if (selectedSlot_ != prevSelectedSlot_) {
		prevSelectedSlot_ = selectedSlot_;
		bounceT_ = 0.0f; // バウンス開始
	}


	// 決定入力処理
	HandleDecideInput();


	for (const auto& sprite : sprites_) {
		sprite->Update();
	}
	slotUIUp_->Update();
	slotUIDown_->Update();
	slotUILeft_->Update();
	slotUIRight_->Update();


	slotUIUp_->SetColor((selectedSlot_ == Slot::Continue) ? selectedSlotColor_ : normalSlotColor_);
	slotUIRight_->SetColor((selectedSlot_ == Slot::Retry) ? selectedSlotColor_ : normalSlotColor_);
	slotUIDown_->SetColor((selectedSlot_ == Slot::Title) ? selectedSlotColor_ : normalSlotColor_);
	slotUILeft_->SetColor((selectedSlot_ == Slot::StageSelect) ? selectedSlotColor_ : normalSlotColor_);

	slotTextUIUp_->Update();
	slotTextUIDown_->Update();
	slotTextUILeft_->Update();
	slotTextUIRight_->Update();

	decideKeyUI_->Update();

	// 方向（外側）ベクトル：上= (0,-1), 右=(1,0), 下=(0,1), 左=(-1,0)
	ApplySelectedStyle(slotUIUp_.get(), basePosUp_, baseSizeUp_,
		selectedSlot_ == Slot::Continue, { 0.0f, -1.0f });

	ApplySelectedStyle(slotUIRight_.get(), basePosRight_, baseSizeRight_,
		selectedSlot_ == Slot::Retry, { 1.0f,  0.0f });

	ApplySelectedStyle(slotUIDown_.get(), basePosDown_, baseSizeDown_,
		selectedSlot_ == Slot::Title, { 0.0f,  1.0f });

	ApplySelectedStyle(slotUILeft_.get(), basePosLeft_, baseSizeLeft_,
		selectedSlot_ == Slot::StageSelect, { -1.0f, 0.0f });


	// ImGuiの描画
	DrawImGui();
}

void PauseUI::Draw()
{
	for (const auto& sprite : sprites_) {
		sprite->Draw();
	}

	// スロットUI描画
	slotUIUp_->Draw();
	slotUIDown_->Draw();
	slotUILeft_->Draw();
	slotUIRight_->Draw();
	// 決定キーUI描画
	decideKeyUI_->Draw();

	// 選択中のスロットテキスト描画
	switch (selectedSlot_)
	{
	case Slot::Continue:
		slotTextUIUp_->Draw();
		break;
	case Slot::Retry:
		slotTextUIRight_->Draw();
		break;
	case Slot::StageSelect:
		slotTextUILeft_->Draw();
		break;
	case Slot::Title:
		slotTextUIDown_->Draw();
		break;
	default:
		break;
	}
}

void PauseUI::LoadSprite()
{
	// 画面を少し暗くする黒いスプライト
	auto overlaySprite = std::make_unique<Sprite>();
	overlaySprite->Initialize("resources/Scenes/Pause/UI/Texture/PauseBlack.dds");
	overlaySprite->SetPosition({ 0.0f,0.0f });
	overlaySprite->SetSize({ 1280.0f,720.0f });
	overlaySprite->SetColor({ 1.0f,1.0f,1.0f,0.55f });
	sprites_.push_back(std::move(overlaySprite));

	auto circleSlot = std::make_unique<Sprite>();
	circleSlot->Initialize("resources/Scenes/Pause/UI/Texture/PauseUICircleSlot_2.dds");
	circleSlot->SetPosition({ 0.0f, 0.0f });
	circleSlot->SetSize({ 1280.0f,720.0f });
	sprites_.push_back(std::move(circleSlot));

	// スロット上
	slotUIUp_ = std::make_unique<Sprite>();
	slotUIUp_->Initialize("resources/Scenes/Pause/UI/Texture/PauseUISlotItemUP_IconB.dds");
	slotUIUp_->SetPosition({ 0.0f, 0.0f });
	slotUIUp_->SetSize({ 1280.0f, 720.0f });


	// スロット下
	slotUIDown_ = std::make_unique<Sprite>();
	slotUIDown_->Initialize("resources/Scenes/Pause/UI/Texture/PauseUISlotItemDown_IconB.dds");
	slotUIDown_->SetPosition({ 0.0f, 0.0f });
	slotUIDown_->SetSize({ 1280.0f, 720.0f });
	// スロット左
	slotUILeft_ = std::make_unique<Sprite>();
	slotUILeft_->Initialize("resources/Scenes/Pause/UI/Texture/PauseUISlotItemLeft_IconB.dds");
	slotUILeft_->SetPosition({ 0.0f, 0.0f });
	slotUILeft_->SetSize({ 1280.0f, 720.0f });
	// スロット右
	slotUIRight_ = std::make_unique<Sprite>();
	slotUIRight_->Initialize("resources/Scenes/Pause/UI/Texture/PauseUISlotItemRight_IconB.dds");
	slotUIRight_->SetPosition({ 0.0f, 0.0f });
	slotUIRight_->SetSize({ 1280.0f, 720.0f });



	basePosUp_ = slotUIUp_->GetPosition();
	baseSizeUp_ = slotUIUp_->GetSize();

	basePosRight_ = slotUIRight_->GetPosition();
	baseSizeRight_ = slotUIRight_->GetSize();

	basePosDown_ = slotUIDown_->GetPosition();
	baseSizeDown_ = slotUIDown_->GetSize();

	basePosLeft_ = slotUILeft_->GetPosition();
	baseSizeLeft_ = slotUILeft_->GetSize();


	slotTextUIUp_ = std::make_unique<Sprite>();
	slotTextUIUp_->Initialize("resources/Scenes/Pause/UI/Texture/PlayText.dds");
	slotTextUIUp_->SetPosition({ 0.0f, 0.0f });
	slotTextUIUp_->SetSize({ 1280.0f, 720.0f });

	slotTextUIDown_ = std::make_unique<Sprite>();
	slotTextUIDown_->Initialize("resources/Scenes/Pause/UI/Texture/TitleText.dds");
	slotTextUIDown_->SetPosition({ 0.0f, 0.0f });
	slotTextUIDown_->SetSize({ 1280.0f, 720.0f });

	slotTextUILeft_ = std::make_unique<Sprite>();
	slotTextUILeft_->Initialize("resources/Scenes/Pause/UI/Texture/SelectText.dds");
	slotTextUILeft_->SetPosition({ 0.0f, 0.0f });
	slotTextUILeft_->SetSize({ 1280.0f, 720.0f });

	slotTextUIRight_ = std::make_unique<Sprite>();
	slotTextUIRight_->Initialize("resources/Scenes/Pause/UI/Texture/ReTryText.dds");
	slotTextUIRight_->SetPosition({ 0.0f, 0.0f });
	slotTextUIRight_->SetSize({ 1280.0f, 720.0f });


	decideKeyUI_ = std::make_unique<Sprite>();
	decideKeyUI_->Initialize("resources/Scenes/Pause/UI/Texture/SpaceText.dds");
	decideKeyUI_->SetPosition({ 0.0f, 0.0f });
	decideKeyUI_->SetSize({ 1280.0f, 720.0f });


}

void PauseUI::ApplySelectedStyle(Sprite* sp,
	const Vector2& basePos,
	const Vector2& baseSize,
	bool selected,
	const Vector2& outwardDir)
{
	float kScale = 1.0f;
	if (selected) {
		kScale = 1.10f + GetBounceExtra();
	}

	const float kNudge = selected ? 12.0f : 0.0f; // 外側へ押し出し

	// サイズ変更
	Vector2 size = { baseSize.x * kScale, baseSize.y * kScale };

	// 左上基準の拡大で中心がズレるのを補正（中心固定）
	Vector2 pos = basePos;
	pos.x -= (size.x - baseSize.x) * 0.5f;
	pos.y -= (size.y - baseSize.y) * 0.5f;

	// 外側へ少しずらす（方向ベクトル）
	pos.x += outwardDir.x * kNudge;
	pos.y += outwardDir.y * kNudge;

	sp->SetSize(size);
	sp->SetPosition(pos);
}


void PauseUI::DrawImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Pause UI");

	// スプライトのカラーセット


	for (const auto& sprite : sprites_) {
		Vector4 color = sprite->GetColor();
		ImGui::DragFloat4("Sprite Color", &color.x, 0.01f, 0.0f, 1.0f);
		sprite->SetColor(color);
	}


	// 選択中表示
	const uint32_t idx = static_cast<uint32_t>(selectedSlot_);
	ImGui::Text("Selected Slot: %u", idx);

	ImGui::Text("%s Continue (W)", (selectedSlot_ == Slot::Continue) ? ">" : " ");
	ImGui::Text("%s Retry (D)", (selectedSlot_ == Slot::Retry) ? ">" : " ");
	ImGui::Text("%s Stage Select (A)", (selectedSlot_ == Slot::StageSelect) ? ">" : " ");
	ImGui::Text("%s Title (S)", (selectedSlot_ == Slot::Title) ? ">" : " ");

	ImGui::Separator();
	ImGui::Text("Decide: SPACE / ENTER");
	ImGui::Text("ResumeRequested: %s", requestPauseRelease_ ? "true" : "false");


	ImGui::End();
#endif
}

bool PauseUI::PauseReleaseRequested()
{
	// すでにポーズ解除要求が出ていれば抜ける
	if (!requestPauseRelease_) {
		return false;
	}
	// ポーズ解除要求フラグをリセット
	requestPauseRelease_ = false;
	// ポーズ解除要求を返す
	return true;
}

void PauseUI::HandleDecideInput()
{
	// スペースキーが押されていなければ抜ける
	if (!Input::GetInstance()->TriggerKey(DIK_SPACE)) return;

	// 選択中のスロットに応じた処理
	switch (selectedSlot_)
	{
	case Slot::Continue:
		pendingAction_ = Action::Continue;
		requestPauseRelease_ = true;
		break;
	case Slot::Retry:
		pendingAction_ = Action::Retry;
		break;
	case Slot::StageSelect:
		pendingAction_ = Action::StageSelect;
		break;
	case Slot::Title:
		pendingAction_ = Action::Title;
		break;

	default:
		break;
	}

}

void PauseUI::HandleDirectionInput()
{
	// 入力によって選択スロットを変更
	if (Input::GetInstance()->TriggerKey(DIK_W)) {
		selectedSlot_ = Slot::Continue;
	} else if (Input::GetInstance()->TriggerKey(DIK_D)) {
		selectedSlot_ = Slot::Retry;
	} else if (Input::GetInstance()->TriggerKey(DIK_A)) {
		selectedSlot_ = Slot::StageSelect;
	} else if (Input::GetInstance()->TriggerKey(DIK_S)) {
		selectedSlot_ = Slot::Title;
	}
}

