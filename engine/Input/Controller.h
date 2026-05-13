#pragma once

#include "InputData.h"

#include <cstdint>
#include <map>
#include <utility>

class Controller {
public:
	Controller() = default;
	~Controller() = default;

	void Initialize();
	void Update();

	bool GetJoystickState(int stickNo, XINPUT_STATE& out) const;
	bool GetJoystickStatePrevious(int stickNo, XINPUT_STATE& out) const;

	bool GetJoystickState(int stickNo, DIJOYSTATE2& out) const;
	bool GetJoystickStatePrevious(int stickNo, DIJOYSTATE2& out) const;

	bool PushButton(int stickNo, ControllerButtonType button) const;
	bool TriggerButton(int stickNo, ControllerButtonType button) const;
	bool ReleaseButton(int stickNo, ControllerButtonType button) const;

	float GetTriggerValue(int stickNo, ControllerButtonType button) const;

	StickState GetLeftStickState(int stickNo) const;
	StickState GetRightStickState(int stickNo) const;

	float GetStickValue(int stickNo, ControllerValueType valueType) const;

	StickState GetLeftStickStatePrevious(int stickNo) const;
	StickState GetRightStickStatePrevious(int stickNo) const;

	bool FlickLeftStick(int stickNo, float threshold = 0.7f) const;
	bool FlickRightStick(int stickNo, float threshold = 0.7f) const;

private:
	static constexpr float NORMALIZE_RANGE = 32768.0f;
	static constexpr float TRIGGER_THRESHOLD = 128.0f;
	static constexpr float DEADZONE = 0.15f;

private:
	XINPUT_STATE currentState_[XUSER_MAX_COUNT]{};
	XINPUT_STATE previousState_[XUSER_MAX_COUNT]{};

	DIJOYSTATE2 currentDIState_[XUSER_MAX_COUNT]{};
	DIJOYSTATE2 previousDIState_[XUSER_MAX_COUNT]{};

	std::map<ControllerButtonType, std::pair<WORD, int>> buttonMapping_;

private:
	bool IsValidStickNo(int stickNo) const;
	std::pair<WORD, int> ConvertToButton(ControllerButtonType button) const;

	float NormalizeStickValue(SHORT value) const;
	float ApplyDeadZone(float value) const;
};