#include "Player.h"
#include <algorithm>
#include <cmath>
#include "engine/Input/Input.h"
#include "Game/Particle/ParticlePresets.h"
#include "Game/Application/Enemy/EnemyBase/EnemyBase.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif

// 補助関数
namespace
{
	constexpr int kControllerNo = 0;
	constexpr float kStickMoveThreshold = 0.25f;
	constexpr float kStickDashThreshold = 0.75f;

	bool IsControllerRightHold(Input* input)
	{
		const StickState leftStick = input->GetLeftStickState(kControllerNo);

		return
			leftStick.x > kStickMoveThreshold ||
			input->PushButton(kControllerNo, ControllerButtonType::DPadRIGHT);
	}

	bool IsControllerLeftHold(Input* input)
	{
		const StickState leftStick = input->GetLeftStickState(kControllerNo);

		return
			leftStick.x < -kStickMoveThreshold ||
			input->PushButton(kControllerNo, ControllerButtonType::DPadLEFT);
	}

	bool IsControllerRightTrigger(Input* input)
	{
		const StickState leftStick = input->GetLeftStickState(kControllerNo);
		const StickState prevLeftStick = input->GetController()->GetLeftStickStatePrevious(kControllerNo);

		const bool stickFlickRight =
			prevLeftStick.x <= kStickMoveThreshold &&
			leftStick.x > kStickDashThreshold;

		return
			input->TriggerButton(kControllerNo, ControllerButtonType::DPadRIGHT) ||
			stickFlickRight;
	}

	bool IsControllerLeftTrigger(Input* input)
	{
		const StickState leftStick = input->GetLeftStickState(kControllerNo);
		const StickState prevLeftStick = input->GetController()->GetLeftStickStatePrevious(kControllerNo);

		const bool stickFlickLeft =
			prevLeftStick.x >= -kStickMoveThreshold &&
			leftStick.x < -kStickDashThreshold;

		return
			input->TriggerButton(kControllerNo, ControllerButtonType::DPadLEFT) ||
			stickFlickLeft;
	}

	bool IsControllerJumpTrigger(Input* input)
	{
		return input->PushButton(kControllerNo, ControllerButtonType::A);
	}
}

// AABB同士の衝突判定
static bool IntersectsAABB3D(const AABB& a, const AABB& b)
{
	return
		a.min.x <= b.max.x &&
		a.max.x >= b.min.x &&
		a.min.y <= b.max.y &&
		a.max.y >= b.min.y &&
		a.min.z <= b.max.z &&
		a.max.z >= b.min.z;
}

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
	if (!other) { return; }

	switch (other->GetType())
	{
	case Collider::Type::Enemy:
	{
		// 攻撃中なら、攻撃判定に敵が入っているか確認
		if (isAttack_) {
			const AABB attackAABB = GetAttackAABB();
			const AABB enemyAABB = other->GetAABB();

			if (IntersectsAABB3D(attackAABB, enemyAABB)) {
				// 既存の踏みつけ処理を再利用
				AttackEnemy(other);
				break;
			}
		}

		// すでにダメージ中なら何もしない
		if (isEnemyHit_) {
			break;
		}

		// 踏みつけ判定
		if (velocity_.y < 0.0f) {
			const AABB playerAABB = GetAABB();
			const AABB enemyAABB = other->GetAABB();

			const float playerBottom = playerAABB.min.y;
			const float enemyTop = enemyAABB.max.y;
			const float enemyCenter = (enemyAABB.min.y + enemyAABB.max.y) * 0.5f;
			const float playerCenter = (playerAABB.min.y + playerAABB.max.y) * 0.5f;

			if (playerCenter > enemyCenter && playerBottom < enemyTop + 0.2f) {
				StompEnemy(other);
				break;
			}
		}

		TakeDamage();
		break;
	}

	default:
		break;
	}
}

AABB Player::GetAttackAABB() const
{
	// 攻撃判定用のAABB
	AABB aabb{};

	// プレイヤー座標を取得する
	Vector3 pos = playerModel_->GetTranslate();

	// 向いている方向
	float dir = direction_ == Direction::kRight ? 1.0f : -1.0f;
	// 攻撃範囲を計算する
	float attackRange =
		status_.kAttackRange +
		status_.kAttackChargeRangeBonus * attackChargeRate_;

	// 攻撃範囲を元にAABBのサイズを設定する
	float halfWidth = attackRange * 0.5f;
	float halfHeight = status_.kAttackHeight * 0.5f;
	float halfDepth = status_.kAttackDepth;

	// 位置から攻撃判定の中心を計算する
	Vector3 center{
		pos.x + dir * (status_.kWidth * 0.5f + halfWidth),
		pos.y,
		pos.z
	};
	// 中心からAABBの最小座標と最大座標
	aabb.min = {
		center.x - halfWidth,
		center.y - halfHeight,
		center.z - halfDepth
	};
	// 最大座標
	aabb.max = {
		center.x + halfWidth,
		center.y + halfHeight,
		center.z + halfDepth
	};
	// AABBを返す
	return aabb;
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
	// 踏みつけのため反動ありのジャンプ
	DefeatEnemy(enemy, true);
}

