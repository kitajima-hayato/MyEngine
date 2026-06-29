#pragma once
#include <cstdint>
#include <string>

/// <summary>
/// プレイコンテキスト
/// </summary>
class PlayContext
{
public:
	// プレイ中のステージID取得
	static PlayContext& GetInstance() {
		static PlayContext instance;
		return instance;
	}
	/// <summary>
	/// 選択中のステージID取得
	/// </summary>
	/// <param name="id">ステージの名前</param>
	void SetSelectedStageId(uint32_t id) { selectedStageId_ = id; }


	/// <summary>
	/// 選択中のステージキー取得
	/// </summary>
	/// <param name="key"></param>
	void SetSelectedStageKey(const std::string& key) { selectedStageKey_ = key; }

	/// <summary>
	/// 選択中のステージ設定
	/// </summary>
	/// <param name="id">セットするステージid</param>
	/// <param name="key">セットするステージキー</param>
	void SetSelectedStage(uint32_t id,const std::string& key){
		selectedStageId_ = id;
		selectedStageKey_ = key;
	}

	/// <summary>
	/// 選択中のステージID取得
	/// </summary>
	/// <returns>選択中のステージIDを返す</returns>
	uint32_t GetSelectedStageId()const { return selectedStageId_; }

	/// <summary>
	/// 選択中のステージキー取得
	/// </summary>
	/// <returns>選択中のステージキーを返す</returns>
	const std::string& GetSelectedStageKey()const { return selectedStageKey_; }
private:
	// ステージID
	uint32_t selectedStageId_ = 0;
	// デフォルトステージキー
	std::string selectedStageKey_ = "1-1";
};

