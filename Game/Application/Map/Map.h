#pragma once
#include <vector>
#include "MyMath.h"
#include "Game/Application/Block/Block.h"
#include "Game/Application/Block/BlockType.h"
#include "Game/Application/Enemy/Types/EnemyType.h"
#include "Game/Particle/ParticleSystem.h"
#include "Game/Particle/ModelParticleManager.h"
#include <cstdint>
#include <string>
#include "Game/Application/Map/CsvLoader.h"
#include "Game/Application/Block/HazardType.h"
/// マップクラス

/// マップチップデータ構造体
struct MapChipData {
	std::vector<std::vector<BlockType>> mapData;
	std::vector<std::vector<HazardType>> hazardData;
};
struct EnemyLayerData {
	std::vector<std::vector<EnemyType>> enemyData;
};

/// インデックス構造体
struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

/// 矩形構造体
struct Rect {
	float left;
	float right;
	float bottom;
	float top;
};

class Map
{
public:
	// １ブロックの大きさ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;
	//マップの大きさ
	// 横
	static inline const uint32_t kMapWidth = 100;
	// 高さ
	static inline const uint32_t kMapHeight = 20;

	// 可変サイズのマップの大きさ
	uint32_t GetWidth()const {
		if (mapChipData_.mapData.empty()) { return 0; }
		return (uint32_t)mapChipData_.mapData[0].size();
	}
	uint32_t GetHeight()const {
		return (uint32_t)mapChipData_.mapData.size();
	}
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(const std::string& mapFilePath);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// マップブロック生成
	/// </summary>
	void GenerateMapBlock();

	/// <summary>
	/// エネミーレイヤー生成
	/// </summary>
	void GenerateEnemyLayer();

	/// <summary>
	/// マップデータの読み込み
	/// </summary>
	void LoadMapData(const std::string& mapFilePath);

	/// <summary>
	/// ハザードブロックの生成
	/// </summary>
	void GenerateHazardObjects();


public:	// Setter / Getter
	/// <summary>
	/// 座標からマップチップのインデックスを取得
	/// </summary>
	/// <param name="position">取得したいマップのインデックス座標</param>
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	/// <summary>
	/// インデックスからマップチップの種類を取得
	/// </summary>
	/// <param name="xIndex/yIndex">取得したいマップの座標インデックス</param>
	/// <returns>指定したインデックスのマップチップの種類</returns>
	BlockType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);


	/// <summary>
	/// インデックスから矩形情報を取得
	/// </summary>
	/// <param name="xIndex/yIndex">インデックス座標</param>
	/// <returns>指定インデックス座標のブロックの矩形情報</returns>
	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// インデックスからマップチップの中心座標を取得
	/// </summary>
	/// <param name="xIndex/yIndex">インデックス座標</param>
	/// <returns>指定インデックス座標のブロックの中心座標</returns>
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// マップチップデータを取得
	/// </summary>
	const EnemyLayerData& GetEnemyLayerData() const { return enemyLayerData_; }

	/// <summary>
	/// 編集用マップチップデータを取得
	/// </summary>
	EnemyLayerData& GetEnemyLayerData() { return enemyLayerData_; }

	/// <summary>
	/// エネミーレイヤー変更検知フラグを消費
	/// </summary>
	/// <returns></returns>
	bool ConsumeEnemyLayerDirtyFlag() {
		bool result = enemyLayerDirty_;
		enemyLayerDirty_ = false;
		return result;
	}

	/// <summary>
	/// インデックスからマップチップの種類を設定
	/// </summary>
	/// <param name="xIndex/yIndex">設定したいマップの座標インデックス</param>
	/// <param name="type">設定するマップチップの種類</param>
	void SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, BlockType type) {
		if (yIndex < mapChipData_.mapData.size() && xIndex < mapChipData_.mapData[yIndex].size()) {
			mapChipData_.mapData[yIndex][xIndex] = type;
			isMapDataChanged_ = true;
		}
	}

	/// <summary>
	/// 破壊可能ブロックを壊す
	/// </summary>
	/// <param name="xIndex">当たって壊せるブロックの横Indexの番号</param>
	/// <param name="yIndex">当たって壊せるブロックの縦Indexの番号</param>
	void BreakBlock(uint32_t xIndex, uint32_t yIndex);

	/// <summary>
	/// インデックスからマップチップの種類を設定
	/// </summary>
	HazardType GetHazardTypeByIndex(uint32_t xIndex, uint32_t yIndex) const;

	/// <summary>
	/// ジャンプブロックのトグル状態を取得
	/// </summary>
	/// <returns>ブロックのOnとOff状態の取得</returns>
	bool GetJumpToggleState()const { return jumpToggleState_; }

	/// <summary>
	/// ジャンプブロックのトグル状態を切り替える
	/// </summary>
	void ToggleJumpSwitch() { jumpToggleState_ = !jumpToggleState_; }

	/// <summary>
	/// 指定インデックスに当たり判定のあるブロックがあるか
	/// </summary>
	/// <param name="xIndex">走査する横インデックス</param>
	/// <param name="yIndex">走査する縦インデックス</param>
	/// <returns>当たり判定のあるブロックがあるかどうか</returns>
	bool IsSolidBlockAt(uint32_t xIndex, uint32_t yIndex) const;
private:
	// マップチップデータ
	MapChipData mapChipData_;
	// エネミーレイヤーデータ
	EnemyLayerData enemyLayerData_;
	// ハザードブロックの描画用
	std::vector<std::vector<std::unique_ptr<Object3D>>>hazardObjects_;


	// 変更検知
	bool isMapDataChanged_ = false;
	MapChipData previousMapData_;

	// マップブロックの配列
	std::vector<std::vector<Block*>> blockArray_;

	// 可変サイズのマップの大きさ
	uint32_t width_ = 0;
	uint32_t height_ = 0;

	// ブロックを少しずらす
	float blockOffset_ = 0.5f;

	// マップ番号
	std::string mapNumber_;

	// エネミーレイヤー変更検知フラグ
	bool enemyLayerDirty_ = false;


	// 破壊可能ブロックのパーティクルエフェクト用
	std::unique_ptr<ParticleSystem> breakParticleEmitter_;

	// ブロックのトグル状態
	bool jumpToggleState_ = true;

};
