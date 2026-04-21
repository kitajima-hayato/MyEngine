#include "Player.h"
#include <algorithm>
#include "Input.h"
#include "Game/Particle/ParticlePresets.h"
#include "Game/Application/Enemy/EnemyBase/EnemyBase.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif



Collider::Type Player::GetType() const
{
	// 判定タイプがプレイヤーであることを返す
	return Collider::Type::Player;
}

AABB Player::GetAABB() const
{
	AABB aabb{};

	// プレイヤーの中心
	Vector3 pos = playerModel_->GetTranslate();

	// プレイヤーの幅と高さを取得
	float halfWidth = status_.kWidth / 2.0f;
	float halfHeight = status_.kHeight / 2.0f;

	// 奥行は気にしない
	float halfDepth = 0.5f;

	// AABBの最小座標と最大座標を設定
	aabb.min = { pos.x - halfWidth, pos.y - halfHeight, pos.z - halfDepth };
	aabb.max = { pos.x + halfWidth, pos.y + halfHeight, pos.z + halfDepth };

	// AABBを返す
	return aabb;
}

static bool IntersectsAABBRect2D(const AABB& aabb, const Rect& rect)
{
	// XYの平面だけで衝突判定をとる
	if (aabb.max.x < rect.left) return false;
	if (aabb.min.x > rect.right) return false;
	if (aabb.max.y < rect.bottom) return false;
	if (aabb.min.y > rect.top) return false;
	return true;
}

void Player::OnCollision(Collider* other)
{
	// 衝突相手が存在しなかったら処理しない
	if (!other)return;

	switch (other->GetType())
	{
		// 敵に衝突したら
	case Collider::Type::Enemy:
		// すでにダメージ中なら何もしない（無敵時間）
		if (isEnemyHit_) {
			break;
		}

		// 踏みつけ判定
		// 条件: プレイヤーが落下中 && プレイヤーが敵より上にいる
		if (velocity_.y < 0.0f) {
			// プレイヤーと敵のAABBを取得
			AABB playerAABB = GetAABB();
			AABB enemyAABB = other->GetAABB();

			// プレイヤーの底辺が敵の上部付近にあるかチェック
			// プレイヤーの中心が敵の中心より上にある
			float playerBottom = playerAABB.min.y;
			float enemyTop = enemyAABB.max.y;
			float enemyCenter = (enemyAABB.min.y + enemyAABB.max.y) * 0.5f;
			float playerCenter = (playerAABB.min.y + playerAABB.max.y) * 0.5f;

			// 踏みつけ判定: プレイヤーの中心が敵の中心より上 && プレイヤーの底が敵の上半分にある
			if (playerCenter > enemyCenter && playerBottom < enemyTop + 0.2f) {
				// 踏みつけ成功
				StompEnemy(other);
				break;
			}
		}

		// 踏みつけ失敗 -> 通常のダメージ処理
		TakeDamage();
		break;

	default:
		// 特に何もしない
		break;
	}
}

void Player::TakeDamage()
{
	// すでに無敵だったら無視
	if (isEnemyHit_) { return; }

	// 点滅スタートの初期化のみ
	isEnemyHit_ = true;
	flashingFrameCount_ = 0;
	isVisible_ = true;

	// 体力を減らす
	status_.kHealth -= 1;
}

void Player::StompEnemy(Collider* enemy)
{
	// エネミーを倒す
	if (EnemyBase* enemyBase = dynamic_cast<EnemyBase*>(enemy)) {
		enemyBase->OnStomped();  // 敵に踏みつけられたことを通知

		// 敵の位置を取得してエフェクト発生
		Vector3 enemyPos = enemyBase->GetTranslate();
		if (stompEffect_) {
			stompEffect_->SetTranslate(enemyPos);
			stompEffect_->Play();
		}
	}

	// プレイヤーは小ジャンプ（踏みつけた反動）
	velocity_.y = status_.kStompJumpPower;
	onGround_ = false;
}

bool Player::IsTouchingDamageBlock()
{
	if (!map_ || !playerModel_) { return false; }

	const AABB playerAABB = GetAABB();

	// AABBの四隅付近でインデックス範囲を作る
	// Map::GetMapChipIndexSetByPosition はマップ外だと (width,height) を返す仕様なので、clampして使う
	IndexSet minIdx = map_->GetMapChipIndexSetByPosition(Vector3(playerAABB.min.x, playerAABB.max.y, 0.0f)); // 左上寄り
	IndexSet maxIdx = map_->GetMapChipIndexSetByPosition(Vector3(playerAABB.max.x, playerAABB.min.y, 0.0f)); // 右下寄り

	const uint32_t w = map_->GetWidth();
	const uint32_t h = map_->GetHeight();
	if (w == 0 || h == 0) { return false; }

	auto clampIndex = [](uint32_t v, uint32_t maxExclusive) {
		return (v >= maxExclusive) ? (maxExclusive - 1) : v;
		};

	// マップ外を含む場合のガード（全部マップ外なら非接触）
	if (minIdx.xIndex >= w && maxIdx.xIndex >= w) return false;
	if (minIdx.yIndex >= h && maxIdx.yIndex >= h) return false;

	const uint32_t x0 = clampIndex((std::min)(minIdx.xIndex, maxIdx.xIndex), w);
	const uint32_t x1 = clampIndex((std::max)(minIdx.xIndex, maxIdx.xIndex), w);
	const uint32_t y0 = clampIndex((std::min)(minIdx.yIndex, maxIdx.yIndex), h);
	const uint32_t y1 = clampIndex((std::max)(minIdx.yIndex, maxIdx.yIndex), h);

	for (uint32_t y = y0; y <= y1; ++y) {
		for (uint32_t x = x0; x <= x1; ++x) {
			const BlockType t = map_->GetMapChipTypeByIndex(x, y);
			if (!IsHitBlockDamageTable(t)) { continue; }

			const Rect r = map_->GetRectByIndex(x, y);
			if (IntersectsAABBRect2D(playerAABB, r)) {
				return true;
			}
		}
	}

	return false;
}

