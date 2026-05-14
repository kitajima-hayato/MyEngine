#pragma once
/// ===Include=== ///
#define DIRECTINPUT_VERSION   0x0800   // DirectInputのバージョン指定
#include <dinput.h>
#include <XInput.h>
#include <Windows.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

/// ===マウスのキーを指定する為の列挙型=== ///
enum class MouseButtonType {
	Left,       // 左クリック
	Right,      // 右クリック
	Scroll,     // スクロールボタン
	Button1,    // ボタン1
};

/// ===コントローラーのキーを指定するための列挙型=== ///
enum class ControllerButtonType {
	A,          // Aボタン
	B,          // Bボタン
	X,          // Xボタン
	Y,          // Yボタン
	RB,         // RBボタン
	RT,         // RTボタン
	LB,         // LBボタン
	LT,         // LTボタン
	LeftStick,  // LeftStick押し込み
	RightStick, // RightStick押し込み
	DPadUP,     // 十字キー上 
	DPadDOWN,   // 十字キー下
	DPadLEFT,   // 十字キー左
	DPadRIGHT,  // 十字キー右
	Select,     // セレクトボタン
	Start,      // スタートボタン
};

/// ===コントローラーのスティックの状況をえるための列挙型=== ///
enum class ControllerValueType {
	LX, // 左スティックX軸
	LY, // 左スティックY軸
	RX, // 右スティックX軸
	RY, // 右スティックY軸
};

/// ===コントローラーの処理を管理するための構造体=== ///
struct StickState {
	float x;
	float y;
};