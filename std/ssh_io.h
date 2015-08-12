
/*
*	Автор:		Шаталов С. В.
*	Создано:	Владикавказ, 19 июля 2015, 1:26
*	Модификация:1 августа 2015 - убрал привязку к либе xinput1_3.dll
*	Описание:	Классы для операций ввода/вывода и геймпада
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
		// структура, описывающая указатель мыши
		struct MOUSE
		{
			MOUSE(const Pts<ssh_u>& hot, const Bar<ssh_u>& bar) : hot(hot), bar(bar) {}
			// горячая точка
			Pts<ssh_u> hot;
			// область
			Bar<ssh_u> bar;
		};
		// установить позицию
		void set_pos(const Pts<ssh_u>& pt) { ::SetCursorPos((int)pt.x, (int)pt.y); position = pt; }
		// добавить
		void add(const String& name, const Pts<ssh_u>& hot, const Bar<ssh_u>& bar) { mouses[name] = new MOUSE(hot, bar); }
		// удалить
		void remove(const String& name) { if(current == mouses[name]) current = nullptr; mouses.remove(name); }
		// сделать текущей
		void set(const String& name) { current = mouses[name]; }
		// вернуть текущую
		MOUSE* get_current() const { return current; }
		// вернуть признак видимости
		bool is_visible() const { return status.testBit(msVisible); }
		// установка признака видимости
		void setVisible(bool use) { status.setBit(msVisible, use); }
		// установка статуса колеса
		void setWheelStatus(long w) { wheel = w; }
		// установка статуса двойного клика
		void setDoubleClickStatus(bool dclickM, bool dclickR, bool dclickL) { status.setBit(msDclickR, dclickR); status.setBit(msDclickL, dclickL); status.setBit(msDclickM, dclickM); }
		// установка текущих параметров
		void update(const Pts<ssh_u>& pt, long k) { old_position = position; position = pt; key = k; if(!(k & (MK_LBUTTON | MK_MBUTTON))) hcapture = 0; }
		// вернуть признак нажатия кнопки Ctrl
		bool is_keyControl() const { return ((key & MK_CONTROL) != 0); }
		// вернуть признак нажатия кнопки Shift
		bool is_keyShift() const { return ((key & MK_SHIFT) != 0); }
		// вернуть признак нажатия левой кнопки
		bool is_keyLeft(ssh_u h) const { return ((hcapture == 0 ? 1 : h == hcapture) && ((key & MK_LBUTTON) != 0)); }
		// вернуть признак нажатия правой кнопки
		bool is_keyRight() const { return ((key & MK_RBUTTON) != 0); }
		// вернуть признак нажатия средней кнопки
		bool is_keyMiddle(ssh_u h) const { return ((hcapture == 0 ? 1 : h == hcapture) && ((key & MK_MBUTTON) != 0)); }
		// вернуть признак правого двойного клика
		bool is_doubleClickRight() const { return status.testBit(msDclickR); }
		// вернуть признак левого двойного клика
		bool is_doubleClickLeft() const { return status.testBit(msDclickL); }
		// вернуть признак среднего двойного клика
		bool is_doubleClickMiddle() const { return status.testBit(msDclickM); }
		// установить захват
		void set_capture(ssh_u h) { hcapture = h; }
		// проверить на захват
		bool is_capture(ssh_u h) const { return (h == hcapture); }
		// вернуть состояние колеса
		long get_wheelState() { long tmp(wheel); wheel = 0; return tmp; }
		// вернуть состояние кнопок
		long get_keyState() const { return key; }
		// вернуть реальную позицию
		const Pts<ssh_u>& get_pos() const { return position; }
		// вернуть разницу между новой и старой позицией
		Pts<ssh_u> get_sub_pos() const { return (position - old_position); }
		// вернуть старую позицию
		Pts<ssh_u> get_old_pos() const { return old_position; }
		// вернуть позицию
		Pts<ssh_u> get_hot_pos() const { return Pts<ssh_u>(current ? position + current->hot : position); }
	protected:
		// конструктор
		Mouse() : wheel(0), key(0), status(8), current(nullptr), hcapture(0) {}
		// деструктор
		virtual ~Mouse() {}
		// позиция
		Pts<ssh_u> position;
		// старая позиция
		Pts<ssh_u> old_position;
		// значение колеса
		long wheel;
		// статус кнопок
		long key;
		// статус
		Bits<WORD> status;
		// текущая мыщь
		MOUSE* current;
		// все мыши
		Map<MOUSE*, String> mouses{ID_STK_MOUSE};
		// хэндл захвата
		ssh_u hcapture;
		// индекс синглота
		static ssh_u const singl_idx = SSH_SINGL_MOUSE;
	};

	class SSH Keyboard
	{
		friend class Singlton<Keyboard>;
	public:
		// установка статуса кнопки
		void setKey(BYTE code, ssh_l repeat, bool use)
		{
			keys[code] = use;
			cur_key = (use ? code : 0);
			cur_char = 0;
			rep = (use ? repeat : 0);
		}
		// установка текущей кнопки
		void setSym(ssh_b code_char, ssh_u repeat) { cur_char = code_char; }
		// вернуть текущую символьную кнопку
		ssh_b getSym() const { return cur_char; }
		// вернуть текущую кнопку
		ssh_b getKey() { rep--; return (rep >= 0 ? cur_key : 0); }
		// вернуть признак нажатия
		bool is_keyPressed(ssh_b code) const { return keys[code]; }
		// установить захват
		void set_capture(ssh_u h) { hcapture = h; }
		// проверить на захват
		bool is_capture(ssh_u h) const { return (h == hcapture || hcapture == 0); }
	protected:
		// конструктор
		Keyboard() : rep(0), cur_key(0), cur_char(0), hcapture(0) { SSH_MEMZERO(keys, sizeof(keys)); }
		// деструктор
		virtual ~Keyboard() {}
		// массив статусов всех кнопок
		bool keys[256];
		// текущий символ
		BYTE cur_char;
		// текущая кнопка
		BYTE cur_key;
		// статус повтора
		ssh_l rep;
		// хэндл захвата
		ssh_u hcapture;
		// индекс синглота
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
			// возможности устройства
			XINPUT_CAPABILITIES caps;
			// признаки
			bool is_connected;
			bool is_inserted;
			bool is_removed;
			// значения стика в диапазоне [-1,+1]
			float fThumbRX;
			float fThumbRY;
			float fThumbLX;
			float fThumbLY;
			// Records which buttons were pressed this frame.
			// These are only set on the first frame that the button is pressed
			ssh_w wPressedButtons;
			bool is_pressedLeftTrigger;
			bool is_pressedRightTrigger;
			// предыдущие состояния кнопок
			ssh_w wLastButtons;
			bool is_lastLeftTrigger;
			bool is_lastRightTrigger;
		};
		// проверка на вновь нажатую кнопку
		bool is_once_pressed(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wPressedButtons & but) != 0) : false); }
		// вернуть признак нажатой кнопки
		bool is_pressed(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wButtons & but) != 0) : false); }
		// проверка на нажатую кнопку
		bool is_released(ssh_d idx, ssh_w but) const { return (_pad[idx].is_connected ? ((_pad[idx].wLastButtons & but) != 0) : false); }
		// проверка на нажатый триггер
		bool is_trigger(ssh_d idx, Side side) const;
		// признак коннекта
		bool is_connect(ssh_d idx) const { return _pad[idx].is_connected; }
		// признак дисконнекта
		bool is_removed(ssh_d idx) const { return _pad[idx].is_removed; }
		// признак нового подключения
		bool is_inserted(ssh_d idx) const { return _pad[idx].is_inserted; }
		// вернуть значения стиков
		Range<float> get_stick(ssh_d idx, Side side) const;
		// обновление состояния и проверка на коннект
		virtual void update();
		// установка скорости вибрации
		virtual void vibration(ssh_d idx, Side side, ssh_w speed) const;
	protected:
		Gamepad();
		virtual ~Gamepad();
		GAMEPAD _pad[MAX_CONTROLLERS];
		// индекс синглота
		static ssh_u const singl_idx = SSH_SINGL_GAMEPAD;
	};
	#define mouse		Singlton<Mouse>::Instance()
	#define keyboard	Singlton<Keyboard>::Instance()
	#define gamepad		Singlton<Gamepad>::Instance()
}