void Player::DefeatEnemy(Collider* enemy, bool bouncePlayer)
{
	if (!enemy) {
		return;
	}

	EnemyBase* enemyBase = dynamic_cast<EnemyBase*>(enemy);
	if (!enemyBase) {
		return;
	}

	// 先に死亡位置を取っておく
	Vector3 enemyPos = enemyBase->GetTranslate();

	// 敵を倒す
	enemyBase->OnStomped();

	// 敵の死亡位置にパーティクルを出す
	EmitEnemyDefeatParticle(enemyPos);

	// 踏みつけの場合だけ反動ジャンプ
	if (bouncePlayer) {
		velocity_.y = status_.kStompJumpPower;
		onGround_ = false;
	}
}

void Player::EmitEnemyDefeatParticle(const Vector3 & position)
{
	if (!stompEffect_) {
		return;
	}

	stompEffect_->SetTranslate(position);
	stompEffect_->Play();
}

void Player::AttackEnemy(Collider * enemy)
{
	// 攻撃なので反動ジャンプなし
	DefeatEnemy(enemy, false);
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

	// 地面に少しだけめり込ませる
	p.y += 0.02f;

	return p;
}

void Player::UpdateInput()
{
	// 入力状態の初期化を行う
	inputState_ = {};

	// 入力が無ければ処理を行わない
	if (!controlEnabled_) { return; }

	// Input クラスのインスタンスを取得
	auto* input = Input::GetInstance();

	inputState_.moveRight =
		input->PushKey(DIK_D) ||
		IsControllerRightHold(input);

	inputState_.moveLeft =
		input->PushKey(DIK_A) ||
		IsControllerLeftHold(input);

	inputState_.triggerRight =
		input->TriggerKey(DIK_D) ||
		IsControllerRightTrigger(input);

	inputState_.triggerLeft =
		input->TriggerKey(DIK_A) ||
		IsControllerLeftTrigger(input);

	inputState_.jump =
		input->TriggerKey(DIK_SPACE) ||
		input->PushKey(DIK_W) ||
		IsControllerJumpTrigger(input);

	// アクションボタンの入力状態を更新 / 押した瞬間と放した瞬間 / コントローラーのボタンも同様に
	// Jキー / Xボタン
	inputState_.actionX.hold =
		input->PushKey(DIK_J) ||
		input->PushButton(kControllerNo, ControllerButtonType::X);

	inputState_.actionX.trigger =
		input->TriggerKey(DIK_J) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::X);

	inputState_.actionX.release =
		input->ReleaseKey(DIK_J) ||
		input->ReleaseButton(kControllerNo, ControllerButtonType::X);

	// Kキー / Bボタン
	inputState_.actionB.hold =
		input->PushKey(DIK_K) ||
		input->PushButton(kControllerNo, ControllerButtonType::B);

	inputState_.actionB.trigger =
		input->TriggerKey(DIK_K) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::B);

	inputState_.actionB.release =
		input->ReleaseKey(DIK_K) ||
		input->ReleaseButton(kControllerNo, ControllerButtonType::B);

	// Lキー / Yボタン
	inputState_.actionY.hold =
		input->PushKey(DIK_L) ||
		input->PushButton(kControllerNo, ControllerButtonType::Y);

	inputState_.actionY.trigger =
		input->TriggerKey(DIK_L) ||
		input->TriggerButton(kControllerNo, ControllerButtonType::Y);

	inputState_.actionY.release =
		input->ReleaseKey(DIK_L) ||
		input->ReleaseButton(kControllerNo, ControllerButtonType::Y);
}

void Player::UpdateChargeActions()
{
	// J / X：攻撃
	UpdateAttackChargeAction();

	// K / B：特殊ダッシュ
	UpdateSpecialDashChargeAction();

	// チャージ式のアクションボタンの処理を行うためのラムダ関数
	auto updateChargeButton =
		[this](ChargeButtonState& charge, const ButtonInputState& input, auto startAction)
		{
			if (input.trigger) {
				charge.charging = true;
				charge.frameCount = 0;
			}

			if (charge.charging && input.hold) {
				if (charge.frameCount < status_.kMaxChargeFrame) {
					++charge.frameCount;
				}
			}

			if (charge.charging && input.release) {
				const float chargeRate = CalcChargeRate(charge);

				startAction(chargeRate);

				charge.charging = false;
				charge.frameCount = 0;
			}
		};

	// L / Y：高ジャンプ
	updateChargeButton(
		highJumpCharge_,
		inputState_.actionY,
		[this](float chargeRate)
		{
			StartHighJump(chargeRate);
		}
	);
}

