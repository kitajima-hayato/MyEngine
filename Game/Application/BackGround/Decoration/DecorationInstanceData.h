#pragma once
#include <string>
#include "MyMath.h"


struct DecorationInstanceData {
	// モデルのファイルパス
	std::string modelPath;
	// トランスフォーム / SRT
	Transform transform;

};
