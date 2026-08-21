#include "StageSelectScene.h"
#include "engine/InsideScene/Framework.h"
#include "Game/Camera/Camera.h"
#include "engine/3d/Object3D.h"
#include "Game/Application/PlayContext.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif

using namespace Engine;

// ========================================
// ユーティリティ関数
// ========================================

/// <summary>
/// イージング関数（アウトキュービック）
/// </summary>
static float EaseOutCubic(float t)
{
	float inv = 1.0f - t;
	return 1.0f - inv * inv * inv;
}

/// <summary>
/// イージング関数（インアウトキュービック）
/// </summary>
static float EaseInOutCubic(float t)
{
	if (t < 0.5f)
		return 4.0f * t * t * t;
	else
	{
		float f = 2.0f * t - 2.0f;
		return 0.5f * f * f * f + 1.0f;
	}
}

/// <summary>
/// 角度を 0 ~ 2π に正規化
/// </summary>
static float WrapAngle0To2Pi(float angle)
{
	const float twoPi = 6.28318530718f;
	while (angle < 0.0f) angle += twoPi;
	while (angle >= twoPi) angle -= twoPi;
	return angle;
}

/// <summary>
/// 角度を -π ~ π に正規化
/// </summary>
static float WrapAngleMinusPiToPi(float a)
{
	const float pi = 3.14159265359f;
	const float twoPi = 6.28318530718f;
	while (a <= -pi) a += twoPi;
	while (a > pi)  a -= twoPi;
	return a;
}

/// <summary>
/// 角度の線形補間（最短経路）
/// </summary>
static float LerpAngle(float from, float to, float t)
{
	float delta = WrapAngleMinusPiToPi(to - from);
	return WrapAngle0To2Pi(from + delta * t);
}

// ========================================
// 座標・方向計算
// ========================================

Vector3 StageSelectScene::CalcNodeWorldPos(uint32_t nodeId) const
{
	// ノードのUV座標を取得
	const Vector2 uv = stageSelectGraph->GetNodeUV(nodeId);

	// マップの表示範囲（ワールド単位）
	const float mapWidth = 40.0f;
	const float mapHeight = 20.0f;

	// マップの基準（中心）位置
	Vector3 origin = stageSelectBase1Transform.translate;
	origin.y = 1.0f;

	// ノード位置をワールド座標に変換
	Vector3 t = origin;
	t.x += (uv.x - 0.5f) * mapWidth;
	// 上下反転済みの式
	t.z += (0.5f - uv.y) * mapHeight;

	return t;
}

float StageSelectScene::CalcYawToTarget(const Vector3& from, const Vector3& to) const
{
	// 2点間の方向ベクトルを計算
	Vector3 delta = to - from;
	delta.y = 0.0f;

	float len = std::sqrtf(delta.x * delta.x + delta.z * delta.z);
	if (len < 1e-6f) return 0.0f;

	// Yaw 0 が +Z方向の場合
	float yaw = std::atan2f(delta.x, delta.z);

	// モデル補正を適用
	yaw += modelYawOffset_;

	return WrapAngle0To2Pi(yaw);
}

float StageSelectScene::CalcYawFaceCamera(const Vector3& pos) const
{
	// カメラ位置を取得
	Vector3 camPos = cameraTransform.translate;

	// カメラ方向のベクトルを計算
	Vector3 delta = camPos - pos;
	delta.y = 0.0f;

	float len = std::sqrtf(delta.x * delta.x + delta.z * delta.z);
	if (len < 1e-6f) return 0.0f;

	// Yaw 0 が +Z方向の場合
	float yaw = std::atan2f(delta.x, delta.z);

	// モデル補正を適用
	yaw += modelYawOffset_;

	return WrapAngle0To2Pi(yaw);
}



void StageSelectScene::UpdateCursorMove()
{
	// 移動中でなければ終了
	if (!isMoving_) return;

	// 経過時間を進める
	const float dt = 1.0f / 60.0f;
	moveTime_ += dt;

	// 進行度を計算（0.0 ~ 1.0）
	float t = moveTime_ / moveDuration_;
	if (t >= 1.0f) t = 1.0f;

	// イージングを適用
	float ease = EaseInOutCubic(t);

	// 位置の補間
	Vector3 position = moveStartPos_ + (moveTargetPos_ - moveStartPos_) * ease;

	// 回転の補間（進行方向を向く）
	float yaw = LerpAngle(moveStartYaw_, moveTargetYaw_, ease);

	// プレイヤートランスフォーム取得
	Transform transform = playerModel->GetTransform();

	// 位置更新
	transform.translate = position;

	// 回転更新（移動中は進行方向を向く）
	transform.rotate.y = yaw;
	transform.rotate.x = 0.0f;
	transform.rotate.z = 0.0f;

	// プレイヤー位置更新
	playerModel->SetTransform(transform);

#ifdef USE_IMGUI
	debugCurrentYawDeg_ = yaw * 57.2957795f;
#endif

	// 移動完了
	if (t >= 1.0f)
	{
		// 移動フラグを下ろす
		isMoving_ = false;

		// カメラ方向を向く処理を開始
		const StageNode& targetNode = stageSelectGraph->GetNode(currentNodeId);

		// カメラ方向を向く処理を開始
		isFacing_ = true;
		faceTime_ = 0.0f;
		faceStartYaw_ = yaw;

		// ノードにdefaultYawが設定されていればそれを使用、なければカメラ方向
		if (targetNode.defaultYaw != StageSelectGraph::AUTO_YAW) {
			faceTargetYaw_ = targetNode.defaultYaw;
		} else {
			faceTargetYaw_ = CalcYawFaceCamera(moveTargetPos_);
		}
	}
}

