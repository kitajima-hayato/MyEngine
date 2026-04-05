#pragma once
#include <memory>
#include "Game/Camera/Camera.h"
#include "Game/Application/Player/Player.h"
#include "engine/2d/Sprite.h"

// 前方宣言
class Player;
class Camera;
class Sprite;
/// <summary>
/// ダメージを受けたら行う処理
/// </summary>
class DamageFeedBack
{

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();
	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="dt">進行時間を基に演出</param>
	void Update(float dt);
	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();
	/// <summary>
	/// 必要情報の取得と設定
	/// </summary>
	void Bind(Player* player,Camera*camera,Transform*cameraTransform);
	/// <summary>
	/// ダメージを受けたら色を付ける
	/// </summary>
	bool IsActive()const;

	/// <summary>
	/// 初期状態に戻す
	/// </summary>
	void Reset();
	/// <summary>
	/// ダメージ時のカメラシェイク
	/// </summary>
	/// <param name="timeSec">進行度</param>
	/// <param name="amp"></param>
	/// <param name="withFlash"></param>
	void StartShake(float timeSec = -1.0f, float amp = -1.0f, bool withFlash = false);

public: // Getter / Setter
	/// <summary>
	/// カメラの基準値を受け取る
	/// </summary>
	/// <param name="basePos">カメラの基準値</param>
	void SetBaseCameraPos(const Vector3& basePos) { baseCamPos_ = basePos; }

private:
	/// <summary>
	/// 演出開始前処理
	/// </summary>
	void BeginHit();
	/// <summary>
	/// ダメージスプライトの減衰を管理
	/// </summary>
	void UpdateFlash();
	/// <summary>
	/// シェイクの更新
	/// </summary>
	void UpdateShake(float dt);

private:
	// 敵に衝突したときのシェイク時間
	float shakeTime_ = 0.5f;
	// シェイクの強さ
	float shakeAmp_ = 0.2f;
	// α値の減衰速度
	float fadeSpeed_ = 0.02f;
	// ヒット時にスプライトが見えるように
	Vector4 flashColorOn_ = { 1.0f,1.0f,1.0f,0.5f };
	// 見えないように色を指定 / 透明
	Vector4 flashColorOff_ = { 1.0f,1.0f,1.0f,0.0f };

	// 画像パス
	const std::string texturePath_ = 
		"resources/Effects/Hit/Textures/HitDamage.dds";

private:
	// 入れ物
	Player* player_ = nullptr;
	Camera* camera_ = nullptr;
	Transform* cameraTransform_ = nullptr;

	// ヒット検出
	bool wasHitPrev_ = false;

	// シェイク
	bool shakeActive_ = false;
	float shakeTimer_ = 0.0f;
	Vector3 baseCamPos_ = {};

	// ヒット時のスプライト
	std::unique_ptr<Sprite> hitSprite_ = nullptr;

};

