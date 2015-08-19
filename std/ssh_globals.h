
#pragma once

extern "C"
{
	long asm_ssh_capability();
	ssh_ws* asm_ssh_to_base64(ssh_cs* ptr, ssh_u count);
	ssh_cs* asm_ssh_from_base64(ssh_ws* str, ssh_u count, ssh_u* len_buf, ssh_u null);
	ssh_l asm_ssh_parse_xml(ssh_ws* src, ssh_w* vec);
};

namespace ssh
{
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

	enum class WindowsTypes : ssh_w
	{
		WINDOWS_UNK = 0, WINDOWS_NT, WINDOWS_2K, WINDOWS_XP, WINDOWS_VISTA, WINDOWS_7, WINDOWS_8, WINDOWS_8_1, WINDOWS_10
	};
	// возможности процессора
	enum class CpuCaps : ssh_w
	{
		SUPPORTS_NONE = 0,
		SUPPORTS_MMX,
		SUPPORTS_SSE,
		SUPPORTS_SSE2,
		SUPPORTS_SSE3,
		SUPPORTS_SSSE3,
		SUPPORTS_SSE4_1,
		SUPPORTS_SSE4_2,
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
		SUPPORTS_AVX512CD,
		SUPPORTS_HALF
	};

	enum class SystemInfo : ssh_w
	{
		siProgFolder, siWorkFolder, siTempFolder, siUserFolder, siNameProg, siUserName, siCompName, siCustom, siPlatform, siTotalMemory, siPhysicalMemory, siCpuSpeed, siCpuCaps
	};

	void* ssh_dll_proc(ssh_wcs dll, ssh_ccs proc, ssh_wcs suffix = L"d");
	void SSH ssh_make_path(ssh_wcs path, bool is_file);
	void SSH ssh_remove_comments(String* lst, ssh_u count, bool is_simple);
	bool SSH ssh_is_null(ssh_wcs str);
	bool SSH ssh_dlg_sel_folder(ssh_wcs title, String& folder, HWND hWnd);
	bool SSH ssh_make_wnd(const DESC_WND& desc, bool is_show_wnd);
	bool SSH ssh_is_wrong_lex(const String& str, ssh_wcs errLexs);
	String SSH ssh_make_params(ssh_wcs fmt, ...);
	String SSH ssh_translate(ssh_wcs text, bool to_eng);
	String SSH ssh_num_volume(ssh_u num);
	String SSH ssh_path_in_range(const String& path, ssh_u range);
	String SSH ssh_make_guid(const GUID& guid);
	String SSH ssh_cnv_string(ssh_u flags, ENUM_DATA* stk, ssh_wcs def, bool enumerate = true);
	String SSH ssh_gen_name(ssh_wcs nm, bool is_long = true);
	String SSH ssh_slash_path(const String& path);
	String SSH ssh_file_ext(const String& path, bool is_pt = false);
	String SSH ssh_file_name(const String& path);
	String SSH ssh_file_title(const String& path);
	String SSH ssh_file_path(const String& path);
	String SSH ssh_file_path_title(const String& path);
	String SSH ssh_system_paths(SystemInfo type, int csidl = CSIDL_LOCAL_APPDATA);
	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs);
	String SSH ssh_md5(const String& str);
	String SSH ssh_base64(ssh_wcs charset, const String& str);
	String SSH ssh_base64(const Buffer<ssh_cs>& buf);
	ssh_u SSH ssh_system_info(SystemInfo type, CpuCaps value);
	ssh_u SSH ssh_rand(ssh_u begin, ssh_u end);
	ssh_u SSH ssh_hash(ssh_wcs wcs);
	ssh_u SSH ssh_hash(ssh_ccs ccs);
	ssh_u SSH ssh_hash_type(ssh_ccs nm);
	ssh_u SSH ssh_offset_line(const String& text, ssh_l ln);
	int SSH ssh_split(ssh_ws split, ssh_wcs src, int* vec, int count_vec);
	ssh_u SSH ssh_dlg_save_or_open(bool bOpen, ssh_wcs title, ssh_wcs filter, ssh_wcs ext, String& folder, HWND hWnd, String* arr, ssh_u count);
	ssh_u SSH ssh_cnv_value(ssh_wcs str, ENUM_DATA* stk, ssh_u def);
	GUID SSH ssh_make_guid(ssh_wcs src);
	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, ssh_wcs str, bool is_null);
	Buffer<ssh_cs> SSH ssh_base64(const String& str, bool is_null);

	// преобразовать значение в ближайшую степень двойки
	template <class T> T ssh_pow2(ssh_u val, bool nearest)
	{
		ssh_d idx;
		_BitScanReverse64(&idx, val);
		ssh_u _val((ssh_u)(1 << idx));
		return (T)(_val != val ? (nearest ? _val : _val << 1) : val);
	}
	// проверяет на кратность значения степени двойки
	template <typename T> bool ssh_is_pow2(const T& value)
	{
		return (value == ssh_pow2<T>(value, true));
	}
	// разбить строку на элементы
	template <typename T> T* ssh_explode(ssh_wcs split, const String& src, T* dst, ssh_u count_dst, const T& def, ENUM_DATA* stk = nullptr, bool is_bool = false, bool is_hex = false)
	{
		ssh_ws* _wcs(src.buffer());
		ssh_ws* t;
		ssh_u i(0), j(wcslen(split));
		T tmp;
		while(i < count_dst)
		{
			if((t = wcsstr(_wcs, split))) *t = 0;
			if(stk) tmp = (T)ssh_cnv_value(_wcs, stk, (ssh_u)def);
			else if(is_bool) tmp = (wcscmp(_wcs, L"true") == 0);
			else tmp = (T)_wcstoi64(_wcs, nullptr, is_hex ? 16 : 10);
			dst[i++] = tmp;
			if(t) { *t = *split; _wcs = t + j; }
			else break;
		}
		// заполняем значениями по умолчанию
		for(; i < count_dst; i++) dst[i] = def;
		return dst;
	}
	// соеденить элементы в строку
	template <typename T> String ssh_implode(ssh_wcs split, T* src, ssh_u count_src, ENUM_DATA* stk, ssh_wcs def, bool is_bool, bool is_hex, bool is_enum)
	{
		String ret, _tmp;

		for(ssh_u i = 0; i < count_src; i++)
		{
			T tmp(src[i]);
			if(stk) _tmp = ssh_cnv_string((ssh_u)tmp, stk, def, is_enum);
			else if(is_bool) _tmp = (tmp == 1 ? L"true" : L"false");
			else _tmp = (tmp, is_hex ? String::_hex : String::_dec);
			if(i) ret += split;
			ret += _tmp;
		}
		return ret;
	}
	template <typename T > String ssh_make_hex_string(T* p, ssh_u count, String& txt, bool is_cont)
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
}
