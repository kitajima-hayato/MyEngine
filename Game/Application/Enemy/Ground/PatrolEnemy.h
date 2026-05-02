#pragma once
#include "../EnemyBase/EnemyBase.h"

/// <summary>
/// 指定した範囲を左右に往復してプレイヤーを見つけたら追いかけるエネミー
/// </summary>
class PatrolEnemy : public EnemyBase
{
	public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;
	/// <summary>
	/// 移動処理
	/// </summary>
	void Move() override;
	/// <summary>
	/// 行動処理
	/// </summary>
	void Action() override;

	/// <summary>
	/// 位置情報の設定
	/// </summary>
	/// <param name="translate">変更したい位置情報の値</param>
	void SetTranslate(const Vector3& translate) override {
		EnemyBase::SetTranslate(translate);
		// 初期位置を設定する
		StartX_ = translate.x; 
	}

	void OnStomped() override {
		// プレイヤーに踏まれたときの処理
		stats.isAlive = false; 
		stats.health = 0;
	}

private:

	// ボディーのモデル
	std::unique_ptr<Object3D> bodyModel;
	// ボディーモデルのトランスフォーム
	Transform bodyTransform;

	// 初期位置 / 巡回の基準点
	float StartX_ = 0.0f;
	// 往復する範囲
	float moveRange_ = 3.0f;
	// 移動速度
	float moveSpeed_ = 0.05f;
	// 進行方向
	int direction_ = 1; // 1:右, -1:左 

};

