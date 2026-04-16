#pragma once
#include "engine/3d/Object3D.h"
#include "BlockType.h"
#include "Game/Collision/Collider.h"
#include <memory>
class Block : public Collider
{
public: // 判定
	///
	///
	/// 
	void OnCollision(Collider* other) override;

	/// <summary>
	/// AABBを取得
	/// </summary>
	/// <returns>AABB</returns>
	AABB GetAABB() const override {
		const Vector3 half = transform.scale * 0.5f; // ← ここを *0.5f に
		return { transform.translate - half, transform.translate + half };
	}

	/// <summary>
	/// コライダーの種類を取得
	/// </summary>
	Type GetType() const override { return Type::Static; }

public:
	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(BlockType blockType, Vector3 position);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ブロックの生成
	/// </summary>
	static Block* CreateBlock(BlockType blockType, Vector3 position);

	/// <summary>
	/// トグル状態に応じてサイズの変更
	/// </summary>
	/// <param name="toggleState">現在のOnOff状態</param>
	void UpdateToggleVisual(bool toggleState);

public:	// Setter / Getter
	/// <summary>
	/// ブロックのSRTを取得
	/// </summary>
	/// <returns>ブロックのSRT</returns>
	const Transform& GetTransform() const { return transform; }

	/// <summary>
	/// ブロックの種類を取得
	/// </summary>
	/// <returns>ブロックの種類</returns>
	BlockType GetBlockType() const { return blockType; }

	/// <summary>
	/// ブロックのScaleをセット
	/// </summary>
	void SetScale(const Vector3& scale) { transform.scale = scale; }

	/// <summary>
	/// ブロックのRotateをセット
	/// </summary>
	void SetRotate(const Vector3& rotate) { transform.rotate = rotate; }

	/// <summary>
	/// ブロックのPositionをセット
	/// </summary>
	void SetTranslate(const Vector3& position) { transform.translate = position; }

	/// <summary>
	/// ブロックのSRTをセット
	/// </summary>
	void SetTransform(const Transform& transform) { this->transform = transform; }

	/// <summary>
	/// ブロックが壊れているかどうか
	/// </summary>
	bool GetAliveBlock() const { return isAlive_; }

	/// <summary>
	/// ブロックを壊す
	/// </summary>
	void SetBroken() { isAlive_ = false; }

	/// <summary>
	/// トグル状態に応じて判定が有効かどうか
	/// </summary>
	/// <param name="toggleState">現状</param>
	/// <returns>切り替える値</returns>
	bool IsSolid(bool toggleState)const;


private:

	/// ブロックモデル
	std::unique_ptr<Object3D> blockModel;
	/// ブロックの位置
	Transform transform;
	/// 表示するブロックの種類
	BlockType blockType;
	/// ブロックのコライダー
	AABB aabb;

	// 移動ブロック用変数
	float moveRange_ = 3.0f;     // 移動範囲
	float moveSpeed_ = 0.01f;    // 基本速度
	float moveTime_ = 0.0f;      // 時間カウンタ
	bool moveRight_ = true;      // 現在の移動方向（true:右 / false:左）

	// ブロックが壊れているかどうか
	bool isAlive_ = true;    

};

