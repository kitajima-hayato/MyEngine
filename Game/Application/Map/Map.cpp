#include "Map.h"
#ifdef USE_IMGUI
#include "engine/base/ImGuiManager.h"
#endif
#include "Game/Particle/ParticleManager.h"

#ifdef USE_IMGUI
// ブロックタイプに応じた色を取得するヘルパー関数
ImVec4 GetBlockColorByType(BlockType blockType) {
	switch (blockType) {
	case BlockType::Air:         return ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
	case BlockType::GrassBlock:  return ImVec4(0.4f, 0.4f, 0.8f, 1.0f);
	case BlockType::SoilBlock:   return ImVec4(0.8f, 0.4f, 0.4f, 1.0f);
	case BlockType::kGoalUp:     return ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
	case BlockType::kGoalDown:   return ImVec4(0.4f, 0.8f, 0.8f, 1.0f);
	case BlockType::breakBlock:  return ImVec4(0.8f, 0.8f, 0.4f, 1.0f);
	case BlockType::moveBlock:   return ImVec4(0.8f, 0.4f, 0.8f, 1.0f);
	case BlockType::sandBlock:   return ImVec4(0.7f, 0.6f, 0.3f, 1.0f);
	case BlockType::unBreakable: return ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	case BlockType::damageBlock: return ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
	default:                     return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	}
}
#endif

void Map::Initialize(const std::string& mapFilePath)
{
	// マップデータの初期化
	mapChipData_.mapData.resize(kMapHeight, std::vector<BlockType>(kMapWidth, BlockType::Air));

	mapChipData_.hazardData.resize(kMapHeight, std::vector<HazardType>(kMapWidth, HazardType::None));

	// マップ番号の保存
	mapNumber_ = mapFilePath;

	// マップデータの読み込み
	LoadMapData(mapFilePath);

	// マップブロックの生成
	GenerateMapBlock();

	// ハザードブロックの生成
	GenerateHazardObjects();

	// マップデータの変更検知
	previousMapData_ = mapChipData_;

}


