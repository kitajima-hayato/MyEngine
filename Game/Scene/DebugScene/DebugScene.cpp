#include "DebugScene.h"
#include "engine/InsideScene/Framework.h"

#include <cmath>

using Engine::DirectXCommon;

namespace
{
	Vector3 Normalize(const Vector3& v)
	{
		float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		if (len <= 0.00001f) {
			return { 0.0f,0.0f,0.0f };
		}
		return { v.x / len, v.y / len, v.z / len };
	}
}

DebugScene::DebugScene()
{}

DebugScene::~DebugScene()
{}

void DebugScene::Initialize(DirectXCommon* dxCommon)
{
	camera = Framework::GetMainCamera();
	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();

	ball = std::make_unique<Object3D>();
	ball->Initialize();
	ball->SetModel("Scenes/Debug/dice");
	ball->SetScale({ 5.0f,5.0f,5.0f });
	ball->SetTranslate({ 0.0f,-4.5f,20.0f });
	ball->SetRotate({ 0.0f,0.0f,0.0f });

	// 今は使わないが、元の構成に合わせて残しておく
	player = std::make_unique<Object3D>();
	player->Initialize();
	player->SetModel("Scenes/Debug/dice");
	player->SetTranslate({ 0.0f,-7.0f,20.0f });
	player->SetRotate({ 0.0f,0.0f,0.0f });
	player->SetScale({ 2.0f,2.0f,2.0f });

	// サイコロの初期面
	diceTop_ = 1;
	diceBottom_ = 6;
	diceFront_ = 2;
	diceBack_ = 5;
	diceLeft_ = 4;
	diceRight_ = 3;

	isDiceRolling_ = false;
	diceRollTimer_ = 0.0f;
	currentDiceRollDirection_ = DiceRollDirection::None;
}

