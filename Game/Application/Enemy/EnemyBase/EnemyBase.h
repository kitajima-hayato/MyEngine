#pragma once
#include "engine/math/MyMath.h"
#include "engine/3d/Object3D.h"
#include "Game/Collision/Collider.h"
#include "Game/Application/Map/IMapCollisionQuery.h"

/// <summary>
/// エネミーの基本構造
/// 継承元として使用する
/// </summary>

/// エネミーのステータス
struct Stats {
	/// 座標
	Transform transform;
	/// 生存フラグ
	bool isAlive = true;
	/// 速度
	Vector3 velocity;
	/// 体力
	uint32_t health;
	/// 攻撃力
	int32_t attackPower;
};

class EnemyBase : public Collider
{

public:
	/// <summary>
	/// Enemy クラスの仮想デストラクタ。派生クラスのオブジェクトが正しく破棄されることを保証
	/// </summary>
	virtual ~EnemyBase() = default;

	/// <summary>
	/// エネミーの初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// エネミーの更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// エネミーの描画処理
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// エネミーの動作処理
	/// </summary>
	virtual void Move() = 0;

	/// <summary>
	/// エネミーの攻撃処理
	/// </summary>
	virtual void Action() = 0;

	virtual void OnStomped() { stats.isAlive = false; }

	/// Collider インターフェースの実装
	Type GetType()const override { return Type::Enemy; }

	/// AABBの取得
	AABB GetAABB() const override {
		const auto& t = stats.transform;
		return { t.translate - t.scale * 0.5f,
				t.translate + t.scale * 0.5f };
	}

	/// <summary>
	/// 衝突時の処理
	/// </summary>
	/// <param name="other">どれと判定をとるか</param>
	void OnCollision(Collider* other) override {
		// 衝突時の処理を実装
		if (other->GetType() == Type::Player) {
			if (stats.health <= 0) {
				stats.isAlive = false; // エネミーが死亡
			}
		}
	}

	/// エネミーの生存フラグ設定/取得
	bool IsAlive() const { return stats.isAlive; }

	/// エネミーのトランスフォーム設定/取得
	virtual void SetTransform(const Transform& transform) { stats.transform = transform; }
	virtual Transform GetTransform() const { return stats.transform; }
	/// エネミーのSRT設定/取得
	virtual void SetScale(const Vector3& scale) { stats.transform.scale = scale; }
	virtual Vector3 GetScale() const { return stats.transform.scale; }
	virtual void SetRotate(const Vector3& rotate) { stats.transform.rotate = rotate; }
	virtual Vector3 GetRotate() const { return stats.transform.rotate; }
	virtual void SetTranslate(const Vector3& translate) { stats.transform.translate = translate; }
	virtual Vector3 GetTranslate() const { return stats.transform.translate; }
	virtual void AddTranslate(const Vector3& offset) { stats.transform.translate += offset; }
	/// エネミーのvelocity設定/取得
	virtual void SetVelocity(const Vector3& velocity) { stats.velocity = velocity; }
	virtual Vector3 GetVelocity() const { return stats.velocity; }


	/// <summary>
	/// マップ衝突クエリーのセット
	/// </summary>
	/// <param name="mapQuery">マップクエリー</param>
	void SetMapQuery(IMapCollisionQuery* mapQuery) { this->mapQuery = mapQuery; }

protected:
	/// エネミーのステータス
	Stats stats;
	/// エネミーの3Dオブジェクト
	std::unique_ptr<Object3D> model;
	/// マップ衝突クエリー
	IMapCollisionQuery* mapQuery = nullptr;
};

