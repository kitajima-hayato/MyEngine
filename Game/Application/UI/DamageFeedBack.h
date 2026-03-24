#pragma once
#include "Game/Camera/Camera.h"
#include "Game/Application/Player/Player.h"
#include "engine/2d/Sprite.h"
class Player;
class Camera;
class DamageFeedBack
{

public:
	void Initialize();

	void Update(float dt);

	void Draw();

	void Bind(Player* player,Camera*camera,Transform*cameraTransform);

	bool IsActive()const;

	void Reset();

	void StartShake(float timeSec = -1.0f, float amp = -1.0f, bool withFlash = false);

public: // Getter / Setter
	void SetBaseCameraPos(const Vector3& basePos) { baseCamPos_ = basePos; }

private:
	void BeginHit();

	void UpdateFlash();

	void UpdateShake(float dt);

private:
	// 敵に衝突したときのシェイク時間
	float shakeTime_ = 0.5f;
	// シェイクの強さ
	float shakeAmp_ = 0.2f;
	// α値の減衰速度
	float fadeSpeed_ = 0.02f;

	Vector4 flashColorOn_ = { 1.0f,1.0f,1.0f,0.5f };
	Vector4 flashColorOff_ = { 1.0f,1.0f,1.0f,0.0f };

	// 画像パス
	const std::string texturePath_ = "resources/Effects/Hit/Textures/HitDamage.dds";

private:
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

