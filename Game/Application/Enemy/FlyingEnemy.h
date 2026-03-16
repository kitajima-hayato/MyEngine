#pragma once
#include "EnemyBase.h"
/// 基底クラス<BaseEnemy>を継承
class FlyingEnemy :
    public EnemyBase
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
	/// トランスフォームの設定。FlyingEnemyはY軸方向に浮遊するため、baseYを更新して高さの基準とする
    /// </summary>
    /// <param name="transform"></param>
    void SetTranslate(const Vector3& translate) override;

private:
    /// 元の高さ（初期位置Y）
    float baseY = 0.0f;
    float timer = 0.0f;


    


};

