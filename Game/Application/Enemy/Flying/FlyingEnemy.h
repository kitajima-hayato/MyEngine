#pragma once
#include "../EnemyBase/EnemyBase.h"
#include "../EnemyBase/FloatingEnemyBase.h"
/// 基底クラス<FloatingEnemyBase>を継承
class FlyingEnemy :
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
    /// <param name="translate"></param>
    void SetTranslate(const Vector3& translate)override;

private:
	std::unique_ptr<Object3D> bodyModel;
    // 脚パーツ用の揺れ設定
    float legSwingSpeed_ = 6.0f;
    float legSwingRot_ = 0.25f;
    float legSwingMoveX_ = 0.03f;
    float legSwingMoveY_ = 0.04f;
};

