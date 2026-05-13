#pragma once

#define DIRECTINPUT_VERSION 0x0800

#include <Windows.h>
#include <dinput.h>
#include <wrl.h>

#include "WinAPI.h"
#include "Controller.h"

class Input
{
public:
	static Input* GetInstance();
	static void DeleteInstance();

private:
	static Input* instance;

	Input() = default;
	~Input() = default;

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

public:
	void Initialize(WinAPI* winAPI);
	void Update();

	//==================================================
	// キーボード
	//==================================================

	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);
	bool ReriseKey(BYTE keyNumber);

	//==================================================
	// マウス
	//==================================================

	bool PushMouse(int button);
	bool TriggerMouse(int button);
	bool ReleaseMouse(int button);

	LONG GetMouseMoveX() { return mouseState.lX; }
	LONG GetMouseMoveY() { return mouseState.lY; }
	LONG GetMouseWheel() { return mouseState.lZ; }

	//==================================================
	// コントローラー
	//==================================================

	Controller* GetController() { return &controller_; }

	bool PushButton(int stickNo, ControllerButtonType button);
	bool TriggerButton(int stickNo, ControllerButtonType button);
	bool ReleaseButton(int stickNo, ControllerButtonType button);

	float GetTriggerValue(int stickNo, ControllerButtonType button);

	StickState GetLeftStickState(int stickNo);
	StickState GetRightStickState(int stickNo);

	float GetStickValue(int stickNo, ControllerValueType valueType);

private:
	template <class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

private:
	ComPtr<IDirectInput8> directInput = nullptr;
	ComPtr<IDirectInputDevice8> keyboard = nullptr;
	ComPtr<IDirectInputDevice8> mouse = nullptr;

	BYTE key[256] = {};
	BYTE keyPre[256] = {};

	DIMOUSESTATE mouseState = {};
	DIMOUSESTATE mousePreState = {};

	Controller controller_;

	WinAPI* winAPI = nullptr;
};