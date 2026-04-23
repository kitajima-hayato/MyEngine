#include "BallWipeTransition.h"
#include <cmath>

// カラフルな色パレット（ポップで明るい色）
const std::vector<Vector4> BallWipeTransition::COLOR_PALETTE = {
	{ 1.0f, 0.3f, 0.3f, 1.0f },  // 赤
	{ 1.0f, 0.6f, 0.2f, 1.0f },  // オレンジ
	{ 1.0f, 0.9f, 0.2f, 1.0f },  // 黄色
	{ 0.4f, 0.9f, 0.4f, 1.0f },  // 緑
	{ 0.3f, 0.7f, 1.0f, 1.0f },  // 水色
	{ 0.6f, 0.4f, 1.0f, 1.0f },  // 紫
	{ 1.0f, 0.4f, 0.8f, 1.0f },  // ピンク
};

BallWipeTransition::BallWipeTransition(bool isStartTransition)
	: isStartMode_(isStartTransition)
	, totalDuration_(0.0f)
	, currentTime_(0.0f)
	, currentPhase_(Phase::BallAppear)
	, coverAlpha_(0.0f)
	, stampColorIndex_(0)
{
	// モードに応じてタイムラインを設定
	if (isStartMode_) {
		// 豪華版のタイムライン
		// 演出の総時間
		totalDuration_ = 1.5f;
		// ボールの出現時間
		ballAppearTime_ = 0.0f;
		// 転がり開始時間
		rollStartTime_ = 0.15f;
		// 転がり終了時間
		rollEndTime_ = 0.75f;
		// 画面を完全に覆う
		coverCompleteTime_ = 0.825f;
		// 覆いを解除開始
		uncoverStartTime_ = 0.975f;
		// ロード開始時間
		loadStartTime_ = 0.84f;
		// スタンプを押す間隔 / 小さければ密度が高まる
		stampInterval_ = 100.0f;
		// スタンプが成長する時間 / 小さければ成長が速くなり、密度が高まる
		stampGrowDuration_ = 1.0f;
		// スタンプが縮む時間 / 小さければ縮むのが速くなり、密度が高まる
		stampShrinkDuration_ = 0.45f;
	} else {
		// ノーマル版
		totalDuration_ = 1.0f;
		ballAppearTime_ = 0.0f;

		rollStartTime_ = 0.08f;
		rollEndTime_ = 0.40f;
		coverCompleteTime_ = 0.44f;
		uncoverStartTime_ = 0.7f;
		loadStartTime_ = 0.45f;

		// スタンプ密度
		stampInterval_ = 150.0f;       

		stampGrowDuration_ = 0.5f;
		stampShrinkDuration_ = 0.24f;
	}
}



void BallWipeTransition::Initialize()
{
	currentTime_ = 0.0f;
	currentPhase_ = Phase::BallAppear;
	coverAlpha_ = 0.0f;
	stampColorIndex_ = 0;
	stampSpawnCount_ = 0;

	// ボールの初期化(２つ)
	balls_.clear();
	balls_.resize(2);

	// 上側のボール
	balls_[0].sprite = std::make_unique<Sprite>();
	// 使用するスプライトの指定
	balls_[0].sprite->Initialize(ballSpriteFilePath_);
	balls_[0].radius = BALL_RADIUS;
	balls_[0].isUpper = true;
	balls_[0].position = { -BALL_RADIUS * 2,UPPER_BALL_Y };
	balls_[0].rotation = 0.0f;
	balls_[0].lastStampX = -BALL_RADIUS * 2;
	balls_[0].nextStampX = -BALL_RADIUS * 2;
	balls_[0].sprite->SetSize({ BALL_RADIUS * 2, BALL_RADIUS * 2 });
	// 中心を基準に回転させる
	balls_[0].sprite->SetAnchorPoint({ 0.5f,0.5f });

	// 下側のボール
	balls_[1].sprite = std::make_unique<Sprite>();
	// 使用するスプライトの指定
	balls_[1].sprite->Initialize(ballSpriteFilePath_);
	balls_[1].radius = BALL_RADIUS;
	balls_[1].isUpper = false;
	balls_[1].position = { -BALL_RADIUS * 2,LOWER_BALL_Y };
	balls_[1].rotation = 0.0f;
	balls_[1].lastStampX = -BALL_RADIUS * 2;
	balls_[1].nextStampX = -BALL_RADIUS * 2;
	balls_[1].sprite->SetSize({ BALL_RADIUS * 2, BALL_RADIUS * 2 });
	// 中心を基準に回転させる
	balls_[1].sprite->SetAnchorPoint({ 0.5f,0.5f });

	// 帯（道）の初期化
	stamps_.clear();



}

void BallWipeTransition::Start()
{
	Initialize();
}

void BallWipeTransition::Update(float deltaTime)
{
	// 遷移時間の更新
	currentTime_ += deltaTime;

	if (currentPhase_ == Phase::Covered && currentTime_ >= uncoverStartTime_) {
		for (auto& stamp : stamps_) {
			stamp.sizeBeforeShrink = stamp.currentSize; // ★現在のサイズを保存
		}
		currentPhase_ = Phase::Uncovering;
	} else if (currentPhase_ == Phase::BallAppear && currentTime_ >= rollStartTime_) {
		currentPhase_ = Phase::Rolling;
	} else if (currentPhase_ == Phase::Rolling && currentTime_ >= rollEndTime_) {
		currentPhase_ = Phase::Covering;
	} else if (currentPhase_ == Phase::Covering && currentTime_ >= coverCompleteTime_) {
		currentPhase_ = Phase::Covered;
	} else if (currentPhase_ == Phase::Uncovering && currentTime_ >= totalDuration_) {
		currentPhase_ = Phase::Complete;
	}

	// 各フェーズの更新
	switch (currentPhase_)
	{
	case BallWipeTransition::Phase::BallAppear:
	case BallWipeTransition::Phase::Rolling:
		UpdateBalls(deltaTime);
		UpdateStamps(deltaTime);
		break;
	case BallWipeTransition::Phase::Covering:
	case BallWipeTransition::Phase::Covered:
	case BallWipeTransition::Phase::Uncovering:
		UpdateStamps(deltaTime);
		break;
	}
}