void Player::StartSpecialDash(float chargeRate)
{
	// 地面にいない場合は特殊ダッシュしない
	if (!onGround_) {
		return;
	}

	// 左右入力中は特殊ダッシュしない
	if (inputState_.moveRight || inputState_.moveLeft) {
		return;
	}

	// 通常ダッシュ中は特殊ダッシュしない
	if (isDash_) {
		return;
	}

	// すでに特殊ダッシュ中なら開始しない
	if (isSpecialDash_) {
		return;
	}

	// スペシャルダッシュ開始
	isSpecialDash_ = true;

	// スペシャルダッシュのフレーム数をリセット
	specialDashTimer_ = status_.kSpecialDashFrame;

	// ダッシュ方向
	specialDashDirection_ = direction_ == Direction::kRight ? 1 : -1;

	// チャージ率に応じたダッシュ速度を計算
	specialDashSpeed_ = std::lerp(
		status_.kSpecialDashMinSpeed,
		status_.kSpecialDashSpeed,
		chargeRate
	);

	// X方向の速度を設定
	velocity_.x = specialDashSpeed_ * specialDashDirection_;

	// 特殊ダッシュ発動時の後方爆発
	EmitSpecialDashBurst();
}

void Player::StartHighJump(float chargeRate)
{
	// ハイジャンプ開始
	// ジャンプができるのは地面にいるときだけ
	if (!onGround_) return;

	// ジャンプスイッチを切り替える
	map_->ToggleJumpSwitch();
	// ジャンプエフェクトを出す
	EmitJumpParticle();

	// チャージ率に応じたジャンプ力を計算
	float jumpPower = std::lerp(
		status_.kHighJumpPower,
		status_.kMaxHighJumpPower,
		chargeRate
	);

	// ジャンプブロックに触れている場合はさらにジャンプ力を上乗せ
	if (IsOnJumpBlock()) {
		jumpPower = (std::max)(jumpPower, status_.kJumpBlockPower);
	}

	// Y方向の速度を設定してジャンプ開始
	velocity_.y = jumpPower;
	onGround_ = false;
}

void Player::StartAttack(float chargeRate)
{
	if(isAttack_) {
		return;
	}

	if(isSpecialDash_) {
		return;
	}

	if (!CanChargeAttack()) {
		return;
	}

	isAttack_ = true;
	attackTimer_ = status_.kAttackFrame;
	attackChargeRate_ = std::clamp(chargeRate, 0.0f, 1.0f);

	// 攻撃開始のエフェクト
	EmitAttackReleaseEffect();
}

bool Player::TryBreakBlockBySpecialDash(const CollisionMapInfo& collisionInfo)
{
	if (!map_ || !playerModel_) {
		return false;
	}

	if (!isSpecialDash_) {
		return false;
	}

	if (specialDashDirection_ == 0) {
		return false;
	}

	Vector3 position = playerModel_->GetTranslate();

	// 縦方向は、衝突判定で使われた移動後の位置に寄せる
	position.y += collisionInfo.move.y;

	// 現在の横端ではなく、特殊ダッシュで進む先まで見る
	float forwardDistance = std::abs(specialDashSpeed_) + status_.kEpsilon;

	float checkX =
		position.x +
		static_cast<float>(specialDashDirection_) *
		(status_.kWidth * 0.5f + forwardDistance);

	// 横3点：上・中央・下
	std::array<Vector3, 3> checkPoints = {
		Vector3{ checkX, position.y + status_.kHeight * 0.35f, position.z },
		Vector3{ checkX, position.y,                         position.z },
		Vector3{ checkX, position.y - status_.kHeight * 0.35f, position.z }
	};

	bool brokeBlock = false;

	for (const Vector3& point : checkPoints) {
		IndexSet indexSet = map_->GetMapChipIndexSetByPosition(point);
		BlockType blockType = map_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (IsHitBlockBreakableTable(blockType)) {
			map_->BreakBlock(indexSet.xIndex, indexSet.yIndex);
			brokeBlock = true;
		}
	}

	return brokeBlock;
}

void Player::UpdateAttack()
{
	// 攻撃中でなければ何もしない
	if (!isAttack_) {
		return;
	}
	// 攻撃時間のカウントダウン
	if (attackTimer_ > 0) {
		--attackTimer_;
	}
	// 攻撃時間が0になったら攻撃終了
	if (attackTimer_ == 0) {
		isAttack_ = false;
		attackChargeRate_ = 0.0f;
	}
}

