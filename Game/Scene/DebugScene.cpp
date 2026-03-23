#include "DebugScene.h"
#include "engine/InsideScene/Framework.h"

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

DebugScene::DebugScene() {

}

DebugScene::~DebugScene()
{

}

void DebugScene::Initialize(DirectXCommon* dxCommon)
{
	camera = Framework::GetMainCamera();
	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();

	ball = std::make_unique<Object3D>();
	ball->Initialize();
	ball->SetModel("Scenes/Debug/BlackDome");
	ball->SetScale({ 5.0f,5.0f,5.0f });;
	ball->SetTranslate({ 0.0f,-4.5f,20.0f });

	player = std::make_unique<Object3D>();
	player->Initialize();
	player->SetModel("GamePlay/Player");
	player->SetTranslate({ 0.0f,-7.0f,20.0f });

	backGround = std::make_unique<BackGround>();
	backGround->Initialize();
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
	// カメラの更新
	camera->Update();
	// ------------オブジェクトの更新------------
	ball->Update();
	player->Update();

	backGround->Update();
	// ---------------------------------------


	// Imguiの描画
	DrawImgui();

}

void DebugScene::Draw()
{
	// ------------オブジェクトの描画------------
	ball->Draw();
	player->Draw();
	backGround->Draw();
	// ---------------------------------------
}

void DebugScene::Finalize()
{
}

void DebugScene::DrawImgui()
{
	// カメラの座標をimguiで編集できるようにする
	ImGui::Begin("DebugCamera");

	ImGui::Text("Debug Camera : %s", isDebugCameraActive_ ? "ON" : "OFF");
	ImGui::Text("Speed Mode   : %s", isFastMoveMode_ ? "FAST" : "NORMAL");
	ImGui::Text("F1 : Toggle Debug Camera");
	ImGui::Text("F2 : Toggle Speed Mode");

	ImGui::DragFloat3("CameraPos", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("CameraRot", &cameraTransform.rotate.x, 0.01f);

	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();
	ImGui::DragFloat3("Translate", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Rotate", &cameraTransform.rotate.x, 0.1f);

	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	// オブジェクトの座標をimguiで編集できるようにする
	Transform ballTransform = ball->GetTransform();
	ImGui::DragFloat3("Ball Translate", &ballTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Ball Rotate", &ballTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("Ball Scale", &ballTransform.scale.x, 0.1f);

	ball->SetTransform(ballTransform);
		
	Transform playerTransform = player->GetTransform();
	ImGui::DragFloat3("Player Translate", &playerTransform.translate.x, 0.1f);
	ImGui::DragFloat3("Player Rotate", &playerTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("Player Scale", &playerTransform.scale.x, 0.1f);

	player->SetTransform(playerTransform);


	ImGui::End();
}

void DebugScene::UpdateDebugCamera()
{
	// 入力関数の取得と省略
	Input* input = Input::GetInstance();
	// カメラの現在の情報を取得
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
	// カメラの前方向と右方向を計算
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

	// 入力した方向にカメラを移動させる
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

	// F2で切り替わる速度
	float speed = isFastMoveMode_ ? cameraShiftSpeed_ : cameraMoveSpeed_;

	// Shift押下中はさらに加速
	if (input->PushKey(DIK_LSHIFT) || input->PushKey(DIK_RSHIFT)) {
		speed *= 2.0f;
	}

	cameraTransform.translate.x += move.x * speed;
	cameraTransform.translate.y += move.y * speed;
	cameraTransform.translate.z += move.z * speed;
	// カメラに反映
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);
}
