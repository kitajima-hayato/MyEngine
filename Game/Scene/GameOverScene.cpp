#include "GameOverScene.h"
#include "engine/InsideScene/Framework.h"

GameOverScene::GameOverScene()
{
}

GameOverScene::~GameOverScene()
{
}

void GameOverScene::Initialize(Engine::DirectXCommon* dxCommon)
{
	// カメラの取得
	camera = Framework::GetMainCamera();
	cameraTransform = { {0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f},{-0.3f,-6.17f,13.47f} };
	camera->SetTranslate(cameraTransform.translate);

	backGround = std::make_unique<BackGround>();
	backGround->Initialize();

	backBlack = std::make_unique<Object3D>();
	backBlack->Initialize();
	backBlack->SetModel("Scenes/GameOver/Models/backblack");
	backBlack->SetTransform(backBlackTransform);

	backBlackTransform = { {1.0f,1.0f,1.0f},{0.0f,-1.6f,0.0f},{0.1f,-6.0f,20.1f} };
	backBlack->SetTransform(backBlackTransform);

	PlayerModel = std::make_unique<Object3D>();
	PlayerModel->Initialize();
	PlayerModel->SetModel("GamePlay/Player");
	playerModelTransform = { {1.0f,1.0f,1.0f},{2.6f,2.3f,2.0f},{0.0f,-7.0f,20.0f} };
	PlayerModel->SetTransform(playerModelTransform);

	gameOverUI = std::make_unique<Sprite>();
	gameOverUI->Initialize("resources/Scenes/GameOver/UI/Texture/GameOverUIText.png");
	gameOverUI->SetPosition({ 0.0f,0.0f });
	gameOverUI->SetSize({ 1280.0f,720.0f });


	// OneMore / Select / Title UI
	oneMore_ = std::make_unique<Sprite>();
	oneMore_->Initialize("resources/Scenes/Clear/UI/Texture/ClearUI_OneMore.dds");
	oneMore_->SetPosition({ 250.0f,570.0f });
	oneMore_->SetSize(oneMoreBaseSize_);

	select_ = std::make_unique<Sprite>();
	select_->Initialize("resources/Scenes/Clear/UI/Texture/ClearUI_Select.dds");
	select_->SetPosition({ 550.0f,570.0f });
	select_->SetSize(selectBaseSize_);

	title_ = std::make_unique<Sprite>();
	title_->Initialize("resources/Scenes/Clear/UI/Texture/ClearUI_Title.dds");
	title_->SetPosition({ 850.0f,570.0f });
	title_->SetSize(titleBaseSize_);

	// KeyIconUi　/ 左下に配置

	keyIcon_A = std::make_unique<Sprite>();
	keyIcon_A->Initialize("resources/_Common/UI/Texture/inputhints/A.dds");
	keyIcon_A->SetPosition({ 30.0f, 635.0f });
	keyIcon_A->SetSize({ 50.0f, 50.0f });

	keyIcon_D = std::make_unique<Sprite>();
	keyIcon_D->Initialize("resources/_Common/UI/Texture/inputhints/D.dds");
	keyIcon_D->SetPosition({ 80.0f, 635.0f });
	keyIcon_D->SetSize({ 50.0f, 50.0f });

	keyIcon_Enter = std::make_unique<Sprite>();
	keyIcon_Enter->Initialize("resources/_Common/UI/Texture/inputhints/Enter.dds");
	keyIcon_Enter->SetPosition({ 55.0f, 565.0f });
	keyIcon_Enter->SetSize({ 50.0f, 50.0f });


	playerFinalTr_ = playerModelTransform;     // scale/rotate/translate を含む最終値
	StartPlayerDropIntro(playerFinalTr_);

	// 初期フレームから上にいる状態を反映
	PlayerModel->SetTransform(playerAnimTr_);
}

