#pragma once
#include "DecorationInstanceData.h"
#include <memory>

class DecorationObject;
class DecorationFactory
{

public:
	/// <summary>
	/// DecorationInstanceDataを受け取ってDecorationObjectを生成する
	/// </summary>
	static std::unique_ptr<DecorationObject> Create(
		const DecorationInstanceData& data);
};

