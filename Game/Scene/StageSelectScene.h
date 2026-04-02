#pragma once
#include "engine/InsideScene/BaseScene.h"
#include <MyMath.h>
#include "SceneSelectGraph/StageSelectGraph.h"
#include "Sprite.h"

/// <summary>
/// ステージセレクトシーン
/// ステージ選択画面のシーン
/// </summary>

class Object3D;
class Camera;
class StageSelectScene :
	public BaseScene
{
public:
	StageSelectScene();
	~StageSelectScene();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(Engine::DirectXCommon* dxCommon)override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update()override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw()override;

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize()override;

	/// <summary>
	/// Imgui一括管理
	/// </summary>
	void DrawImgui();

	/// <summary>
	/// プレイヤー移動処理
	/// </summary>
	void PlayerMove();

private:
	/// <summary>
	/// ステージセレクト入力処理
	/// </summary>
	void HandleSelectInput();

	/// <summary>
	/// ノード位置をカーソル位置に合わせる
	/// </summary>
	void ApplyNodeToCursorTransform();

	/// <summary>
	/// ステージセレクトグラフ編集ImGui
	/// </summary>
	void DrawSelectGraphImGui();

	/// <summary>
	/// ノードIDからワールド座標を計算
	/// </summary>
	/// <param name="nodeId">取得するノード</param>
	/// <returns>ノードのワールド座標</returns>
	Vector3 CalcNodeWorldPos(uint32_t nodeId)const;

	/// <summary>
	/// カーソル移動更新
	/// </summary>
	void UpdateCursorMove();

	/// <summary>
	/// 2点間の方向を向くためのYaw角を計算
	/// </summary>
	/// <param name="from">開始位置</param>
	/// <param name="to">目標位置</param>
	/// <returns>Yaw角（ラジアン）</returns>
	float CalcYawToTarget(const Vector3& from, const Vector3& to) const;

	/// <summary>
	/// 指定座標からカメラ方向を向くためのヨー角を計算
	/// </summary>
	/// <param name="pos">基準位置</param>
	/// <returns>Yaw角（ラジアン）</returns>
	float CalcYawFaceCamera(const Vector3& pos)const;

	/// <summary>
	/// カメラ方向を向く更新
	/// </summary>
	void UpdateFaceYaw();

	/// <summary>
	/// ルートモデルの構築
	/// </summary>
	void BuildRoutes();

private:
	// カメラ / トランスフォーム
	Camera* camera = nullptr;
	Transform cameraTransform;

	// ステージ選択オブジェクト
	std::unique_ptr<Object3D> playerModel;
	Transform playerTransform;

	// セレクト画面１の土台
	std::unique_ptr<Object3D> stageSelectBase1;
	Transform stageSelectBase1Transform;

	// 空背景
	std::unique_ptr<Object3D> skyBack;
	Transform skyBackTransform;

	// ステージセレクトグラフ
	std::unique_ptr<StageSelectGraph> stageSelectGraph;
	uint32_t currentNodeId = 0;
	uint32_t startNodeId = 0;

	// ノードモデル群
	std::vector<std::unique_ptr<Object3D>>nodeModels_;
	std::vector<Transform>nodeTransforms_;

	// ルートモデル群
	std::vector<std::unique_ptr<Object3D>> routeModels_;
	std::vector<Transform> routeTransforms_;

	// ステージセレクトUI
	std::unique_ptr<Sprite>stageSelect_;

	// キーアイコンUI
	std::unique_ptr<Sprite> keyIcon_WASD;
	Vector2 keyIcon_WASD_Pos = { 100.0f,600.0f };

	// Escキー
	std::unique_ptr<Sprite> keyIcon_Esc;
	Vector2 keyIcon_Esc_Pos = { 100.0f,650.0f };
	// Enterキー
	std::unique_ptr<Sprite> keyIcon_Space;
	Vector2 keyIcon_Enter_Pos = { 200.0f,650.0f };

	// MoveUI
	std::unique_ptr<Sprite>moveUI_;
	Vector2 moveUI_Pos_ = { 160.0f,620.0f };
	// CheckUI
	std::unique_ptr<Sprite>checkUI_;
	Vector2 checkUI_Pos_ = { 160.0f, 500.0f };
	// BackUI
	std::unique_ptr<Sprite>backUI_;
	Vector2 backUI_Pos_ = { 160.0f, 560.0f };

	// コロン
	std::vector<std::unique_ptr<Sprite>>colonSprites_;
	// コロンの位置
	std::vector<Vector2>colonPositions_;

private:
	// ===== 移動アニメーション用 =====
	// 移動中フラグ
	bool isMoving_ = false;
	// 移動経過時間
	float moveTime_ = 0.0f;
	// 移動にかかる時間
	float moveDuration_ = 0.35f;

	// 移動開始位置
	Vector3 moveStartPos_ = {};
	// 移動目標位置
	Vector3 moveTargetPos_ = {};

	// 移動開始時のYaw角
	float moveStartYaw_ = 0.0f;
	// 移動目標のYaw角（進行方向）
	float moveTargetYaw_ = 0.0f;

	// ===== 停止後の「カメラ方向を向く」補間用 =====
	// カメラ方向を向いている最中フラグ
	bool  isFacing_ = false;
	// 向き変更の経過時間
	float faceTime_ = 0.0f;
	// 向き変更にかかる時間
	float faceDuration_ = 0.25f;
	// 向き変更開始時のYaw角
	float faceStartYaw_ = 0.0f;
	// 向き変更目標のYaw角（カメラ方向）
	float faceTargetYaw_ = 0.0f;

	// モデルの向き補正値（モデルが右向きの場合 -90度）
	const float modelYawOffset_ = -1.570796326f;

	// 一定時間たつまでは入力を受け付けない（連打防止）
	float inputLockTime_ = 0.0f;
	const float inputLockDuration_ = 2.0f;

#ifdef USE_IMGUI
	// ===== ImGui編集用変数 =====
	// 選択中ノードが変わったか��定用
	uint32_t prevEditNodeId_ = UINT32_MAX;
	// デバッグ情報表示用
	bool showDebugWindow_ = true;
	float debugMoveDistance_ = 0.0f;
	float debugStartYawDeg_ = 0.0f;
	float debugTargetYawDeg_ = 0.0f;
	float debugCurrentYawDeg_ = 0.0f;
	float debugYawChangeDeg_ = 0.0f;
	Vector3 debugDirection_ = {};

	// 編集バッファ（毎フレーム上書きされないように保持）
	int editX_ = 0;
	int editY_ = 0;
	int editStageId_ = 0;
	// デフォルトステージキー
	char editStageKey_[64] = "1-1";
	char newStageKey_[64] = "1-1";
	bool editUnlocked_ = false;

	// 隣接ノード編集用
	int editNeighUp_ = -1;
	int editNeighDown_ = -1;
	int editNeighLeft_ = -1;
	int editNeighRight_ = -1;

	// フィルタID（-1で無効）
	int filterStageId_ = -1;
	bool filterUnlockedOnly_ = false;

	// JSON表示用
	bool jsonDirty_ = true;
	char jsonBuf_[32768] = {};

	// 編集用ノードID
	uint32_t editNodeId_ = 0;

	// 追加用入力
	int newX_ = 0;
	int newY_ = 0;
	int newStageId_ = 0;
	bool newUnlocked_ = true;

	// 接続編集用（プルダウン）
	int neighborUp_ = -1;
	int neighborDown_ = -1;
	int neighborLeft_ = -1;
	int neighborRight_ = -1;
#endif

	
};