void BallWipeTransition::UpdateBalls(float deltaTime)
{
	float rollDuration = rollEndTime_ - rollStartTime_;
	float rollTime = (std::max)(0.0f, currentTime_ - rollStartTime_);
	float rollProgress = (std::min)(1.0f, rollTime / rollDuration);

	float targetX = SCREEN_WIDTH + BALL_RADIUS * 2;

	for (auto& ball : balls_) {
		float startX = -BALL_RADIUS * 2;
		float newX = startX + (targetX - startX) * rollProgress;
		float distance = newX - ball.position.x;
		ball.position.x = newX;

		ball.rotation += distance / ball.radius;

		// ★等間隔でスタンプ生成（Xを固定）
		while (ball.position.x >= ball.nextStampX) {
			Vector2 stampPos = ball.position;
			stampPos.x = ball.nextStampX;          
			CreateStamp(stampPos, ball.isUpper);

			ball.lastStampX = ball.nextStampX;
			ball.nextStampX += stampInterval_;     
		}

		ball.sprite->SetPosition(ball.position);
		ball.sprite->SetRotation(ball.rotation);
		ball.sprite->Update();
	}
}


void BallWipeTransition::CreateStamp(const Vector2& position, bool /*isUpper*/)
{
	// スタンプの基本情報の設定
	Stamp stamp;
	stamp.sprite = std::make_unique<Sprite>();
	stamp.sprite->Initialize(stampSpriteFilePath_);
	stamp.position = position;
	stamp.currentSize = 0.0f;
	stamp.sizeBeforeShrink = 0.0f;

	// スタンプの大きさ
	stamp.targetSize = 1300.0f;

	stamp.age = 0.0f;

	// 3ラインを順番に回す（必ず下段も出る)
	const float lineY[3] = {
		// Top (120)
		SCREEN_HEIGHT * (1.0f / 6.0f), 
		// Middle (360)
		SCREEN_HEIGHT * (3.0f / 6.0f),
		// Bottom (600)
		SCREEN_HEIGHT * (5.0f / 6.0f)  
	};

	const uint32_t i = stampSpawnCount_++;
	stamp.position.y = lineY[i % 3];

	const float yJitterTable[3] = { -12.0f, 0.0f, 12.0f };
	stamp.position.y += yJitterTable[i % 3];


	// 色はパレット順
	stamp.color = GetColorForStamp(stampColorIndex_);
	stampColorIndex_ = (stampColorIndex_ + 1) % COLOR_PALETTE.size();

	stamp.sprite->SetAnchorPoint({ 0.5f,0.5f });
	stamps_.push_back(std::move(stamp));
}

void BallWipeTransition::UpdateStamps(float deltaTime)
{
	for (auto& stamp : stamps_) {

		// Coveredに入るまでは成長
		if (currentPhase_ == Phase::BallAppear ||
			currentPhase_ == Phase::Rolling ||
			currentPhase_ == Phase::Covering) {
			stamp.age += deltaTime;
		}

		if (currentPhase_ == Phase::Uncovering) {
			float shrinkTime = currentTime_ - uncoverStartTime_;
			float shrinkProgress = shrinkTime / stampShrinkDuration_;
			shrinkProgress = (std::min)(1.0f, shrinkProgress);

			float shrinkEase = shrinkProgress * shrinkProgress;
			stamp.currentSize = stamp.sizeBeforeShrink * (1.0f - shrinkEase);
		} else {
			float growProgress = stamp.age / stampGrowDuration_;
			growProgress = (std::min)(1.0f, growProgress);

			float eased = 1.0f - (1.0f - growProgress) * (1.0f - growProgress);
			stamp.currentSize = stamp.targetSize * eased;
		}

		stamp.sprite->SetPosition(stamp.position);
		stamp.sprite->SetSize({ stamp.currentSize, stamp.currentSize });
		stamp.sprite->SetColor(stamp.color);
		stamp.sprite->Update();
	}
}

void BallWipeTransition::UpdateCover(float deltaTime)
{

}

Vector4 BallWipeTransition::GetColorForStamp(int index)
{
	return COLOR_PALETTE[index % COLOR_PALETTE.size()];
}

bool BallWipeTransition::AreAllStampsFullyGrown() const
{
	// スタンプがまだ無いなら false
	if (stamps_.empty()) {
		return false;
	}

	for (const auto& stamp : stamps_) {
		if (stamp.age < stampGrowDuration_) {
			return false;
		}
	}
	return true;
}
void BallWipeTransition::Draw()
{
	// スタンプの描画
	for (const auto& stamp : stamps_) {
		if (stamp.currentSize > 0.0f) {
			stamp.sprite->Draw();
		}
	}
	// ボール
	if (currentPhase_ == Phase::Rolling) {

		for (const auto& ball : balls_) {
			ball.sprite->Draw();
		}
	}


}

bool BallWipeTransition::IsHalfway()
{
	// 画面が完全に覆われてスタンプがすべて成長しきっているか
	return currentTime_ >= loadStartTime_;
}

bool BallWipeTransition::IsComplete()
{
	return currentPhase_ == Phase::Complete;
}

float BallWipeTransition::GetProgress()
{
	return (std::min)(1.0f, currentTime_ / totalDuration_);
}