void DebugScene::Update()
{
	Input* input = Input::GetInstance();

	// F1でデバッグカメラON/OFF
	if (input->TriggerKey(DIK_F1)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	// F2で速度切り替え
	if (input->TriggerKey(DIK_F2)) {
		isFastMoveMode_ = !isFastMoveMode_;
	}

	// デバッグカメラが有効な時だけ更新
	if (isDebugCameraActive_) {
		UpdateDebugCamera();
	}

	// デバッグカメラOFFのときだけサイコロを操作
	if (!isDebugCameraActive_) {
		UpdateDice();
	}

	// カメラの更新
	camera->Update();

	// ------------オブジェクトの更新------------
	ball->Update();
	// player->Update();
	// ---------------------------------------

	DrawImgui();
}

void DebugScene::Draw()
{
	// ------------オブジェクトの描画------------
	ball->Draw();
	// player->Draw();
	// ---------------------------------------
}

void DebugScene::Finalize()
{}

void DebugScene::DrawImgui()
{
#ifdef USE_IMGUI
	ImGui::Begin("DebugCamera");

	ImGui::Separator();
	ImGui::Text("Dice");
	ImGui::Text("Top    : %d", diceTop_);
	ImGui::Text("Bottom : %d", diceBottom_);
	ImGui::Text("Front  : %d", diceFront_);
	ImGui::Text("Back   : %d", diceBack_);
	ImGui::Text("Left   : %d", diceLeft_);
	ImGui::Text("Right  : %d", diceRight_);
	ImGui::Text("F1 OFF : Dice Control Mode");
	ImGui::Separator();

	ImGui::Text("Debug Camera : %s", isDebugCameraActive_ ? "ON" : "OFF");
	ImGui::Text("Speed Mode   : %s", isFastMoveMode_ ? "FAST" : "NORMAL");
	ImGui::Text("F1 : Toggle Debug Camera");
	ImGui::Text("F2 : Toggle Speed Mode");

	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();

	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.01f);

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	ImGui::Separator();

	Transform ballTransform = ball->GetTransform();

	ImGui::DragFloat3("Ball Translate", &ballTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Ball Rotate", &ballTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("Ball Scale", &ballTransform.scale.x, 0.1f);

	// 回転中にImGuiからTransformを書き換えると補間が崩れるので、
	// サイコロ操作中はImGuiのTransform反映を止める
	if (!isDiceRolling_) {
		ball->SetTransform(ballTransform);
	}

	ImGui::End();
#endif
}

void DebugScene::UpdateDebugCamera()
{
	Input* input = Input::GetInstance();

	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();

	// 右クリック中だけ視点回転
	if (input->PushMouse(1)) {
		cameraTransform.rotate.y += input->GetMouseMoveX() * mouseSensitivity_;
		cameraTransform.rotate.x += input->GetMouseMoveY() * mouseSensitivity_;

		const float limit = 1.55f;
		if (cameraTransform.rotate.x > limit) {
			cameraTransform.rotate.x = limit;
		}
		if (cameraTransform.rotate.x < -limit) {
			cameraTransform.rotate.x = -limit;
		}
	}

	float yaw = cameraTransform.rotate.y;
	float pitch = cameraTransform.rotate.x;

	Vector3 forward = {
		std::sinf(yaw) * std::cosf(pitch),
		std::sinf(pitch),
		std::cosf(yaw) * std::cosf(pitch)
	};

	float forwardLen = std::sqrt(
		forward.x * forward.x +
		forward.y * forward.y +
		forward.z * forward.z
	);

	if (forwardLen > 0.00001f) {
		forward.x /= forwardLen;
		forward.y /= forwardLen;
		forward.z /= forwardLen;
	}

	Vector3 right = {
		std::cosf(yaw),
		0.0f,
		-std::sinf(yaw)
	};

	float rightLen = std::sqrt(
		right.x * right.x +
		right.y * right.y +
		right.z * right.z
	);

	if (rightLen > 0.00001f) {
		right.x /= rightLen;
		right.y /= rightLen;
		right.z /= rightLen;
	}

	Vector3 move = { 0.0f, 0.0f, 0.0f };

	if (input->PushKey(DIK_W)) {
		move.x += forward.x;
		move.y += forward.y;
		move.z += forward.z;
	}
	if (input->PushKey(DIK_S)) {
		move.x -= forward.x;
		move.y -= forward.y;
		move.z -= forward.z;
	}
	if (input->PushKey(DIK_D)) {
		move.x += right.x;
		move.z += right.z;
	}
	if (input->PushKey(DIK_A)) {
		move.x -= right.x;
		move.z -= right.z;
	}
	if (input->PushKey(DIK_E)) {
		move.y += 1.0f;
	}
	if (input->PushKey(DIK_Q)) {
		move.y -= 1.0f;
	}

	float moveLen = std::sqrt(
		move.x * move.x +
		move.y * move.y +
		move.z * move.z
	);

	if (moveLen > 0.00001f) {
		move.x /= moveLen;
		move.y /= moveLen;
		move.z /= moveLen;
	}

	float speed = isFastMoveMode_ ? cameraShiftSpeed_ : cameraMoveSpeed_;

	if (input->PushKey(DIK_LSHIFT) || input->PushKey(DIK_RSHIFT)) {
		speed *= 2.0f;
	}

	cameraTransform.translate.x += move.x * speed;
	cameraTransform.translate.y += move.y * speed;
	cameraTransform.translate.z += move.z * speed;

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
}

void DebugScene::UpdateDice()
{
	Input* input = Input::GetInstance();

	if (!isDiceRolling_) {
		if (input->TriggerKey(DIK_W)) {
			StartDiceRoll(DiceRollDirection::Forward);
		} else if (input->TriggerKey(DIK_S)) {
			StartDiceRoll(DiceRollDirection::Back);
		} else if (input->TriggerKey(DIK_A)) {
			StartDiceRoll(DiceRollDirection::Left);
		} else if (input->TriggerKey(DIK_D)) {
			StartDiceRoll(DiceRollDirection::Right);
		}
	}

	UpdateDiceRoll();
}

void DebugScene::StartDiceRoll(DiceRollDirection direction)
{
	const float angle90 = 3.1415926535f / 2.0f;

	isDiceRolling_ = true;
	diceRollTimer_ = 0.0f;
	currentDiceRollDirection_ = direction;

	diceStartTransform_ = ball->GetTransform();
	diceTargetTransform_ = diceStartTransform_;

	// 入力方向に対して固定軸で回転させる。
	// サイコロ自身の現在の軸には影響されない。
	switch (direction) {
	case DiceRollDirection::Forward:
		// W：前方向へ移動して、前に倒れる
		diceTargetTransform_.translate.z += diceMoveDistance_;
		diceTargetTransform_.rotate.x += angle90;
		break;

	case DiceRollDirection::Back:
		// S：後方向へ移動して、後ろに倒れる
		diceTargetTransform_.translate.z -= diceMoveDistance_;
		diceTargetTransform_.rotate.x -= angle90;
		break;

	case DiceRollDirection::Left:
		// A：左方向へ移動して、左に倒れる
		diceTargetTransform_.translate.x -= diceMoveDistance_;
		diceTargetTransform_.rotate.z += angle90;
		break;

	case DiceRollDirection::Right:
		// D：右方向へ移動して、右に倒れる
		diceTargetTransform_.translate.x += diceMoveDistance_;
		diceTargetTransform_.rotate.z -= angle90;
		break;

	default:
		isDiceRolling_ = false;
		currentDiceRollDirection_ = DiceRollDirection::None;
		return;
	}

	ApplyDiceFaceRoll(direction);
}

void DebugScene::UpdateDiceRoll()
{
	if (!isDiceRolling_) {
		return;
	}

	diceRollTimer_ += 1.0f / 60.0f;

	float t = diceRollTimer_ / diceRollDuration_;

	if (t >= 1.0f) {
		t = 1.0f;
		isDiceRolling_ = false;
	}

	float easedT = EaseOutQuad(t);

	Transform currentTransform = diceStartTransform_;

	currentTransform.translate.x = Lerp(
		diceStartTransform_.translate.x,
		diceTargetTransform_.translate.x,
		easedT
	);

	currentTransform.translate.y = Lerp(
		diceStartTransform_.translate.y,
		diceTargetTransform_.translate.y,
		easedT
	);

	currentTransform.translate.z = Lerp(
		diceStartTransform_.translate.z,
		diceTargetTransform_.translate.z,
		easedT
	);

	currentTransform.rotate.x = Lerp(
		diceStartTransform_.rotate.x,
		diceTargetTransform_.rotate.x,
		easedT
	);

	currentTransform.rotate.y = Lerp(
		diceStartTransform_.rotate.y,
		diceTargetTransform_.rotate.y,
		easedT
	);

	currentTransform.rotate.z = Lerp(
		diceStartTransform_.rotate.z,
		diceTargetTransform_.rotate.z,
		easedT
	);

	// 終了時は誤差防止で目標値にぴったり合わせる
	if (!isDiceRolling_) {
		currentTransform = diceTargetTransform_;
		currentDiceRollDirection_ = DiceRollDirection::None;
	}

	ball->SetTransform(currentTransform);
}

void DebugScene::ApplyDiceFaceRoll(DiceRollDirection direction)
{
	int oldTop = diceTop_;
	int oldBottom = diceBottom_;
	int oldFront = diceFront_;
	int oldBack = diceBack_;
	int oldLeft = diceLeft_;
	int oldRight = diceRight_;

	switch (direction) {
	case DiceRollDirection::Forward:
		// 奥へ転がる
		diceTop_ = oldFront;
		diceBack_ = oldTop;
		diceBottom_ = oldBack;
		diceFront_ = oldBottom;
		break;

	case DiceRollDirection::Back:
		// 手前へ転がる
		diceTop_ = oldBack;
		diceFront_ = oldTop;
		diceBottom_ = oldFront;
		diceBack_ = oldBottom;
		break;

	case DiceRollDirection::Left:
		// 左へ転がる
		diceTop_ = oldRight;
		diceLeft_ = oldTop;
		diceBottom_ = oldLeft;
		diceRight_ = oldBottom;
		break;

	case DiceRollDirection::Right:
		// 右へ転がる
		diceTop_ = oldLeft;
		diceRight_ = oldTop;
		diceBottom_ = oldRight;
		diceLeft_ = oldBottom;
		break;

	default:
		break;
	}
}

float DebugScene::EaseOutQuad(float t)
{
	return 1.0f - (1.0f - t) * (1.0f - t);
}

float DebugScene::Lerp(float start, float end, float t)
{
	return start + (end - start) * t;
}