#pragma once
#include "DecorationSpawnSetting.h"
#include "DecorationObject.h"
#include <memory>
#include <vector>
class DecorationObject;
class DecorationField
{
public:
	/// <summary>
	/// DecorationSpawnSettingを受け取ってDecorationObjectを生成する
	/// </summary>
	/// <param name="setting"></param>
	void Generate(const DecorationSpawnSetting& setting);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	std::vector<std::unique_ptr<DecorationObject>> objects_;

};