bool Player::IsTouchingHazardSpike() const
{
	if (!map_ || !playerModel_) { return false; }

	const AABB playerAABB = GetAABB();

	// AABBの四隅付近でインデックス範囲を作る（damageBlock の実装と同じ考え方）
	IndexSet minIdx = map_->GetMapChipIndexSetByPosition(Vector3(playerAABB.min.x, playerAABB.max.y, 0.0f));
	IndexSet maxIdx = map_->GetMapChipIndexSetByPosition(Vector3(playerAABB.max.x, playerAABB.min.y, 0.0f));

	const uint32_t w = map_->GetWidth();
	const uint32_t h = map_->GetHeight();
	if (w == 0 || h == 0) { return false; }

	auto clampIndex = [](uint32_t v, uint32_t maxExclusive) {
		return (v >= maxExclusive) ? (maxExclusive - 1) : v;
		};

	// マップ外を含む場合のガード
	if (minIdx.xIndex >= w && maxIdx.xIndex >= w) return false;
	if (minIdx.yIndex >= h && maxIdx.yIndex >= h) return false;

	const uint32_t x0 = clampIndex((std::min)(minIdx.xIndex, maxIdx.xIndex), w);
	const uint32_t x1 = clampIndex((std::max)(minIdx.xIndex, maxIdx.xIndex), w);
	const uint32_t y0 = clampIndex((std::min)(minIdx.yIndex, maxIdx.yIndex), h);
	const uint32_t y1 = clampIndex((std::max)(minIdx.yIndex, maxIdx.yIndex), h);

	for (uint32_t y = y0; y <= y1; ++y) {
		for (uint32_t x = x0; x <= x1; ++x) {
			const HazardType hz = map_->GetHazardTypeByIndex(x, y);
			if (hz != HazardType::Spike) { continue; }

			// ちゃんと触れてるか Rect で確認（判定が安定する）
			const Rect r = map_->GetRectByIndex(x, y);
			if (IntersectsAABBRect2D(playerAABB, r)) {
				return true;
			}
		}
	}

	return false;
}

void Player::EmitJumpParticle()
{
	jumpEffect_->SetTranslate(GetFootParticlePos());
	jumpEffect_->Play();
}

void Player::EmitLandParticle()
{
	// 足元にパーティクルを出す
	if (!landEffect_ || !playerModel_) { return; }
	// 足元の位置を取得
	Vector3 base = GetFootParticlePos();

	// 左右に少しずらして2回
	landEffect_->SetTranslate(base + Vector3{ -0.20f, 0.0f, 0.0f });
	landEffect_->Play();

	landEffect_->SetTranslate(base + Vector3{ +0.20f, 0.0f, 0.0f });
	landEffect_->Play();
}

Vector3 Player::GetFootParticlePos() const
{
	// プレイヤーの位置を取得
	Vector3 p = playerModel_->GetTranslate();

	// AABBの一番下（足元）に合わせる
	const AABB aabb = GetAABB();
	p.y = aabb.min.y;

	// 地面に少しだけめり込ませる（見た目が安定する）
	p.y += 0.02f;

	// 2D寄りならZはそのままでOK。必要なら少し手前に出す等も可
	return p;
}

void Player::Respawn(const Vector3& pos)
{
	// 位置をリスポーン地点に移動
	velocity_ = {};
	if (playerModel_) {
		playerModel_->SetTranslate(pos);
		playerModel_->SetRotate({ 0.0f,0.0f,0.0f });
	}

	// 状態をリセット
	isDead_ = false;
	isDeathByFalling_ = false;
	onGround_ = true;
}

void Player::BeginDeathDemo(const Vector3& cameraPos)
{
	if (isDying_) return;

	isDying_ = true;
	deathDemoFinished_ = false;
	deathDemoTimer_ = 0.0f;

	controlEnabled_ = false;

	onGround_ = false;
	// その場でジャンプ開始
	deathVel_ = { 0.0f, deathJumpSpeed_, 0.0f };

	// こちらを向く
	deathFaceRot_ = { 0.0f, 1.6f, 0.0f };
	playerModel_->SetRotate(deathFaceRot_);
}

void Player::UpdateDeathDemo()
{
	deathDemoTimer_ += (1.0f / 60.0f);

	// 重力
	deathVel_.y -= deathGravity_ * (1.0f / 60.0f);

	Vector3 p = GetTranslate();
	p.y += deathVel_.y * (1.0f / 60.0f);
	playerModel_->SetTranslate(p);

	// 回転は固定でこちら向きにキープ（任意）
	playerModel_->SetRotate(deathFaceRot_);

	if ((deathDemoTimer_ >= deathDemoMinDuration_ && p.y < deathHeight_) ||
		deathDemoTimer_ >= 2.0f) {
		isDying_ = false;
		deathDemoFinished_ = true;
	}
}