bool Player::CanChargeAttack() const
{
	// 操作不能中は攻撃不可
	if (!controlEnabled_) {
		return false;
	}

	// 地面にいないなら攻撃不可
	if (!onGround_) {
		return false;
	}

	// すでに攻撃中なら不可
	if (isAttack_) {
		return false;
	}

	// 特殊ダッシュ中は不可
	if (isSpecialDash_) {
		return false;
	}

	// 通常ダッシュ中は不可
	if (isDash_) {
		return false;
	}

	// 左右入力中は不可
	if (inputState_.moveRight || inputState_.moveLeft) {
		return false;
	}

	// 横速度がまだ残っているなら不可
	if (std::abs(velocity_.x) > status_.kAttackStopVelocityThreshold) {
		return false;
	}

	return true;
}

void Player::UpdateAttackChargeAction()
{
	// J / Xを押した瞬間
	if (inputState_.actionX.trigger) {
		// その場にいないならチャージ開始しない
		if (!CanChargeAttack()) {
			attackCharge_ = {};
			return;
		}

		attackCharge_.charging = true;
		attackCharge_.frameCount = 0;
	}

	// チャージ中でなければ何もしない
	if (!attackCharge_.charging) {
		return;
	}

	// チャージ中に動いたらキャンセル
	if (!CanChargeAttack()) {
		attackCharge_ = {};
		return;
	}

	// 押している間はチャージ
	if (inputState_.actionX.hold) {
		if (attackCharge_.frameCount < status_.kMaxChargeFrame) {
			++attackCharge_.frameCount;
		}
	}

	// 離した瞬間に攻撃
	if (inputState_.actionX.release) {
		float chargeRate = 0.0f;

		if (attackCharge_.frameCount >= status_.kMinChargeFrame) {
			chargeRate =
				static_cast<float>(attackCharge_.frameCount - status_.kMinChargeFrame) /
				static_cast<float>(status_.kMaxChargeFrame - status_.kMinChargeFrame);

			chargeRate = std::clamp(chargeRate, 0.0f, 1.0f);
		}

		StartAttack(chargeRate);

		attackCharge_ = {};
	}
}

void Player::UpdateSpecialDashChargeAction()
{
	// K / B を押した瞬間
	if (inputState_.actionB.trigger) {
		// 止まっていないならチャージ開始しない
		if (!CanChargeSpecialDash()) {
			specialDashCharge_ = {};
			return;
		}

		specialDashCharge_.charging = true;
		specialDashCharge_.frameCount = 0;
	}

	// チャージ中でなければ何もしない
	if (!specialDashCharge_.charging) {
		return;
	}

	// チャージ中に動いたらキャンセル
	if (!CanChargeSpecialDash()) {
		specialDashCharge_ = {};
		return;
	}

	// 押している間はチャージ
	if (inputState_.actionB.hold) {
		if (specialDashCharge_.frameCount < status_.kMaxChargeFrame) {
			++specialDashCharge_.frameCount;
		}
	}

	// 離した瞬間に特殊ダッシュ
	if (inputState_.actionB.release) {
		const float chargeRate = CalcChargeRate(specialDashCharge_);

		StartSpecialDash(chargeRate);

		specialDashCharge_ = {};
	}
}

float Player::CalcChargeRate(const ChargeButtonState& charge) const
{
	// チャージ中でなければ0
	if (charge.frameCount < status_.kMinChargeFrame) {
		return 0.0f;
	}

	// チャージ率を計算
	const uint32_t chargeFrameRange = status_.kMaxChargeFrame - status_.kMinChargeFrame;

	// チャージフレームの範囲が0の場合は、チャージ率を1.0f
	if(chargeFrameRange == 0) {
		return 1.0f;
	}

	// チャージ率を0.0f～1.0fの範囲で計算
	float chargeRate = 
		static_cast<float>(charge.frameCount - status_.kMinChargeFrame) /
		static_cast<float>(chargeFrameRange);

	// チャージ率を0.0f～1.0fの範囲に収める
	return std::clamp(chargeRate, 0.0f, 1.0f);
}

void Player::UpdateChargeEffects()
{
	// エフェクトがない場合は何もしない
	if (!playerModel_) {
		return;
	}

	// チャージダッシュのエフェクト更新
	UpdateSpecialDashChargeEffect();
	// ハイジャンプのエフェクト更新
	UpdateHighJumpChargeEffect();
	// チャージ攻撃のエフェクト更新
	UpdateAttackChargeEffect();
	
}



