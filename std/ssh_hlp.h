
#pragma once

#include "ssh_singl.h"

namespace ssh
{
	struct ENUM_DATA
	{
		ssh_wcs _wcs;
		ssh_u	_val;
	};

	struct DESC_WND
	{
		DESC_WND() : bkg(0), icon(0), iconSm(0), cursor(0), processingWnd(nullptr), stylesClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS), stylesWnd(WS_OVERLAPPEDWINDOW), hWndParent(nullptr), hInst(::GetModuleHandle(nullptr)), bar(0, 0, 256, 256) {}
		// заголовок
		String caption;
		// курсор
		HCURSOR cursor;
		// иконка
		HICON icon;
		// малая иконка
		HICON iconSm;
		// фон
		HBRUSH bkg;
		// имя класса
		mutable String className;
		// стили класса
		UINT stylesClass;
		// стили окна
		UINT stylesWnd;
		// хэндл родительского окна
		HWND hWndParent;
		// хэндл окна
		mutable HWND hWnd;
		// хэндл модуля
		HINSTANCE hInst;
		// габариты окна
		Bar<int> bar;
		// процедура обработки окна
		WNDPROC processingWnd;
	};

	class SSH Helpers
	{
		friend class Singlton < Helpers > ;
	public:
		// тип системы
		enum WindowsTypes
		{
			WINDOWS_UNK = 0,
			WINDOWS_95,
			WINDOWS_95_SR2,
			WINDOWS_98,
			WINDOWS_98_SR2,
			WINDOWS_ME,
			WINDOWS_NT,
			WINDOWS_2K,
			WINDOWS_XP,
			WINDOWS_VISTA,
			WINDOWS_7,
			WINDOWS_FUTURE
		};
		// версия системы
		struct OS_VERSION
		{
			long MajorVersion;
			long MinorVersion;
			long Build;
		};
		// возможности процессора
		enum CpuFlags
		{
			SUPPORTS_NONE = 0,
			SUPPORTS_MMX = 1,
			SUPPORTS_SSE = 2,
			SUPPORTS_SSE2 = 3,
			SUPPORTS_SSE3 = 4,
			SUPPORTS_SSSE3 = 5,
			SUPPORTS_SSE4_1 = 6,
			SUPPORTS_SSE4_2 = 7,
			SUPPORTS_PCLMULQDQ,
			SUPPORTS_FMA,
			SUPPORTS_CMPXCHG16B,
			SUPPORTS_MOVBE,
			SUPPORTS_POPCNT,
			SUPPORTS_AES,
			SUPPORTS_AVX,
			SUPPORTS_RDRAND,
			SUPPORTS_CMOV,
			SUPPORTS_BMI1,
			SUPPORTS_AVX2,
			SUPPORTS_BMI2,
			SUPPORTS_AVX512F,
			SUPPORTS_RDSEED,
			SUPPORTS_AVX512PF,
			SUPPORTS_AVX512ER,
			SUPPORTS_AVX512CD
		};
		enum SystemInfo
		{
			siProgFolder = 0,
			siWorkFolder,
			siTempFolder,
			siUserFolder,
			siNameProg,
			siUserName,
			siCompName,
			siCustom
		};
		// удалить комментарии из текста
		void remove_comments(String* lst, ssh_u count, bool is_simple);
		// вернуть системные папки
		void system_info(String* arr, int csidl) const;
		// создать путь из папок
		void make_path(const String& path, bool is_file) const;
		// проверить на недопустимую лексему
		bool is_wrong_lex(const String& str, ssh_wcs errLexs) const;
		// получение типа операционной системы
		ssh_u platformType() const { return platform; }
		// получение количества системной памяти
		ssh_u totalMemory() const { return MemStatus.dwAvailPhys + MemStatus.dwAvailPageFile; }
		// получение количества физической памяти
		ssh_u physicalMemory() const { return MemStatus.dwTotalPhys; }
		// получение относительной скорости процессора
		ssh_u cpuSpeed() const { return processorSpeed; }
		// вернуть поддерживаемые наборы инструкций
		bool is_cpu_caps(ssh_u caps) const { return cpuFlags.testBit(caps); }
		// вернуть версию операционной системы
		const OS_VERSION& windowsVersion() { return osVersion; }
		// вернуть смещение строки от начала текста
		ssh_u offset_line(const String& text, ssh_l ln) const;
		// преобразовать число в объем
		String num_volume(ssh_u num) const;
		// поместить путь в диапазон
		String pathInRange(const String& path, ssh_u range) const;
		// разбить строку на подстроки
		ssh_u split(ssh_wcs split, const String& src, String* dst, ssh_u count_dst, ssh_wcs def) const;
		// добавить слеш на конец пути
		String slash_path(const String& path) const
		{
			return ((path[path.length() - 1] == L'\\') ? path : path + L'\\');
		}
		// извлечь только имя файла
		String file_title(const String& path) const
		{
			ssh_l i;
			String nm((i = path.find_rev(L'\\') + 1) ? path.substr(i) : L"");
			return ((i = nm.find_rev(L'.')) >= 0 ? nm.left(i) : nm);
		}
		// извлечь расширение файла
		String file_ext(const String& path, bool is_pt) const
		{
			ssh_l i;
			String nm((i = path.find_rev(L'\\') + 1) ? path.substr(i) : L"");
			return ((i = nm.find_rev(L'.') + !is_pt) ? nm.substr(i) : L"");
		}
		// извлечь имя файла с расширением
		String file_name(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'\\')) < 0 ? "" : path.substr(i + 1));
		}
		// извлечь путь
		String file_path(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'\\') + 1) ? path.left(i) : L""); 
		}
		// извлечь путь c файлом
		String file_path_title(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'.')) ? path.left(i) : path); 
		}
		// сгенерировать случайное имя
		String gen_name(ssh_wcs nm) const
		{
			static ssh_u gen_count(0);
			String message;
			gen_count++;
			return message.fmt(L"%s%I64X%016I64X", nm, gen_count, __rdtsc());
		}
		// преобразовать значение в ближайшую степень двойки
		template <class T> T pow2(ssh_u val, bool nearest) const
		{
			ssh_d idx;
			_BitScanReverse64(&idx, val);
			ssh_u _val((ssh_u)(1 << idx));
			return (T)(_val != val ? (nearest ? _val : _val << 1) : val);
		}
		// вернуть системную информацию
		const String& get_system_info(SystemInfo idx) const { return si[idx]; }
		// проверить строку на "пустоту"
		bool is_null(ssh_wcs str) const { return (!str || !str[0]); }
		// проверяет на кратность значения степени двойки
		template <typename T> bool is_pow2(const T& value) const { return (value == pow2<T>(value, true)); }
		// разбить строку на элементы
		template <typename T> T* explode(ssh_wcs split, const String& src, T* dst, ssh_u count_dst, const T& def, ENUM_DATA* stk = nullptr, bool is_bool = false, bool is_hex = false) const
		{
			ssh_ws* _wcs(src.buffer());
			ssh_ws* t;
			ssh_u i(0), j(wcslen(split));
			T tmp;
			while(i < count_dst)
			{
				if((t = wcsstr(_wcs, split))) *t = 0;
				if(stk) tmp = (T)cnvValue(_wcs, stk, (ssh_u)def);
				else if(is_bool) tmp = (wcscmp(_wcs, L"true") == 0);
				else tmp = (T)_wcstoi64(_wcs, nullptr, is_hex ? 16 : 10);
				dst[i++] = tmp;
				if(t) { *t = *split; _wcs = t + j; } else break;
			}
			// заполняем значениями по умолчанию
			for(; i < count_dst; i++) dst[i] = def;
			return dst;
		}
		// соеденить элементы в строку
		template <typename T> String implode(ssh_wcs split, T* src, ssh_u count_src, ENUM_DATA* stk, ssh_wcs def, bool is_bool, bool is_hex, bool is_enum) const
		{
			String ret, _tmp;

			for(ssh_u i = 0; i < count_src; i++)
			{
				T tmp(src[i]);
				if(stk) _tmp = cnvString((ssh_u)tmp, stk, def, is_enum);
				else if(is_bool) _tmp = (tmp == 1 ? L"true" : L"false");
				else _tmp = (tmp, is_hex ? String::_hex : String::_dec);
				if(i) ret += split;
				ret += _tmp;
			}
			return ret;
		}
		// форма для выбора папки
		bool dlgSelectFolder(ssh_wcs title, String& folder, HWND hWnd) const;
		// форма для открытия/записи файла
		ssh_u dlgSaveOrOpenFile(bool bOpen, ssh_wcs title, ssh_wcs filter, ssh_wcs ext, String& folder, HWND hWnd, String* arr, ssh_u count) const;
		// регистрация класса и создание окна
		bool make_wnd(const DESC_WND& desc, bool is_show_wnd) const;
		// обработка стандартных сообщений окна
		virtual LRESULT proc_wndmsg(HWND hWnd, UINT msg, WPARAM w, LPARAM l) const;
		// формирование гуида
		GUID make_guid(ssh_wcs src) const;
		String make_guid(const GUID& guid) const;
		template <typename T > String make_hex_string(T* p, ssh_u count, String& txt, bool is_cont)
		{
			String bytes(L'\0', count * 3);
			String gran;
			ssh_ws* _ws(bytes.buffer());
			gran.fmt(L"%%0%ix ", sizeof(T) * 2);
			for(ssh_u i = 0; i < count / sizeof(T); i++)
			{
				swprintf(_ws, gran, (T*)p[i]);
				_ws += (sizeof(T) * 2) + 1;
			}
			_ws = (ssh_ws*)p;
			txt.empty();
			for(ssh_u i = 0; i < count / 2; i++)
			{
				ssh_ws val(*_ws++);
				if(val < 33) val = L'.';
				txt += val;
			}
			if(is_cont) bytes += L"...";
			return bytes;
		}
	protected:
		// конструктор
		Helpers();
		// деструктор
		virtual ~Helpers() {}
		// преобразование строковых значений флагов в двоичные
		ssh_u cnvValue(ssh_wcs str, ENUM_DATA* stk, ssh_u def) const;
		// преобразование двоичных значений флагов в строковые
		String cnvString(ssh_u flags, ENUM_DATA* stk, ssh_wcs def, bool enumerate = true) const;
		// платформа
		WindowsTypes platform;
		// статус памяти
		MEMORYSTATUS MemStatus;
		// скорость процессора
		ssh_u processorSpeed;
		// системные строки
		String si[8];
		// флаги процессора
		Bits32 cpuFlags;
		// версия операционной системы
		OS_VERSION osVersion;
		// индекс сиглтона
		static const ssh_u singl_idx = SSH_SINGL_HELPER;
	};

#define hlp		Singlton<Helpers>::Instance()
}