#pragma once
#include "EnemyBase.h"

/// <summary>
/// 落下攻撃をしてくるエネミーの基底クラス
/// </summary>
class Player; // プレイヤークラスの前方宣言
class DropEnemyBase : public EnemyBase
{
public:
	/// <summary>
	/// プレイヤーの情報をセットする関数 / 落下攻撃の条件でプレイヤーの位置を参照するため
	/// </summary>
	/// <param name="player">プレイヤーのポインタ</param>
	void SetPlayer(Player* player) { player_ = player; }

protected:

	enum class DropState
	{
		Waiting,	// 天井での待機状態
		Warning,	// 落下前の警告状態 / 演出用
		Falling,	// 落下中
		Cooldown,	// 落下地点で少し待機する
		Returning,	// 元の位置へ戻る
	};

	/// <summary>
	/// 落下攻撃の初期化
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 落下攻撃の描画処理 
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 落下攻撃の攻撃処理 
	/// </summary>
	void Action() override;

	/// <summary>
	///	 落下攻撃の位置設定 
	/// </summary>
	/// <param name="translate"></param>
	void SetTranslate(const Vector3& translate) override;

	/// <summary>
	/// 落下攻撃の共通処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 落下攻撃の動作処理 / ステート管理
	/// </summary>
	void Move() override;
	
	/// <summary>
	/// 落下攻撃開始の共通処理
	/// </summary>
	/// <returns>落下攻撃を開始するかどうか</returns>
	/// 継承先のクラスで各自の条件で開始できるようにするために純粋仮想関数とする
	virtual bool ShouldStartDrop() = 0;

	/// <summary>
	/// モデル名の取得 継承先で各自モデルを指定できるように
	/// </summary>
	virtual const char* GetModelName() const = 0;
private:

	/// <summary>
	/// 落下前の警告状態の更新処理 / 演出用の共通処理
	/// </summary>
	void UpdateWarning();

	/// <summary>
	/// 落下中の更新処理 / 落下の挙動を共通化
	/// </summary>
	void UpdateFalling();

	/// <summary>
	/// 元の位置へ戻る更新処理 / 戻る挙動を共通化
	/// </summary>
	void UpdateCooldown();

	/// <summary>
	/// 元の位置へ戻る更新処理 / 戻る挙動を共通化
	/// </summary>
	void UpdateReturning();

	/// <summary>
	/// 落下地点に到達したかどうかの判定 / 落下挙動を共通化
	/// </summary>
	/// <returns>落下地点に到達しているか</returns>
	bool HasReachedGround() const {
		// 落下地点に到達したかどうかの判定
		return stats.transform.translate.y <= targetY_;
	}

private:
	// 待機位置
	float startY_ = 0.0f; 
	// 落下速度
	float fallSpeed_ = 0.15f;
	// 戻る速度
	float returnSpeed_ = 0.08f;
	// 落下地点での待機時間
	float coolDownTime_ = 1.0f;
	// 落下地点での待機タイマー
	float coolDownTimer_ = 0.0f;

	// 仮の地面のY座標
	float dropDistance_ = 3.0f;
	float targetY_ = 0.0f;

	// 落下前の演出管理用のタイマー
	float warningTime_ = 0.25f;
	float warningTimer_ = 0.0f;

	// 落下前のぶるぶる震える演出の管理用の変数
	float startX_ = 0.0f;
	float warningShakeAmplitude_ = 0.05f;
	float warningShakeFrequency_ = 40.0f;
	
	// 落下を加速させるための変数
	float currentFallSpeed_ = 0.0f;
	float initialFallSpeed_ = 0.05f;
	float fallAcceleration_ = 0.015f;
	float maxFallSpeed_ = 0.35f;


	// 現在の状態
	DropState state_ = DropState::Waiting;
protected:
	// 追尾するプレイヤー
	Player* player_ = nullptr;
};

