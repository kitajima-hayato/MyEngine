#pragma once
#include "../EnemyBase/FloatingEnemyBase.h"
class SideMoveFlyingEnemy :
    public FloatingEnemyBase
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
    /// 動作処理
    /// </summary>
    void Move() override;

    /// <summary>
    /// 攻撃処理
    /// </summary>
	void Action() override;

    /// <summary>
    /// 位置情報の設定
    /// </summary>
    /// <param name="translate">設定する位置情報</param>
    void SetTranslate(const Vector3& translate)override;

	/// <summary>
	/// 飛行しながら左右に移動する処理
	/// </summary>
	void SideMove();

private:
	// 画面外としての左端の座標
	float leftBorder_ = -10.0f;

};