void GameOverScene::Update()
{
	// カメラの更新
	camera->Update();
	if (Input::GetInstance()->TriggerKey(DIK_A) || Input::GetInstance()->TriggerKey(DIK_LEFT)) {
		int idx = static_cast<int>(selectedItem_);
		idx = (idx - 1 + static_cast<int>(ClearMenuItem::Count)) % static_cast<int>(ClearMenuItem::Count);
		selectedItem_ = static_cast<ClearMenuItem>(idx);
	}
	// 右へ（D or →）
	else if (Input::GetInstance()->TriggerKey(DIK_D) || Input::GetInstance()->TriggerKey(DIK_RIGHT)) {
		int idx = static_cast<int>(selectedItem_);
		idx = (idx + 1) % static_cast<int>(ClearMenuItem::Count);
		selectedItem_ = static_cast<ClearMenuItem>(idx);
	}

	// ========= 決定（Enter / Space） =========
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		switch (selectedItem_) {
		case ClearMenuItem::OneMore:
			SceneManager::GetInstance()->ChangeSceneWithTransition("GAMEPLAY", TransitionType::Normal);     // もう一回 = 今のステージをやり直すなら GAMEPLAY
			break;
		case ClearMenuItem::Select:
			SceneManager::GetInstance()->ChangeSceneWithTransition("STAGESELECT", TransitionType::Normal);
			break;
		case ClearMenuItem::Title:
			SceneManager::GetInstance()->ChangeSceneWithTransition("TITLE", TransitionType::Normal);
			break;
		default:
			break;
		}
		return; // シーン遷移したら以降の更新を止めたい場合
	}
	oneMore_->SetSize(oneMoreBaseSize_);
	select_->SetSize(selectBaseSize_);
	title_->SetSize(titleBaseSize_);

#ifdef USE_IMGUI
	// Inputクラスの取得と省略
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
#endif
	// 選択中だけ少し大きく
	auto Scale = [](const Vector2& v, float s) { return Vector2{ v.x * s, v.y * s }; };

	switch (selectedItem_) {
	case ClearMenuItem::OneMore:
		oneMore_->SetSize(Scale(oneMoreBaseSize_, selectScale_));
		break;
	case ClearMenuItem::Select:
		select_->SetSize(Scale(selectBaseSize_, selectScale_));
		break;
	case ClearMenuItem::Title:
		title_->SetSize(Scale(titleBaseSize_, selectScale_));
		break;
	default:
		break;
	}
	// タイトルへ戻る (1)
	if (Input::GetInstance()->TriggerKey(DIK_1)) {
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}
	// ステージセレクトへ戻る (2)
	else if (Input::GetInstance()->TriggerKey(DIK_2)) {
		SceneManager::GetInstance()->ChangeScene("STAGESELECT");
	}
	// ゲームプレイへ戻る (3)
	else if (Input::GetInstance()->TriggerKey(DIK_3)) {
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
	}
	// カメラの更新
	camera->Update();

	backGround->Update();
	backBlack->Update();

	const float dt = 1.0f / 60.0f;

	// 状態に応じて更新
	if (playerOutroState_ == PlayerOutroState::DropBounce) {
		UpdatePlayerDropIntro(dt);
	} else if (playerOutroState_ == PlayerOutroState::ShrinkSpin) {
		UpdatePlayerShrinkSpin(dt);
	}
	// Doneは何もしない

	// Transform反映（消えていない時だけ）
	if (isPlayerVisible_) {
		PlayerModel->SetTransform(playerAnimTr_);
	}

	PlayerModel->Update();

	gameOverUI->Update();

	keyIcon_A->Update();

	keyIcon_D->Update();
	keyIcon_Enter->Update();


	oneMore_->Update();
	select_->Update();
	title_->Update();
	// ImGuiの描画
	DrawImgui();
}

void GameOverScene::Draw()
{
	backGround->Draw();
	backBlack->Draw(); 
	if (isPlayerVisible_) {
		PlayerModel->Draw();
	}

	gameOverUI->Draw();

	keyIcon_A->Draw();

	keyIcon_D->Draw();
	keyIcon_Enter->Draw();
	oneMore_->Draw();
	select_->Draw();
	title_->Draw();
}

void GameOverScene::Finalize()
{
	// カメラの終了処理
	camera->Finalize();
}

