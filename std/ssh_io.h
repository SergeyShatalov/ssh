
/*
*	�����:		������� �. �.
*	�������:	�����������, 19 ���� 2015, 1:26
*	�����������:1 ������� 2015 - ����� �������� � ���� xinput1_3.dll
*	��������:	������ ��� �������� �����/������ � ��������
*/

#pragma once

#include "ssh_singl.h"
#include "ssh_map.h"
#include <Xinput.h>

#define IS_KEY(vk)			keyboard->is_keyPressed(vk)
#define GET_KEY				keyboard->getKey()
#define GET_SYM				keyboard->getSym()

#define IS_MCAPT(ptr)		mouse->is_capture(ptr)
#define IS_KCAPT(ptr)		keyboard->is_capture(ptr)


namespace ssh
{
	class SSH Mouse
	{
		friend class Singlton<Mouse>;
	public:
		enum Flags
		{
			msDclickR = 0,
			msDclickL = 1,
			msDclickM = 2,
			msVisible = 3
		};
		// ���������, ����������� ��������� ����
		struct MOUSE
		{
			MOUSE(const Pts<ssh_u>& hot, const Bar<ssh_u>& bar) : hot(hot), bar(bar) {}
			// ������� �����
			Pts<ssh_u> hot;
			// �������
			Bar<ssh_u> bar;
		};
		// ���������� �������
		void set_pos(const Pts<ssh_u>& pt) { ::SetCursorPos((int)pt.x, (int)pt.y); position = pt; }
		// ��������
		void add(const String& name, const Pts<ssh_u>& hot, const Bar<ssh_u>& bar) { mouses[name] = new MOUSE(hot, bar); }
		// �������
		void remove(const String& name) { if(current == mouses[name]) current = nullptr; mouses.remove(name); }
		// ������� �������
		void set(const String& name) { current = mouses[name]; }
		// ������� �������
		MOUSE* get_current() const { return current; }
		// ������� ������� ���������
		bool is_visible() const { return status.testBit(msVisible); }
		// ��������� �������� ���������
		void setVisible(bool use) { status.setBit(msVisible, use); }
		// ��������� ������� ������
		void setWheelStatus(long w) { wheel = w; }
		// ��������� ������� �������� �����
		void setDoubleClickStatus(bool dclickM, bool dclickR, bool dclickL) { status.setBit(msDclickR, dclickR); status.setBit(msDclickL, dclickL); status.setBit(msDclickM, dclickM); }
		// ��������� ������� ����������
		void update(const Pts<ssh_u>& pt, long k) { old_position = position; position = pt; key = k; if(!(k & (MK_LBUTTON | MK_MBUTTON))) hcapture = 0; }
		// ������� ������� ������� ������ Ctrl
		bool is_keyControl() const { return ((key & MK_CONTROL) != 0); }
		// ������� ������� ������� ������ Shift
		bool is_keyShift() const { return ((key & MK_SHIFT) != 0); }
		// ������� ������� ������� ����� ������
		bool is_keyLeft(ssh_u h) const { return ((hcapture == 0 ? 1 : h == hcapture) && ((key & MK_LBUTTON) != 0)); }
		// ������� ������� ������� ������ ������
		bool is_keyRight() const { return ((key & MK_RBUTTON) != 0); }
		// ������� ������� ������� ������� ������
		bool is_keyMiddle(ssh_u h) const { return ((hcapture == 0 ? 1 : h == hcapture) && ((key & MK_MBUTTON) != 0)); }
		// ������� ������� ������� �������� �����
		bool is_doubleClickRight() const { return status.testBit(msDclickR); }
		// ������� ������� ������ �������� �����
		bool is_doubleClickLeft() const { return status.testBit(msDclickL); }
		// ������� ������� �������� �������� �����
		bool is_doubleClickMiddle() const { return status.testBit(msDclickM); }
		// ���������� ������
		void set_capture(ssh_u h) { hcapture = h; }
		// ��������� �� ������
		bool is_capture(ssh_u h) const { return (h == hcapture); }
		// ������� ��������� ������
		long get_wheelState() { long tmp(wheel); wheel = 0; return tmp; }
		// ������� ��������� ������
		long get_keyState() const { return key; }
		// ������� �������� �������
		const Pts<ssh_u>& get_pos() const { return position; }
		// ������� ������� ����� ����� � ������ ��������
		Pts<ssh_u> get_sub_pos() const { return (position - old_position); }
		// ������� ������ �������
		Pts<ssh_u> get_old_pos() const { return old_position; }
		// ������� �������
		Pts<ssh_u> get_hot_pos() const { return Pts<ssh_u>(current ? position + current->hot : position); }
	protected:
		// �����������
		Mouse() : wheel(0), key(0), status(8), current(nullptr), hcapture(0) {}
		// ����������
		virtual ~Mouse() {}
		// �������
		Pts<ssh_u> position;
		// ������ �������
		Pts<ssh_u> old_position;
		// �������� ������
		long wheel;
		// ������ ������
		long key;
		// ������
		Bits<WORD> status;
		// ������� ����
		MOUSE* current;
		// ��� ����
		Map<MOUSE*, String> mouses{ID_STK_MOUSE};
		// ����� �������
		ssh_u hcapture;
		// ������ ��������
		static ssh_u const singl_idx = SSH_SINGL_MOUSE;
	};

