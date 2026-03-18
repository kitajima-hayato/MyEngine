#include "FloatingEnemyBase.h"
#include <numbers>

void FloatingEnemyBase::UpdateFloating()
{
	// Y軸方向に浮遊する挙動を実装
	// ふり幅と速度を調整して、自然な浮遊感を出す
	const float amplitude = 2.0f;
	const float frequency = 0.01f;

	timer += frequency;
	// timerの値を元に、正弦波を使ってY軸方向のオフセットを計算
	float offsetY = std::sin(timer * radian * std::numbers::pi_v<float>) * amplitude;
	// baseYにオフセットを加えて、Y軸方向の位置を更新
	stats.transform.translate.y = baseY + offsetY;
}