void GameOverScene::DrawImgui()
{
#ifdef USE_IMGUI

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



	ImGui::End();
	// カメラの調整
	// カメラの配置情報の変更
	ImGui::Begin("Camera Transform");
	cameraTransform = { {0.0f,0.0f,0.0f}, camera->GetRotate(),camera->GetTranslate() };
	ImGui::DragFloat3("Camera Rotate", &cameraTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("Camera Translate", &cameraTransform.translate.x, 0.1f);

	// カメラの配置情報の反映
	camera->SetRotate(cameraTransform.rotate);
	camera->SetTranslate(cameraTransform.translate);


	// 背景の調整
	backBlackTransform = backBlack->GetTransform();
	ImGui::DragFloat3("BackBlack Scale", &backBlackTransform.scale.x, 0.1f);
	ImGui::DragFloat3("BackBlack Rotate", &backBlackTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("BackBlack Translate", &backBlackTransform.translate.x, 0.1f);
	backBlack->SetTransform(backBlackTransform);

	// プレイヤーモデルの調整
	playerModelTransform = PlayerModel->GetTransform();
	ImGui::DragFloat3("PlayerModel Scale", &playerModelTransform.scale.x, 0.1f);
	ImGui::DragFloat3("PlayerModel Rotate", &playerModelTransform.rotate.x, 0.1f);
	ImGui::DragFloat3("PlayerModel Translate", &playerModelTransform.translate.x, 0.1f);
	PlayerModel->SetTransform(playerModelTransform);


	ImGui::End();
#endif
}

void GameOverScene::UpdateDebugCamera()
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

void GameOverScene::StartPlayerDropIntro(const Transform& finalTr)
{
	playerFinalTr_ = finalTr;

	playerAnimTr_ = finalTr;
	playerAnimTr_.translate.y += spawnHeight_;

	vy_ = 0.0f;
	bounceCount_ = 0;
	rotBlendT_ = 0.0f;

	isDropIntroRunning_ = true;
	playerOutroState_ = PlayerOutroState::DropBounce;
	isPlayerVisible_ = true;
	loopDelayTimer_ = 0.0f;
}

void GameOverScene::UpdatePlayerDropIntro(float dt)
{
	if (!isDropIntroRunning_) { return; }

	// 重力
	vy_ += gravity_ * dt;
	playerAnimTr_.translate.y += vy_ * dt;

	const float floorY = playerFinalTr_.translate.y;

	if (playerAnimTr_.translate.y <= floorY)
	{
		playerAnimTr_.translate.y = floorY;

		// まだバウンドできるなら跳ねる
		if (bounceCount_ < maxBounces_)
		{
			// バウンド回数に応じて反発を弱める（自然に収束）
			float r = restitution_;
			for (int i = 0; i < bounceCount_; ++i) {
				r *= bounceDamping_;
			}

			if (std::fabs(vy_) < 0.2f) {
				vy_ = 0.0f;
				isDropIntroRunning_ = false;
				playerAnimTr_ = playerFinalTr_;

				// 追加：縮小回転へ
				StartPlayerShrinkSpin();
			} else {
				vy_ = -vy_ * r;
				bounceCount_++;
			}
		} else
		{
			// 規定回数跳ねたら固定
			vy_ = 0.0f;
			isDropIntroRunning_ = false;
			playerAnimTr_ = playerFinalTr_;
			
			StartPlayerShrinkSpin();
		}
	}

	// 回転補間（今のままでもOK）
	rotBlendT_ += dt * 2.5f;
	float t = (std::min)(rotBlendT_, 1.0f);

	playerAnimTr_.rotate.x = playerAnimTr_.rotate.x + (playerFinalTr_.rotate.x - playerAnimTr_.rotate.x) * t;
	playerAnimTr_.rotate.y = playerAnimTr_.rotate.y + (playerFinalTr_.rotate.y - playerAnimTr_.rotate.y) * t;
	playerAnimTr_.rotate.z = playerAnimTr_.rotate.z + (playerFinalTr_.rotate.z - playerAnimTr_.rotate.z) * t;
}

void GameOverScene::StartPlayerShrinkSpin()
{
	// バウンドが終わった瞬間の姿勢から開始
	playerOutroState_ = PlayerOutroState::ShrinkSpin;

	// もし「最終姿勢にピタッとしてから」縮小させたいならこれもOK
	// playerAnimTr_ = playerFinalTr_;
}

void GameOverScene::UpdatePlayerShrinkSpin(float dt)
{
	if (playerOutroState_ != PlayerOutroState::ShrinkSpin) { return; }

	// 回転し続ける
	playerAnimTr_.rotate.y += spinSpeedY_ * dt;
	playerAnimTr_.rotate.x += spinSpeedX_ * dt;

	// 縮小（等方縮小）
	playerAnimTr_.scale.x = (std::max)(0.0f, playerAnimTr_.scale.x - shrinkSpeed_ * dt);
	playerAnimTr_.scale.y = (std::max)(0.0f, playerAnimTr_.scale.y - shrinkSpeed_ * dt);
	playerAnimTr_.scale.z = (std::max)(0.0f, playerAnimTr_.scale.z - shrinkSpeed_ * dt);

	// 小さくなったら消す
	if (playerAnimTr_.scale.x <= minScale_ &&
		playerAnimTr_.scale.y <= minScale_ &&
		playerAnimTr_.scale.z <= minScale_)
	{
		if (playerAnimTr_.scale.x <= minScale_ &&
			playerAnimTr_.scale.y <= minScale_ &&
			playerAnimTr_.scale.z <= minScale_)
		{
			if (loopPlayerIntro_) {
				// すぐ最初からやり直す
				StartPlayerDropIntro(playerFinalTr_);
			} else {
				playerOutroState_ = PlayerOutroState::Done;
				isPlayerVisible_ = false;
			}
			return;
		}
	}
}