	class SSH Keyboard
	{
		friend class Singlton<Keyboard>;
	public:
		// ��������� ������� ������
		void setKey(BYTE code, ssh_l repeat, bool use)
		{
			keys[code] = use;
			cur_key = (use ? code : 0);
			cur_char = 0;
			rep = (use ? repeat : 0);
		}
		// ��������� ������� ������
		void setSym(ssh_b code_char, ssh_u repeat) { cur_char = code_char; }
		// ������� ������� ���������� ������
		ssh_b getSym() const { return cur_char; }
		// ������� ������� ������
		ssh_b getKey() { rep--; return (rep >= 0 ? cur_key : 0); }
		// ������� ������� �������
		bool is_keyPressed(ssh_b code) const { return keys[code]; }
		// ���������� ������
		void set_capture(ssh_u h) { hcapture = h; }
		// ��������� �� ������
		bool is_capture(ssh_u h) const { return (h == hcapture || hcapture == 0); }
	protected:
		// �����������
		Keyboard() : rep(0), cur_key(0), cur_char(0), hcapture(0) { SSH_MEMZERO(keys, sizeof(keys)); }
		// ����������
		virtual ~Keyboard() {}
		// ������ �������� ���� ������
		bool keys[256];
		// ������� ������
		BYTE cur_char;
		// ������� ������
		BYTE cur_key;
		// ������ �������
		ssh_l rep;
		// ����� �������
		ssh_u hcapture;
		// ������ ��������
		static ssh_u const singl_idx = SSH_SINGL_KEYBOARD;
	};

	#define MAX_CONTROLLERS	4

	class SSH Gamepad
	{
		friend class Singlton<Gamepad>;
	public:
		enum Side : int
		{
			left	= 1,
			right	= 2
		};
		enum Buttons
		{
			cross_up	= XINPUT_GAMEPAD_DPAD_UP,
			cross_down	= XINPUT_GAMEPAD_DPAD_DOWN,
			cross_left	= XINPUT_GAMEPAD_DPAD_LEFT,
			cross_right = XINPUT_GAMEPAD_DPAD_RIGHT,
			left_turn	= XINPUT_GAMEPAD_LEFT_SHOULDER,
			right_turn	= XINPUT_GAMEPAD_RIGHT_SHOULDER,
			start		= XINPUT_GAMEPAD_START,
			escape		= XINPUT_GAMEPAD_BACK,
			a			= XINPUT_GAMEPAD_A,
			b			= XINPUT_GAMEPAD_B,
			x			= XINPUT_GAMEPAD_X,
			y			= XINPUT_GAMEPAD_Y,
			left_stick	= XINPUT_GAMEPAD_LEFT_THUMB,
			right_stick = XINPUT_GAMEPAD_RIGHT_THUMB
		};
		struct GAMEPAD
		{
			GAMEPAD() { SSH_MEMSET(this, 0, sizeof(GAMEPAD)); }
			ssh_w wButtons;
			ssh_b bLeftTrigger;
			ssh_b bRightTrigger;
			short sThumbLX;
			short sThumbLY;
			short sThumbRX;
			short sThumbRY;
			// ����������� ����������
			XINPUT_CAPABILITIES caps;
			// ��������
			bool is_connected;
			bool is_inserted;
			bool is_removed;
			// �������� ����� � ��������� [-1,+1]
			float fThumbRX;
			float fThumbRY;
			float fThumbLX;
			float fThumbLY;
			// Records which buttons were pressed this frame.
			// These are only set on the first frame that the button is pressed
			ssh_w wPressedButtons;
			bool is_pressedLeftTrigger;
			bool is_pressedRightTrigger;
			// ���������� ��������� ������
			ssh_w wLastButtons;
			bool is_lastLeftTrigger;
			bool is_lastRightTrigger;
		};
		// �������� �� ����� ������� ������
		bool is_once_pressed(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wPressedButtons & but) != 0) : false); }
		// ������� ������� ������� ������
		bool is_pressed(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wButtons & but) != 0) : false); }
		// �������� �� ������� ������
		bool is_released(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wLastButtons & but) != 0) : false); }
		// �������� �� ������� �������
		bool is_trigger(ssh_d idx, Side side) const;
		// ������� ��������
		bool is_connect(ssh_d idx) const { return _pad[idx].is_connected; }
		// ������� �����������
		bool is_removed(ssh_d idx) const { return _pad[idx].is_removed; }
		// ������� ������ �����������
		bool is_inserted(ssh_d idx) const { return _pad[idx].is_inserted; }
		// ������� �������� ������
		Range<float> get_stick(ssh_d idx, Side side) const;
		// ���������� ��������� � �������� �� �������
		virtual void update();
		// ��������� �������� ��������
		virtual void vibration(ssh_d idx, Side side, ssh_w speed) const;
	protected:
		Gamepad();
		virtual ~Gamepad();
		GAMEPAD _pad[MAX_CONTROLLERS];
		// ������ ��������
		static ssh_u const singl_idx = SSH_SINGL_GAMEPAD;
	};
	#define mouse		Singlton<Mouse>::Instance()
	#define keyboard	Singlton<Keyboard>::Instance()
	#define gamepad		Singlton<Gamepad>::Instance()
}
