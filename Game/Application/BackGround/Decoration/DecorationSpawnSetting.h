#pragma once
#include <vector>
#include <string>
#include "MyMath.h"
/// <summary>
///	発生の設定項目
/// </summary>
struct DecorationSpawnSetting {
	// 使用するモデルのファイルパス達
	std::vector<std::string> modelPaths;
	// 生成するオブジェクトの範囲 / 最小 最大
	Vector3 areaMin{ 0.0f,0.0f,0.0f };
	Vector3 areaMax{ 0.0f,0.0f,0.0f };
	// 生成する数
	uint32_t objectCount = 0;
	// 生成する間隔の最低値 / 絶対に空ける距離
	float spawnRangeInterval = 1.0f;
	// 大きさのランダム幅 / 最小 最大
	Vector3 minScale{ 1.0f,1.0f,1.0f };
	Vector3 maxScale{ 1.0f,1.0f,1.0f };
	// 回転のランダム幅 / 最小 最大
	Vector3 minRotation{ 0.0f,0.0f,0.0f };
	Vector3 maxRotation{ 0.0f,0.0f,0.0f };
	// 試行回数上限
	uint32_t maxTrialCount = 100;

	/// 花を群れさせるための設定項目

	// 群れの数
	uint32_t clusterCountMin = 3;
	uint32_t clusterCountMax = 5;

	// １群れ当たりの花の数
	uint32_t clusterSizeMin = 2;
	uint32_t clusterSizeMax = 5;

	// 群れの人広がり半径
	float clusterRadiusMin = 4.0f;
	float clusterRadiusMax = 8.0f;

	// 単独で生える花の割合（0.0f - 1.0f）
	float singleSpawnRatio = 0.3f;

	// 横スクロール用：YZの固定
	bool fixY = true;
	bool fixZ = true;
};