void Player::UpdateSpecialDashChargeEffect()
{
	if (!specialDashChargeEffect_ || !playerModel_) {
		return;
	}

	if (!specialDashCharge_.charging) {
		if (wasSpecialDashCharging_) {
			specialDashChargeEffect_->Pause();
		}

		wasSpecialDashCharging_ = false;
		return;
	}

	const float chargeRate = CalcChargeRate(specialDashCharge_);
	const float dir = direction_ == Direction::kRight ? 1.0f : -1.0f;

	Vector3 pos = GetFootParticlePos();

	// 進行方向とは逆側に出すと、タイヤが空回りしている感じになる
	pos.x -= dir * 0.35f;
	pos.y += 0.05f;
	pos.z -= 0.5f;


	const float emissionRate = std::lerp(35.0f, 90.0f, chargeRate);

	specialDashChargeEffect_->SetTranslate(pos);
	specialDashChargeEffect_->SetEmissionRate(emissionRate);

	if (!wasSpecialDashCharging_) {
		specialDashChargeEffect_->Play();
	}

	wasSpecialDashCharging_ = true;
}

void Player::UpdateHighJumpChargeEffect()
{
	if (!highJumpChargeEffect_ || !playerModel_) {
		return;
	}

	if (!highJumpCharge_.charging) {
		if (wasHighJumpCharging_) {
			highJumpChargeEffect_->Pause();
		}

		wasHighJumpCharging_ = false;
		return;
	}

	const float chargeRate = CalcChargeRate(highJumpCharge_);

	Vector3 pos = playerModel_->GetTranslate();
	pos.y += status_.kHeight * 0.4f;
	pos.z -= 0.5f;

	highJumpChargeEffect_->SetTranslate(pos);
	highJumpChargeEffect_->SetEmissionRate(std::lerp(8.0f, 24.0f, chargeRate));

	if (!wasHighJumpCharging_) {
		highJumpChargeEffect_->Play();
	}

	wasHighJumpCharging_ = true;
}

void Player::UpdateAttackChargeEffect()
{
	if (!attackChargeEffect_ || !playerModel_) {
		return;
	}

	if (!attackCharge_.charging) {
		if (wasAttackCharging_) {
			attackChargeEffect_->Pause();

			Vector3 rot = playerModel_->GetRotate();
			rot.x = 0.0f;
			playerModel_->SetRotate(rot);
		}

		wasAttackCharging_ = false;
		attackChargeVisualAngle_ = 0.0f;
		return;
	}

	const float chargeRate = CalcChargeRate(attackCharge_);
	const float dir = direction_ == Direction::kRight ? 1.0f : -1.0f;

	Vector3 pos = playerModel_->GetTranslate();

	// 少し手前に出して見やすくする
	pos.z -= 0.5f;

	attackChargeEffect_->SetTranslate(pos);
	attackChargeEffect_->SetEmissionRate(std::lerp(25.0f, 80.0f, chargeRate));

	const float spinSpeed = std::lerp(0.25f, 0.75f, chargeRate);
	attackChargeVisualAngle_ += spinSpeed;

	Vector3 rot = playerModel_->GetRotate();

	// X軸高速回転 + Y軸にもひねり
	rot.x = attackChargeVisualAngle_;
	rot.y += dir * spinSpeed * 0.6f;

	playerModel_->SetRotate(rot);

	if (!wasAttackCharging_) {
		attackChargeEffect_->Play();
	}

	wasAttackCharging_ = true;
}

void Player::EmitSpecialDashBurst()
{
	if (!specialDashBurstEffect_ || !playerModel_) {
		return;
	}

	Vector3 pos = playerModel_->GetTranslate();

	// 特殊ダッシュの進行方向
	const float dir = static_cast<float>(specialDashDirection_);

	// 進行方向とは逆側に爆発を置く
	pos.x -= dir * 0.45f;

	// 足元寄り
	pos.y -= status_.kHeight * 0.25f;

	// 少し手前に出して見やすくする
	pos.z -= 0.5f;

	specialDashBurstEffect_->SetTranslate(pos);
	specialDashBurstEffect_->Play();
}


void Player::EmitAttackReleaseEffect()
{
	if (!attackReleaseEffect_ || !playerModel_) {
		return;
	}

	const float dir = direction_ == Direction::kRight ? 1.0f : -1.0f;

	Vector3 pos = playerModel_->GetTranslate();

	const float attackRange =
		status_.kAttackRange +
		status_.kAttackChargeRangeBonus * attackChargeRate_;

	// プレイヤー前方、攻撃判定の始点〜中央あたり
	pos.x += dir * (status_.kWidth * 0.5f + attackRange * 0.25f);
	pos.y += 0.05f;
	pos.z -= 0.5f;

	if (direction_ == Direction::kRight) {
		attackReleaseEffect_->SetEffectType(
			ParticleManager::EffectType::AttackReleaseEnergyRight
		);
	} else {
		attackReleaseEffect_->SetEffectType(
			ParticleManager::EffectType::AttackReleaseEnergyLeft
		);
	}

	attackReleaseEffect_->SetTranslate(pos);

	// チャージ率で強さを変える
	// 弧を見せるため、粒子数を少し多めにする
	attackReleaseEffect_->SetEmissionRate(
		std::lerp(160.0f, 260.0f, attackChargeRate_)
	);

	attackReleaseEffect_->SetBurstCount(
		static_cast<uint32_t>(std::lerp(24.0f, 46.0f, attackChargeRate_))
	);

	attackReleaseEffect_->Play();
}


