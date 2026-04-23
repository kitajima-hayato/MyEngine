#pragma once
#include "ITransitionEffect.h"
#include "Sprite.h"
#include <memory>
#include <vector>
#include <MyMath.h>
/// <summary>
/// ボール転がりワイプ遷移演出
/// </summary>
class BallWipeTransition :public ITransitionEffect
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	BallWipeTransition(bool isStartTransition = false);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~BallWipeTransition() override = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	// ITransitionEffect インターフェース
	void Start() override;
	void Update(float deltaTime) override;
	void Draw() override;
	bool IsHalfway()override;
	bool IsComplete()override;
	float GetProgress()override;

private:
	/// <summary>
	/// ボールの更新
	/// </summary>
	void UpdateBalls(float deltaTime);

	/// <summary>
	/// スタンプの生成
	/// </summary>
	/// <param name="position">スタンプの位置</param>
	/// <param name="isUpper">上側のスタンプかどうか</param>
	void CreateStamp(const Vector2& position, bool isUpper);

	/// <summary>
	/// スタンプの更新
	/// </summary>
	void UpdateStamps(float deltaTime);

	/// <summary>
	/// ワイプマスクの更新
	/// </summary>
	void UpdateCover(float deltaTime);

	/// <summary>
	/// カラフルな色を取得
	/// </summary>
	Vector4 GetColorForStamp(int index);

	/// <summary>
	/// 全てのスタンプが完全に成長したか
	/// </summary>
	/// <returns>成長が済んでいるか</returns>
	bool AreAllStampsFullyGrown() const;

	// カラフルな色パレット（コロンワールド風）
	static const std::vector<Vector4> COLOR_PALETTE;
private:
	// モード設定
	// スタートモード豪華版か
	bool isStartMode_;
	// 演出の総時間
	float totalDuration_;
	// 現在の経過時間
	float currentTime_;

	// タイムライン
	// ボールの出現時間
	float ballAppearTime_;
	// 転がり開始時間
	float rollStartTime_;
	// 転がり終了時間
	float rollEndTime_;
	// カバー完了時間
	float coverCompleteTime_;
	// カバー解除開始時間
	float uncoverStartTime_;
	// ロード開始時間
	float loadStartTime_;

	// 演出のフェーズ
	enum class Phase {
		// ボール出現
		BallAppear,
		// 転がり中
		Rolling,
		// 画面を覆う
		Covering,
		// 完全に覆った状態（ロード中）
		Covered,
		// 覆いを解除する
		Uncovering,
		// 完了
		Complete
	};
	Phase currentPhase_;

	// ボール関連
	struct Ball {

		std::unique_ptr<Sprite>sprite;
		Vector2 position;
		// 回転角度
		float rotation;
		// ボールの半径
		float radius;
		// 上側のボールか
		bool isUpper;
		// 最後にスタンプを押したX座標
		float lastStampX;
		// 次にスタンプを生成するX座標
		float nextStampX;
	};
	std::vector<Ball> balls_;


	struct Stamp
	{
		// 画像インスタンス
		std::unique_ptr<Sprite>sprite;
		// 座標
		Vector2 position;
		// 現在のサイズ
		float currentSize;
		// 目標サイズ
		float targetSize;
		// 生成からの経過時間
		float age;
		// スタンプの色
		Vector4 color;
		// 上側の帯か
		bool isUpper;
		// スタンプのサイズ前フレーム
		float sizeBeforeShrink;
	};
	std::vector<Stamp>stamps_;

	// スタンプ生成の設定
	// スタンプを押す間隔
	float stampInterval_;
	// スタンプが成長する時間
	float stampGrowDuration_;
	// スタンプ縮小時間
	float stampShrinkDuration_;
	// 現在の色インデックス
	int stampColorIndex_;
	// スタンプの生成回数
	uint32_t stampSpawnCount_;

	// カバー（画面全体を覆うスプライト）
	//std::unique_ptr<Sprite> coverSprite_;
	float coverAlpha_ = 0.0f; // カバーの透明度（0.0f ~ 1.0f）

	// スプライトのファイルパス
	const std::string ballSpriteFilePath_ = "resources/Scenes/Transition/UI/Texture/ball.dds";
	const std::string trailSpriteFilePath_ = "resources/Scenes/Transition/UI/Texture/trail.dds";
	const std::string stampSpriteFilePath_ = "resources/Scenes/Transition/UI/Texture/stamp.dds";


	// 画面のサイズ
	static constexpr float SCREEN_WIDTH = 1280.0f;
	static constexpr float SCREEN_HEIGHT = 720.0f;

	// ボールと帯の設定
	static constexpr float BALL_RADIUS = 32.0f;
	static constexpr float UPPER_BALL_Y = 180.0f;  // 上側ボールのY座標
	static constexpr float LOWER_BALL_Y = 540.0f;  // 下側ボールのY座標
	static constexpr float STAMP_SIZE_MIN = 80.0f;
	static constexpr float STAMP_SIZE_MAX = 120.0f;
};