void StageSelectScene::UpdateFaceYaw()
{
	// カメラを向いていない場合は終了
	if (!isFacing_) return;

	// 経過時間を進める
	const float dt = 1.0f / 60.0f;
	faceTime_ += dt;

	// 進行度を計算
	float t = faceTime_ / faceDuration_;
	if (t >= 1.0f) t = 1.0f;

	// イージングを適用
	float ease = EaseOutCubic(t);

	// Yaw補間を反映
	float yaw = LerpAngle(faceStartYaw_, faceTargetYaw_, ease);

	// トランスフォーム取得
	Transform tr = playerModel->GetTransform();

	// Yaw角を設定
	tr.rotate.y = yaw;
	tr.rotate.x = 0.0f;
	tr.rotate.z = 0.0f;

	// プレイヤー位置更新
	playerModel->SetTransform(tr);

	// 回転完了
	if (t >= 1.0f)
	{
		isFacing_ = false;
	}
}

void StageSelectScene::HandleSelectInput()
{
	// 移動中・回転中は入力を受け付けない
	if (isMoving_ || isFacing_) return;

	// 現在のノードIDを保存
	uint32_t next = currentNodeId;

	Input* input = Input::GetInstance();

	constexpr int kControllerNo = 0;
	constexpr float kStickThreshold = 0.6f;

	StickState stick = input->GetLeftStickState(kControllerNo);
	StickState prevStick = input->GetController()->GetLeftStickStatePrevious(kControllerNo);

	const bool stickUp =
		prevStick.y <= kStickThreshold &&
		stick.y > kStickThreshold;

	const bool stickDown =
		prevStick.y >= -kStickThreshold &&
		stick.y < -kStickThreshold;

	const bool stickLeft =
		prevStick.x >= -kStickThreshold &&
		stick.x < -kStickThreshold;

	const bool stickRight =
		prevStick.x <= kStickThreshold &&
		stick.x > kStickThreshold;

	// 入力した方向に移動できるノードがあれば移動
	if (input->TriggerKey(DIK_W) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadUP) ||
		stickUp) {

		next = stageSelectGraph->Move(currentNodeId, Direction::Up);

	} else if (input->TriggerKey(DIK_S) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadDOWN) ||
		stickDown) {

		next = stageSelectGraph->Move(currentNodeId, Direction::Down);

	} else if (input->TriggerKey(DIK_A) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadLEFT) ||
		stickLeft) {

		next = stageSelectGraph->Move(currentNodeId, Direction::Left);

	} else if (input->TriggerKey(DIK_D) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::DPadRIGHT) ||
		stickRight) {

		next = stageSelectGraph->Move(currentNodeId, Direction::Right);
	}

	// ノードが変わらなければ何もしない
	if (next == currentNodeId) return;

	// 移動開始
	currentNodeId = next;
	isMoving_ = true;
	moveTime_ = 0.0f;

	// ノード更新
	Transform transform = playerModel->GetTransform();
	moveStartPos_ = transform.translate;
	moveTargetPos_ = CalcNodeWorldPos(next);

	// 現在のYawと目標Yaw（進行方向）を計算
	moveStartYaw_ = transform.rotate.y;
	moveTargetYaw_ = CalcYawToTarget(moveStartPos_, moveTargetPos_);

#ifdef USE_IMGUI
	// ↓デバッグ情報を保存
	Vector3 direction = moveTargetPos_ - moveStartPos_;
	direction.y = 0.0f;
	debugDirection_ = direction;
	debugMoveDistance_ = std::sqrtf(direction.x * direction.x + direction.z * direction.z);
	debugStartYawDeg_ = moveStartYaw_ * 57.2957795f;
	debugTargetYawDeg_ = moveTargetYaw_ * 57.2957795f;
	debugYawChangeDeg_ = (moveTargetYaw_ - moveStartYaw_) * 57.2957795f;
#endif
}

void StageSelectScene::ApplyNodeToCursorTransform()
{
	// 絶対座標でセット
	Transform tr = playerModel->GetTransform();
	tr.translate = CalcNodeWorldPos(currentNodeId);

	// 初期状態ではカメラの方を向く
	tr.rotate.y = CalcYawFaceCamera(tr.translate);
	tr.rotate.x = 0.0f;
	tr.rotate.z = 0.0f;

	playerModel->SetTransform(tr);
}

void StageSelectScene::BuildRoutes()
{
	// ルートモデルのクリア
	routeModels_.clear();
	routeTransforms_.clear();

	const uint32_t count = stageSelectGraph->GetNodeCount();
	if (count == 0) return;

	for (uint32_t i = 0; i < count; ++i)
	{
		const StageNode& n = stageSelectGraph->GetNode(i);

		for (int d = 0; d < (int)Direction::count; ++d)
		{
			uint32_t to = n.neighbor[d];
			if (to == StageSelectGraph::INVALID_NODE_ID) continue;

			if (to < i) continue;

			// 端点（ワールド座標）
			Vector3 a = CalcNodeWorldPos(i);
			Vector3 b = CalcNodeWorldPos(to);

			// 少し浮かせる（床に埋まらないように）
			a.y += 0.05f;
			b.y += 0.05f;

			Vector3 diff = b - a;
			diff.y = 0.0f;

			// 中点を計算
			Vector3 mid = (a + b) * 0.5f;

			// yaw（Yaw 0 が +Z 前提）
			float yaw = std::atan2f(diff.x, diff.z);

			// Object3D作成
			auto obj = std::make_unique<Object3D>();
			obj->Initialize();
			obj->SetModel("Scenes/StageSelect/Models/route");

			Transform tr{};
			tr.translate = mid;
			tr.translate.y -= 1.0f; // 少し浮かせる
			tr.rotate = { 0.0f, yaw, 0.0f };
			tr.scale = { 0.5f, 0.5f, 1.0f };

			obj->SetTransform(tr);

			routeTransforms_.push_back(tr);
			routeModels_.push_back(std::move(obj));
		}
	}
}


