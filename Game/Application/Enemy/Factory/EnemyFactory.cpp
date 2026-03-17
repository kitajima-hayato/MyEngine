#include "EnemyFactory.h"
#include <Logger.h>
#include "../Ground/NormalEnemy.h"
#include "../Flying/FlyingEnemy.h"
#include "../Flying/SideMoveFlyingEnemy.h"
#include "../Ground/SideMoveEnemy.h"

std::unique_ptr<EnemyBase> EnemyFactory::CreateEnemy(const std::string& type)
{
	if(type == "NormalEnemy")
	{
		auto enemy = std::make_unique<NormalEnemy>();
		return enemy;
	}
	else if(type == "FlyingEnemy")
	{
		auto enemy = std::make_unique<FlyingEnemy>();
		return enemy;
	}
	else if (type == "SideMoveFlyingEnemy")
	{
		auto enemy = std::make_unique<SideMoveFlyingEnemy>();
		return enemy;
	}
	else if (type == "SideMoveEnemy") {
		auto enemy = std::make_unique<SideMoveEnemy>();
		return enemy;
	}
	Logger::Log("\nEnemy generation failed\nNo matching data found\n		or\nSpelling error\n\n");
	return nullptr; // 対応するエネミーがない場合はnullptrを返す
}
