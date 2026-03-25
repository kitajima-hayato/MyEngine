#include "StageSelectGraph.h"
#include <fstream>  

StageSelectGraph::StageSelectGraph()
{
}

StageSelectGraph::~StageSelectGraph()
{
}

void StageSelectGraph::Initialize()
{
	// ノード情報を
	// ファイルから読み込み
	LoadMapNodeFromJson("World_2");
	// ノード境界情報の再計算     
	RecalculateBounds();
}


void StageSelectGraph::Finalize()
{
}

uint32_t StageSelectGraph::AddNode(MapPos pos, uint32_t stageId, const std::string& stageKey, bool unlocked, float defaultYaw)
{
	// 新しいノードを作成
	StageNode newNode{};
	newNode.id = static_cast<uint32_t>(nodes_.size());
	newNode.position = pos;
	newNode.stageId = stageId;
	newNode.stageKey = stageKey;
	newNode.unlocked = unlocked;
	newNode.defaultYaw = defaultYaw;

	// 全方向の隣接ノードを無効に設定
	for (uint32_t i = 0; i < (uint32_t)Direction::count; i++) {
		newNode.neighbor[i] = INVALID_NODE_ID;
	}
	nodes_.push_back(newNode);
	// ノード境界情報の再計算
	RecalculateBounds();
	// 追加したノードのIDを返す
	return newNode.id;
}




void StageSelectGraph::Link(uint32_t fromNodeId, uint32_t toNodeId, Direction dir)
{
	// ノード同士を接続
	nodes_[fromNodeId].neighbor[(uint32_t)dir] = toNodeId;
	// 逆方向も接続
	Direction reverseDir{};
	switch (dir) {
	case Direction::Up:		reverseDir = Direction::Down;	break;
	case Direction::Down:	reverseDir = Direction::Up;		break;
	case Direction::Left:	reverseDir = Direction::Right;	break;
	case Direction::Right:	reverseDir = Direction::Left;	break;
	}

	// 逆方向の接続
	nodes_[toNodeId].neighbor[(uint32_t)reverseDir] = fromNodeId;
}

uint32_t StageSelectGraph::Move(uint32_t currentNodeId, Direction dir) const
{
	// 移動先ノードIDを取得
	uint32_t nextNodeId = nodes_[currentNodeId].neighbor[(uint32_t)dir];

	// 未接続であれば現在のノードを返す
	if (nextNodeId == INVALID_NODE_ID) {
		return currentNodeId;
	}
	// ロックされていたら移動できない
	/*if (!nodes_[nextNodeId].unlocked) {
		return currentNodeId;
	}*/

	return nextNodeId;
}

const StageNode& StageSelectGraph::GetNode(uint32_t id) const
{

	return nodes_.at(id);

}