bool Player::IsInDeathDemo() const
{
	return isDying_;
}

bool Player::IsDeathDemoFinished() const
{
	return deathDemoFinished_;
}

bool Player::ConsumeDeathByFalling()
{
	if (!isDeathByFalling_) { return false; }
	isDeathByFalling_ = false;
	return true;
}


void Player::FlashingUpdate()
{
	// エネミーにヒットしていたら点滅処理 / 無敵時間
	if (isEnemyHit_) {
		// フレームカウント更新
		flashingFrameCount_++;
		// 点滅処理
		if (flashingFrameCount_ <= maxFlashingFlame_) {
			// 一定間隔で表示・非表示を切り替え
			if (flashingFrameCount_ % flashingIntervalFrame_ == 1) {
				isVisible_ = !isVisible_;
			}
		} else {
			// 終了処理
			isEnemyHit_ = false;
			isVisible_ = true;
			flashingFrameCount_ = 0;
		}
	}
}

void Player::PlayerTurn()
{
	switch (direction_)
	{
	case Direction::kRight:

		// 右向き
		targetYaw_ = 0.0f;

		break;
	case Direction::kLeft:
		// 左向き
		targetYaw_ = 3.5f;

		break;

	default:
		break;
	}

	// 現在の回転を取得
	Vector3 currentRotate = playerModel_->GetRotate();
	// 補間係数の計算
	float t = 1.0f - std::pow(0.5f, static_cast<float>(1.0f) / (status_.kTurnTime * 60.0f));
	// ヨー角の補間
	currentRotate.y += (targetYaw_ - currentRotate.y) * t;
	// 回転の設定
	playerModel_->SetRotate(currentRotate);

}

void Player::UpdateDashEffect()
{
	// 現在のダッシュ状態を取得
	const bool movingRight = Input::GetInstance()->PushKey(DIK_D) && !Input::GetInstance()->PushKey(DIK_A);
	const bool movingLeft = Input::GetInstance()->PushKey(DIK_A) && !Input::GetInstance()->PushKey(DIK_D);
	// ダッシュ状態の判定
	const bool dashActiveRight = isDash_ && (dashDirection_ == +1) && movingRight;
	const bool dashActiveLeft = isDash_ && (dashDirection_ == -1) && movingLeft;
	const bool dashActive = dashActiveRight || dashActiveLeft;

	// 地面に接地しているか
	const bool canShowEffect = dashActive && onGround_;
	// プレイヤーの現在地を取得
	Vector3 playerPos = playerModel_->GetTranslate();

	// ダッシュ開始時の衝撃波エフェクト
	if (!wasDashing_ && canShowEffect) {
		dashStartEffect_->SetTranslate(playerPos);
		// 発生
		dashStartEffect_->Play();
	}

	// ダッシュ中の継続エフェクト
	if (canShowEffect) {
		// 足元の位置を計算
		Vector3 smokePos = playerPos;
		// 発生位置を足元らへんに
		smokePos.y -= particleSpawnPosOffset_;

		// 進行方向とは逆方向に
		if (dashDirection_ == +1) {
			smokePos.x -= particleSpawnPosOffset_;
		} else if (dashDirection_ == -1) {
			smokePos.x += particleSpawnPosOffset_;
		}

		// エフェクトの位置更新
		dashSmokeEffect_->SetTranslate(smokePos);

		// 発生
		if (!wasDashing_) {
			dashSmokeEffect_->Play();
		}
	} else {
		// ダッシュしていないときは煙のエフェクトを停止
		dashSmokeEffect_->Stop();
	}
	// 前フレームの状態を保持
	wasDashing_ = canShowEffect;
}

void Player::UpdateMoveEffect()
{
	if (!moveEffect_ || !playerModel_) { return; }

	// 入力状態（Move()と同じ判定に合わせる）
	auto* input = Input::GetInstance();
	const bool rightHold = input->PushKey(DIK_D);
	const bool leftHold = input->PushKey(DIK_A);

	// 左右どちらかだけ押している＝移動中
	const bool movingRight = rightHold && !leftHold;
	const bool movingLeft = leftHold && !rightHold;
	const bool moving = movingRight || movingLeft;

	// 通常移動として扱いたい条件
	const bool canShowEffect = controlEnabled_ && onGround_ && moving && !isDash_;

	Vector3 playerPos = playerModel_->GetTranslate();

	if (canShowEffect) {
		// 足元に出す
		Vector3 smokePos = playerPos;
		smokePos.y -= particleSpawnPosOffset_;

		// 進行方向とは逆に少しずらす（ダッシュ煙と同じ見た目の思想）
		if (movingRight) {
			smokePos.x -= particleSpawnPosOffset_;
		} else if (movingLeft) {
			smokePos.x += particleSpawnPosOffset_;
		}

		moveEffect_->SetTranslate(smokePos);
		if (!wasMoving_) {
			moveEffect_->Play();
		}
	} else {
		moveEffect_->Pause();
	}
	wasMoving_ = canShowEffect;
}



