#include "Input.h"

#include <cassert>
#include <cstring>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "xinput.lib")

Input* Input::instance = nullptr;

Input* Input::GetInstance()
{
	if (instance == nullptr) {
		instance = new Input();
	}

	return instance;
}

void Input::DeleteInstance()
{
	delete instance;
	instance = nullptr;
}

void Input::Initialize(WinAPI* winAPI)
{
	this->winAPI = winAPI;

	HRESULT result;

	result = DirectInput8Create(
		winAPI->GetHInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		reinterpret_cast<void**>(directInput.GetAddressOf()),
		nullptr
	);
	assert(SUCCEEDED(result));

	//==================================================
	// キーボード
	//==================================================

	result = directInput->CreateDevice(
		GUID_SysKeyboard,
		keyboard.GetAddressOf(),
		nullptr
	);
	assert(SUCCEEDED(result));

	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	result = keyboard->SetCooperativeLevel(
		winAPI->GetHwnd(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY
	);
	assert(SUCCEEDED(result));

	//==================================================
	// マウス
	//==================================================

	result = directInput->CreateDevice(
		GUID_SysMouse,
		mouse.GetAddressOf(),
		nullptr
	);
	assert(SUCCEEDED(result));

	result = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));

	result = mouse->SetCooperativeLevel(
		winAPI->GetHwnd(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE
	);
	assert(SUCCEEDED(result));

	//==================================================
	// コントローラー
	//==================================================

	controller_.Initialize();
}

void Input::Update()
{
	//==================================================
	// 前フレーム状態を保存
	//==================================================

	std::memcpy(keyPre, key, sizeof(key));
	mousePreState = mouseState;

	//==================================================
	// キーボード更新
	//==================================================

	if (keyboard) {
		HRESULT result = keyboard->Acquire();
		result = keyboard->GetDeviceState(sizeof(key), key);

		if (FAILED(result)) {
			keyboard->Acquire();
			result = keyboard->GetDeviceState(sizeof(key), key);

			if (FAILED(result)) {
				ZeroMemory(key, sizeof(key));
			}
		}
	}

	//==================================================
	// マウス更新
	//==================================================

	if (mouse) {
		HRESULT result = mouse->Acquire();
		result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

		if (FAILED(result)) {
			mouse->Acquire();
			result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

			if (FAILED(result)) {
				ZeroMemory(&mouseState, sizeof(DIMOUSESTATE));
			}
		}
	}

	//==================================================
	// コントローラー更新
	//==================================================

	controller_.Update();
}

//==================================================
// キーボード
//==================================================

bool Input::PushKey(BYTE keyNumber)
{
	return key[keyNumber] != 0;
}

bool Input::TriggerKey(BYTE keyNumber)
{
	return key[keyNumber] != 0 && keyPre[keyNumber] == 0;
}

bool Input::ReriseKey(BYTE keyNumber)
{
	return key[keyNumber] == 0 && keyPre[keyNumber] != 0;
}

//==================================================
// マウス
//==================================================

bool Input::PushMouse(int button)
{
	if (button < 0 || button >= 4) {
		return false;
	}

	return (mouseState.rgbButtons[button] & 0x80) != 0;
}

bool Input::TriggerMouse(int button)
{
	if (button < 0 || button >= 4) {
		return false;
	}

	return (mouseState.rgbButtons[button] & 0x80) != 0 &&
		(mousePreState.rgbButtons[button] & 0x80) == 0;
}

bool Input::ReleaseMouse(int button)
{
	if (button < 0 || button >= 4) {
		return false;
	}

	return (mouseState.rgbButtons[button] & 0x80) == 0 &&
		(mousePreState.rgbButtons[button] & 0x80) != 0;
}

//==================================================
// コントローラー
//==================================================

bool Input::PushButton(int stickNo, ControllerButtonType button)
{
	return controller_.PushButton(stickNo, button);
}

bool Input::TriggerButton(int stickNo, ControllerButtonType button)
{
	return controller_.TriggerButton(stickNo, button);
}

bool Input::ReleaseButton(int stickNo, ControllerButtonType button)
{
	return controller_.ReleaseButton(stickNo, button);
}

float Input::GetTriggerValue(int stickNo, ControllerButtonType button)
{
	return controller_.GetTriggerValue(stickNo, button);
}

StickState Input::GetLeftStickState(int stickNo)
{
	return controller_.GetLeftStickState(stickNo);
}

StickState Input::GetRightStickState(int stickNo)
{
	return controller_.GetRightStickState(stickNo);
}

float Input::GetStickValue(int stickNo, ControllerValueType valueType)
{
	return controller_.GetStickValue(stickNo, valueType);
}