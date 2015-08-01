
#include "stdafx.h"
#include "ssh_io.h"

namespace ssh
{
	typedef void (CALLBACK* _ssh_xenable)(int is);
	typedef ssh_d(CALLBACK* _ssh_xgstate)(ssh_d idx, XINPUT_STATE* state);
	typedef ssh_d(CALLBACK* _ssh_xsstate)(ssh_d idx, XINPUT_VIBRATION* state);
	typedef ssh_d(CALLBACK* _ssh_xcaps)(ssh_d idx, ssh_d flags, XINPUT_CAPABILITIES* caps);
	
	static _ssh_xenable _enable((_ssh_xenable)hlp->get_procedure(L"xinput1_3.dll", "XInputEnable", L''));
	static _ssh_xgstate _gstate((_ssh_xgstate)hlp->get_procedure(L"xinput1_3.dll", "XInputGetState", L''));
	static _ssh_xsstate _sstate((_ssh_xsstate)hlp->get_procedure(L"xinput1_3.dll", "XInputSetState", L''));
	static _ssh_xcaps _caps((_ssh_xcaps)hlp->get_procedure(L"xinput1_3.dll", "XInputGetCapabilities", L''));

	Gamepad::Gamepad()
	{
		if(_enable) _enable(true);
	}
	
	Gamepad::~Gamepad()
	{
		if(_enable) _enable(false);
	}

	void Gamepad::update()
	{
		for(ssh_d idx = 0; idx < MAX_CONTROLLERS; idx++)
		{
			XINPUT_STATE state;
			GAMEPAD* pad(&_pad[idx]);

			bool bWasConnected(pad->is_connected);
			HRESULT hr(_gstate ? _gstate(idx, &state) : ERROR_INVALID_FUNCTION);
			pad->is_connected = (hr == ERROR_SUCCESS);
			pad->is_removed = (bWasConnected && !pad->is_connected);
			pad->is_inserted = (!bWasConnected && pad->is_connected);
			if(!pad->is_connected) continue;
			if(pad->is_inserted)
			{
				SSH_MEMSET(pad, 0, sizeof(GAMEPAD));
				pad->is_connected = true;
				pad->is_inserted = true;
				if(_caps) _caps(idx, XINPUT_DEVTYPE_GAMEPAD, &pad->caps);
			}
			SSH_MEMCPY(pad, &state.Gamepad, sizeof(XINPUT_GAMEPAD));
			// проверка на deadzone
			if((pad->sThumbLX < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad->sThumbLX > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) && (pad->sThumbLY < XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE && pad->sThumbLY > -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE))
			{
				pad->sThumbLX = 0;
				pad->sThumbLY = 0;
			}
			if((pad->sThumbRX < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad->sThumbRX > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE) && (pad->sThumbRY < XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE && pad->sThumbRY > -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE))
			{
				pad->sThumbRX = 0;
				pad->sThumbRY = 0;
			}
			pad->fThumbLX = pad->sThumbLX / 32767.0f;
			pad->fThumbLY = pad->sThumbLY / 32767.0f;
			pad->fThumbRX = pad->sThumbRX / 32767.0f;
			pad->fThumbRY = pad->sThumbRY / 32767.0f;

			pad->wPressedButtons = (pad->wLastButtons ^ pad->wButtons) & pad->wButtons;
			pad->wLastButtons = pad->wButtons;
			// признак левый триггер был нажат или отпущен
			bool bPressed = (pad->bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			pad->is_pressedLeftTrigger = (bPressed) ? !pad->is_lastLeftTrigger : false;
			pad->is_lastLeftTrigger = bPressed;
			// признак правый триггер был нажат или отпущен
			bPressed = (pad->bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
			pad->is_pressedRightTrigger = (bPressed) ? !pad->is_lastRightTrigger : false;
			pad->is_lastRightTrigger = bPressed;
		}
	}

	void Gamepad::vibration(ssh_d idx, Side side, ssh_w speed) const
	{
		if(_pad[idx].is_connected && _sstate)
		{
			XINPUT_VIBRATION vibration;
			vibration.wLeftMotorSpeed = (side & left ? speed : 0);
			vibration.wRightMotorSpeed = (side & right ? speed : 0);;
			_sstate(idx, &vibration);
		}
	}

	Range<float> Gamepad::get_stick(ssh_d idx, Side side) const
	{
		if(_pad[idx].is_connected)
		{
			if((side & left)) return Range<float>(-_pad[idx].fThumbLX, -_pad[idx].fThumbLY);
			if((side & right)) return Range<float>(-_pad[idx].fThumbRX, _pad[idx].fThumbRY);
		}
		return Range<float>();
	}

	bool Gamepad::is_trigger(ssh_d idx, Side side) const
	{
		if(_pad[idx].is_connected)
		{
			if((side & left)) return _pad[idx].is_pressedLeftTrigger;
			if((side & right)) return _pad[idx].is_pressedRightTrigger;
		}
		return false;
	}
}