void StageSelectGraph::LoadMapNodeFromJson(const std::string& fileName)
{
	// JSONファイルからノード情報を読み込む処理
	// filePathはリソースフォルダからの相対パス
	const std::string frontFilePath = "resources/Scenes/StageSelect/Data";
	const std::string kExtension = ".json";
	const std::string fullPath = frontFilePath + "/" + fileName + kExtension;

	// ファイル読み込み
	std::ifstream file;

	// ファイルを開く
	file.open(fullPath);

	// ファイルが開けたかどうか
	if (file.fail()) {
		// エラーメッセージを表示して終了
		printf("Failed to open file: %s\n", fullPath.c_str());
		return;
	}

	// JSON文字列
	nlohmann::json deserialized;
	// パース(解凍)
	file >> deserialized;

	// ノード情報の確認
	assert(deserialized.contains("nodes"));
	assert(deserialized["nodes"].is_array());



	// 既存ノード情報のクリア
	nodes_.clear();

	// -------- 1パス目：ノード生成のみ --------
	for (const auto& jsonNode : deserialized["nodes"]) {
		// ノード情報の確認
		assert(jsonNode.contains("position"));
		assert(jsonNode["position"].contains("x"));
		assert(jsonNode["position"].contains("y"));
		assert(jsonNode["position"]["x"].is_number_integer());
		assert(jsonNode["position"]["y"].is_number_integer());

		assert(jsonNode.contains("stage_id"));
		assert(jsonNode["stage_id"].is_number_integer());
		assert(jsonNode.contains("stage_key"));
		assert(jsonNode["stage_key"].is_string());
		assert(jsonNode.contains("unlocked"));
		assert(jsonNode["unlocked"].is_boolean());

		// ノード情報の取得
		MapPos pos{};
		pos.x = jsonNode["position"]["x"].get<uint32_t>();
		pos.y = jsonNode["position"]["y"].get<uint32_t>();
		uint32_t stageId = jsonNode["stage_id"].get<uint32_t>();
		bool unlocked = jsonNode["unlocked"].get<bool>();
		std::string stageKey = jsonNode["stage_key"].get<std::string>();

		// デフォルトYaw情報の取得（存在しない場合は自動計算）
		float defaultYaw = AUTO_YAW;
		if (jsonNode.contains("default_yaw")) {
			if (jsonNode["default_yaw"].is_number()) {
				defaultYaw = jsonNode["default_yaw"].get<float>();
			}
		}

		// ノードの追加
		AddNode(pos, stageId, stageKey, unlocked,defaultYaw);
	}

	// -------- 2パス目：neighbors 設定 --------
	for (uint32_t i = 0; i < (uint32_t)nodes_.size(); ++i) {
		const auto& jsonNode = deserialized["nodes"][i];

		// neighbors情報の確認
		if (!jsonNode.contains("neighbors")) continue;
		assert(jsonNode["neighbors"].is_object());

		// neighbors情報の取得と設定
		for (const auto& it : jsonNode["neighbors"].items()) {
			const std::string& dirStr = it.key();
			uint32_t neighborId = it.value().get<uint32_t>();

			// 方向の文字列をDirection列挙型に変換
			Direction dir{};
			if (dirStr == "Up") dir = Direction::Up;
			else if (dirStr == "Down") dir = Direction::Down;
			else if (dirStr == "Left") dir = Direction::Left;
			else if (dirStr == "Right") dir = Direction::Right;
			else continue;

			// 範囲外参照の保護
			if (neighborId >= nodes_.size()) continue;

			// 隣接ノードの設定
			SetNeighbor(i, dir, neighborId);
		}
	}



}


const char* DirectionToString(Direction d)
{
	switch (d) {
	case Direction::Up:    return "Up";
	case Direction::Down:  return "Down";
	case Direction::Left:  return "Left";
	case Direction::Right: return "Right";
	default:               return "Unknown";
	}
}


nlohmann::json StageSelectGraph::ToJson() const
{
	// json
	nlohmann::json root;
	// nodes 
	root["nodes"] = nlohmann::json::array();

	for (const auto& n : nodes_) {
		nlohmann::json jn;
		jn["position"] = { {"x",n.position.x},{"y",n.position.y} };
		jn["stage_id"] = n.stageId;
		jn["stage_key"] = n.stageKey;
		jn["unlocked"] = n.unlocked;
		if (n.defaultYaw != AUTO_YAW) {
			jn["default_yaw"] = n.defaultYaw;
		}
		nlohmann::json neigh = nlohmann::json::object();
		for (uint32_t i = 0; i < (uint32_t)Direction::count; i++) {
			uint32_t to = n.neighbor[i];
			if (to != INVALID_NODE_ID) {
				neigh[DirectionToString((Direction)i)] = to;
			}
		}
		jn["neighbors"] = neigh;

		root["nodes"].push_back(jn);
	}
	return root;
}

std::string StageSelectGraph::ToJsonString(int indent) const
{
	return ToJson().dump(indent);
}