bool Player::OnAttackCollision(Collider* other)
{
	// 攻撃中でなければ攻撃判定はない
	if (!other) {
		return false;
	}
	// 攻撃中でなければ攻撃判定はない
	if (!isAttack_) {
		return false;
	}
	// 攻撃判定は敵にしかない
	if (other->GetType() != Collider::Type::Enemy) {
		return false;
	}

	// 攻撃で敵を倒す
	AttackEnemy(other);

	return true;
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
	isDying_ = false;
	deathDemoFinished_ = false;
	onGround_ = true;

	// 攻撃状態のリセット
	isAttack_ = false;
	attackCharge_ = {};
	attackTimer_ = 0;
	attackChargeRate_ = 0.0f;

	// ハイジャンプ状態のリセット
	highJumpCharge_ = {};

	// 特殊ダッシュ状態のリセット
	specialDashCharge_ = {};
	isSpecialDash_ = false;
	specialDashTimer_ = 0;
	specialDashDirection_ = 0;
	specialDashSpeed_ = 0.0f;

	// チャージエフェクトの解放
	if (attackChargeEffect_) attackChargeEffect_->Pause();
	if (specialDashChargeEffect_) specialDashChargeEffect_->Pause();
	if (highJumpChargeEffect_) highJumpChargeEffect_->Pause();
	if (specialDashBurstEffect_) specialDashBurstEffect_->Pause();
	if (attackReleaseEffect_) attackReleaseEffect_->Pause();
	// チャージエフェクトの解放
	wasAttackCharging_ = false;
	wasSpecialDashCharging_ = false;
	wasHighJumpCharging_ = false;
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

bool Player::CanChargeSpecialDash() const
{
	// 操作不能中は不可
	if (!controlEnabled_) {
		return false;
	}

	// 地面にいないなら不可
	if (!onGround_) {
		return false;
	}

	// 左右入力中は不可
	if (inputState_.moveRight || inputState_.moveLeft) {
		return false;
	}

	// 通常ダッシュ中は不可
	if (isDash_) {
		return false;
	}

	// すでに特殊ダッシュ中なら不可
	if (isSpecialDash_) {
		return false;
	}

	// 横速度がまだ残っているなら不可
	if (std::abs(velocity_.x) > status_.kAttackStopVelocityThreshold) {
		return false;
	}

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

	// ダッシュエフェクトの更新
	const bool movingRight = inputState_.moveRight && !inputState_.moveLeft;
	const bool movingLeft = inputState_.moveLeft && !inputState_.moveRight;
	// ダッシュが有効で、かつダッシュ方向に移動しているか
	const bool dashActiveRight = isDash_ && (dashDirection_ == +1) && movingRight;
	const bool dashActiveLeft = isDash_ && (dashDirection_ == -1) && movingLeft;
	const bool dashActive = dashActiveRight || dashActiveLeft;
	// ダッシュエフェクトを表示できる条件 / ダッシュ中かつ地面にいる
	const bool canShowEffect = dashActive && onGround_;

	Vector3 playerPos = playerModel_->GetTranslate();

	if (!wasDashing_ && canShowEffect) {
		dashStartEffect_->SetTranslate(playerPos);
		dashStartEffect_->Play();
	}

	if (canShowEffect) {
		Vector3 smokePos = playerPos;
		smokePos.y -= particleSpawnPosOffset_;

		if (dashDirection_ == +1) {
			smokePos.x -= particleSpawnPosOffset_;
		} else if (dashDirection_ == -1) {
			smokePos.x += particleSpawnPosOffset_;
		}

		dashSmokeEffect_->SetTranslate(smokePos);

		if (!wasDashing_) {
			dashSmokeEffect_->Play();
		}
	} else {
		dashSmokeEffect_->Stop();
	}

	wasDashing_ = canShowEffect;
}

void Player::UpdateMoveEffect()
{
	if (!moveEffect_ || !playerModel_) { return; }

	const bool rightHold = inputState_.moveRight;
	const bool leftHold = inputState_.moveLeft;

	const bool movingRight = rightHold && !leftHold;
	const bool movingLeft = leftHold && !rightHold;
	const bool moving = movingRight || movingLeft;

	const bool canShowEffect = controlEnabled_ && onGround_ && moving && !isDash_;

	Vector3 playerPos = playerModel_->GetTranslate();

	if (canShowEffect) {
		Vector3 smokePos = playerPos;
		smokePos.y -= particleSpawnPosOffset_;

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
	moveEffect_->SetColor(Vector4(1.0f, 1.0f, 1.0f, 1.0f));


	// ダッシュエフェクトの初期化
	// 足元の煙エフェクト(ダッシュ)
	dashSmokeEffect_ = ParticlePresets::CreateSmoke(position);
	// 初期は停止
	dashSmokeEffect_->Pause();
	dashSmokeEffect_->SetEmissionRate(15.0f);
	dashSmokeEffect_->SetLoop(true);
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


	// 攻撃チャージエフェクト
	attackChargeEffect_ = ParticlePresets::CreateAttackChargeEnergy(position);
	attackChargeEffect_->Pause();

	// 攻撃発動時の放出エフェクト
	attackReleaseEffect_ = ParticlePresets::CreateAttackReleaseEnergy(position);
	attackReleaseEffect_->Pause();

	// 特殊ダッシュチャージエフェクト
	specialDashChargeEffect_ = ParticlePresets::CreateFireSparkCharge(position);
	specialDashChargeEffect_->Pause();
	specialDashChargeEffect_->SetLoop(true);
	specialDashChargeEffect_->SetEmissionRate(25.0f);
	specialDashChargeEffect_->SetColor(Vector4(1.0f, 0.55f, 0.15f, 1.0f));

	specialDashBurstEffect_ = ParticlePresets::CreateSpecialDashBurst(position);
	specialDashBurstEffect_->Pause();

	// 高ジャンプチャージエフェクト
	highJumpChargeEffect_ = ParticlePresets::CreateJumpBlockArrow(position);
	highJumpChargeEffect_->Pause();
	highJumpChargeEffect_->SetLoop(true);
	highJumpChargeEffect_->SetEmissionRate(8.0f);
	highJumpChargeEffect_->SetColor(Vector4(0.5f, 0.8f, 1.0f, 1.0f));

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
	// 入力の更新
	UpdateInput();
	// プレイヤーの挙動更新
	UpdateBehavior();
	// 移動エフェクトの更新
	UpdateMoveEffect();
	// ダッシュエフェクトの更新
	UpdateDashEffect();
	// プレイヤーの回転
	PlayerTurn();
	// チャージエフェクトの更新
	UpdateChargeEffects();
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


	// チャージ類のエフェクト更新
	if (attackChargeEffect_) attackChargeEffect_->Update();
	if (specialDashChargeEffect_) specialDashChargeEffect_->Update();
	if (highJumpChargeEffect_) highJumpChargeEffect_->Update();

	if (specialDashBurstEffect_) specialDashBurstEffect_->Update();
	if (attackReleaseEffect_) attackReleaseEffect_->Update();
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
	// 攻撃処理
	UpdateAttack();
	// チャージ項目の更新
	UpdateChargeActions();

	// 当たり判定処理
	CollisionMapInfo collisionInfo;

	// 地面にいなかったらリセット
	if (playerModel_->GetTranslate().y < deathHeight_ && !onGround_)
	{
		// 死亡フラグ
		status_.kHealth -= 1;
		isDead_ = true;
		isDeathByFalling_ = true;
		return;
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
		// 走り続け防止
		velocity_.x *= (1.0f - status_.kAttenuation);

		// ダッシュ状態も解除
		isDash_ = false;
		dashDirection_ = 0;
		rightTapTimer_ = 0;
		leftTapTimer_ = 0;

		return;
	}

	// スペシャルダッシュ中は通常の移動処理を行わない
	if (isSpecialDash_) {
		// チャージ率で決まった速度を維持
		velocity_.x = specialDashSpeed_ * specialDashDirection_;

		// ダッシュ中も転がっているように回転させる
		Vector3 rotate = playerModel_->GetRotate();

		// 通常移動と同じ向きで回転
		// 倍率を掛けることで、特殊ダッシュらしく速く転がる
		rotate.z -= velocity_.x * kSpecialDashRollScale;

		playerModel_->SetRotate(rotate);

		// ダッシュ時間のカウントダウン
		if (specialDashTimer_ > 0) {
			--specialDashTimer_;
		} else {
			isSpecialDash_ = false;
			specialDashDirection_ = 0;
			specialDashSpeed_ = 0.0f;
		}

		// 通常の移動処理は行わない
		return;
	}


	//==================================================
	// 入力状態
	// キーボード + コントローラー
	//==================================================

	const bool rightHold = inputState_.moveRight;
	const bool leftHold = inputState_.moveLeft;

	const bool rightTrig = inputState_.triggerRight;
	const bool leftTrig = inputState_.triggerLeft;

	//==================================================
	// ダッシュ判定用タイマー更新
	//==================================================

	if (rightTapTimer_ > 0) { --rightTapTimer_; }
	if (leftTapTimer_ > 0) { --leftTapTimer_; }

	// 右ダブルタップ判定
	if (rightTrig) {
		if (rightTapTimer_ > 0) {
			isDash_ = true;
			dashDirection_ = +1;
		}

		rightTapTimer_ = status_.kDashDoubleTapFrame;
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

	//==================================================
	// ダッシュ継続／解除判定
	//==================================================

	const bool movingRight = rightHold && !leftHold;
	const bool movingLeft = leftHold && !rightHold;

	if (!movingRight && !movingLeft) {
		isDash_ = false;
		dashDirection_ = 0;
	} else if (isDash_) {
		if ((dashDirection_ == +1 && movingLeft) ||
			(dashDirection_ == -1 && movingRight)) {
			isDash_ = false;
			dashDirection_ = 0;
		}
	}

	//==================================================
	// 実際の速度更新
	//==================================================

	Vector3 acceleration{};

	// ダッシュが有効で、かつダッシュ方向に移動しているか
	const bool dashActiveRight = isDash_ && (dashDirection_ == +1) && movingRight;
	const bool dashActiveLeft = isDash_ && (dashDirection_ == -1) && movingLeft;
	const bool dashActive = dashActiveRight || dashActiveLeft;

	// ダッシュ中は加速度と最大速度が上がる
	float dashScale = dashActive ? status_.kDashSpeedScale : 1.0f;
	float maxSpeed = status_.kMaxSpeed * dashScale;

	// 移動入力に応じた加速度の計算
	if (movingRight) {
		if (velocity_.x < 0.0f) {
			velocity_.x *= (1.0f - status_.kAttenuation);
		}

		acceleration.x += status_.kAcceleration * dashScale;
		direction_ = Direction::kRight;
	} else if (movingLeft) {
		if (velocity_.x > 0.0f) {
			velocity_.x *= (1.0f - status_.kAttenuation);
		}

		acceleration.x -= status_.kAcceleration * dashScale;
		direction_ = Direction::kLeft;
	}
	// 移動入力がない場合は速度を減衰させる
	else {
		velocity_.x *= (1.0f - status_.kAttenuation);
	}

	// 加速度反映
	velocity_.x += acceleration.x;

	// 速度に応じて回転
	Vector3 rotate = playerModel_->GetRotate();
	rotate -= {0.0f, 0.0f, velocity_.x};

	if ((-playerTurnAround_ >= rotate.z && movingRight) ||
		(playerTurnAround_ <= rotate.z && movingLeft)) {
		rotate.z = 0.0f;
	}

	// 回転の設定
	playerModel_->SetRotate(rotate);

	// 最大速度制限
	velocity_.x = std::clamp(velocity_.x, -maxSpeed, maxSpeed);
}


void Player::Jump()
{
	// 操作ロック中：ジャンプ入力は無視
	if (!controlEnabled_) {
		// 重力だけは通常通り掛ける
		if (!onGround_) {
			velocity_.y += -status_.kGravity;
			velocity_.y = (std::max)(velocity_.y, -status_.kMaxFallSpeed);
		}
		return;
	}

	const bool jumpTrigger = inputState_.jump;

	// 地面にいる場合
	if (onGround_) {
		if (jumpTrigger) {
			// ジャンプスイッチの切り替え
			map_->ToggleJumpSwitch();

			// ジャンプの開始エフェクト
			EmitJumpParticle();

			// ジャンプブロックの上にいたら高く跳ねる
			if (IsOnJumpBlock()) {
				velocity_.y = status_.kJumpBlockPower;
			} else {
				velocity_.y = status_.kJumpPower;
			}

			// 空中状態にする
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
		if (isSpecialDash_) {
			// 特殊ダッシュで破壊可能ブロックを壊せたか
			bool brokeBlock = TryBreakBlockBySpecialDash(collisionInfo);

			// 特殊ダッシュは壁に衝突したら終了
			isSpecialDash_ = false;
			specialDashDirection_ = 0;
			specialDashSpeed_ = 0.0f;

			// 特殊ダッシュのキャンセル
			if (!brokeBlock) {
				velocity_.x = 0.0f;
				collisionInfo.move.x = 0.0f;
				return;
			}
			// 壊せないブロックに衝突
			velocity_.x = 0.0f;
			collisionInfo.move.x = 0.0f;
			return;
		}
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

	// チャージエフェクト類の解放
	if (attackChargeEffect_) {
		attackChargeEffect_->Stop();
		attackChargeEffect_.reset();
	}

	if (specialDashChargeEffect_) {
		specialDashChargeEffect_->Stop();
		specialDashChargeEffect_.reset();
	}

	if (highJumpChargeEffect_) {
		highJumpChargeEffect_->Stop();
		highJumpChargeEffect_.reset();
	}
	if (specialDashBurstEffect_) {
		specialDashBurstEffect_->Stop();
		specialDashBurstEffect_.reset();
	}
	if (attackReleaseEffect_) {
		attackReleaseEffect_->Stop();
		attackReleaseEffect_.reset();
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