void Player::Initialize(Vector3 position)
{
	// プレイヤーモデルの生成と初期化
	playerModel_ = std::make_unique<Object3D>();
	playerModel_->Initialize();
	playerModel_->SetTranslate(position);
	playerModel_->SetModel("GamePlay/Player");
	// 死ぬ高さの設定
	SetDeathHeight(-3.0f);

	// 通常移動のエフェクトの初期化
	// 足元の煙エフェクト(通常移動)
	moveEffect_ = ParticlePresets::CreateSmoke(position);
	// 初期は停止
	moveEffect_->Pause();
	moveEffect_->SetEmissionRate(5.0f);
	moveEffect_->SetLoop(true);
	//moveEffect_->SetColor(Vector4(0.7f, 0.6f, 0.4f, 1.0f));
	moveEffect_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));


	// ダッシュエフェクトの初期化
	// 足元の煙エフェクト(ダッシュ)
	dashSmokeEffect_ = ParticlePresets::CreateSmoke(position);
	// 初期は停止
	dashSmokeEffect_->Pause();
	dashSmokeEffect_->SetEmissionRate(15.0f);
	dashSmokeEffect_->SetLoop(true);
	//dashSmokeEffect_->SetColor(Vector4(0.7f, 0.6f, 0.4f, 1.0f));
	dashSmokeEffect_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));


	// ダッシュ開始時の衝撃波
	dashStartEffect_ = ParticlePresets::CreateSparks(position);
	dashStartEffect_->Pause();
	// 一気に
	dashStartEffect_->SetEmissionRate(20.0f);
	// 一度の発生のためループはしない
	dashStartEffect_->SetLoop(false);
	// 色 / 黄色
	dashStartEffect_->SetColor(Vector4(1.0f, 1.0f, 0.5f, 1.0f));

	// 踏みつけエフェクトの初期化
	stompEffect_ = ParticlePresets::CreateSparks(position);
	stompEffect_->Pause();
	stompEffect_->SetEmissionRate(30.0f);
	stompEffect_->SetLoop(false);
	stompEffect_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));
 

	// ジャンプの開始と着地のパーティクル
	jumpEffect_ = ParticlePresets::CreateJumpDust(position);
	landEffect_ = ParticlePresets::CreateLandDust(position);

	// ジャンプブロックに乗っているときの上昇エフェクト / 初期状態は停止
	jumpBlockArrowEffect_ = ParticlePresets::CreateJumpBlockArrow(position);
	jumpBlockArrowEffect_->Pause();

}


void Player::Update()
{
	// ジャンプブロックに乗っているか
	bool onJumpBlockNow = onGround_ && IsOnJumpBlock();

	// ジャンプブロックに乗っているときだけ上昇エフェクトを出す
	if (jumpBlockArrowEffect_ && playerModel_) {
		// 上昇エフェクトの発生位置
		Vector3 effectPosition = playerModel_->GetTranslate();
		effectPosition.z -= 0.5f;
		// プレイヤーの周りに出す
		jumpBlockArrowEffect_->SetTranslate(effectPosition);

		// プレイヤーの頭上にエフェクトを出す / 乗っていたら
		if (onJumpBlockNow) {
			// プレイヤーの頭上にエフェクトを出す / 乗っていたら
			if (!wasOnJumpBlock_) {
				jumpBlockArrowEffect_->Play();
			}
		} 
		// 乗っていないときはエフェクトを停止
		else {
			jumpBlockArrowEffect_->Pause();
		}
	}

	// 前フレームの状態を保持
	wasOnJumpBlock_ = onJumpBlockNow;

	
	if (isDying_) {
		UpdateDeathDemo();
		playerModel_->Update();
		return;
	}
	// プレイヤーの挙動更新
	UpdateBehavior();
	// 通常移動エフェクトの更新
	UpdateMoveEffect();
	// ダッシュエフェクトの更新
	UpdateDashEffect();
	// プレイヤーの回転
	PlayerTurn();
	// エネミーにヒットしたら
	FlashingUpdate();

#ifdef USE_IMGUI
	ImGui();
#endif 
	// モデルの更新
	playerModel_->Update();

	// パーティクルシステムの更新
	if (moveEffect_)moveEffect_->Update();
	if (dashSmokeEffect_)dashSmokeEffect_->Update();
	if (dashStartEffect_)dashStartEffect_->Update();
	if (stompEffect_)stompEffect_->Update();

	if (jumpEffect_)jumpEffect_->Update();
	if (landEffect_)landEffect_->Update();
	if (jumpBlockArrowEffect_)jumpBlockArrowEffect_->Update();
	

}



void Player::Draw()
{
	// モデルの描画
	if (isVisible_) {
		playerModel_->Draw();
	}
}

void Player::UpdateBehavior()
{
	// マップがセットされていなかったら処理しない
	if (!map_)return;

	if (!IsAlive()) {
		velocity_ = {};
		return;
	}

	// 死亡していたらリセット処理
	if (isDead_) {
		DebugPlayerReset();
		return;
	}

	// 移動処理
	Move();
	// ジャンプ処理
	Jump();

	// 当たり判定処理
	CollisionMapInfo collisionInfo;

	// 地面にいなかったらリセット
	if (playerModel_->GetTranslate().y < deathHeight_ && !onGround_)
	{
		// 死亡フラグ
		status_.kHealth -= 1;
		isDead_ = true;
		isDeathByFalling_ = true;
	}

	// 移動量の反映
	collisionInfo.move = velocity_;

	// 上昇中は確実に地面から離れている（追加）
	if (velocity_.y > 0.0f) {
		onGround_ = false;
	}

	// マップ衝突チェック
	MapCollision(collisionInfo);
	// 天井衝突処理
	CellingCollisionMove(collisionInfo);
	// 床衝突処理
	LandingCollisionMove(collisionInfo);
	// 壁衝突処理
	WallCollisionMove(collisionInfo);
	// 速度反映
	PlayerCollisionMove(collisionInfo);

	// ダメージブロックに触れているか
	if (IsTouchingHazardSpike()) {
		TakeDamage();
	}
}