void StageSelectGraph::SaveToJsonFile(const std::string& fileName) const
{
	// JSONファイルに保存する処理
	const std::string frontFilePath = "resources/Scenes/StageSelect/Data";
	const std::string kExtension = ".json";
	const std::string fullPath = frontFilePath + "/" + fileName + kExtension;
	// ファイル出力ストリームを開く
	std::ofstream ofs(fullPath);
	// JSON文字列を書き込む
	ofs << ToJsonString(2);
}

Vector2 StageSelectGraph::GetNodeUV(uint32_t nodeId) const
{
	// ノードIDの範囲チェック
	const StageNode& node = nodes_.at(nodeId);
	// 範囲計算
	const float rangeX = (bounds_.max.x > bounds_.min.x) ? static_cast<float>(bounds_.max.x - bounds_.min.x) : 1.0f;
	const float rangeY = (bounds_.max.y > bounds_.min.y) ? static_cast<float>(bounds_.max.y - bounds_.min.y) : 1.0f;
	// UV座標計算
	const float u = (static_cast<float>(node.position.x - bounds_.min.x)) / rangeX;
	const float v = (static_cast<float>(node.position.y - bounds_.min.y)) / rangeY;
	// 返却
	return Vector2{ u,v };
}

void StageSelectGraph::RecalculateBounds()
{
	// ノードの境界情報を再計算する処理
	// ノードが存在しない場合は初期化して終了
	if (nodes_.empty()) {
		bounds_ = {};
		return;
	}

	// 最小・最大座標を初期化
	bounds_.min = nodes_[0].position;
	bounds_.max = nodes_[0].position;

	// 全ノードを走査して最小・最大座標を更新
	for (const auto& n : nodes_) {
		if (n.position.x < bounds_.min.x) bounds_.min.x = n.position.x;
		if (n.position.y < bounds_.min.y) bounds_.min.y = n.position.y;
		if (n.position.x > bounds_.max.x) bounds_.max.x = n.position.x;
		if (n.position.y > bounds_.max.y) bounds_.max.y = n.position.y;
	}

}



bool StageSelectGraph::SetNodePos(uint32_t id, MapPos pos)
{
	// IDのチェック / 範囲外なら失敗
	if (id >= nodes_.size()) return false;
	// 座標の設定
	nodes_[id].position = pos;
	// ノード境界情報の再計算
	RecalculateBounds();
	return true;
}


bool StageSelectGraph::SetNodeStageId(uint32_t id, uint32_t stageId)
{
	// IDのチェック / 範囲外なら失敗
	if (id >= nodes_.size()) return false;
	// ステージIDの設定
	nodes_[id].stageId = stageId;
	return true;
}

bool StageSelectGraph::SetNodeStageKey(uint32_t id, const std::string& stageKey)
{
	if (id >= nodes_.size()) return false;
	nodes_[id].stageKey = stageKey;
	return true;
}

bool StageSelectGraph::SetNodeUnlocked(uint32_t id, bool unlocked)
{
	// IDのチェック / 範囲外なら失敗
	if (id >= nodes_.size()) return false;
	// アンロック状態の設定
	nodes_[id].unlocked = unlocked;
	return true;
}

bool StageSelectGraph::SetNodeYaw(uint32_t id, float yaw)
{
	// IDのチェック / 範囲外なら失敗
	if (id >= nodes_.size()) return false;
	// Yaw角の設定
	nodes_[id].defaultYaw = yaw;
	return true;
}

bool StageSelectGraph::SetNeighbor(uint32_t from, Direction dir, uint32_t toOrInvalid)
{
	// IDのチェック / 範囲外なら失敗
	if (from >= nodes_.size()) return false;
	// toが無効なノードIDでない場合、範囲外なら失敗
	if (toOrInvalid != INVALID_NODE_ID && toOrInvalid >= nodes_.size()) return false;
	// 隣接ノードの設定
	nodes_[from].neighbor[(int)dir] = toOrInvalid;
	return true;
}


bool StageSelectGraph::ClearNeighbor(uint32_t from, Direction dir)
{
	// 隣接ノードを無効に設定
	return SetNeighbor(from, dir, INVALID_NODE_ID);
}

