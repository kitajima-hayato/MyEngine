#include "Controller.h"

#include <algorithm>
#include <cmath>

void Controller::Initialize()
{
	ZeroMemory(currentState_, sizeof(currentState_));
	ZeroMemory(previousState_, sizeof(previousState_));
	ZeroMemory(currentDIState_, sizeof(currentDIState_));
	ZeroMemory(previousDIState_, sizeof(previousDIState_));

	buttonMapping_ = {
		{ ControllerButtonType::A,          { static_cast<WORD>(XINPUT_GAMEPAD_A),              0  } },
		{ ControllerButtonType::B,          { static_cast<WORD>(XINPUT_GAMEPAD_B),              1  } },
		{ ControllerButtonType::X,          { static_cast<WORD>(XINPUT_GAMEPAD_X),              2  } },
		{ ControllerButtonType::Y,          { static_cast<WORD>(XINPUT_GAMEPAD_Y),              3  } },
		{ ControllerButtonType::LB,         { static_cast<WORD>(XINPUT_GAMEPAD_LEFT_SHOULDER),  4  } },
		{ ControllerButtonType::RB,         { static_cast<WORD>(XINPUT_GAMEPAD_RIGHT_SHOULDER), 5  } },
		{ ControllerButtonType::LeftStick,  { static_cast<WORD>(XINPUT_GAMEPAD_LEFT_THUMB),     6  } },
		{ ControllerButtonType::RightStick, { static_cast<WORD>(XINPUT_GAMEPAD_RIGHT_THUMB),    7  } },
		{ ControllerButtonType::Select,     { static_cast<WORD>(XINPUT_GAMEPAD_BACK),           8  } },
		{ ControllerButtonType::Start,      { static_cast<WORD>(XINPUT_GAMEPAD_START),          9  } },
		{ ControllerButtonType::DPadUP,     { static_cast<WORD>(XINPUT_GAMEPAD_DPAD_UP),        10 } },
		{ ControllerButtonType::DPadRIGHT,  { static_cast<WORD>(XINPUT_GAMEPAD_DPAD_RIGHT),     11 } },
		{ ControllerButtonType::DPadDOWN,   { static_cast<WORD>(XINPUT_GAMEPAD_DPAD_DOWN),      12 } },
		{ ControllerButtonType::DPadLEFT,   { static_cast<WORD>(XINPUT_GAMEPAD_DPAD_LEFT),      13 } },
		{ ControllerButtonType::LT,         { 0, -1 } },
		{ ControllerButtonType::RT,         { 0, -1 } },
	};
}

void Controller::Update()
{
	for (int i = 0; i < XUSER_MAX_COUNT; ++i) {
		previousState_[i] = currentState_[i];

		ZeroMemory(&currentState_[i], sizeof(XINPUT_STATE));

		DWORD result = XInputGetState(i, &currentState_[i]);

		if (result != ERROR_SUCCESS) {
			ZeroMemory(&currentState_[i], sizeof(XINPUT_STATE));
		}

		previousDIState_[i] = currentDIState_[i];
		ZeroMemory(&currentDIState_[i], sizeof(DIJOYSTATE2));
	}
}

bool Controller::GetJoystickState(int stickNo, XINPUT_STATE& out) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	out = currentState_[stickNo];
	return true;
}

bool Controller::GetJoystickStatePrevious(int stickNo, XINPUT_STATE& out) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	out = previousState_[stickNo];
	return true;
}

bool Controller::GetJoystickState(int stickNo, DIJOYSTATE2& out) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	out = currentDIState_[stickNo];
	return true;
}

bool Controller::GetJoystickStatePrevious(int stickNo, DIJOYSTATE2& out) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	out = previousDIState_[stickNo];
	return true;
}

bool Controller::PushButton(int stickNo, ControllerButtonType button) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	if (button == ControllerButtonType::LT) {
		return currentState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD;
	}

	if (button == ControllerButtonType::RT) {
		return currentState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD;
	}

	auto [xInputButton, dInputButton] = ConvertToButton(button);

	if (xInputButton != 0) {
		return (currentState_[stickNo].Gamepad.wButtons & xInputButton) != 0;
	}

	return false;
}

bool Controller::TriggerButton(int stickNo, ControllerButtonType button) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	if (button == ControllerButtonType::LT) {
		return currentState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD &&
			previousState_[stickNo].Gamepad.bLeftTrigger <= TRIGGER_THRESHOLD;
	}

	if (button == ControllerButtonType::RT) {
		return currentState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD &&
			previousState_[stickNo].Gamepad.bRightTrigger <= TRIGGER_THRESHOLD;
	}

	auto [xInputButton, dInputButton] = ConvertToButton(button);

	if (xInputButton != 0) {
		return (currentState_[stickNo].Gamepad.wButtons & xInputButton) != 0 &&
			(previousState_[stickNo].Gamepad.wButtons & xInputButton) == 0;
	}

	return false;
}