StageSelectScene::StageSelectScene()
{}

StageSelectScene::~StageSelectScene()
{}

void StageSelectScene::Initialize(DirectXCommon* dxCommon)
{
	// カメラの取得
	camera = Framework::GetMainCamera();
	cameraTransform.translate = { 0.0f,20.0f,0.0f };
	cameraTransform.rotate = { 0.35f,0.0f,0.0f };
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	// プレイヤーオブジェクトの初期化
	playerModel = std::make_unique<Object3D>();
	playerModel->Initialize();
	playerModel->SetModel("GamePlay/Player");
	playerTransform = {
		// scale
		{2.5f,2.5f,2.5f},
		// rotate
		{0.0f,0.0f,0.0f},
		// translate
		{-20.0f,1.0f,50.0f},
	};
	playerModel->SetTransform(playerTransform);

	// 背景オブジェクトの初期化
	stageSelectBase1 = std::make_unique<Object3D>();
	stageSelectBase1->Initialize();
	stageSelectBase1->SetModel("Scenes/StageSelect/Models/baseground");
	stageSelectBase1Transform = {
		// Scale
		{ 1.5f, 1.0f, 2.5f },
		// Rotate
		{ 0.0f, 0.0f, 0.0f },
		// Translate
		{ 0.0f, 0.0f, 50.0f }
	};
	stageSelectBase1->SetTransform(stageSelectBase1Transform);

	// 空背景の初期化
	skyBack = std::make_unique<Object3D>();
	skyBack->Initialize();
	skyBack->SetModel("GamePlay/BackGround/sky");
	skyBackTransform = {
		// Scale
		{ 50.0f, 50.0f, 40.0f },
		// Rotate
		{ 3.2f, 0.0f, 0.0f },
		// Translate
		{ 0.0f, 0.0f, 100.0f }
	};
	skyBack->SetTransform(skyBackTransform);

	// ステージセレクトグラフの初期化
	stageSelectGraph = std::make_unique<StageSelectGraph>();
	stageSelectGraph->Initialize();

	// ルートモデルの構築
	BuildRoutes();

	// ノードの追加
	startNodeId = 0;
	currentNodeId = startNodeId;

	// カーソルを初期位置に配置
	ApplyNodeToCursorTransform();

	// ノードモデル生成
	const uint32_t count = stageSelectGraph->GetNodeCount();
	nodeModels_.clear();
	nodeTransforms_.clear();
	nodeModels_.reserve(count);
	nodeTransforms_.reserve(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		auto obj = std::make_unique<Object3D>();
		obj->Initialize();
		obj->SetModel("Scenes/StageSelect/Models/node");

		Transform tr{};
		tr.scale = { 1.8f, 1.8f, 1.8f };   // 目印なら少し小さく
		tr.rotate = { 0.0f, 0.0f, 0.0f };
		tr.translate = CalcNodeWorldPos(i);

		// ちょい浮かせたいなら
		tr.translate.y += -1.1f;

		// 「常にカメラ向き」にしたいなら
		tr.rotate.y = CalcYawFaceCamera(tr.translate);

		obj->SetTransform(tr);

		nodeTransforms_.push_back(tr);
		nodeModels_.push_back(std::move(obj));
	}

	// UI初期化
	stageSelect_ = std::make_unique<Sprite>();
	stageSelect_->Initialize("resources/Scenes/StageSelect/UI/Texture/StageSelect.dds");
	stageSelect_->SetPosition({ 0.0f,0.0f });
	stageSelect_->SetSize({ 1280.0f,720.0f });

	// ===== 操作説明UI =====
// アンカーを中央にし、行ごとに同じ中心Yを与えて揃える
// 行の中心Y: 上段525 / 中段585 / 下段645（行間60px）
	const Vector2 kIconSize = { 110.0f, 110.0f };   // 元画像500x300(5:3)に合わせる
	const Vector2 kLabelSize = { 110.0f, 110.0f };  // 元画像512x512(1:1)に合わせる

	// KeyIconUI / 左下に配置
	keyIcon_WASD = std::make_unique<Sprite>();
	keyIcon_WASD->Initialize("resources/_Common/UI/Texture/inputhints/WASD.png");
	keyIcon_WASD->SetAnchorPoint({ 0.5f, 0.5f });
	keyIcon_WASD->SetSize(kIconSize);
	keyIcon_WASD->SetPosition(keyIcon_WASD_Pos);

	// esc / Space UI配置
	keyIcon_Esc = std::make_unique<Sprite>();
	keyIcon_Esc->Initialize("resources/_Common/UI/Texture/inputhints/Esc.png");
	keyIcon_Esc->SetAnchorPoint({ 0.5f, 0.5f });
	keyIcon_Esc->SetSize(kIconSize);
	keyIcon_Esc->SetPosition(keyIcon_Esc_Pos);

	keyIcon_Space = std::make_unique<Sprite>();
	keyIcon_Space->Initialize("resources/_Common/UI/Texture/inputhints/Space.png");
	keyIcon_Space->SetAnchorPoint({ 0.5f, 0.5f });
	keyIcon_Space->SetSize(kIconSize);
	keyIcon_Space->SetPosition(keyIcon_Enter_Pos);

	// MoveUI（WASD行）
	moveUI_ = std::make_unique<Sprite>();
	moveUI_->Initialize("resources/Scenes/StageSelect/UI/Texture/Move.dds");
	moveUI_->SetAnchorPoint({ 0.5f, 0.5f });
	moveUI_->SetSize(kLabelSize);
	moveUI_->SetPosition(moveUI_Pos_);

	// CheckUI（Space行）
	checkUI_ = std::make_unique<Sprite>();
	checkUI_->Initialize("resources/Scenes/StageSelect/UI/Texture/Check.dds");
	checkUI_->SetAnchorPoint({ 0.5f, 0.5f });
	checkUI_->SetSize(kLabelSize);
	checkUI_->SetPosition(checkUI_Pos_);

	// BackUI（Esc行）
	backUI_ = std::make_unique<Sprite>();
	backUI_->Initialize("resources/Scenes/StageSelect/UI/Texture/Back.dds");
	backUI_->SetAnchorPoint({ 0.5f, 0.5f });
	backUI_->SetSize(kLabelSize);
	backUI_->SetPosition(backUI_Pos_);

	// コロンUI初期化（各行の中心に配置）
	colonSprites_.clear();
	colonSprites_.reserve(3);
	colonPositions_.clear();
	colonPositions_.reserve(3);

	// アイコン(右端~140)とラベル(左端~170)の間、各行の中心Yに合わせる
	colonPositions_.push_back({ 152.0f, 645.0f }); // WASD 行
	colonPositions_.push_back({ 152.0f, 585.0f }); // ESC 行
	colonPositions_.push_back({ 152.0f, 525.0f }); // SPACE 行

	for (int i = 0; i < 3; ++i) {
		auto s = std::make_unique<Sprite>();
		s->Initialize("resources/_Common/UI/Texture/inputhints/Colon.png");
		s->SetAnchorPoint({ 0.5f, 0.5f });
		s->SetSize({ 125.0f, 75.0f });
		s->SetPosition(colonPositions_[i]);
		colonSprites_.push_back(std::move(s));
	}



}

