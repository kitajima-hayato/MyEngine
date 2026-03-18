#pragma once
#include "EnemyBase.h"

class FloatingEnemyBase : public EnemyBase
{
protected: // 派生クラスでアクセス可能なメンバ
    /// <summary>
    /// 浮遊の共通処理 / ふわふわと浮遊する挙動
    /// </summary>
    void UpdateFloating();

    // 元の高さ（初期位置Y）
    float baseY = 0.0f;
	// 経過時間
   	float timer = 0.0f;
	// ラジアンの定数（2πで1周）
    const float radian = 2.0f;
};