bool Controller::ReleaseButton(int stickNo, ControllerButtonType button) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	if (button == ControllerButtonType::LT) {
		return currentState_[stickNo].Gamepad.bLeftTrigger <= TRIGGER_THRESHOLD &&
			previousState_[stickNo].Gamepad.bLeftTrigger > TRIGGER_THRESHOLD;
	}

	if (button == ControllerButtonType::RT) {
		return currentState_[stickNo].Gamepad.bRightTrigger <= TRIGGER_THRESHOLD &&
			previousState_[stickNo].Gamepad.bRightTrigger > TRIGGER_THRESHOLD;
	}

	auto [xInputButton, dInputButton] = ConvertToButton(button);

	if (xInputButton != 0) {
		return (currentState_[stickNo].Gamepad.wButtons & xInputButton) == 0 &&
			(previousState_[stickNo].Gamepad.wButtons & xInputButton) != 0;
	}

	return false;
}

float Controller::GetTriggerValue(int stickNo, ControllerButtonType button) const
{
	if (!IsValidStickNo(stickNo)) {
		return 0.0f;
	}

	switch (button) {
	case ControllerButtonType::LT:
		return static_cast<float>(currentState_[stickNo].Gamepad.bLeftTrigger) / 255.0f;

	case ControllerButtonType::RT:
		return static_cast<float>(currentState_[stickNo].Gamepad.bRightTrigger) / 255.0f;

	case ControllerButtonType::LB:
		return PushButton(stickNo, ControllerButtonType::LB) ? 1.0f : 0.0f;

	case ControllerButtonType::RB:
		return PushButton(stickNo, ControllerButtonType::RB) ? 1.0f : 0.0f;

	default:
		return 0.0f;
	}
}

StickState Controller::GetLeftStickState(int stickNo) const
{
	StickState state{ 0.0f, 0.0f };

	if (!IsValidStickNo(stickNo)) {
		return state;
	}

	state.x = ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbLX));
	state.y = ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbLY));

	return state;
}

StickState Controller::GetRightStickState(int stickNo) const
{
	StickState state{ 0.0f, 0.0f };

	if (!IsValidStickNo(stickNo)) {
		return state;
	}

	state.x = ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbRX));
	state.y = ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbRY));

	return state;
}

float Controller::GetStickValue(int stickNo, ControllerValueType valueType) const
{
	if (!IsValidStickNo(stickNo)) {
		return 0.0f;
	}

	switch (valueType) {
	case ControllerValueType::LX:
		return ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbLX));

	case ControllerValueType::LY:
		return ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbLY));

	case ControllerValueType::RX:
		return ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbRX));

	case ControllerValueType::RY:
		return ApplyDeadZone(NormalizeStickValue(currentState_[stickNo].Gamepad.sThumbRY));

	default:
		return 0.0f;
	}
}

StickState Controller::GetLeftStickStatePrevious(int stickNo) const
{
	StickState state{ 0.0f, 0.0f };

	if (!IsValidStickNo(stickNo)) {
		return state;
	}

	state.x = ApplyDeadZone(NormalizeStickValue(previousState_[stickNo].Gamepad.sThumbLX));
	state.y = ApplyDeadZone(NormalizeStickValue(previousState_[stickNo].Gamepad.sThumbLY));

	return state;
}

StickState Controller::GetRightStickStatePrevious(int stickNo) const
{
	StickState state{ 0.0f, 0.0f };

	if (!IsValidStickNo(stickNo)) {
		return state;
	}

	state.x = ApplyDeadZone(NormalizeStickValue(previousState_[stickNo].Gamepad.sThumbRX));
	state.y = ApplyDeadZone(NormalizeStickValue(previousState_[stickNo].Gamepad.sThumbRY));

	return state;
}

bool Controller::FlickLeftStick(int stickNo, float threshold) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	StickState prev = GetLeftStickStatePrevious(stickNo);
	StickState curr = GetLeftStickState(stickNo);

	float prevLen = std::sqrt(prev.x * prev.x + prev.y * prev.y);
	float currLen = std::sqrt(curr.x * curr.x + curr.y * curr.y);

	return prevLen < DEADZONE && currLen >= threshold;
}

bool Controller::FlickRightStick(int stickNo, float threshold) const
{
	if (!IsValidStickNo(stickNo)) {
		return false;
	}

	StickState prev = GetRightStickStatePrevious(stickNo);
	StickState curr = GetRightStickState(stickNo);

	float prevLen = std::sqrt(prev.x * prev.x + prev.y * prev.y);
	float currLen = std::sqrt(curr.x * curr.x + curr.y * curr.y);

	return prevLen < DEADZONE && currLen >= threshold;
}

bool Controller::IsValidStickNo(int stickNo) const
{
	return stickNo >= 0 && stickNo < XUSER_MAX_COUNT;
}

std::pair<WORD, int> Controller::ConvertToButton(ControllerButtonType button) const
{
	auto it = buttonMapping_.find(button);

	if (it != buttonMapping_.end()) {
		return it->second;
	}

	return { 0, -1 };
}

float Controller::NormalizeStickValue(SHORT value) const
{
	float result = static_cast<float>(value) / NORMALIZE_RANGE;
	return std::clamp(result, -1.0f, 1.0f);
}

float Controller::ApplyDeadZone(float value) const
{
	if (std::abs(value) < DEADZONE) {
		return 0.0f;
	}

	return value;
}