void StageSelectScene::Update()
{

	// 一定時間経過するまで操作ができないようにする
	if (inputLockTime_ >= inputLockDuration_) {
		// Spaceが押されたらステージを開始する
		Input* input = Input::GetInstance();

		if (input->TriggerKey(DIK_SPACE) ||
			input->TriggerButton(0, ControllerButtonType::A)) {

			const StageNode& node = stageSelectGraph->GetNode(currentNodeId);
			PlayContext::GetInstance().SetSelectedStage(node.stageId, node.stageKey);

			// ステージシーンへ切り替え
			SceneManager::GetInstance()->ChangeSceneWithTransition(
				"GAMEPLAY",
				TransitionType::Start
			);
		}
	} else {
		// 入力ロック時間を進める
		inputLockTime_ += 1.0f / 60.0f;
	}


	Input* input = Input::GetInstance();

	constexpr int kControllerNo = 0;

	// Escキー / コントローラーStartボタンでタイトルへ
	if (input->TriggerKey(DIK_ESCAPE) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::Start)) {

		// タイトルシーンへ切り替え
		SceneManager::GetInstance()->ChangeSceneWithTransition(
			"TITLE",
			TransitionType::Start
		);
	}
	cameraTransform.translate = { 0.0f,20.0f,0.0f };
	cameraTransform.rotate = { 0.35f,0.0f,0.0f };
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	camera->Update();

	// 空背景の更新
	skyBack->Update();

	// ステージセレクト土台の更新
	stageSelectBase1->Update();

	// ノードモデルの更新
	for (auto& m : nodeModels_) { m->Update(); }

	// ルートモデルの更新
	for (auto& m : routeModels_) { m->Update(); }

	// 入力した方向に移動 
	HandleSelectInput();

	// カーソル移動更新
	UpdateCursorMove();

	// 停止後の向き補間更新
	UpdateFaceYaw();

	// プレイヤー移動処理
	PlayerMove();

	// プレイヤーモデルの更新
	playerModel->Update();

	// ステージセレクトUIの更新
	stageSelect_->Update();

	// KeyUIの更新
	keyIcon_WASD->Update();
	keyIcon_Esc->Update();
	keyIcon_Space->Update();

	// コロンUIの位置反映（ImGui調整を反映）
	for (size_t i = 0; i < colonSprites_.size() && i < colonPositions_.size(); ++i) {
		colonSprites_[i]->SetPosition(colonPositions_[i]);
	}
	// コロンUIの更新
	for (auto& s : colonSprites_) { s->Update(); }

	// Move / Check / Back UIの更新
	moveUI_->Update();
	checkUI_->Update();
	backUI_->Update();

	// ImGuiの描画
	DrawImgui();
}

void StageSelectScene::Draw()
{
	// 空背景の描画
	skyBack->Draw();

	// ステージセレクト土台の描画
	stageSelectBase1->Draw();

	// プレイヤーモデルの描画
	playerModel->Draw();

	// ノードモデルの描画
	for (auto& m : nodeModels_) { m->Draw(); }

	// ルートモデルの描画
	for (auto& m : routeModels_) { m->Draw(); }

	// ステージセレクトUIの描画
	stageSelect_->Draw();

	// KeyUIの描画
	keyIcon_WASD->Draw();
	keyIcon_Esc->Draw();
	keyIcon_Space->Draw();

	// コロンUIの描画
	for (auto& s : colonSprites_) { s->Draw(); }

	// Move / Check / Back UIの描画
	moveUI_->Draw();
	checkUI_->Draw();
	backUI_->Draw();
}

void StageSelectScene::Finalize()
{
	// カメラの終了処理
	camera->Finalize();
}

void StageSelectScene::PlayerMove()
{
	// プレイヤーの座標取得
	Transform transform = playerModel->GetTransform();

	// 必要に応じて処理を追加
	playerModel->SetTransform(transform);
}

