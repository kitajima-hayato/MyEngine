#pragma once
#include "BossEnemy.h"
#include "BossSpawnedObject.h"
#include "BossProjectile.h"
#include "BossHPBar.h"

#include <memory>
#include <vector>

// 前方宣言
class Map;
class Player;
class CollisionManager;


class BossStageManager
{
public:	// メンバ関数 
	/// <summary>
	/// ボスステージマネージャーの初期化
	/// </summary>
	/// <param name="map">マップのポインタ</param>
	/// <param name="player">プレイヤーのポインタ</param>
	/// <param name="collision">当たり判定マネージャーのポインタ</param>
	void Initialize(Map* map, Player* player,CollisionManager* collision);


	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 当たり判定
	/// </summary>
	void CheckCollision();


	/// <summary>
	/// ボスが倒されたかどうかを返す
	/// </summary>
	/// <returns>ボスが倒された場合はtrue、それ以外はfalse</returns>
	bool IsBossDefeated()const;


	/// <summary>
	/// ボスのHPバーを描画する
	/// </summary>
	void DrawHUD();
#ifdef USE_IMGUI
	void DrawImgui();
#endif
private:	// メンバ関数

	/// <summary>
	/// マップをスキャンして、スポーンされたオブジェクトを生成する
	/// </summary>
	void ScanAndCreateSpawnedObjects();




private:	// メンバ変数

	// ボスのインスタンス
	std::unique_ptr<BossEnemy> bossEnemy; 
	// ボスのHPバーのインスタンス
	std::unique_ptr<BossHPBar> bossHPBar;

	// ステージ上にスポーンされたオブジェクトのリスト
	std::vector<std::unique_ptr<BossSpawnedObject>> spawnedObjects;

	// ボスの攻撃プロジェクトタイルのリスト
	std::vector<std::unique_ptr<BossProjectile>> bossProjectiles;
	
	// マップとプレイヤーのポインタ
	Map* map;
	Player* player;
	CollisionManager* collision;

};

