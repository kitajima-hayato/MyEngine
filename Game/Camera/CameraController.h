#pragma once
#include "engine/math/MyMath.h"
class CameraController
{

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	void Update(float deltaTime);

public:
	// 位置のセット
	void SetTargetPosition(const Vector3& targetPosition) {
		
		// 移動方向の判定
		float dx = targetPosition.x - targetPosition_.x;
		// 判定用の微小値
		const float epsilon = 0.01f;
		// 移動方向の設定
		if (dx > epsilon) {
			moveDirection = 1; // 右移動
		} else if (dx < -epsilon)
		{
			moveDirection = -1; // 左移動
		}
		// 現在のターゲット位置を更新
		targetPosition_ = targetPosition;
	}
	void SetCameraPosition(const Vector3& cameraPosition) { cameraPosition_ = cameraPosition; }
	// 位置の取得
	const Vector3& GetTargetPosition() const { return targetPosition_; }
	const Vector3& GetCameraPosition() const { return cameraPosition_; }
	// カメラの追従範囲の設定
	void SetFollowRange(float leftLimit, float rightLimit) { leftLimit_ = leftLimit; rightLimit_ = rightLimit; };

private:
	// 受け取る対象の位置
	Vector3 targetPosition_;

	// 受け取るカメラの位置
	Vector3 cameraPosition_;

	// デッドゾーン
	Vector2 deadZone_{ 2.0f, 1.0f };

	// 追従速度
	float followSpeed_ = 60.0f;

	// カメラの移動可能範囲
	float leftLimit_ = 8.0f;
	float rightLimit_ = 92.0f;

	// どちらの方向に移動しているか
	int32_t moveDirection = 0;
	// 進行方向に対するカメラのオフセット量
	float followOffsetX_ = 3.0f;
	// 現在のオフセット量
	float currentOffsetX_ = 0.0f;
	// オフセットの補間速度
	float offsetLerpSpeed_ = 2.0f;
};