void StageSelectScene::DrawImgui()
{
#ifdef USE_IMGUI
	ImGui::Begin("StageSelectScene");

	// カメラ位置の調整
	cameraTransform.translate = camera->GetTranslate();
	cameraTransform.rotate = camera->GetRotate();
	ImGui::DragFloat3("CameraPos", &cameraTransform.translate.x, 0.1f);
	ImGui::DragFloat3("CameraRot", &cameraTransform.rotate.x, 0.01f);
	camera->SetTranslate(cameraTransform.translate);
	camera->SetRotate(cameraTransform.rotate);

	ImGui::Separator();

	// プレイヤー
	Transform transform = playerModel->GetTransform();
	ImGui::DragFloat3("PlayerPos", &transform.translate.x, 0.1f);
	ImGui::DragFloat3("PlayerRot", &transform.rotate.x, 0.1f);
	ImGui::DragFloat3("PlayerScale", &transform.scale.x, 0.1f);
	playerModel->SetTransform(transform);

	ImGui::Separator();

	// ステージセレクト土台１
	stageSelectBase1Transform = stageSelectBase1->GetTransform();
	ImGui::DragFloat3("StageSelectBase1Pos", &stageSelectBase1Transform.translate.x, 0.1f);
	ImGui::DragFloat3("StageSelectBase1Rot", &stageSelectBase1Transform.rotate.x, 0.1f);
	ImGui::DragFloat3("StageSelectBase1Scale", &stageSelectBase1Transform.scale.x, 0.1f);
	stageSelectBase1->SetTransform(stageSelectBase1Transform);

	ImGui::Separator();

	// 空背景
	skyBackTransform = skyBack->GetTransform();
	ImGui::DragFloat3("SkyBackPos", &skyBackTransform.translate.x, 0.1f);
	ImGui::DragFloat3("SkyBackRot", &skyBackTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("SkyBackScale", &skyBackTransform.scale.x, 0.1f);
	skyBack->SetTransform(skyBackTransform);

	ImGui::Separator();

	// UIKeyIcon配置変更

	keyIcon_WASD_Pos = keyIcon_WASD->GetPosition();
	keyIcon_Esc_Pos = keyIcon_Esc->GetPosition();
	keyIcon_Enter_Pos = keyIcon_Space->GetPosition();

	ImGui::DragFloat2("KeyIcon_WASD_Pos", &keyIcon_WASD_Pos.x, 1.0f);
	ImGui::DragFloat2("KeyIcon_Esc_Pos", &keyIcon_Esc_Pos.x, 1.0f);
	ImGui::DragFloat2("KeyIcon_Enter_Pos", &keyIcon_Enter_Pos.x, 1.0f);

	keyIcon_WASD->SetPosition(keyIcon_WASD_Pos);
	keyIcon_Esc->SetPosition(keyIcon_Esc_Pos);
	keyIcon_Space->SetPosition(keyIcon_Enter_Pos);

	// Colon UI配置変更（3つ個別に調整）
	if (colonPositions_.size() < 3) {
		colonPositions_.resize(3, { 140.0f, 490.0f });
	}

	ImGui::Separator();
	ImGui::Text("Colon Positions");

	ImGui::DragFloat2("Colon 0 (WASD row)", &colonPositions_[0].x, 1.0f);
	ImGui::DragFloat2("Colon 1 (ESC row)", &colonPositions_[1].x, 1.0f);
	ImGui::DragFloat2("Colon 2 (SPACE row)", &colonPositions_[2].x, 1.0f);

	// Move / Check / Back UI配置変更
	moveUI_Pos_ = moveUI_->GetPosition();
	ImGui::DragFloat2("MoveUI_Pos", &moveUI_Pos_.x, 1.0f);
	moveUI_->SetPosition(moveUI_Pos_);

	checkUI_Pos_ = checkUI_->GetPosition();
	ImGui::DragFloat2("CheckUI_Pos", &checkUI_Pos_.x, 1.0f);
	checkUI_->SetPosition(checkUI_Pos_);

	backUI_Pos_ = backUI_->GetPosition();
	ImGui::DragFloat2("BackUI_Pos", &backUI_Pos_.x, 1.0f);
	backUI_->SetPosition(backUI_Pos_);

	ImGui::End();
	if (showDebugWindow_) {
		ImGui::Begin("Movement Debug", &showDebugWindow_);

		ImGui::Text("=== Current State ===");
		ImGui::Text("Current Node ID: %u", currentNodeId);
		ImGui::Text("Is Moving: %s", isMoving_ ? "YES" : "NO");
		ImGui::Text("Is Facing: %s", isFacing_ ? "YES" : "NO");

		ImGui::Separator();
		ImGui::Text("=== Position ===");
		Transform currentTr = playerModel->GetTransform();
		ImGui::Text("Current Pos: (%.2f, %.2f, %.2f)",
			currentTr.translate.x, currentTr.translate.y, currentTr.translate.z);

		if (isMoving_) {
			ImGui::Text("Start Pos:   (%.2f, %.2f, %.2f)",
				moveStartPos_.x, moveStartPos_.y, moveStartPos_.z);
			ImGui::Text("Target Pos:  (%.2f, %.2f, %.2f)",
				moveTargetPos_.x, moveTargetPos_.y, moveTargetPos_.z);
			ImGui::Text("Direction:   (%.3f, %.3f, %.3f)",
				debugDirection_.x, debugDirection_.y, debugDirection_.z);
			ImGui::Text("Distance:    %.3f", debugMoveDistance_);
		}

		ImGui::Separator();
		ImGui::Text("=== Rotation ===");
		ImGui::Text("Current Yaw:  %.1f deg (%.3f rad)",
			debugCurrentYawDeg_, currentTr.rotate.y);

		if (isMoving_) {
			ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
				">>> MOVING <<<");
			ImGui::Text("Start Yaw:    %.1f deg", debugStartYawDeg_);
			ImGui::Text("Target Yaw:   %.1f deg", debugTargetYawDeg_);
			ImGui::Text("Yaw Change:   %.1f deg", debugYawChangeDeg_);
			ImGui::Text("Progress:     %.1f%%", (moveTime_ / moveDuration_) * 100.0f);

			// プログレスバー
			ImGui::ProgressBar(moveTime_ / moveDuration_, ImVec2(-1, 0));

			// 回転が小さい場合の警告
			if (std::abs(debugYawChangeDeg_) < 5.0f) {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
					"WARNING: Small rotation (< 5 deg)");
			}

			// 距離が短い場合の警告
			if (debugMoveDistance_ < 1.0f) {
				ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
					"WARNING: Short distance (< 1.0)");
			}
		}

		if (isFacing_) {
			ImGui::TextColored(ImVec4(0.0f, 0.5f, 1.0f, 1.0f),
				">>> FACING CAMERA <<<");
			ImGui::Text("Face Start:   %.1f deg", faceStartYaw_ * 57.2957795f);
			ImGui::Text("Face Target:  %.1f deg", faceTargetYaw_ * 57.2957795f);
			ImGui::Text("Progress:     %.1f%%", (faceTime_ / faceDuration_) * 100.0f);
			ImGui::ProgressBar(faceTime_ / faceDuration_, ImVec2(-1, 0));
		}

		ImGui::Separator();
		ImGui::Text("=== Settings ===");
		ImGui::DragFloat("Move Duration", &moveDuration_, 0.01f, 0.1f, 1.0f);
		ImGui::DragFloat("Face Duration", &faceDuration_, 0.01f, 0.1f, 1.0f);

		// モデル補正値の調整
		ImGui::Text("Model Yaw Offset:");
		float offsetDeg = modelYawOffset_ * 57.2957795f;
		ImGui::Text("  Current: %.1f deg (%.3f rad)", offsetDeg, modelYawOffset_);

		ImGui::Separator();
		ImGui::Text("=== Quick Tests ===");
		if (ImGui::Button("Test: 0 deg")) {
			const_cast<float&>(modelYawOffset_) = 0.0f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Test: 90 deg")) {
			const_cast<float&>(modelYawOffset_) = 1.570796326f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Test: -90 deg")) {
			const_cast<float&>(modelYawOffset_) = -1.570796326f;
		}
		ImGui::SameLine();
		if (ImGui::Button("Test: 180 deg")) {
			const_cast<float&>(modelYawOffset_) = 3.14159265f;
		}

		ImGui::End();
	}
	// ステージセレクトグラフのデバッグ表示
	DrawSelectGraphImGui();