void Player::Move()
{
	// 操作ロック中：入力は無視して速度減衰のみ行う
	if (!controlEnabled_) {
		// 走り続け防止（キー押しっぱなし解除直後の事故対策）
		velocity_.x *= (1.0f - status_.kAttenuation);

		// ダッシュ状態も解除
		isDash_ = false;
		dashDirection_ = 0;
		rightTapTimer_ = 0;
		leftTapTimer_ = 0;

		return;
	}


	auto* input = Input::GetInstance();

	// 入力状態
	const bool rightHold = input->PushKey(DIK_D);
	const bool leftHold = input->PushKey(DIK_A);

	const bool rightTrig = input->TriggerKey(DIK_D);
	const bool leftTrig = input->TriggerKey(DIK_A);

	// ダブルタップ用タイマー更新
	if (rightTapTimer_ > 0) { --rightTapTimer_; }
	if (leftTapTimer_ > 0) { --leftTapTimer_; }

	// 右ダブルタップ判定
	if (rightTrig) {
		// 一定時間内に2回目が来たらダッシュ開始
		if (rightTapTimer_ > 0) {
			isDash_ = true;
			dashDirection_ = +1;
		}
		// タイマーリセット
		rightTapTimer_ = status_.kDashDoubleTapFrame;
		// 反対側はリセット
		leftTapTimer_ = 0;
	}

	// 左ダブルタップ判定
	if (leftTrig) {
		if (leftTapTimer_ > 0) {
			isDash_ = true;
			dashDirection_ = -1;
		}
		leftTapTimer_ = status_.kDashDoubleTapFrame;
		rightTapTimer_ = 0;
	}

	// ダッシュ継続／解除判定
	const bool movingRight = rightHold && !leftHold;
	const bool movingLeft = leftHold && !rightHold;

	// 入力が止まったらダッシュ解除
	if (!movingRight && !movingLeft) {
		isDash_ = false;
		dashDirection_ = 0;
	} else if (isDash_) {
		// 逆方向に入力したらダッシュ解除
		if ((dashDirection_ == +1 && movingLeft) ||
			(dashDirection_ == -1 && movingRight)) {
			isDash_ = false;
			dashDirection_ = 0;
		}
	}

	// 実際の速度更新
	Vector3 acceleration{};

	// 左右のどちらの方向にダッシュしているか判別
	const bool dashActiveRight = isDash_ && (dashDirection_ == +1) && movingRight;
	const bool dashActiveLeft = isDash_ && (dashDirection_ == -1) && movingLeft;
	// どちらに向いているかを持つ
	const bool dashActive = dashActiveRight || dashActiveLeft;


	// ダッシュ中だけ少し速くする
	float dashScale = dashActive ? status_.kDashSpeedScale : 1.0f;
	float maxSpeed = status_.kMaxSpeed * dashScale;


	// 横方向速度更新
	if (movingRight) {
		// 左に動いていた場合は少し減速
		if (velocity_.x < 0.0f) {
			velocity_.x *= (1.0f - status_.kAttenuation);
		}
		acceleration.x += status_.kAcceleration * dashScale;

		// 押された方向にディレクションを向ける
		direction_ = Direction::kRight;
	} else if (movingLeft) {
		if (velocity_.x > 0.0f) {
			velocity_.x *= (1.0f - status_.kAttenuation);
		}
		acceleration.x -= status_.kAcceleration * dashScale;

		// 押された方向にディレクションを向ける
		direction_ = Direction::kLeft;
	} else {
		// 入力がないときは減速のみ（ダッシュ倍率はかけない）
		velocity_.x *= (1.0f - status_.kAttenuation);
	}

	// 加速度反映
	velocity_.x += acceleration.x;

	// 速度に応じて回転
	Vector3 rotate = playerModel_->GetRotate();
	rotate -= {0.0f, 0.0f, velocity_.x};
	// 回転が規定値であるplayerTurnAround_が6.3fになったら回転の値をリセットする
	if (-playerTurnAround_ >= rotate.z && movingRight ||
		playerTurnAround_ <= rotate.z && movingLeft) {
		rotate.z = 0.0f;
	}
	// プレイヤーの回転
	playerModel_->SetRotate(rotate);

	// 最大速度をダッシュ状態に応じて変える
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
}


void Player::Jump()
{
	// 操作ロック中：ジャンプ入力は無視
	if (!controlEnabled_) {
		// 重力だけは通常通り掛ける（空中で停止しないように）
		if (!onGround_) {
			velocity_.y += -status_.kGravity;
			velocity_.y = (std::max)(velocity_.y, -status_.kMaxFallSpeed);
		}
		return;
	}

	// 地面にいる場合
	if (onGround_) {
		// ジャンプキーが押されたら
		if (Input::GetInstance()->TriggerKey(DIK_SPACE) || Input::GetInstance()->PushKey(DIK_W)) {
			// ジャンプスイッチの切り替え
			map_->ToggleJumpSwitch();

			// ジャンプの開始エフェクトの発生
			EmitJumpParticle();
			
			// ジャンプブロックの上にいたら高く跳ねる
			if (IsOnJumpBlock()) {
				velocity_.y = status_.kJumpBlockPower;
			} 
			// 通常のジャンプ
			else {
				velocity_.y = status_.kJumpPower;
			}

			// 空中にいる状態にする
			onGround_ = false;
		}
	} else {
		// 重力適応処理
		velocity_.y += -status_.kGravity;
		velocity_.y = (std::max)(velocity_.y, -status_.kMaxFallSpeed);
	}

}



