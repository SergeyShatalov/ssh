
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
		// ���������
		String caption;
		// ������
		HCURSOR cursor;
		// ������
		HICON icon;
		// ����� ������
		HICON iconSm;
		// ���
		HBRUSH bkg;
		// ��� ������
		mutable String className;
		// ����� ������
		UINT stylesClass;
		// ����� ����
		UINT stylesWnd;
		// ����� ������������� ����
		HWND hWndParent;
		// ����� ����
		mutable HWND hWnd;
		// ����� ������
		HINSTANCE hInst;
		// �������� ����
		Bar<int> bar;
		// ��������� ��������� ����
		WNDPROC processingWnd;
	};

	class SSH Helpers
	{
		friend class Singlton < Helpers > ;
	public:
		// ��� �������
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
		// ������ �������
		struct OS_VERSION
		{
			long MajorVersion;
			long MinorVersion;
			long Build;
		};
		// ����������� ����������
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
		// ������� ����������� �� ������
		void remove_comments(String* lst, ssh_u count, bool is_simple);
		// ������� ��������� �����
		void system_info(String* arr, int csidl) const;
		// ������� ���� �� �����
		void make_path(const String& path, bool is_file) const;
		// ��������� �� ������������ �������
		bool is_wrong_lex(const String& str, ssh_wcs errLexs) const;
		// ��������� ���� ������������ �������
		ssh_u platformType() const { return platform; }
		// ��������� ���������� ��������� ������
		ssh_u totalMemory() const { return MemStatus.dwAvailPhys + MemStatus.dwAvailPageFile; }
		// ��������� ���������� ���������� ������
		ssh_u physicalMemory() const { return MemStatus.dwTotalPhys; }
		// ��������� ������������� �������� ����������
		ssh_u cpuSpeed() const { return processorSpeed; }
		// ������� �������������� ������ ����������
		bool is_cpu_caps(ssh_u caps) const { return cpuFlags.testBit(caps); }
		// ������� ������ ������������ �������
		const OS_VERSION& windowsVersion() { return osVersion; }
		// ������� �������� ������ �� ������ ������
		ssh_u offset_line(const String& text, ssh_l ln) const;
		// ������������� ����� � �����
		String num_volume(ssh_u num) const;
		// ��������� ���� � ��������
		String pathInRange(const String& path, ssh_u range) const;
		// ������� ������ �� ���������
		ssh_u split(ssh_wcs split, const String& src, String* dst, ssh_u count_dst, ssh_wcs def) const;
		// �������� ���� �� ����� ����
		String slash_path(const String& path) const
		{
			return ((path[path.length() - 1] == L'\\') ? path : path + L'\\');
		}
		// ������� ������ ��� �����
		String file_title(const String& path) const
		{
			ssh_l i;
			String nm((i = path.find_rev(L'\\') + 1) ? path.substr(i) : L"");
			return ((i = nm.find_rev(L'.')) >= 0 ? nm.left(i) : nm);
		}
		// ������� ���������� �����
		String file_ext(const String& path, bool is_pt) const
		{
			ssh_l i;
			String nm((i = path.find_rev(L'\\') + 1) ? path.substr(i) : L"");
			return ((i = nm.find_rev(L'.') + !is_pt) ? nm.substr(i) : L"");
		}
		// ������� ��� ����� � �����������
		String file_name(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'\\')) < 0 ? "" : path.substr(i + 1));
		}
		// ������� ����
		String file_path(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'\\') + 1) ? path.left(i) : L""); 
		}
		// ������� ���� c ������
		String file_path_title(const String& path) const
		{
			ssh_l i;
			return ((i = path.find_rev(L'.')) ? path.left(i) : path); 
		}
		// ������������� ��������� ���
		String gen_name(ssh_wcs nm) const
		{
			static ssh_u gen_count(0);
			String message;
			gen_count++;
			return message.fmt(L"%s%I64X%016I64X", nm, gen_count, __rdtsc());
		}
		// ������������� �������� � ��������� ������� ������
		template <class T> T pow2(ssh_u val, bool nearest) const
		{
			ssh_d idx;
			_BitScanReverse64(&idx, val);
			ssh_u _val((ssh_u)(1 << idx));
			return (T)(_val != val ? (nearest ? _val : _val << 1) : val);
		}
		// ������� ��������� ����������
		const String& get_system_info(SystemInfo idx) const { return si[idx]; }
		// ��������� ������ �� "�������"
		bool is_null(ssh_wcs str) const { return (!str || !str[0]); }
		// ��������� �� ��������� �������� ������� ������
		template <typename T> bool is_pow2(const T& value) const { return (value == pow2<T>(value, true)); }
		// ������� ������ �� ��������
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
			// ��������� ���������� �� ���������
			for(; i < count_dst; i++) dst[i] = def;
			return dst;
		}
		// ��������� �������� � ������
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
		// ����� ��� ������ �����
		bool dlgSelectFolder(ssh_wcs title, String& folder, HWND hWnd) const;
		// ����� ��� ��������/������ �����
		ssh_u dlgSaveOrOpenFile(bool bOpen, ssh_wcs title, ssh_wcs filter, ssh_wcs ext, String& folder, HWND hWnd, String* arr, ssh_u count) const;
		// ����������� ������ � �������� ����
		bool make_wnd(const DESC_WND& desc, bool is_show_wnd) const;
		// ��������� ����������� ��������� ����
		virtual LRESULT proc_wndmsg(HWND hWnd, UINT msg, WPARAM w, LPARAM l) const;
		// ������������ �����
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
		// �����������
		Helpers();
		// ����������
		virtual ~Helpers() {}
		// �������������� ��������� �������� ������ � ��������
		ssh_u cnvValue(ssh_wcs str, ENUM_DATA* stk, ssh_u def) const;
		// �������������� �������� �������� ������ � ���������
		String cnvString(ssh_u flags, ENUM_DATA* stk, ssh_wcs def, bool enumerate = true) const;
		// ���������
		WindowsTypes platform;
		// ������ ������
		MEMORYSTATUS MemStatus;
		// �������� ����������
		ssh_u processorSpeed;
		// ��������� ������
		String si[8];
		// ����� ����������
		Bits32 cpuFlags;
		// ������ ������������ �������
		OS_VERSION osVersion;
		// ������ ��������
		static const ssh_u singl_idx = SSH_SINGL_HELPER;
	};

#define hlp		Singlton<Helpers>::Instance()
}