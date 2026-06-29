#include "CsvLoader.h"

const std::string CsvLoader::frontFilePath = "resources/Data/MapData/";
const std::string CsvLoader::extensionCsv_ = ".csv";



std::vector<std::vector<int>> CsvLoader::LoadMapInt(const std::string& filePath)
{
	// 読み込んだものを格納する変数
	std::vector<std::vector<int>> mapData;
	// ファイルを開く
	std::ifstream file(frontFilePath + filePath);

	// ファイルが開けなかった場合のエラーチェック
	if (!file.is_open()) {
		throw std::runtime_error("CSVファイルが開けません: " + filePath);
	}
	// 1行ずつ読み込む
	std::string line;
	while (std::getline(file, line)) {
		std::vector<int> row;
		std::stringstream ss(line);
		std::string value;

		while (std::getline(ss, value, ',')) {

			// 文字列を整数に変換
			row.push_back(std::stoi(value));

		}
		mapData.push_back(row);
	}
	// ファイルを閉じる
	file.close();
	// 読み込んだデータを返す
	return mapData;

}

std::vector<std::vector<BlockType>> CsvLoader::LoadMapBlockType(const std::string& filePath)
{
	// 読み込んだ物を格納する変数	
	std::vector<std::vector<BlockType>> mapData;
	// ファイルを開く
	std::ifstream file(frontFilePath + filePath + extensionCsv_);
	// ファイルが開けなかった場合のエラーチェック
	if (!file.is_open()) {
		// マップデータが存在しないもしくは開けなかった場合のエラー処理
		throw std::runtime_error("CSVファイルが開けません: " + filePath);
	}
	// 1行ずつ読み込む
	std::string line;
	// 各行をカンマで区切って処理
	while (std::getline(file, line)) {
		std::vector<BlockType> row;
		std::stringstream ss(line);
		std::string value;
		// カンマで区切られた各値を処理
		while (std::getline(ss, value, ',')) {
			// 文字列をintに変換し、それをBlockTypeへ変換
			int intVal = std::stoi(value);
			BlockType type = static_cast<BlockType>(intVal);
			row.push_back(type);
		}
		// 行をmapDataに追加
		mapData.push_back(row);
	}

	file.close();
	return mapData;
}

std::vector<std::vector<EnemyType>> CsvLoader::LoadMapEnemyType(const std::string& filePath)
{
	// 読み込んだ物を格納する変数	
	std::vector<std::vector<EnemyType>> enemyData;
	// ファイルを開く
	std::ifstream file(frontFilePath + filePath + extensionCsv_);
	// ファイルが開けなかった場合のエラーチェック
	if (!file.is_open()) {
		throw std::runtime_error("CSVファイルが開けません: " + filePath);
	}
	// 1行ずつ読み込む
	std::string line;
	// 各行をカンマで区切って処理
	while (std::getline(file, line)) {
		std::vector<EnemyType> row;
		std::stringstream ss(line);
		std::string value;
		// カンマで区切られた各値を処理
		while (std::getline(ss, value, ',')) {
			// 文字列をintに変換し、それをBlockTypeへ変換
			int intVal = std::stoi(value);
			EnemyType type = static_cast<EnemyType>(intVal);
			row.push_back(type);
		}
		// 行をenemyDataに追加
		enemyData.push_back(row);
	}

	file.close();
	return enemyData;
}


std::vector<std::vector<HazardType>> CsvLoader::LoadMapHazardType(const std::string& filePath, uint32_t width, uint32_t height)
{
	// まず None 埋めで作っておく（ファイルが無くても必ず正しいサイズになる）
	std::vector<std::vector<HazardType>> hazardData(
		height, std::vector<HazardType>(width, HazardType::None));

	std::ifstream file(frontFilePath + filePath + extensionCsv_);
	if (!file.is_open()) {
		return hazardData; // ファイル無い = Noneのまま返す
	}

	std::string line;
	uint32_t y = 0;
	while (std::getline(file, line) && y < height) {
		std::stringstream ss(line);
		std::string value;
		uint32_t x = 0;
		while (std::getline(ss, value, ',') && x < width) {
			int intVal = std::stoi(value);
			hazardData[y][x] = static_cast<HazardType>(intVal);
			++x;
		}
		++y;
	}
	return hazardData;
}

void CsvLoader::SaveMapBlockType(
	const std::string& filePath,
	const std::vector<std::vector<BlockType>>& mapData)
{
	// frontFilePath を使っているなら読み込みと同じように
	std::string filePathFull = frontFilePath + filePath;
	std::ofstream file(filePathFull);

	if (!file.is_open()) {
		throw std::runtime_error("CSVファイルを書き込み用に開けません: " + filePath);
	}
	// 行ごとにループ
	const uint32_t height = static_cast<uint32_t>(mapData.size());
	for (uint32_t y = 0; y < height; ++y) {
		const uint32_t width = static_cast<uint32_t>(mapData[y].size());
		for (uint32_t x = 0; x < width; ++x) {
			int val = static_cast<int>(mapData[y][x]);
			file << val;
			if (x < width - 1) {
				file << ",";        // カンマ区切り
			}
		}
		file << "\n";               // 行ごとに改行
	}
}

void CsvLoader::SaveMapEnemyType(
	const std::string& filePath, 
	const std::vector<std::vector<EnemyType>>& enemyData)
{

	// frontFilePath を使っているなら読み込みと同じように
	std::string filePathFull = frontFilePath + filePath;
	std::ofstream file(filePathFull);
	// ファイルが開けなかった場合のエラーチェック
	if (!file.is_open()) {
		throw std::runtime_error("CSVファイルを書き込み用に開けません: " + filePath);
	}
	// 行ごとにループ
	const uint32_t height = static_cast<uint32_t>(enemyData.size());
	for (uint32_t y = 0; y < height; ++y) {
		const uint32_t width = static_cast<uint32_t>(enemyData[y].size());
		for (uint32_t x = 0; x < width; ++x) {
			int val = static_cast<int>(enemyData[y][x]);
			file << val;
			if (x < width - 1) {
				file << ",";        // カンマ区切り
			}
		}
		file << "\n";               // 行ごとに改行
	}

}

void CsvLoader::SaveMapHazardType(const std::string& filePath, 
	const std::vector<std::vector<HazardType>>& hazardData)
{
	std::string filePathFull = frontFilePath + filePath;
	std::ofstream file(filePathFull);
	if (!file.is_open()) {
		throw std::runtime_error("CSVファイルを書き込み用に開けません: " + filePath);
	}
	const uint32_t height = static_cast<uint32_t>(hazardData.size());
	for (uint32_t y = 0; y < height; ++y) {
		const uint32_t width = static_cast<uint32_t>(hazardData[y].size());
		for (uint32_t x = 0; x < width; ++x) {
			int val = static_cast<int>(hazardData[y][x]);
			file << val;
			if (x < width - 1) {
				file << ",";        // カンマ区切り
			}
		}
		file << "\n";               // 行ごとに改行
	}

}

