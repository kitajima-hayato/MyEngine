#include "DecorationPlacer.h"
#include <random>
#include <cassert>
#include <algorithm>

std::vector<DecorationInstanceData> DecorationPlacer::CreateLayout(
	const DecorationSpawnSetting& setting)
{
	std::vector<DecorationInstanceData> result;
	result.reserve(setting.objectCount);

	if (setting.modelPaths.empty() || setting.objectCount == 0) {
		return result;
	}

	static std::random_device rd;
	static std::mt19937 mt(rd());

	// 群れ数を決定
	std::uniform_int_distribution<uint32_t> clusterCountDist(
		setting.clusterCountMin, setting.clusterCountMax);
	uint32_t clusterCount = clusterCountDist(mt);

	// 単独花の数を計算
	uint32_t singleCount = static_cast<uint32_t>(
		static_cast<float>(setting.objectCount) * setting.singleSpawnRatio);

	if (singleCount > setting.objectCount) {
		singleCount = setting.objectCount;
	}

	uint32_t clusterObjectCount = setting.objectCount - singleCount;
	uint32_t createdCount = 0;

	// -------------------------
	// 群れの花を生成
	// -------------------------
	for (uint32_t clusterIndex = 0;
		clusterIndex < clusterCount && createdCount < clusterObjectCount;
		++clusterIndex) {

		std::uniform_real_distribution<float> centerXDist(
			(std::min)(setting.areaMin.x, setting.areaMax.x),
			(std::max)(setting.areaMin.x, setting.areaMax.x));

		float clusterCenterX = centerXDist(mt);

		std::uniform_real_distribution<float> radiusDist(
			(std::min)(setting.clusterRadiusMin, setting.clusterRadiusMax),
			(std::max)(setting.clusterRadiusMin, setting.clusterRadiusMax));

		float clusterRadius = radiusDist(mt);

		std::uniform_int_distribution<uint32_t> clusterSizeDist(
			setting.clusterSizeMin, setting.clusterSizeMax);

		uint32_t clusterSize = clusterSizeDist(mt);

		for (uint32_t i = 0;
			i < clusterSize && createdCount < clusterObjectCount;
			++i) {

			bool placed = false;

			for (uint32_t trial = 0; trial < setting.maxTrialCount; ++trial) {

				std::uniform_real_distribution<float> offsetDist(
					-clusterRadius, clusterRadius);

				float x = clusterCenterX + offsetDist(mt);

				x = (std::max)(
					(std::min)(setting.areaMin.x, setting.areaMax.x),
					(std::min)(
						x,
						(std::max)(setting.areaMin.x, setting.areaMax.x)));

				Vector3 pos{};
				pos.x = x;
				pos.y = setting.fixY ? setting.areaMin.y :
					RandomVector3(setting.areaMin, setting.areaMax).y;
				pos.z = setting.fixZ ? setting.areaMin.z :
					RandomVector3(setting.areaMin, setting.areaMax).z;

				if (!IsFarEnough(pos, result, setting.spawnRangeInterval)) {
					continue;
				}

				DecorationInstanceData data;
				data.modelPath = RandomModelPath(setting.modelPaths);
				data.transform.translate = pos;
				data.transform.rotate = RandomVector3(
					setting.minRotation, setting.maxRotation);
				data.transform.scale = RandomVector3(
					setting.minScale, setting.maxScale);

				result.push_back(data);
				++createdCount;
				placed = true;
				break;
			}

		}
	}

	// -------------------------
	// 単独花を生成
	// -------------------------
	while (createdCount < setting.objectCount) {
		bool placed = false;

		for (uint32_t trial = 0; trial < setting.maxTrialCount; ++trial) {

			std::uniform_real_distribution<float> xDist(
				(std::min)(setting.areaMin.x, setting.areaMax.x),
				(std::max)(setting.areaMin.x, setting.areaMax.x));

			Vector3 pos{};
			pos.x = xDist(mt);
			pos.y = setting.fixY ? setting.areaMin.y :
				RandomVector3(setting.areaMin, setting.areaMax).y;
			pos.z = setting.fixZ ? setting.areaMin.z :
				RandomVector3(setting.areaMin, setting.areaMax).z;

			if (!IsFarEnough(pos, result, setting.spawnRangeInterval)) {
				continue;
			}

			DecorationInstanceData data;
			data.modelPath = RandomModelPath(setting.modelPaths);
			data.transform.translate = pos;
			data.transform.rotate = RandomVector3(
				setting.minRotation, setting.maxRotation);
			data.transform.scale = RandomVector3(
				setting.minScale, setting.maxScale);

			result.push_back(data);
			++createdCount;
			placed = true;
			break;
		}

		if (!placed) {
			break;
		}
	}

	return result;
}

bool DecorationPlacer::IsFarEnough(const Vector3& candidate, const std::vector<DecorationInstanceData>& placedList, float minDistance) const
{
	for (const auto& placed : placedList) {
		float dx = candidate.x - placed.transform.translate.x;

		if (std::abs(dx) < minDistance) {
			return false;
		}
	}

	return true;
}

Vector3 DecorationPlacer::RandomPosition(const Vector3& min, const Vector3& max) const
{
	return RandomVector3(min, max);

}

Vector3 DecorationPlacer::RandomVector3(const Vector3& min, const Vector3& max) const
{
	static std::random_device rd;
	static std::mt19937 mt(rd());

	std::uniform_real_distribution<float> distX((std::min)(min.x, max.x), (std::max)(min.x, max.x));
	std::uniform_real_distribution<float> distY((std::min)(min.y, max.y), (std::max)(min.y, max.y));
	std::uniform_real_distribution<float> distZ((std::min)(min.z, max.z), (std::max)(min.z, max.z));

	Vector3 value;
	value.x = distX(mt);
	value.y = distY(mt);
	value.z = distZ(mt);

	return value;
}

std::string DecorationPlacer::RandomModelPath(const std::vector<std::string>& modelPaths) const
{
	assert(!modelPaths.empty());

	static std::random_device rd;
	static std::mt19937 mt(rd());

	std::uniform_int_distribution<size_t> dist(0, modelPaths.size() - 1);
	return modelPaths[dist(mt)];
}
