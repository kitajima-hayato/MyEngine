#pragma once
#include <memory>
/// <summary>
/// ファクトリメソッドパターンのファクトリー部分
/// エネミーを生成する
/// </summary>
#include "../SpawnParams/EnemySpawnParams.h"

/// エネミー全てのファクトリークラス
#include "../EnemyBase/EnemyBase.h"
class NormalEnemy;
class FlyingEnemy;
class SideMoveFlyingEnemy;
class SideMoveEnemy;
class PatrolEnemy;
class TimedDropEnemy;
class ReactiveDropEnemy;
class EnemyFactory
{
public:
	/// <summary>
	///  エネミーを生成するファクトリーメソッド
	/// </summary>
	/// <param name="params">エネミー生成パラメーター</param>
	/// <returns>エネミーデータのインスタンス</returns>
	static std::unique_ptr<EnemyBase> CreateEnemy(const EnemySpawnParams& params);

};

