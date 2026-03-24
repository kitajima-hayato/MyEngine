#include "Input.h"
#include <cassert>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

Input* Input::instance = nullptr;
Input* Input::GetInstance()
{
	if (instance == nullptr)
	{
		instance = new Input();
	}
	return instance;
}

void Input::DeleteInstance()
{
	if (instance != nullptr)
	{
		delete instance;
		instance = nullptr;
	}
}

void Input::Initialize(WinAPI *winAPI)
{
	//借りてきたWinAPIのインスタンスを記録
	this->winAPI = winAPI;
	//DirectInputの初期化
	HRESULT result = DirectInput8Create(
		winAPI->GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
	//キーボードデバイスの作成
	result = directInput->CreateDevice(GUID_SysKeyboard, &keyboard, NULL);
	assert(SUCCEEDED(result));

	//入力データ形式のセット
	result = keyboard->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(result));

	//排他制御のレベルのセット
	result = keyboard->SetCooperativeLevel(winAPI->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));

	// マウスデバイスの作成
	result = directInput->CreateDevice(GUID_SysMouse, &mouse, NULL);
	assert(SUCCEEDED(result));
	// 入力データ形式のセット
	result = mouse->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(result));
	// 排他制御のレベルのセット
	result = mouse->SetCooperativeLevel(
		winAPI->GetHwnd(),
		DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	HRESULT result;
	//前回のキー入力を保存
	memcpy(keyPre, key, sizeof(key));
	mousePreState = mouseState;
	
	////全キーの入力状態を取得する
	result = keyboard->Acquire();
	result = keyboard->GetDeviceState(sizeof(key), key);

	// マウス取得
	result = mouse->Acquire();
	result = mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);

	if (TriggerKey(DIK_1)) {
		OutputDebugStringA("Hit_1\n");
	}
}
bool Input::ReriseKey(BYTE keyNumber)
{
	// 前フレームで押されていて、今フレームで押されていない場合
	return (keyPre[keyNumber] != 0) && (key[keyNumber] == 0);
	return false;
}
bool Input::PushKey(BYTE keyNumber)
{
	//指定キーを押していればtrueを返す
	 // 今フレームでキーが押されている場合
	return key[keyNumber] != 0;
}
bool Input::TriggerKey(BYTE keyNumber)
{
	// 前フレームで押されておらず、今フレームで押されている場合
	return (keyPre[keyNumber] == 0) && (key[keyNumber] != 0);
}

bool Input::PushMouse(int button)
{
	return (mouseState.rgbButtons[button] & 0x80) != 0;
}

bool Input::TriggerMouse(int button)
{
	return !(mousePreState.rgbButtons[button] & 0x80) &&
		(mouseState.rgbButtons[button] & 0x80);
}

bool Input::ReleaseMouse(int button)
{
	return  (mousePreState.rgbButtons[button] & 0x80) &&
		!(mouseState.rgbButtons[button] & 0x80);
}