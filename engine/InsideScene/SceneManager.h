#pragma once
#include "BaseScene.h"
#include "AbstractSceneFactory.h"
#include "Game/Scene/Transition/TransitionManager.h"
#include <memory>
// シーン管理クラス
// シングルトンクラス
class SceneManager
{
public:
	/// <summary>
	/// シングルトンの取得
	/// </summary>
	/// <returns></returns>
	static SceneManager* GetInstance();
	static void DestroyInstance();
	/// <summary>
	/// コンストラクタ
	/// </summary>
	SceneManager() = default;
	/// <summary>
	/// デストラクタ
	/// </summary>
	~SceneManager() = default;
	/// <summary>
	/// コピーコンストラクタの無効化
	/// </summary>
	/// <param name=""></param>
	SceneManager(SceneManager&) = delete;
	/// <summary>
	/// 代入演算子の無効化
	/// </summary>
	/// <param name=""></param>
	/// <returns></returns>
	SceneManager& operator=(SceneManager&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="dxCommon"></param>
	void Update(Engine::DirectXCommon*dxCommon);
	/// <summary>
	/// 描画
	/// </summary>
	void Draw();
	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

public:
	/// <summary>
	/// シーンファクトリーのセット
	/// </summary>
	/// <param name="sceneFactory"></param>
	void SetSceneFactory(AbstractSceneFactory* sceneFactory) { sceneFactory_ = sceneFactory; }
	/// <summary>
	/// シーン変更
	/// </summary>
	/// <param name="sceneName"></param>
	void ChangeScene(const std::string& sceneName);

	/// <summary>
	/// シーン変更（遷移演出あり）
	/// </summary>
	/// <param name="sceneName">変更するシーンの名前</param>
	/// <param name="transitionType">遷移のタイプ</param>
	void ChangeSceneWithTransition(const std::string& sceneName, TransitionType transitionType = TransitionType::Start);
	/// <summary>
	/// 遷移マネージャーの取得
	/// </summary>
	TransitionManager* GetTransitionManager() { return transitionManager_.get(); }
private:
/// <summary>
/// シーン切り替えの実行（内部用）
/// </summary>
	void ExecuteSceneChange(Engine::DirectXCommon* dxCommon);
private:

	// 今のシーン
	std::unique_ptr<BaseScene> scene_ = nullptr;
	// 次のシーン
	std::unique_ptr<BaseScene> nextScene_ = nullptr;

	// シーンファクトリー // 借り物
	AbstractSceneFactory* sceneFactory_ = nullptr;

	// 遷移マネージャー
	std::unique_ptr<TransitionManager> transitionManager_ = nullptr;
	// 遷移中の次のシーン名
	std::string pendingSceneName_;
	bool isTransitionRequested_ = false;

	// 固定フレームレート用のデルタタイム
	static constexpr float kFixedDeltaTime_ = 1.0f / 60.0f;
	
};

