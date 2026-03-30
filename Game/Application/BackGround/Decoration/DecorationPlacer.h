#pragma once
#include <vector>
#include "DecorationSpawnSetting.h"
#include "DecorationInstanceData.h"
#include "MyMath.h"
#include <string>


/// <summary>
/// DecorationSpawnSettingを受けとる
/// モデルをランダムに選択しランダムな位置に生成して近すぎないようににする
/// </summary>
/// 
class DecorationPlacer
{
public:
	std::vector<DecorationInstanceData>CreateLayout(
		const DecorationSpawnSetting& setting);

private:
	bool IsFarEnough(const Vector3& candidate,
		const std::vector<DecorationInstanceData>& placedList,
		float minDistance)const;

	Vector3 RandomPosition(const Vector3& min, const Vector3& max)	const;
	Vector3 RandomVector3(const Vector3& min, const Vector3& max)	const;
	std::string RandomModelPath(const std::vector<std::string>& modelPaths)	const;
};

