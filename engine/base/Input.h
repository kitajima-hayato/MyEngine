#pragma once
#define DIRECTINPUT_VERSION		0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include <stdint.h>
#include <Windows.h>
#include <wrl.h>
#include "WinAPI.h"

/// <summary>
/// 入力クラス
/// </summary>
/// <remarks>
/// DirectInputを使用してキーボード入力を管理する
/// </remarks>
class Input
{
public:
	/// <summary>
	/// シングルトンインスタンスの取得
	/// </summary>
	/// <returns></returns>
	static Input* GetInstance();
	/// <summary>
	/// シングルトンインスタンスの解放
	/// </summary>
	static void DeleteInstance();
private:
	// シングルトンインスタンス
	static Input* instance;
	// コンストラクタ
	Input() = default;
	// デストラクタ
	~Input() = default;
	// コピーコンストラクタ・代入演算子削除
	Input(Input&) = delete;
	Input& operator=(Input&) = delete;

public:

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="winAPI"></param>
	void Initialize(WinAPI*winAPI);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	///特定のキーが話された瞬間を判定
	/// </summary>
	/// <param name="keyNumber"></param>
	bool ReriseKey(BYTE keyNumber);

	/// <summary>
	///キーが押されているか確認
	/// </summary>
	/// <param name="keyNumber"></param>
	bool PushKey(BYTE keyNumber);
	/// <summary>
	/// キーが押された瞬間を判定
	/// </summary>
	/// <param name="keyNumber"></param>
	/// <returns></returns>
	bool TriggerKey(BYTE keyNumber);

	// マウス追加
	bool PushMouse(int button);
	bool TriggerMouse(int button);
	bool ReleaseMouse(int button);

	LONG GetMouseMoveX() const { return mouseState.lX; }
	LONG GetMouseMoveY() const { return mouseState.lY; }
	LONG GetMouseWheel() const { return mouseState.lZ; }

	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
private:

	ComPtr<IDirectInputDevice8> keyboard;
	ComPtr<IDirectInputDevice8> mouse;
	ComPtr<IDirectInput8>directInput = nullptr;
	BYTE key[256] = {};
	BYTE keyPre[256] = {};

	// マウス追加
	DIMOUSESTATE mouseState = {};       
	DIMOUSESTATE mousePreState = {};

	//WindowsAPI
	WinAPI* winAPI = nullptr;

	
};