void Player::MapCollision(CollisionMapInfo& collisionInfo)
{
	CollisionMapInfoDirection(
		collisionInfo,
		CollisionType::kRight,
		{ kRightTop,kRightBottom },
		Vector3(status_.kEpsilon, 0.0f, 0.0f),
		[](const CollisionMapInfo& info) {
			return info.move.x > 0.0f;
		});

	CollisionMapInfoDirection(
		collisionInfo,
		CollisionType::kLeft,
		{ kLeftTop,kLeftBottom },
		Vector3(-status_.kEpsilon, 0.0f, 0.0f),
		[](const CollisionMapInfo& info) {
			return info.move.x < 0.0f;
		});

	CollisionMapInfoDirection(
		collisionInfo,
		CollisionType::kTop,
		{ kRightTop,kLeftTop },
		Vector3(0.0f, 0.0f, 0.0f),
		[](const CollisionMapInfo& info) {
			return info.move.y > 0.0f;
		});

	CollisionMapInfoDirection(
		collisionInfo,
		CollisionType::kBottom,
		{ kRightBottom,kLeftBottom },
		Vector3(0.0f, -status_.kEpsilon, 0.0f),
		[](const CollisionMapInfo& info) {
			return info.move.y < 0.0f;
		});
}

void Player::CellingCollisionMove(CollisionMapInfo& collisionInfo)
{
	// 天井に衝突しているかどうか
	if (collisionInfo.celling) {
		// 天井に衝突したら移動量を調整
		velocity_.y = 0.0f;

		// プレイヤーの頭に当たったブロックを調べる
		Vector3 position = playerModel_->GetTranslate();

		// 頭の2点の位置を計算
		std::array<Vector3, 2> topCorners = {
			CornerPosition(position,kLeftTop),
			CornerPosition(position,kRightTop)
		};
		// 各コーナーの新しい位置で当たったブロックを調べる
		for (const auto& cornerPosition : topCorners) {
			IndexSet indexSet = map_->GetMapChipIndexSetByPosition(cornerPosition);
			BlockType blockType = map_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

			// Break Blockだった場合破壊する 
			if (IsHitBlockBreakableTable(blockType)) {
				// ブロック破壊
				map_->BreakBlock(indexSet.xIndex, indexSet.yIndex - 1);
			}
		}

	}
}

void Player::LandingCollisionMove(CollisionMapInfo& collisionInfo)
{
	// 着地判定（空中から地面への衝突）
	if (!onGround_ && collisionInfo.landing) {
		// 床に衝突したら移動量を調整
		velocity_.y = 0.0f;
		onGround_ = true;

		// 着地エフェクトの発生
		EmitLandParticle();

		return;
	}

	// 地面にいる場合の継続判定
	if (onGround_) {
		// 
		if (velocity_.y <= 0.0f) {
			Vector3 position = playerModel_->GetTranslate();

			Vector3 leftBottom = CornerPosition(position, kLeftBottom);
			Vector3 rightBottom = CornerPosition(position, kRightBottom);

			Vector3 checkOffset = Vector3(0.0f, -status_.kEpsilon * 2.0f, 0.0f);

			IndexSet leftIndex = map_->GetMapChipIndexSetByPosition(leftBottom + checkOffset);
			BlockType leftBlock = map_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);

			IndexSet rightIndex = map_->GetMapChipIndexSetByPosition(rightBottom + checkOffset);
			BlockType rightBlock = map_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);

			if (!map_->IsSolidBlockAt(leftIndex.xIndex, leftIndex.yIndex) &&
				!map_->IsSolidBlockAt(rightIndex.xIndex, rightIndex.yIndex)) {
				onGround_ = false;
			}

			if (IsHitGoalBlockTable(leftBlock) || IsHitGoalBlockTable(rightBlock)) {
				isGoal_ = true;
			}

			if (IsHitBlockDamageTable(leftBlock) || IsHitBlockDamageTable(rightBlock)) {
				TakeDamage();
			}
		}
	}
}


void Player::WallCollisionMove(CollisionMapInfo& collisionInfo)
{
	if (collisionInfo.hitWall) {
		// 壁に衝突したら移動量を調整
		velocity_.x *= (1.0f - status_.kEpsilon);
	}
}

void Player::PlayerCollisionMove(const CollisionMapInfo& info)
{
	// 移動量を反映
	// 判定を取った後の移動量を取得
	Vector3 position = playerModel_->GetTranslate();
	// 移動量を加算
	position += info.move;
	// 位置を設定
	playerModel_->SetTranslate(position);
}

