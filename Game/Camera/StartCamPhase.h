#pragma once
#include "MyMath.h"
#include "Game/Camera/Camera.h"
#include <algorithm>
#include <memory>

class Camera;
class Sprite;
class StartCamPhase
{
public:
	// スタート演出
	enum class Phase {
		None,
		MoveToLeft,   // 左端へ移動（初期位置から）
		PanToRight,   // 左→右へパン
		Hold,         // 少し止める
		ReturnToStart // プレイヤー開始地点へ戻る
	};

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();
	/// <summary>
	/// UIの描画を行う
	/// </summary>
	void DrawUI();

	/// <summary>
	/// カメラとその配置情報のバインド
	/// </summary>
	/// <param name="camera">ゲームシーンのカメラを受け取る</param>
	/// <param name="cameraTransform">ゲームシーンの配置情報を受け取る</param>
	void Bind(Camera* camera, Transform* cameraTransform);
	/// <summary>
	/// スタート演出
	/// </summary>
	void Start();

	/// <summary>
	/// スタートカメラ演出の更新
	/// </summary>
	/// <param name="dt"></param>
	void Update(float dt);

	/// <summary>
	/// スタートカメラの演出スキップ
	/// </summary>
	void Skip();

	/// <summary>
	/// スタートカメラの演出が進行中か
	/// </summary>
	/// <returns>演出が進行中でなければPhase::Noneを返す</returns>
	bool IsRunning()const;



public: // Getter
	/// <summary>
	/// 現在の演出フェーズの取得
	/// </summary>
	/// <returns></returns>
	Phase GetCurrentPhase() const { return phase_; }


private: // イージング関数 / クラス内のみ
	static inline float EaseOutCubic(float t) {
		t = std::clamp(t, 0.0f, 1.0f);
		return 1.0f - std::pow(1.0f - t, 3.0f);
	}
	static inline float EaseOutBack(float t, float s = 1.70158f) {
		t = std::clamp(t, 0.0f, 1.0f);
		return 1.0f + (s + 1.0f) * std::pow(t - 1.0f, 3.0f) + s * std::pow(t - 1.0f, 2.0f);
	}
	static inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t) {
		return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t };
	}

	/// <summary>
	/// ImGui描画
	/// </summary>
	void DrawImgui();

	/// <summary>
	/// UIの点滅
	/// </summary>
	void SkipUIBlink(float dt);

private: // 演出に必要な変数
	// カメラの代入先ポインタ
	Camera* camera_ = nullptr;
	// カメラの配置情報の代入先ポインタ
	Transform* cameraTransform_;
	// 現在のフェーズ
	Phase phase_ = Phase::None;
	float timer_ = 0.0f;

	// 開始地点
	Vector3 startPos_ = {};

	/// 横パン演出
	// 目標位置
	Vector3 camTargetPos_ = { 8.0f,3.5f,-20.0f };
	// 左端へ行く時間
	float introMoveDur_ = 0.6f;
	// 左から右パン時間
	float introPanDur_ = 1.6f;
	// 右端での停止時間
	float introHoldDur_ = 1.0f;
	// 開始地点へ戻る時間
	float introReturnDur_ = 3.0f;

	// 左端と右端
	float introLeftX_ = 8.0f;
	float introRightX_ = 92.0f;
	// 背景が崩壊しない高さ
	float introFixedY_ = 3.5f;
	// 固定Z座標 / 通常距離
	float introFixedZ_ = -20.0f;

	// skip spaceのUI
	std::unique_ptr <Sprite> skipUI_ = nullptr;

	// α値の減衰速度
	float fadeSpeed_ = 0.02f;
	// スキップUIの折り返し値 / 最大
	Vector4 colorOn_ = { 1.0f,1.0f,1.0f,0.5f };
	// スキップUI折り返し値 / 最小
	Vector4 colorOff_ = { 1.0f,1.0f,1.0f,0.0f };

	// 点滅用にタイマー
	float blinkTimer_ = 0.8f;
	// 数値が大きければ大きいほど速い
	float blinkSpeed_ = 7.0f;

	// UIのテクスチャファイルパス
	const std::string textureFilePath_ = "resources/GamePlay/UI/Texture/skipUI.png";

};

