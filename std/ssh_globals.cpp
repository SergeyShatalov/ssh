
#include "stdafx.h"
#include "ssh_globals.h"
#include <Common\ssh_ext.h>

namespace ssh
{
	typedef ssh_cs* (CALLBACK* __ext_undname)(ssh_cs* out, ssh_ccs name, int len_out, ssh_d flags);
	typedef void* (CALLBACK* __cnv_open)(ssh_wcs to, ssh_wcs from);
	typedef int (CALLBACK* __cnv_close)(void* h);
	typedef size_t(CALLBACK* __cnv_make)(void* cd, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);

	static __ext_undname _und((__ext_undname)ssh_dll_proc(L"sshEXT.dll", "ext_undname"));
	static __cnv_open _open((__cnv_open)ssh_dll_proc(L"sshCNV.dll", "iconv_open"));
	static __cnv_close _close((__cnv_close)ssh_dll_proc(L"sshCNV.dll", "iconv_close"));
	static __cnv_make _make((__cnv_make)ssh_dll_proc(L"sshCNV.dll", "iconv"));

#define IDC_FOLDERTREE		0x3741
#define IDC_STATUSTEXT		0x3743
#define IDC_NEW_EDIT_PATH	0x3744

	static int CALLBACK SelectFolderCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM pData)
	{
		SSH_TRACE;
		String* folder((String*)pData);
		ssh_ws tmp[MAX_PATH];

		switch(uMsg)
		{
			case BFFM_INITIALIZED:
				RECT rc;
				HWND hEdit;
				HFONT hFont;
				::ShowWindow(::GetDlgItem(hwnd, IDC_STATUSTEXT), SW_HIDE);
				::GetWindowRect(::GetDlgItem(hwnd, IDC_FOLDERTREE), &rc);
				rc.bottom = rc.top - 8;
				rc.top = rc.bottom - 23;
				::ScreenToClient(hwnd, (LPPOINT)&rc);
				::ScreenToClient(hwnd, ((LPPOINT)&rc) + 1);
				hEdit = ::CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_TABSTOP | WS_VISIBLE | ES_AUTOHSCROLL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hwnd, nullptr, nullptr, nullptr);
				::SetWindowLong(hEdit, GWL_ID, IDC_NEW_EDIT_PATH);
				::ShowWindow(hEdit, SW_SHOW);

				hFont = (HFONT)::SendMessage(hwnd, WM_GETFONT, 0, 0);
				::SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(TRUE, 0));

				if(folder->is_empty())
				{
					::GetCurrentDirectory(MAX_PATH, tmp);
					*folder = tmp;
				}
				::SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)folder->buffer());
				break;
			case BFFM_SELCHANGED:
				HWND hDlg(::GetDlgItem(hwnd, IDC_NEW_EDIT_PATH));
				if(hDlg)
				{
					::SendMessage(hwnd, BFFM_ENABLEOK, 0, SHGetPathFromIDList((LPITEMIDLIST)lParam, folder->buffer()));
					::SetWindowText(hDlg, folder->buffer());
				}
				break;
		}

		return 0;
	}

	ssh_u SSH ssh_hash_type(ssh_ccs nm)
	{
		static ssh_cs out[256];
		ssh_cs* _cs;
		if(_und) _cs = _und(out, nm + 1, 256, UNDNAME_NO_RETURN_CONSTABLES | UNDNAME_NO_RETURN_ARRAY | UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY);
		return (_und ? ssh_hash(_cs) : 0);
	}

	ssh_u SSH ssh_hash(ssh_wcs wcs)
	{
		ssh_u _val(14695981039346656037ULL);
		while(*wcs)
		{
			_val ^= (ssh_u)*wcs++;
			_val *= (ssh_u)1099511628211ULL;
		}
		_val ^= _val >> 32;
		return _val;
	};

	ssh_u SSH ssh_hash(ssh_ccs ccs)
	{
		ssh_u _val(14695981039346656037ULL);
		while(*ccs)
		{
			_val ^= (ssh_u)*ccs++;
			_val *= (ssh_u)1099511628211ULL;
		}
		_val ^= _val >> 32;
		return _val;
	};

	ssh_u singltons[32];

	void SSH ssh_singlton(ssh_u ptr, ssh_u index)
	{
		if(index >= 32) SSH_THROW(L"Недопустимый индекс синглона %i!", index);
		if(singltons[index]) SSH_THROW(L"Синглетон с индексом %i уже существует!", index);
		singltons[index] = ptr;
	}

	ssh_u SSH ssh_rand(ssh_u begin, ssh_u end)
	{
		static ssh_u _genRnd(0);
		ssh_u tmp;
		bool is(false);
		if(ssh_system_info(SystemInfo::siCpuCaps, CpuCaps::SUPPORTS_RDRAND)) is = (_rdrand64_step(&tmp) == 1);
		if(!is)
		{
			tmp = _genRnd;
			if(!tmp) tmp = _time64((__time64_t*)tmp);
			tmp *= 1103515245;
			_genRnd = tmp;
			tmp = ((tmp >> 16) & 0x7fff);
		}
		return begin + (tmp % ((end - begin) + 1));
	}

	Buffer<ssh_cs> SSH ssh_base64(const String& str, bool is_null)
	{
		ssh_u len_buf(0);
		return Buffer<ssh_cs>(asm_ssh_from_base64(str.buffer(), str.length(), &len_buf, is_null * 2), len_buf);
	}

	String SSH ssh_base64(ssh_wcs charset, const String& str)
	{
		return ssh_base64(ssh_cnv(charset, str, false));
	}

	String SSH ssh_base64(const Buffer<ssh_cs>& buf)
	{
		return String(Buffer<ssh_ws>(asm_ssh_to_base64(buf, buf.count()), 1).to<ssh_ws>());
	}

	String SSH ssh_md5(const String& str)
	{
		ssh_b md[16];
		String ret(L'\0', 32);
		ssh_ws* _ret(ret.buffer());
		MD5(ssh_cnv(cp_ansi, str, false).to<ssh_b>(), str.length(), md);
		for(ssh_u i = 0; i < 16; i++) wsprintf(_ret + i * 2, L"%02x", md[i]);
		return ret;
	}

	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, ssh_wcs str, bool is_null)
	{
		iconv_t h;
		Buffer<ssh_cs> out((wcslen(str) + is_null) * 2);
		ssh_u in_c(out.count()), out_c(in_c);
		ssh_ccs _in((ssh_ccs)str);
		ssh_cs* _out(out);
		if(_open && _close && _make)
		{
			if((h = _open(to, cp_utf)) != (iconv_t)-1)
			{
				while(in_c > 0)
				{
					if(_make(h, &_in, &in_c, &_out, &out_c) == -1) { out_c = 0; break; }
				}
				_close(h);
			}
		}
		return Buffer<ssh_cs>(out, _out - out);
	}

	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs)
	{
		iconv_t h;
		ssh_u in_c(in.count() - offs), out_c(in_c * 2);
		Buffer<ssh_cs> out(out_c);
		ssh_ccs _in(in + offs);
		ssh_cs* _out(out);
		if(_open && _close && _make)
		{
			if((h = _open(cp_utf, from)) != (iconv_t)-1)
			{
				while(in_c > 0)
				{
					if(_make(h, &_in, &in_c, &_out, &out_c) == -1) { out_c = 0; break; }
				}
				_close(h);
			}
		}
		return String(out.to<ssh_ws>(), (_out - out) / 2);
	}

	ssh_u SSH ssh_dll_proc(ssh_wcs dll, ssh_ccs proc, ssh_wcs suffix)
	{
		SSH_TRACE;
		// хэндлы загруженных dll
		static Map<HMODULE, String, SSH_TYPE, SSH_TYPE> dlls(ID_DLL_MODULES);

		HMODULE hdll;
		String module(ssh_file_path_title(dll));
#ifdef _DEBUG
		if(suffix) module += suffix;
#endif
		module += (ssh_file_ext(dll, true));
		if(!(hdll = dlls[module]))
		{
			if(!(hdll = LoadLibrary(module)))
			{
				::GetLastError();
				return 0;
			}
			dlls[module] = hdll;
		}
		return (ssh_u)GetProcAddress(hdll, proc);
	}

	String SSH ssh_system_paths(SystemInfo type, int csidl)
	{
		SSH_TRACE;
		ssh_ws tmp[MAX_PATH];
		ssh_d sz(MAX_PATH);
		String ret;
		switch(type)
		{
			case SystemInfo::siProgFolder:
			case SystemInfo::siNameProg:
				::GetModuleFileName(::GetModuleHandle(nullptr), tmp, MAX_PATH);
				ret = (type == SystemInfo::siProgFolder ? ssh_file_path(tmp) : ssh_file_title(tmp));
				break;
			case SystemInfo::siWorkFolder:
				::GetCurrentDirectory(MAX_PATH, tmp);
				ret = ssh_slash_path(tmp);
				break;
			case SystemInfo::siTempFolder:
				::GetTempPath(MAX_PATH, tmp);
				ret = ssh_slash_path(tmp);
				break;
			case SystemInfo::siUserFolder:
				if(SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, tmp))) ret = ssh_slash_path(tmp);
				break;
			case SystemInfo::siUserName:
				if(GetUserName(tmp, &sz)) ret = tmp;
				break;
			case SystemInfo::siCompName:
				if(GetComputerName(tmp, &sz)) ret = tmp;
				break;
			case SystemInfo::siCustom:
				if(SUCCEEDED(SHGetFolderPath(nullptr, csidl, nullptr, 0, tmp))) ret = ssh_slash_path(tmp);
		}
		return ret;
	}

	// добавить слеш на конец пути
	String SSH ssh_slash_path(const String& path)
	{
		return ((path[path.length() - 1] != L'\\') ? path + L'\\' : path);
	}
	// извлечь только имя файла
	String SSH ssh_file_title(const String& path)
	{
		ssh_l sl(wcsrchr(path, L'\\') - (ssh_wcs)path + 1);
		ssh_l pt(wcsrchr(path, L'.') - (ssh_wcs)path);
		return path.substr(sl, pt - sl);
	}
	// извлечь расширение файла
	String SSH ssh_file_ext(const String& path, bool is_pt)
	{
		ssh_l pt((wcsrchr(path, L'.') - (ssh_wcs)path) + !is_pt);
		return path.substr(pt);
	}
	// извлечь имя файла с расширением
	String SSH ssh_file_name(const String& path)
	{
		ssh_l sl(wcsrchr(path, L'\\') - (ssh_wcs)path + 1);
		return path.substr(sl);
	}
	// извлечь путь
	String SSH ssh_file_path(const String& path)
	{
		ssh_l sl(wcsrchr(path, L'\\') - (ssh_wcs)path + 1);
		return path.left(sl);
	}
	// извлечь путь c файлом
	String SSH ssh_file_path_title(const String& path)
	{
		ssh_l sl(wcsrchr(path, L'.') - (ssh_wcs)path);
		return path.left(sl);
	}
	// сгенерировать случайное имя
	String SSH ssh_gen_name(ssh_wcs nm, bool is_long)
	{
		static ssh_u gen_count(0);
		String message;
		gen_count++;
		return (is_long ? message.fmt(L"%s%I64X%016I64X", nm, gen_count, __rdtsc()) : message.fmt(L"%s%I64X", nm, gen_count));
	}

	ssh_u SSH ssh_system_info(SystemInfo type, CpuCaps value)
	{
		// платформа
		static WindowsTypes platform(WindowsTypes::WINDOWS_UNK);
		// версия платформы
		static OSVERSIONINFOW osVersion;
		// статус памяти
		static MEMORYSTATUS memStatus;
		// скорость процессора
		static ssh_u cpuSpeed(0);
		// флаги процессора
		static Bits cpuCaps;

		SSH_TRACE;

		if(!cpuSpeed)
		{
			memStatus.dwLength = sizeof(MEMORYSTATUS);
			GlobalMemoryStatus(&memStatus);

			osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
			if(GetVersionEx(&osVersion))
			{
				DWORD ver1 = osVersion.dwMajorVersion;
				DWORD ver2 = osVersion.dwMinorVersion;

				if(osVersion.dwPlatformId == VER_PLATFORM_WIN32_NT)
				{
					if(ver1 < 4) platform = WindowsTypes::WINDOWS_NT;
					else if(ver1 == 4) platform = WindowsTypes::WINDOWS_2K;
					else if(ver1 == 5) platform = WindowsTypes::WINDOWS_XP;
					else if(ver1 == 6)
					{
						switch(ver2)
						{
							case 0: platform = WindowsTypes::WINDOWS_VISTA; break;
							case 1: platform = WindowsTypes::WINDOWS_7; break;
							case 2: platform = WindowsTypes::WINDOWS_8; break;
							case 3: platform = WindowsTypes::WINDOWS_8_1; break;
						}
					}
					else if(ver1 == 10 && ver2 == 0) platform = WindowsTypes::WINDOWS_10;
				}
			}

			ssh_u startTime(__rdtsc());
			Sleep(100);
			ssh_u endTime(__rdtsc());
			cpuSpeed = (endTime - startTime) / 100;

			cpuCaps = asm_ssh_capability();
		}

		switch(type)
		{
			case SystemInfo::siCpuCaps: return cpuCaps.test_bit((ssh_u)value);;
			case SystemInfo::siPlatform: return (ssh_u)platform;
			case SystemInfo::siTotalMemory: return memStatus.dwAvailPhys + memStatus.dwAvailPageFile;
			case SystemInfo::siPhysicalMemory: return memStatus.dwTotalPhys;
			case SystemInfo::siCpuSpeed: return cpuSpeed;
		}
		return 0;
	}

	ssh_u SSH ssh_offset_line(const String& text, ssh_l ln)
	{
		SSH_TRACE;
		ssh_wcs _text(text);
		ssh_wcs txt(_text);
		while(ln-- > 0)
		{
			if(!(_text = wcsstr(_text, L"\r\n"))) return text.length();
			_text += 2;
		}
		return (_text - txt);
	}
	
	int SSH ssh_split(ssh_ws split, ssh_wcs src, int* vec, int count)
	{
		int i(0);
		if(split && src && vec && count)
		{
			ssh_ws* _src((ssh_ws*)src), *esrc(_src + wcslen(src)), *_wcs;
			while(i < count && _src < esrc)
			{
				vec[i * 2 + 0] = (int)(_src - src);
				if(!(_wcs = wcschr(_src, split))) _wcs = esrc;
				vec[i * 2 + 1] = (int)(_wcs - _src);
				_src = _wcs + 1;
				i++;
			}
		}
		return i;
	}
	
	ssh_u SSH ssh_dlg_save_or_open(bool bOpen, ssh_wcs title, ssh_wcs filter, ssh_wcs ext, String& folder, HWND hWnd, String* arr, ssh_u count)
	{
		SSH_TRACE;
		ssh_u result(0);
		ssh_ws flt[256];
		ssh_u nBytesFiles(1), i(0);

		int flags(OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT);

		OPENFILENAME ofn;
		SSH_MEMZERO(&ofn, sizeof(OPENFILENAME));

		while((flt[i] = *filter))
		{
			if(*filter == L'|') flt[i] = 0;
			i++; filter++;
		}
		if(arr)
		{
			flags |= OFN_ALLOWMULTISELECT;
			nBytesFiles = count;
		}
		if(bOpen) flags |= OFN_FILEMUSTEXIST;
		nBytesFiles *= MAX_PATH;

		Buffer<ssh_ws> files(nBytesFiles); ssh_ws* _files(files);
		SSH_MEMZERO(files, nBytesFiles);

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrDefExt = ext;
		ofn.lpstrFilter = flt;
		ofn.lpstrInitialDir = folder;
		ofn.lpstrTitle = title;
		ofn.lpstrFile = files;
		ofn.lpstrFileTitle = nullptr;
		ofn.nFilterIndex = 1;
		ofn.nMaxFile = (DWORD)nBytesFiles;
		ofn.Flags = flags;

		if((bOpen ? ::GetOpenFileName(&ofn) : ::GetSaveFileNameW(&ofn)))
		{
			folder = _files;
			_files += (wcslen(_files) + 1);
			if(arr && *_files)
			{
				folder = ssh_slash_path(folder);
				while(*_files && result < count)
				{
					arr[result++] = (folder + (ssh_wcs)_files);
					_files += (wcslen(_files) + 1);
				}
			}
			else result = 1;
		}

		return result;
	}
	
	ssh_u SSH ssh_cnv_value(ssh_wcs str, ENUM_DATA* stk, ssh_u def)
	{
		SSH_TRACE;
		ssh_ws* t;
		ssh_wcs _wcs;
		ssh_u val(0);
		bool is(false);
		while(true)
		{
			if((t = (ssh_ws*)wcschr(str, L'|'))) *t = 0;
			ssh_u i(0);
			while((_wcs = stk[i]._wcs))
			{
				if(wcscmp(_wcs, str) == 0)
				{
					is = true;
					val |= stk[i]._val;
					break;
				}
				i++;
			}
			if(!t) break;
			*t = L'|';
			str++;
		}
		return (is ? val : def);
	}
	
	GUID SSH ssh_make_guid(ssh_wcs src)
	{
		SSH_TRACE;
		src += (src[0] == L'{' ? 1 : 0);
		GUID _guid;
		ssh_ws* dst((ssh_ws*)src);
		_guid.Data1 = (ssh_d)_wcstoi64(dst, &dst, 16); dst++;
		_guid.Data2 = (ssh_w)_wcstoi64(dst, &dst, 16); dst++;
		_guid.Data3 = (ssh_w)_wcstoi64(dst, &dst, 16); dst++;
		ssh_w _w((ssh_w)_wcstoi64(dst, &dst, 16)); dst++; ssh_b* _wp((ssh_b*)&_w);
		ssh_u _u(_wcstoi64(dst, nullptr, 16)); ssh_b* _up((ssh_b*)&_u);
		_guid.Data4[0] = _wp[1]; _guid.Data4[1] = _wp[0];
		_guid.Data4[2] = _up[5]; _guid.Data4[3] = _up[4]; _guid.Data4[4] = _up[3];
		_guid.Data4[5] = _up[2]; _guid.Data4[6] = _up[1]; _guid.Data4[7] = _up[0];
		return _guid;
	}
	
	void SSH ssh_make_path(ssh_wcs path, bool is_file)
	{
		SSH_TRACE;
		ssh_ws dir[MAX_PATH];
		ssh_ws* _path((ssh_ws*)path);
		bool is;
		GetCurrentDirectory(MAX_PATH, dir);
		while(true)
		{
			if((is = (_path = wcschr(_path, L'\\')) != 0)) *_path = 0;
			if(!SetCurrentDirectory(path))
			{
				if(!is) is = !is_file;
				if(is) CreateDirectory(path, nullptr);
			}
			if(!_path) break;
			*_path++ = L'\\';
		}
		SetCurrentDirectory(dir);
	}
	
	void SSH ssh_remove_comments(String* lst, ssh_u count, bool is_simple)
	{
		if(lst)
		{
			regx rx;
			rx.set_pattern(0, LR"((?mUs)/\*.*\*/)");
			rx.set_pattern(1, LR"((?m)\s*//.*[\r\n]*)");

			for(ssh_u i = 0; i < count; i++)
			{
				try
				{
					String path(lst[i]), npath(ssh_file_path_title(path) + L'+' + ssh_file_ext(path, true));
					File of(path, File::open_read);
					File nf(npath, File::create_write);
					String text(of.read(cp_ansi, 0));
					if(is_simple) rx.replace(text, L"", (ssh_u)1);
					rx.replace(text, L"", (ssh_u)0);
					nf.write(text, cp_ansi);
					of.close();
					nf.close();
				}
				catch(const Exception& e)
				{
					e.add(L"Ошибка при удалении комментариев из файла <%s>", lst[i]);
				}
			}
		}
	}
	
	bool SSH ssh_is_null(ssh_wcs str)
	{
		return (!str || !str[0]);
	}
	
	bool SSH ssh_dlg_sel_folder(ssh_wcs title, String& folder, HWND hWnd)
	{
		SSH_TRACE;
		bool result = false;
		BROWSEINFO m_bi;
		LPITEMIDLIST pidl;
		LPMALLOC pMalloc;
		Buffer<ssh_ws> tmp(MAX_PATH);
		wcscpy(tmp, folder.buffer());

		SSH_MEMZERO(&m_bi, sizeof(BROWSEINFO));

		m_bi.hwndOwner = hWnd;
		m_bi.pszDisplayName = nullptr;
		m_bi.pidlRoot = 0;
		m_bi.ulFlags = BIF_VALIDATE | BIF_STATUSTEXT | BIF_RETURNFSANCESTORS;
		m_bi.lpfn = SelectFolderCallbackProc;
		m_bi.lParam = (LPARAM)&tmp;
		m_bi.lpszTitle = title;

		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			if((pidl = SHBrowseForFolder(&m_bi)))
			{
				pMalloc->Free(pidl);
				folder = ssh_slash_path((ssh_wcs)(ssh_ws*)tmp);
				result = true;
			}
			pMalloc->Release();
		}

		return result;
	}
	
	bool SSH ssh_make_wnd(const DESC_WND& desc, bool is_show_wnd)
	{
		SSH_TRACE;
		WNDCLASSEX wc;
		if(desc.className.is_empty()) desc.className = ssh_gen_name(L"");

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = desc.stylesClass;
		wc.lpfnWndProc = desc.processingWnd;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = desc.hInst;
		wc.hIcon = (desc.icon ? desc.icon : LoadIcon(0, IDI_APPLICATION));
		wc.hCursor = (desc.cursor ? desc.cursor : LoadCursor(0, IDC_ARROW));
		wc.hbrBackground = desc.bkg ? desc.bkg : (HBRUSH)COLOR_WINDOW;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = desc.className;
		wc.hIconSm = desc.iconSm ? desc.iconSm : LoadIcon(0, IDI_APPLICATION);
		if(RegisterClassEx(&wc))
		{
			if((desc.hWnd = CreateWindow(desc.className, desc.caption, desc.stylesWnd, desc.bar.x, desc.bar.y, desc.bar.w, desc.bar.h, desc.hWndParent, nullptr, desc.hInst, nullptr)))
			{
				if(is_show_wnd)
				{
					ShowWindow(desc.hWnd, SW_NORMAL);
					UpdateWindow(desc.hWnd);
				}
				return true;
			}
		}
		return false;
	}
	
	bool SSH ssh_is_wrong_lex(const String& str, ssh_wcs errLexs)
	{
		static ssh_wcs wrong_lexem = L",#\"?&^%$:;\'|~<>/!{}[]";

		SSH_TRACE;
		ssh_u l(str.length());
		ssh_wcs err(errLexs ? errLexs : wrong_lexem);
		for(ssh_u i = 0; i < l; i++)
		{
			if(wcschr(err, str[i])) return true;
		}
		return false;
	}
	
	String SSH ssh_num_volume(ssh_u num)
	{
		SSH_TRACE;
		ssh_wcs suffix;
		double fNum((double)num);
		if(num > (1024 * 1024 * 1024))
		{
			suffix = L"ГБ";
			fNum /= 1073741824.0;
		}
		else if(num > (1024 * 1024))
		{
			suffix = L"МБ";
			fNum /= 1048576;
		}
		else if(num > 1024)
		{
			suffix = L"КБ";
			fNum /= 1024.0;
		}
		else
		{
			suffix = L"Б";
		}
		String ret;
		return ret.fmt(L"%.4f %s", fNum, suffix);
	}
	
	String SSH ssh_path_in_range(const String& path, ssh_u range)
	{
		SSH_TRACE;
		ssh_wcs result(path);
		if(path.length() > range)
		{
			int strs[64];
			ssh_u pf(0), pl(ssh_split(L'\\', result, strs, 32) - 1), pll(pl);
			if(pl)
			{
				String resultF, resultL;
				if((strs[1] + strs[pl * 2 + 1] + 5) <= range)
				{
					while(pf < pl)
					{
						String tmpF(resultF + String(result[strs[pf * 2]], strs[pf * 2 + 1]) + L'\\'), tmpL(resultL);
						if(!tmpL.is_empty()) tmpL = L'\\' + tmpL;
						tmpL = String(result[strs[pl * 2]], strs[pl * 2 + 1]) + tmpL;
						if((tmpL.length() + tmpF.length() + 4) > range)
						{
							ssh_u ll(resultF.length() + tmpL.length() + 4);
							ssh_u lf(resultL.length() + tmpF.length() + 4);
							if(ll > range || lf > range) break;
							if(lf > ll) resultL = tmpL; else resultF = tmpF;
							break;
						}
						resultF = tmpF;
						resultL = tmpL;
						pl--;
						pf++;
					}
				}
				if(resultL.is_empty()) resultL = String(result[strs[pll * 2]], strs[pll * 2 + 1]);
				result = resultF + L"...\\" + resultL;
			}
		}
		return result;
	}
	
	String SSH ssh_make_guid(const GUID& guid)
	{
		ssh_ws buf[64];
		_swprintf_p(buf, 64, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		return buf;
	}
	
	String SSH ssh_cnv_string(ssh_u flags, ENUM_DATA* stk, ssh_wcs def, bool enumerate)
	{
		SSH_TRACE;
		String ret;
		ssh_u i(0);
		ssh_wcs _wcs;

		while((_wcs = stk[i]._wcs))
		{
			ssh_u f(stk[i]._val);
			if(enumerate ? flags == f : (flags & f) == f)
			{
				if(!ret.is_empty()) ret += L'|';
				ret += _wcs;
				if(enumerate) break;
			}
			i++;
		}
		return (ret.is_empty() ? def : ret);
	}

	String SSH ssh_translate(ssh_wcs text, bool to_eng)
	{
		static ssh_wcs rus1[] = {	L"Ч", L"Ш", L"Э", L"А", L"Б", L"В", L"Г", L"Д", L"Е", L"Ё", L"Ж", L"З", L"И", L"Й", L"К", L"Л", L"М", L"Н", L"О", L"П", L"Р", L"С", L"Т", L"У", L"Ф", L"Х", L"Ц", L"Щ", L"Ю", L"Я", L"Ь", L"Ъ", L"Ы",
									L"ч", L"ш", L"э", L"а", L"б", L"в", L"г", L"д", L"е", L"ё", L"ж", L"з", L"и", L"й", L"к", L"л", L"м", L"н", L"о", L"п", L"р", L"с", L"т", L"у", L"ф", L"х", L"ц", L"щ", L"ю", L"я", L"ь", L"ъ", L"ы", nullptr };
		static ssh_wcs eng1 = L"Ch\0Sh\0Je\0A\0B\0V\0G\0D\0E\0Jo\0Zh\0Z\0I\0J\0K\0L\0M\0N\0O\0P\0R\0S\0T\0U\0F\0H\0C\0W\0Yu\0Ya\0Q\0X\0Y\0ch\0sh\0je\0a\0b\0v\0g\0d\0e\0jo\0zh\0z\0i\0j\0k\0l\0m\0n\0o\0p\0r\0s\0t\0u\0f\0h\0c\0w\0yu\0ya\0q\0x\0y\0\0";

		static ssh_wcs eng2[] = {	L"Ch", L"Sh", L"Je", L"Yu", L"Ya", L"A", L"B", L"V", L"G", L"D", L"E", L"Jo", L"Zh", L"Z", L"I", L"J", L"K", L"L", L"M", L"N", L"O", L"P", L"R", L"S", L"T", L"U", L"F", L"H", L"C", L"W", L"Q", L"X", L"Y",
									L"ch", L"sh", L"je", L"yu", L"ya", L"a", L"b", L"v", L"g", L"d", L"e", L"jo", L"zh", L"z", L"i", L"j", L"k", L"l", L"m", L"n", L"o", L"p", L"r", L"s", L"t", L"u", L"f", L"h", L"c", L"w", L"q", L"x", L"y", nullptr};
		static ssh_wcs rus2 = L"Ч\0Ш\0Э\0Ю\0Я\0А\0Б\0В\0Г\0Д\0Е\0Ё\0Ж\0З\0И\0Й\0К\0Л\0М\0Н\0О\0П\0Р\0С\0Т\0У\0Ф\0Х\0Ц\0Щ\0Ь\0Ъ\0Ы\0ч\0ш\0э\0ю\0я\0а\0б\0в\0г\0д\0е\0ё\0ж\0з\0и\0й\0к\0л\0м\0н\0о\0п\0р\0с\0т\0у\0ф\0х\0ц\0щ\0ь\0ъ\0ы\0\0" ;

		String txt(text);
		if(to_eng) return txt.replace(rus1, eng1);
		return txt.replace(eng2, rus2);
	}

	String SSH ssh_make_params(ssh_wcs fmt, ...)
	{
		String q;

		va_list args;
		va_start(args, fmt);
		q.fmt(fmt, args);
		va_end(args);

		return q;
	}
}