void Player::CollisionMapInfoDirection(
	CollisionMapInfo& collisionInfo,
	CollisionType type,
	const std::array<Corner, 2>& checkCorners,
	const Vector3& offset,
	std::function<bool(const CollisionMapInfo&)> moveCondition)
{
	// 移動量が0なら処理しない
	if (!moveCondition(collisionInfo))return;
	// 現在の位置に移動量を加算した位置を取得
	Vector3 position = playerModel_->GetTranslate() + collisionInfo.move;

	// ２つのコーナー位置の当たり判定を取得
	std::array<Vector3, 2> points = {
		CornerPosition(position, checkCorners[0]) + offset,
		CornerPosition(position, checkCorners[1]) + offset
	};
	// 当たり判定があったか
	if (CheckCollisionPoints(points, static_cast<CollisionType>(type), collisionInfo)) {
		// 必要であればImGui表示する
		switch (type) {
		case CollisionType::kTop:

			break;
		case CollisionType::kBottom:
			//Logger::Log("hit floor");
			break;
		case CollisionType::kRight:
			//Logger::Log("hit right");
			break;
		case CollisionType::kLeft:
			//Logger::Log("hit left");
			break;
		}
	}
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner)
{
	Vector3 offsetTable[kNumCorners] = {
		{+status_.kWidth / 2.0f, -status_.kHeight / 2.0f, 0.0f},// 右下
		{-status_.kWidth / 2.0f, -status_.kHeight / 2.0f, 0.0f},// 左下
		{+status_.kWidth / 2.0f, +status_.kHeight / 2.0f, 0.0f},// 右上
		{-status_.kWidth / 2.0f, +status_.kHeight / 2.0f, 0.0f},// 左上
	};
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

bool Player::CheckCollisionPoints(const std::array<Vector3, 2>& posList, CollisionType type, CollisionMapInfo& collisionInfo)
{
	// 当たり判定フラグ
	bool isHit = false;

	for (const auto& pos : posList) {
		// 判定を行うマップチップのインデックスを取得
		IndexSet index = map_->GetMapChipIndexSetByPosition(pos);
		// マップチップの種類を取得
		BlockType chip = map_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		// 当たり判定を取るブロックの種類かどうか
		if (map_->IsSolidBlockAt(index.xIndex, index.yIndex)) {
			isHit = true;
		} 
		// ゴールの判定をとるブロックかどうか
		else if (IsHitGoalBlockTable(chip)) {
			isGoal_ = true;
		}
	}
	// 衝突していたら移動量を調整
	if (isHit) {
		// プレイヤーの現在位置を取得
		Vector3 position = playerModel_->GetTranslate();
		IndexSet index = map_->GetMapChipIndexSetByPosition(position);

		Rect rect = map_->GetRectByIndex(index.xIndex, index.yIndex);
		// 移動量を調整
		switch (type)
		{
		case CollisionType::kTop:
			collisionInfo.celling = true;
			collisionInfo.move.y = (std::max)(0.0f, rect.bottom - position.y - (status_.kHeight / 2.0f + status_.kEpsilon));
			break;
		case CollisionType::kBottom:
			collisionInfo.landing = true;
			collisionInfo.move.y = (std::min)(0.0f, rect.top - position.y + (status_.kHeight / 2.0f + status_.kEpsilon));
			break;
		case CollisionType::kRight:
			collisionInfo.hitWall = true;
			collisionInfo.move.x = (std::max)(0.0f, rect.left - position.x - (status_.kWidth / 2.0f + status_.kEpsilon));
			break;
		case CollisionType::kLeft:
			collisionInfo.hitWall = true;
			collisionInfo.move.x = (std::min)(0.0f, rect.right - position.x + (status_.kWidth / 2.0f + status_.kEpsilon));
			break;
		}
	}
	return isHit;
}

bool Player::IsHitBlockTable(BlockType type)
{
	// 判定を取るブロックの種類かどうか
	switch (type)
	{
	case BlockType::GrassBlock:
	case BlockType::SoilBlock:
	case BlockType::breakBlock:
	case BlockType::unBreakable:
	case BlockType::jumpBlock:
		return true;

	default:
		return false;
	}
}

bool Player::IsHitGoalBlockTable(BlockType type)
{
	// ゴールブロックかどうか
	switch (type) {
	case BlockType::kGoalUp:
	case BlockType::kGoalDown:
		return true;
	default:
		return false;
	}
}

bool Player::IsHitBlockBreakableTable(BlockType type)
{
	// 破壊可能ブロックかどうか
	switch (type) {
	case BlockType::breakBlock:
		return true;
	default:
		return false;
	}
}

bool Player::IsHitBlockDamageTable(BlockType type)
{
	// ダメージブロックかどうか
	// ダメージブロックに当たったらダメージ処理へ
	switch (type) {
	case BlockType::damageBlock:
		return true;
	}
	return false;
}

void Player::DebugPlayerReset()
{
	//// デバッグ用にリセット
	//velocity_ = {};
	//playerModel_->SetTranslate({ 1.5f,1.5f,0.0f });
	//playerModel_->SetRotate({ 0.0f,0.0f,0.0f });
	//isDead_ = false;
}


void Player::Finalize()
{
	// パーティクルエフェクトの解放
	if (moveEffect_) {
		moveEffect_->Stop();
		moveEffect_.reset();
	}
	if (dashSmokeEffect_) {
		dashSmokeEffect_->Stop();
		dashSmokeEffect_.reset();
	}
	if (dashStartEffect_) {
		dashStartEffect_->Stop();
		dashStartEffect_.reset();
	}
	if (stompEffect_) {
		stompEffect_->Stop();
		stompEffect_.reset();
	}
	if (jumpEffect_) {
		jumpEffect_->Stop();
		jumpEffect_.reset();
	}
	if (landEffect_) {
		landEffect_->Stop();
		landEffect_.reset();
	}
	if (jumpBlockArrowEffect_) {
		jumpBlockArrowEffect_->Stop();
		jumpBlockArrowEffect_.reset();
	}
}

void Player::ImGui()
{
#ifdef USE_IMGUI
	ImGui::Begin("Player Status");

	if (ImGui::BeginTabBar("Player Status Tab"))
	{
		// タブ１：パラメーター設定
		if (ImGui::BeginTabItem("Parameters"))
		{
			ImGui::Text("=== Player Parameters ===");
			ImGui::SliderFloat("Acceleration", &status_.kAcceleration, 0.01f, 0.2f);
			ImGui::SliderFloat("Attenuation", &status_.kAttenuation, 0.0f, 0.5f);
			ImGui::SliderFloat("Max Speed", &status_.kMaxSpeed, 0.05f, 0.5f);
			ImGui::SliderFloat("Dash Speed Scale", &status_.kDashSpeedScale, 1.0f, 5.0f);
			ImGui::SliderFloat("Gravity", &status_.kGravity, 0.01f, 0.2f);
			ImGui::SliderFloat("Max Fall Speed", &status_.kMaxFallSpeed, 0.1f, 2.0f);
			ImGui::SliderFloat("Jump Power", &status_.kJumpPower, 0.1f, 1.0f);
			ImGui::Text("HP: %d", status_.kHealth);

			// Playerのスタッツの変更
			Vector3 pos = playerModel_->GetTranslate();
			ImGui::DragFloat3("Player Position", &pos.x, 0.1f);
			playerModel_->SetTranslate(pos);
			Vector3 rot = playerModel_->GetRotate();
			ImGui::DragFloat3("Player Rotation", &rot.x, 0.1f);
			playerModel_->SetRotate(rot);
			ImGui::EndTabItem();
		}

		// タブ２：敵衝突・点滅関連
		if (ImGui::BeginTabItem("Hit Enemy")) {
			// プレイヤー用デバッグウィンドウ


			ImGui::Text("=== Enemy Collision / Flashing ===");

			// 敵ヒットフラグ
			ImGui::Checkbox("isEnemyHit_", &isEnemyHit_);

			// 今の表示状態
			ImGui::Checkbox("isVisible_", &isVisible_);

			// フレームカウンタ
			ImGui::Text("flashingFrameCount_: %d", flashingFrameCount_);

			// 最大点滅フレーム
			{
				int tmp = static_cast<int>(maxFlashingFlame_);
				ImGui::DragInt("maxFlashingFlame_", &tmp, 1, 0, 600);
				maxFlashingFlame_ = static_cast<uint32_t>(tmp);
			}

			// 点滅間隔フレーム
			{
				int tmp = static_cast<int>(flashingIntervalFrame_);
				ImGui::DragInt("flashingIntervalFrame_", &tmp, 1, 1, 120);
				flashingIntervalFrame_ = static_cast<uint32_t>(tmp);
			}

			// 余りの確認（デバッグ用）
			if (flashingIntervalFrame_ > 0) {
				int mod = flashingFrameCount_ % flashingIntervalFrame_;
				ImGui::Text("flashingFrameCount_ %% flashingIntervalFrame_ = %d", mod);
			}

			// 点滅中かどうかの判定表示
			bool isFlashing = (flashingFrameCount_ <= maxFlashingFlame_);
			ImGui::Text("isFlashing: %s", isFlashing ? "true" : "false");

			ImGui::EndTabItem();
		}



		// タブ３：プレイヤー情報表示
		if (ImGui::BeginTabItem("Player Info"))
		{
			ImGui::Text("=== Player Info ===");
			Vector3 pos = playerModel_->GetTranslate();
			Vector3 rotate = playerModel_->GetRotate();
			ImGui::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
			ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
			ImGui::Text("Rotate: (%.2f, %.2f, %.2f)", rotate.x, rotate.y, rotate.z);
			ImGui::Text("On Ground: %s", onGround_ ? "Yes" : "No");
			ImGui::Text("Is Dead: %s", isDead_ ? "Yes" : "No");
			ImGui::Text("Is Goal: %s", isGoal_ ? "Yes" : "No");
			ImGui::Text("Direction: %s",
				(direction_ == Direction::kRight) ? "Right" : "Left");
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();

	}

	ImGui::End();


#endif
}

bool Player::IsOnJumpBlock()
{
	if (!map_ || !playerModel_) { return false; }

	Vector3 position = playerModel_->GetTranslate();

	// 足元2点を取得
	Vector3 leftBottom = CornerPosition(position, kLeftBottom);
	Vector3 rightBottom = CornerPosition(position, kRightBottom);

	// 少し下を調べる
	Vector3 checkOffset = Vector3(0.0f, -status_.kEpsilon * 2.0f, 0.0f);

	IndexSet leftIndex = map_->GetMapChipIndexSetByPosition(leftBottom + checkOffset);
	IndexSet rightIndex = map_->GetMapChipIndexSetByPosition(rightBottom + checkOffset);

	BlockType leftBlock = map_->GetMapChipTypeByIndex(leftIndex.xIndex, leftIndex.yIndex);
	BlockType rightBlock = map_->GetMapChipTypeByIndex(rightIndex.xIndex, rightIndex.yIndex);

	return (leftBlock == BlockType::jumpBlock || rightBlock == BlockType::jumpBlock);
}