#endif
}

void StageSelectScene::DrawSelectGraphImGui()
{
#ifdef USE_IMGUI
	// Graph が無い場合は何もしない
	if (!stageSelectGraph)
	{
		ImGui::Begin("StageSelect Graph");
		ImGui::TextDisabled("stageSelectGraph is null.");
		ImGui::End();
		return;
	}

	const uint32_t count = stageSelectGraph->GetNodeCount();

	// 選択IDの補正
	if (count == 0)
	{
		editNodeId_ = 0;
		prevEditNodeId_ = UINT32_MAX;
	} else if (editNodeId_ >= count)
	{
		editNodeId_ = count - 1;
	}

	// 選択が変わったときだけ編集バッファへロード
	auto toUi = [&](uint32_t id)->int {
		return (id == StageSelectGraph::INVALID_NODE_ID) ? -1 : (int)id;
		};

	if (count > 0 && editNodeId_ != prevEditNodeId_)
	{
		prevEditNodeId_ = editNodeId_;
		const StageNode& n = stageSelectGraph->GetNode(editNodeId_);

		editX_ = (int)n.position.x;
		editY_ = (int)n.position.y;
		editStageId_ = (int)n.stageId;
		strncpy_s(editStageKey_, sizeof(editStageKey_), n.stageKey.c_str(), _TRUNCATE);
		editUnlocked_ = n.unlocked;

		editNeighUp_ = toUi(n.neighbor[(int)Direction::Up]);
		editNeighDown_ = toUi(n.neighbor[(int)Direction::Down]);
		editNeighLeft_ = toUi(n.neighbor[(int)Direction::Left]);
		editNeighRight_ = toUi(n.neighbor[(int)Direction::Right]);

		jsonDirty_ = true;
	}

	auto toId = [&](int v)->uint32_t {
		return (v < 0) ? StageSelectGraph::INVALID_NODE_ID : (uint32_t)v;
		};

	auto validateNeighbor = [&](int v)->int {
		if (v < 0) return 0;
		if ((uint32_t)v >= count) return 2;
		return 1;
		};

	// UI 
	ImGui::Begin("StageSelect Graph", nullptr, ImGuiWindowFlags_NoCollapse);

	ImGui::Text("Nodes: %u", count);
	ImGui::SameLine();
	ImGui::TextDisabled("Selected: %u", (count > 0) ? editNodeId_ : 0);
	ImGui::Separator();

	// Tabs 
	if (ImGui::BeginTabBar("GraphTabs"))
	{
		if (ImGui::BeginTabItem("List"))
		{
			ImGui::Checkbox("Unlocked only", &filterUnlockedOnly_);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(140.0f);
			ImGui::InputInt("Filter StageId", &filterStageId_);
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				filterUnlockedOnly_ = false;
				filterStageId_ = -1;
			}

			ImGui::Separator();

			ImGui::BeginChild("NodeList", ImVec2(0, 0), true);

			if (ImGui::BeginTable("nodes_table", 6,
				ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
				ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
			{
				ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 60);
				ImGui::TableSetupColumn("StageId", ImGuiTableColumnFlags_WidthFixed, 70);
				ImGui::TableSetupColumn("Unlocked", ImGuiTableColumnFlags_WidthFixed, 80);
				ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 60);
				ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 60);
				ImGui::TableSetupColumn("Neigh", ImGuiTableColumnFlags_WidthFixed, 70);
				ImGui::TableHeadersRow();

				for (uint32_t i = 0; i < count; ++i)
				{
					const StageNode& n = stageSelectGraph->GetNode(i);

					if (filterUnlockedOnly_ && !n.unlocked) continue;
					if (filterStageId_ >= 0 && (int)n.stageId != filterStageId_) continue;

					int neighCount = 0;
					for (int d = 0; d < (int)Direction::count; ++d)
						if (n.neighbor[d] != StageSelectGraph::INVALID_NODE_ID) neighCount++;

					ImGui::TableNextRow();

					ImGui::TableNextColumn();
					bool selected = (editNodeId_ == i);

					ImGui::PushID((int)i);
					if (!n.unlocked) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(160, 160, 160, 255));
					if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_SpanAllColumns))
					{
						editNodeId_ = i;
					}
					if (!n.unlocked) ImGui::PopStyleColor();
					ImGui::PopID();

					ImGui::SameLine();
					ImGui::Text("%u", i);

					ImGui::TableNextColumn(); ImGui::Text("%u", n.stageId);
					ImGui::TableNextColumn(); ImGui::TextUnformatted(n.unlocked ? "Yes" : "No");
					ImGui::TableNextColumn(); ImGui::Text("%u", n.position.x);
					ImGui::TableNextColumn(); ImGui::Text("%u", n.position.y);
					ImGui::TableNextColumn(); ImGui::Text("%d", neighCount);
				}

				ImGui::EndTable();
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		// Node tab
		if (ImGui::BeginTabItem("Node"))
		{
			if (count == 0)
			{
				ImGui::TextDisabled("No nodes.");
			} else
			{
				ImGui::Text("Editing node: %u", editNodeId_);
				ImGui::Separator();

				ImGui::SetNextItemWidth(200.0f);
				ImGui::InputInt("StageId", &editStageId_);
				ImGui::SetNextItemWidth(200.0f);
				ImGui::InputText("StageKey", editStageKey_, sizeof(editStageKey_));
				ImGui::Checkbox("Unlocked", &editUnlocked_);

				ImGui::Separator();

				ImGui::SetNextItemWidth(120.0f);
				ImGui::InputInt("PosX", &editX_);
				ImGui::SameLine();
				ImGui::SetNextItemWidth(120.0f);
				ImGui::InputInt("PosY", &editY_);
				ImGui::Separator();
				ImGui::Text("Rotation Settings");

				// Yaw角度の編集（度数法で表示）
				const StageNode& currentNode = stageSelectGraph->GetNode(editNodeId_);
				bool isAutoYaw = (currentNode.defaultYaw == StageSelectGraph::AUTO_YAW);

				ImGui::Checkbox("Auto Calculate Yaw", &isAutoYaw);
				ImGui::SameLine();
				ImGui::TextDisabled("(?)");
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("If checked, yaw will be automatically calculated to face the camera");
				}

				if (!isAutoYaw) {
					// ラジアンを度に変換して表示
					float yawDegrees = currentNode.defaultYaw * 57.2957795f;
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::SliderFloat("Yaw (degrees)", &yawDegrees, 0.0f, 360.0f, "%.1f")) {
						// 度をラジアンに変換して設定
						float yawRad = yawDegrees * 0.0174532925f;
						stageSelectGraph->SetNodeYaw(editNodeId_, yawRad);
						jsonDirty_ = true;
					}
				} else {
					// AUTO_YAWに設定
					if (currentNode.defaultYaw != StageSelectGraph::AUTO_YAW) {
						stageSelectGraph->SetNodeYaw(editNodeId_, StageSelectGraph::AUTO_YAW);
						jsonDirty_ = true;
					}
				}
				if (ImGui::Button("Apply Node"))
				{
					stageSelectGraph->SetNodeStageId(editNodeId_, (uint32_t)(std::max)(editStageId_, 0));
					stageSelectGraph->SetNodeStageKey(editNodeId_, std::string(editStageKey_));

					stageSelectGraph->SetNodeUnlocked(editNodeId_, editUnlocked_);
					stageSelectGraph->SetNodePos(editNodeId_, {
						(uint32_t)(std::max)(editX_, 0),
						(uint32_t)(std::max)(editY_, 0)
						});

					jsonDirty_ = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Reload From Graph"))
				{
					prevEditNodeId_ = UINT32_MAX;
				}
			}

			ImGui::EndTabItem();
		}

		// Neighbors tab
		if (ImGui::BeginTabItem("Neighbors"))
		{
			if (count == 0)
			{
				ImGui::TextDisabled("No nodes.");
			} else
			{
				ImGui::Text("Editing node: %u", editNodeId_);
				ImGui::TextDisabled("Use -1 for none. Valid range: 0..%u", (count > 0) ? (count - 1) : 0);
				ImGui::Separator();

				ImGui::SetNextItemWidth(140.0f); ImGui::InputInt("Up", &editNeighUp_);
				{
					int s = validateNeighbor(editNeighUp_);
					ImGui::SameLine();
					if (s == 0) ImGui::TextDisabled("none");
					else if (s == 2) ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "invalid");
					else ImGui::Text("ok");
				}

				ImGui::SetNextItemWidth(140.0f); ImGui::InputInt("Down", &editNeighDown_);
				{
					int s = validateNeighbor(editNeighDown_);
					ImGui::SameLine();
					if (s == 0) ImGui::TextDisabled("none");
					else if (s == 2) ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "invalid");
					else ImGui::Text("ok");
				}

				ImGui::SetNextItemWidth(140.0f); ImGui::InputInt("Left", &editNeighLeft_);
				{
					int s = validateNeighbor(editNeighLeft_);
					ImGui::SameLine();
					if (s == 0) ImGui::TextDisabled("none");
					else if (s == 2) ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "invalid");
					else ImGui::Text("ok");
				}

				ImGui::SetNextItemWidth(140.0f); ImGui::InputInt("Right", &editNeighRight_);
				{
					int s = validateNeighbor(editNeighRight_);
					ImGui::SameLine();
					if (s == 0) ImGui::TextDisabled("none");
					else if (s == 2) ImGui::TextColored(ImVec4(1, 0.2f, 0.2f, 1), "invalid");
					else ImGui::Text("ok");
				}

				if (ImGui::Button("Apply Neighbors"))
				{
					stageSelectGraph->SetNeighbor(editNodeId_, Direction::Up, toId(editNeighUp_));
					stageSelectGraph->SetNeighbor(editNodeId_, Direction::Down, toId(editNeighDown_));
					stageSelectGraph->SetNeighbor(editNodeId_, Direction::Left, toId(editNeighLeft_));
					stageSelectGraph->SetNeighbor(editNodeId_, Direction::Right, toId(editNeighRight_));
					jsonDirty_ = true;
				}

				ImGui::SameLine();
				if (ImGui::Button("Jump To Up") && editNeighUp_ >= 0 && (uint32_t)editNeighUp_ < count)    editNodeId_ = (uint32_t)editNeighUp_;
				ImGui::SameLine();
				if (ImGui::Button("Jump To Down") && editNeighDown_ >= 0 && (uint32_t)editNeighDown_ < count) editNodeId_ = (uint32_t)editNeighDown_;

				if (ImGui::Button("Jump To Left") && editNeighLeft_ >= 0 && (uint32_t)editNeighLeft_ < count) editNodeId_ = (uint32_t)editNeighLeft_;
				ImGui::SameLine();
				if (ImGui::Button("Jump To Right") && editNeighRight_ >= 0 && (uint32_t)editNeighRight_ < count) editNodeId_ = (uint32_t)editNeighRight_;
			}

			ImGui::EndTabItem();
		}


		// JSON tab

		if (ImGui::BeginTabItem("JSON"))
		{
			ImGui::TextDisabled("Graph JSON preview (read-only).");

			if (ImGui::Button("Refresh"))
			{
				jsonDirty_ = true;
			}
			ImGui::SameLine();

			static char saveName[128] = "StageSelectNodes";
			ImGui::SetNextItemWidth(220.0f);
			ImGui::InputText("Save BaseName", saveName, sizeof(saveName));
			ImGui::SameLine();
			if (ImGui::Button("Save JSON"))
			{
				stageSelectGraph->SaveToJsonFile(saveName);
			}

			ImGui::Separator();

			if (jsonDirty_)
			{
				std::string s = stageSelectGraph->ToJsonString(2);
				strncpy_s(jsonBuf_, sizeof(jsonBuf_), s.c_str(), _TRUNCATE);
				jsonDirty_ = false;
			}

			ImGui::InputTextMultiline("##json",
				jsonBuf_, sizeof(jsonBuf_),
				ImVec2(-1.0f, -1.0f),
				ImGuiInputTextFlags_ReadOnly);

			ImGui::EndTabItem();
		}

		// Ops tab
		if (ImGui::BeginTabItem("Ops"))
		{
			ImGui::Text("Add Node");
			ImGui::SetNextItemWidth(90.0f);  ImGui::InputInt("New X", &newX_);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(90.0f);  ImGui::InputInt("New Y", &newY_);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120.0f); ImGui::InputInt("New StageId", &newStageId_);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(120.0f); ImGui::InputText("New StageKey", newStageKey_, sizeof(newStageKey_));
			ImGui::SameLine();
			ImGui::Checkbox("New Unlocked", &newUnlocked_);

			if (ImGui::Button("Add"))
			{
				uint32_t newId = stageSelectGraph->AddNode(
					{ (uint32_t)(std::max)(newX_, 0), (uint32_t)(std::max)(newY_, 0) },
					(uint32_t)(std::max)(newStageId_, 0),
					std::string(newStageKey_),
					newUnlocked_
				);

				editNodeId_ = newId;
				prevEditNodeId_ = UINT32_MAX;
				jsonDirty_ = true;
			}

			ImGui::Separator();
			ImGui::TextDisabled("Tips: Edit values here, then press Apply in each tab.");

			ImGui::EndTabItem();
		}

		// Check tab
		if (ImGui::BeginTabItem("Check"))
		{
			ImGui::Text("CurrentNodeId: %u", currentNodeId);

			const StageNode& cn = stageSelectGraph->GetNode(currentNodeId);
			ImGui::Text("Pos: (%u, %u)", cn.position.x, cn.position.y);

			auto showTo = [&](Direction dir, const char* name) {
				uint32_t to = cn.neighbor[(int)dir];
				ImGui::Text("%s -> %u", name, to);
				};
			showTo(Direction::Up, "Up");
			showTo(Direction::Down, "Down");
			showTo(Direction::Left, "Left");
			showTo(Direction::Right, "Right");

			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
#endif
}