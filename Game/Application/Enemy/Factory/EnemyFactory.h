#pragma once
#include <memory>

/// <summary>
/// ファクトリメソッドパターンのファクトリー部分
/// エネミーを生成する
/// </summary>

/// エネミー全てのファクトリークラス
#include "../EnemyBase/EnemyBase.h"
class NormalEnemy;
class FlyingEnemy;
class SideMoveFlyingEnemy;
class SideMoveEnemy;
class EnemyFactory
{
public:
	/// <summary>
	///  エネミーを生成するファクトリーメソッド
	/// </summary>
	/// <param name="type">生成するエネミーの名前</param>
	/// <returns>エネミーデータのインスタンス</returns>
	static std::unique_ptr<EnemyBase> CreateEnemy(const std::string& type);

};