void Map::Update()
{
	//breakParticleEmitter_->Update();
	// マップの更新
	for (std::vector <Block*>& row : blockArray_) {
		for (Block* block : row) {
			if (!block) {
				continue;
			}
			block->Update();
		}
	}
	// ハザードの更新
	for (auto& row : hazardObjects_) {
		for (auto& hazard : row) {
			if (hazard) {
				hazard->Update();
			}
		}
	}

#pragma region MapEditor
#ifdef USE_IMGUI
	// ------------------------------------------------------------
	// Map Tools (ImGui)
	// ------------------------------------------------------------
	ImGui::Begin("Map Tools");

	// 画面上部に情報をコンパクトに
	ImGui::Text("Size: %u x %u", GetWidth(), GetHeight());
	ImGui::Separator();

	// 使い回す：ファイル名入力とメッセージ
	static char mapFileName[256] = "1-1.csv";
	static char enemyFileName[256] = "1-1_EnemyLayer.csv";
	static std::string message;

	// ヘルパー：拡張子を除いたベース名を作る
	auto MakeBaseName = [](const char* fileName) -> std::string {
		std::string base = fileName ? fileName : "";
		size_t dotPos = base.rfind('.');
		if (dotPos != std::string::npos) {
			base = base.substr(0, dotPos);
		}
		return base;
		};

	// ヘルパー：同一行にボタンを並べるための幅
	const float buttonW = 150.0f;

	// ------------------------------------------------------------
	// TabBar
	// ------------------------------------------------------------
	if (ImGui::BeginTabBar("MapTabs"))
	{
		// ============================================================
		// TAB: Block & Hazard Editor
		// ============================================================
		if (ImGui::BeginTabItem("Block/Hazard"))
		{
			// ------------------------
			// File Panel
			// ------------------------
			ImGui::Text("Map File (Block CSV):");
			ImGui::SameLine();
			ImGui::InputText("##MapFile", mapFileName, IM_ARRAYSIZE(mapFileName));

			// Save/Load (Block)
			if (ImGui::Button("Save Block CSV", ImVec2(buttonW, 0))) {
				try {
					CsvLoader::SaveMapBlockType(mapFileName, mapChipData_.mapData);
					message = std::string("Saved Block: ") + mapFileName;
				}
				catch (const std::exception& e) {
					message = std::string("Save Failed(Block): ") + e.what();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Block CSV", ImVec2(buttonW, 0))) {
				try {
					std::string baseName = MakeBaseName(mapFileName);

					// 変更し保存したマップを読み込む
					LoadMapData(baseName.c_str());
					// マップブロックの再生成
					GenerateMapBlock();
					// ハザードブロックの再生成
					GenerateHazardObjects();

					message = std::string("Loaded Block: ") + mapFileName;
				}
				catch (const std::exception& e) {
					message = std::string("Load Failed(Block): ") + e.what();
				}
			}

			// ------------------------------------------------------------
// Enemy Save/Load (auto file name from mapFileName)
// ------------------------------------------------------------
			ImGui::Separator();
			std::string mapBase = MakeBaseName(mapFileName);
			std::string enemyCsv = mapBase + "_EnemyLayer.csv";
			std::string enemyBase = mapBase + "_EnemyLayer"; // LoadMapEnemyType は baseName を渡す（拡張子なし）

			ImGui::Text("Enemy CSV: %s", enemyCsv.c_str());

			if (ImGui::Button("Save Enemy CSV", ImVec2(buttonW, 0))) {
				try {
					CsvLoader::SaveMapEnemyType(enemyCsv, enemyLayerData_.enemyData);
					message = std::string("Saved Enemy: ") + enemyCsv;
				}
				catch (const std::exception& e) {
					message = std::string("Save Failed(Enemy): ") + e.what();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Enemy CSV", ImVec2(buttonW, 0))) {
				try {
					CsvLoader loader;
					enemyLayerData_.enemyData = loader.LoadMapEnemyType(enemyBase);

					// サイズをマップに合わせる（安全）
					const uint32_t h = GetHeight();
					const uint32_t w = GetWidth();
					enemyLayerData_.enemyData.resize(h);
					for (uint32_t y = 0; y < h; ++y) {
						enemyLayerData_.enemyData[y].resize(w, EnemyType::None);
					}

					enemyLayerDirty_ = true;
					message = std::string("Loaded Enemy: ") + enemyCsv;
				}
				catch (const std::exception& e) {
					message = std::string("Load Failed(Enemy): ") + e.what();
				}
			}

			// Save/Load (Hazard)
			ImGui::SameLine();
			if (ImGui::Button("Save Hazard CSV", ImVec2(buttonW, 0))) {
				try {
					std::string baseName = MakeBaseName(mapFileName);
					std::string hazardCsv = baseName + "_HazardLayer.csv";
					CsvLoader::SaveMapHazardType(hazardCsv, mapChipData_.hazardData);
					message = std::string("Saved Hazard: ") + hazardCsv;
				}
				catch (const std::exception& e) {
					message = std::string("Save Failed(Hazard): ") + e.what();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Hazard CSV", ImVec2(buttonW, 0))) {
				try {
					std::string baseName = MakeBaseName(mapFileName);
					std::string hazardBase = baseName + "_HazardLayer";

					CsvLoader loader;
					mapChipData_.hazardData = loader.LoadMapHazardType(hazardBase, GetWidth(), GetHeight());

					message = std::string("Loaded Hazard: ") + (baseName + "_HazardLayer.csv");
				}
				catch (const std::exception& e) {
					message = std::string("Load Failed(Hazard): ") + e.what();
				}
			}

			if (!message.empty()) {
				ImGui::TextWrapped("%s", message.c_str());
			}

			// ------------------------
			// Edit Settings
			// ------------------------
			enum class EditLayerMode { Block = 0, Hazard = 1, Enemy = 2 };
			static int editModeInt = 0;

			ImGui::Text("Edit Layer:");
			ImGui::SameLine();
			ImGui::RadioButton("Block", &editModeInt, 0);
			ImGui::SameLine();
			ImGui::RadioButton("Hazard", &editModeInt, 1);
			ImGui::SameLine();
			ImGui::RadioButton("Enemy", &editModeInt, 2);

			// Paint type
			static int currentBlockTypeInt = 0;
			const char* blockTypeNames[] = {
				"Air",
				"GrassBlock",
				"SoilBlock",
				"GoalUp",
				"GoalDown",
				"BreakBlock",
				"MoveBlock",
				"SandBlock",
				"Unbreakable",
				"DamageBlock",
			};

			static int currentHazardTypeInt = 0;
			const char* hazardTypeNames[] = {
				"None",
				"Spike",
			};

			static int currentEnemyTypeInt = 0;
			const char* enemyTypeNames[] = {
				"None",
				"NormalEnemy",
				"FlyingEnemy",
			};

			if (static_cast<EditLayerMode>(editModeInt) == EditLayerMode::Block) {
				ImGui::Text("Paint Block:");
				ImGui::SameLine();
				ImGui::Combo("##PaintBlock", &currentBlockTypeInt, blockTypeNames, IM_ARRAYSIZE(blockTypeNames));
			} else if (static_cast<EditLayerMode>(editModeInt) == EditLayerMode::Hazard) {
				ImGui::Text("Paint Hazard:");
				ImGui::SameLine();
				ImGui::Combo("##PaintHazard", &currentHazardTypeInt, hazardTypeNames, IM_ARRAYSIZE(hazardTypeNames));
			} else {
				ImGui::Text("Paint Enemy:");
				ImGui::SameLine();
				ImGui::Combo("##PaintEnemy", &currentEnemyTypeInt, enemyTypeNames, IM_ARRAYSIZE(enemyTypeNames));
			}

			// 表示設定（見やすさ）
			static float cellSize = 20.0f;
			static bool showBlockNumber = false;
			static bool showOverlays = true;

			ImGui::Separator();
			ImGui::Text("View:");
			ImGui::SameLine();
			ImGui::SliderFloat("Cell Size", &cellSize, 12.0f, 32.0f, "%.0f");
			ImGui::SameLine();
			ImGui::Checkbox("Show Block ID", &showBlockNumber);
			ImGui::SameLine();
			ImGui::Checkbox("Show Overlays", &showOverlays);

			ImGui::Separator();

			// ------------------------
			// Grid
			// ------------------------
			ImGui::Text("Grid (drag to paint)");
			ImGui::BeginChild("MapGrid", ImVec2(0, 420), true, ImGuiWindowFlags_HorizontalScrollbar);

			const uint32_t mapHeight = GetHeight();
			const uint32_t mapWidth = GetWidth();

			// 念のため Enemy のサイズをマップに合わせる（エディタ実行中にズレても落ちない）
			if (enemyLayerData_.enemyData.size() != mapHeight) {
				enemyLayerData_.enemyData.resize(mapHeight);
			}
			for (uint32_t y = 0; y < mapHeight; ++y) {
				if (enemyLayerData_.enemyData[y].size() != mapWidth) {
					enemyLayerData_.enemyData[y].resize(mapWidth, EnemyType::None);
				}
			}

			for (uint32_t y = 0; y < mapHeight; ++y) {
				for (uint32_t x = 0; x < mapWidth; ++x) {

					BlockType& cellBlock = mapChipData_.mapData[y][x];
					HazardType& cellHazard = mapChipData_.hazardData[y][x];
					EnemyType& cellEnemy = enemyLayerData_.enemyData[y][x];

					ImGui::PushID(static_cast<int>(y * mapWidth + x));

					// Block color
					ImVec4 color = GetBlockColorByType(cellBlock);
					ImGui::PushStyleColor(ImGuiCol_Button, color);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

					// Label
					std::string label = showBlockNumber ? std::to_string(static_cast<int>(cellBlock)) : " ";
					ImGui::Button(label.c_str(), ImVec2(cellSize, cellSize));

					// Paint
					if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
						const EditLayerMode mode = static_cast<EditLayerMode>(editModeInt);

						if (mode == EditLayerMode::Block) {
							cellBlock = static_cast<BlockType>(currentBlockTypeInt);
							isMapDataChanged_ = true;
						} else if (mode == EditLayerMode::Hazard) {
							cellHazard = static_cast<HazardType>(currentHazardTypeInt);
							isMapDataChanged_ = true;
							// ★Hazardを見た目に反映したいなら（重いなら Dirty化推奨）
							GenerateHazardObjects();
						} else {
							cellEnemy = static_cast<EnemyType>(currentEnemyTypeInt);
							enemyLayerDirty_ = true;
						}
					}

					ImGui::PopStyleColor(3);

					// Overlay
					if (showOverlays) {
						ImDrawList* dl = ImGui::GetWindowDrawList();
						ImVec2 pMin = ImGui::GetItemRectMin();
						ImVec2 pMax = ImGui::GetItemRectMax();

						// Hazard overlay
						if (cellHazard != HazardType::None) {
							dl->AddRect(pMin, pMax, IM_COL32(255, 60, 60, 255), 0.0f, 0, 2.0f);
							dl->AddText(ImVec2(pMin.x + 2.0f, pMin.y + 1.0f), IM_COL32(255, 255, 255, 255), "H");
						}

						// Enemy overlay
						if (cellEnemy != EnemyType::None) {
							ImU32 col = IM_COL32(60, 255, 60, 255);
							if (cellEnemy == EnemyType::FlyingEnemy) {
								col = IM_COL32(60, 140, 255, 255);
							}
							ImVec2 eMin(pMin.x + 2.0f, pMin.y + 2.0f);
							ImVec2 eMax(pMax.x - 2.0f, pMax.y - 2.0f);
							dl->AddRect(eMin, eMax, col, 0.0f, 0, 2.0f);
							dl->AddText(ImVec2(pMin.x + 2.0f, pMax.y - 14.0f), IM_COL32(255, 255, 255, 255), "E");
						}
					}

					ImGui::PopID();

					if (x < mapWidth - 1) {
						ImGui::SameLine();
					}
				}
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		// ============================================================
		// TAB: Enemy Layer
		// ============================================================
		if (ImGui::BeginTabItem("Enemy"))
		{
			// Paint type
			static int currentEnemyTypeInt = 0;
			const char* enemyTypeNames[] = {
				"None",
				"NormalEnemy",
				"FlyingEnemy",
			};

			ImGui::Text("Paint Enemy:");
			ImGui::SameLine();
			ImGui::Combo("##EnemyType", &currentEnemyTypeInt, enemyTypeNames, IM_ARRAYSIZE(enemyTypeNames));

			// File
			ImGui::Text("Enemy CSV:");
			ImGui::SameLine();
			ImGui::InputText("##EnemyFile", enemyFileName, IM_ARRAYSIZE(enemyFileName));

			if (ImGui::Button("Save Enemy CSV", ImVec2(buttonW, 0))) {
				try {
					CsvLoader::SaveMapEnemyType(enemyFileName, enemyLayerData_.enemyData);
					message = std::string("Saved Enemy: ") + enemyFileName;
				}
				catch (const std::exception& e) {
					message = std::string("Save Failed(Enemy): ") + e.what();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Load Enemy CSV", ImVec2(buttonW, 0))) {
				try {
					std::string baseName = MakeBaseName(enemyFileName);

					CsvLoader loader;
					enemyLayerData_.enemyData = loader.LoadMapEnemyType(baseName);
					enemyLayerDirty_ = true;

					message = std::string("Loaded Enemy: ") + enemyFileName;
				}
				catch (const std::exception& e) {
					message = std::string("Load Failed(Enemy): ") + e.what();
				}
			}

			if (!message.empty()) {
				ImGui::TextWrapped("%s", message.c_str());
			}

			ImGui::Separator();

			// Grid
			auto& enemyLayer = enemyLayerData_.enemyData;
			if (enemyLayer.empty() || enemyLayer[0].empty()) {
				ImGui::Text("Enemy layer is empty or not loaded.");
			} else {
				static float enemyCellSize = 20.0f;
				ImGui::SliderFloat("Cell Size##Enemy", &enemyCellSize, 12.0f, 32.0f, "%.0f");

				ImGui::BeginChild("EnemyLayerGrid", ImVec2(0, 420), true, ImGuiWindowFlags_HorizontalScrollbar);

				const uint32_t h = static_cast<uint32_t>(enemyLayer.size());
				const uint32_t w = static_cast<uint32_t>(enemyLayer[0].size());

				for (uint32_t y = 0; y < h; ++y) {
					for (uint32_t x = 0; x < w; ++x) {
						EnemyType& cell = enemyLayer[y][x];

						ImGui::PushID(static_cast<int>(y * w + x));

						ImVec4 color;
						switch (cell) {
						case EnemyType::NormalEnemy: color = ImVec4(0.2f, 0.8f, 0.2f, 1.0f); break;
						case EnemyType::FlyingEnemy: color = ImVec4(0.2f, 0.4f, 1.0f, 1.0f); break;
						default:                     color = ImVec4(0.4f, 0.4f, 0.4f, 1.0f); break;
						}

						ImGui::PushStyleColor(ImGuiCol_Button, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

						ImGui::Button(" ", ImVec2(enemyCellSize, enemyCellSize));

						if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
							cell = static_cast<EnemyType>(currentEnemyTypeInt);
						}

						ImGui::PopStyleColor(3);
						ImGui::PopID();

						if (x < w - 1) {
							ImGui::SameLine();
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::EndTabItem();
		}

		// ============================================================
		// TAB: Preview
		// ============================================================
		if (ImGui::BeginTabItem("Preview"))
		{
			ImGui::Text("Preview (read-only)");
			ImGui::BeginChild("MapPreview", ImVec2(0, 520), true, ImGuiWindowFlags_HorizontalScrollbar);

			const float previewCell = 20.0f;
			const uint32_t mapHeight = GetHeight();
			const uint32_t mapWidth = GetWidth();

			for (uint32_t y = 0; y < mapHeight; ++y) {
				for (uint32_t x = 0; x < mapWidth; ++x) {
					BlockType cell = mapChipData_.mapData[y][x];

					ImGui::PushID(static_cast<int>(y * mapWidth + x));
					ImVec4 color = GetBlockColorByType(cell);
					ImGui::PushStyleColor(ImGuiCol_Button, color);
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);

					ImGui::Button(" ", ImVec2(previewCell, previewCell));

					ImGui::PopStyleColor(3);
					ImGui::PopID();

					if (x < mapWidth - 1) {
						ImGui::SameLine();
					}
				}
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End(); // Map Tools
#endif // USE_IMGUI
#pragma endregion MapEditor
	// 末尾に追加：死んだブロックだけ回収
	for (auto& row : blockArray_) {
		for (Block*& block : row) {
			if (block && !block->GetAliveBlock()) {
				delete block;
				block = nullptr;
			}
		}
	}
	// マップデータの変更を検知してブロックを再生成
	if (isMapDataChanged_) {
		//GenerateMapBlock();
		//GenerateHazardObjects();
		isMapDataChanged_ = false;

	}

}



void Map::Draw()
{
	// マップの描画
	for (std::vector <Block*>& row : blockArray_) {
		for (Block* block : row) {
			if (!block) {
				continue;
			}
			block->Draw();
		}
	}

	// ハザードの描画
	for (auto& row : hazardObjects_) {
		for (auto& hazard : row) {
			if (hazard) {
				hazard->Draw();
			}
		}
	}

}

void Map::Finalize()
{
	// マップブロックの解放
	for (std::vector <Block*>& row : blockArray_) {
		for (Block* block : row) {
			if (block) {
				delete block;
				block = nullptr;
			}
		}
	}
}

void Map::GenerateMapBlock()
{
	const uint32_t h = GetHeight();
	const uint32_t w = GetWidth();

	// 既存のブロックを解放
	for (auto& row : blockArray_) {
		for (Block*& block : row) {
			if (block) {
				delete block;
				block = nullptr;
			}
		}
	}
	// ポインタを解放したので配列自体もクリア
	blockArray_.clear();

	// 新しいサイズで配列を作り直す
	blockArray_.assign(h, std::vector<Block*>(w, nullptr));

	// マップデータからブロックを生成
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			const BlockType type = mapChipData_.mapData[y][x];
			if (type == BlockType::Air) {
				continue;
			}
			// ブロックの生成
			Vector3 pos = GetMapChipPositionByIndex(x, y);
			pos.x += blockOffset_;
			pos.y -= blockOffset_;
			blockArray_[y][x] = Block::CreateBlock(type, pos);
		}
	}
}

void Map::GenerateEnemyLayer()
{
	const uint32_t h = GetHeight();
	const uint32_t w = GetWidth();

	// エネミーデータから敵を生成
	for (uint32_t y = 0; y < h; y++) {
		for (uint32_t x = 0; x < w; x++) {
			const EnemyType type = enemyLayerData_.enemyData[y][x];
			if (type == EnemyType::None) {
				continue;
			}
			// 敵の生成
			Vector3 pos = GetMapChipPositionByIndex(x, y);
			pos.x += blockOffset_;
			pos.y -= blockOffset_;
		}
	}
}


void Map::LoadMapData(const std::string& mapFilePath)
{
	// CSVファイルからマップデータを読み込む
	CsvLoader csvLoader;
	mapChipData_.mapData = csvLoader.LoadMapBlockType(mapFilePath);

	std::string enemyLayerFilePath = mapFilePath + std::string("_EnemyLayer");
	// 敵レイヤーデータの読み込み
	enemyLayerData_.enemyData = csvLoader.LoadMapEnemyType(enemyLayerFilePath);
	const uint32_t h = GetHeight();
	const uint32_t w = GetWidth();
	enemyLayerData_.enemyData.resize(h);
	for (uint32_t y = 0; y < h; ++y) {
		enemyLayerData_.enemyData[y].resize(w, EnemyType::None);
	}

	std::string hazardFilePath = mapFilePath + std::string("_HazardLayer");
	mapChipData_.hazardData = csvLoader.LoadMapHazardType(hazardFilePath, GetWidth(), GetHeight());

}

void Map::GenerateHazardObjects()
{
	// サイズの取得
	const uint32_t h = GetHeight();
	const uint32_t w = GetWidth();

	// サイズの確保
	hazardObjects_.clear();
	hazardObjects_.resize(h);
	for (uint32_t y = 0; y < h; ++y) {
		hazardObjects_[y].resize(w);
	}
	// ハザードデータから Object3D を生成
	for (uint32_t y = 0; y < h; ++y) {
		for (uint32_t x = 0; x < w; ++x) {
			const HazardType type = mapChipData_.hazardData[y][x];
			if (type == HazardType::Spike) {
				//if (hazardObjects_[y][x]) {
					// 既にオブジェクトがある場合は再利用
				hazardObjects_[y][x] = std::make_unique<Object3D>();
				hazardObjects_[y][x]->Initialize();
				// モデルの指定
				hazardObjects_[y][x]->SetModel("GamePlay/Blocks/damageblock");
				//}
				// ブロックの座標系に合わせて配置
				Vector3 pos = GetMapChipPositionByIndex(x, y);
				pos.x += blockOffset_;
				pos.y -= blockOffset_;
				pos.z = 00.0f;
				hazardObjects_[y][x]->SetTranslate(pos);

			} else {
				// Noneなら消す
				hazardObjects_[y][x].reset();

			}
		}
	}


}


IndexSet Map::GetMapChipIndexSetByPosition(const Vector3& position)
{
	IndexSet indexSet{};

	// 座標からインデックスを計算
	float x = std::floor(position.x / kBlockWidth);
	float y = std::floor((GetHeight() - 1) - (position.y / kBlockHeight));

	// 座標がマップ外の場合は最大値を返す
	int ix = static_cast<int>(x);
	int iy = static_cast<int>(y);

	// マップ外チェック / マップ外なら範囲外を示すインデックスを返す
	if (ix < 0 || ix >= static_cast<int>(GetWidth()) ||
		iy < 0 || iy >= static_cast<int>(GetHeight())) {
		indexSet.xIndex = GetWidth();
		indexSet.yIndex = GetHeight();
		return indexSet;
	}
	// インデックスをセット
	indexSet.xIndex = static_cast<uint32_t>(ix);
	indexSet.yIndex = static_cast<uint32_t>(iy);

	// インデックスを返す
	return indexSet;
}

BlockType Map::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex)
{
	// マップ外チェック
	uint32_t w = GetWidth();
	uint32_t h = GetHeight();
	// マップ外ならAirを返す
	if (xIndex >= w || yIndex >= h) {
		return BlockType::Air;
	}
	// 指定インデックスのマップチップの種類を返す
	return mapChipData_.mapData[yIndex][xIndex];
}


Rect Map::GetRectByIndex(uint32_t xIndex, uint32_t yIndex)
{
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);
	Rect rect{};
	rect.left = center.x - (kBlockWidth / 2);
	rect.right = center.x + (kBlockWidth / 2);
	rect.bottom = center.y - (kBlockHeight / 2);
	rect.top = center.y + (kBlockHeight / 2);
	return rect;
}

Vector3 Map::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex)
{
	const uint32_t h = GetHeight();
	return Vector3(kBlockWidth * xIndex,
		kBlockHeight * (h - 1 - yIndex),
		0.0f);
}

void Map::BreakBlock(uint32_t xIndex, uint32_t yIndex)
{
	// マップ外チェック
	if (yIndex >= mapChipData_.mapData.size() ||
		xIndex >= mapChipData_.mapData[yIndex].size()) {
		return;
	}
	// 破壊可能ブロックかチェック
	if (mapChipData_.mapData[yIndex][xIndex] != BlockType::breakBlock) {
		return;
	}

	// 該当データの書き換え / Airに変更

	isMapDataChanged_ = true;

	// 実体（描画/更新）をその場で消す（Map全再生成しない）
	if (yIndex < blockArray_.size() && xIndex < blockArray_[yIndex].size()) {
		if (blockArray_[yIndex][xIndex]) {
			// 生存フラグ方式（演出を挟むなら Kill のみにしてもOK）
			blockArray_[yIndex][xIndex]->SetBroken();

			// 破壊位置の取得
			Vector3 breakPos = GetMapChipPositionByIndex(xIndex, yIndex);
			// 破壊時にパーティクルの発生
			ModelParticleManager::GetInstance().EmitBlockDebris(
				breakPos,
				Vector4(1.0f, 1.0f, 1.0f, 1.0f),
				15);
			// 該当のBreakBlockをAirに変更
			mapChipData_.mapData[yIndex][xIndex] = BlockType::Air;
		}
	}
}

HazardType Map::GetHazardTypeByIndex(uint32_t xIndex, uint32_t yIndex) const
{
	uint32_t w = GetWidth();
	uint32_t h = GetHeight();
	if (xIndex >= w || yIndex >= h) { return HazardType::None; }
	if (mapChipData_.hazardData.empty()) { return HazardType::None; }
	return mapChipData_.hazardData[yIndex][xIndex];
}

