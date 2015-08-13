
#include "stdafx.h"
#include "ssh_hlp.h"
#include "ssh_io.h"

namespace ssh
{
	static ssh_wcs wrong_lexem = L",#\"?&^%$:;\'|~<>/!{}[]";
	Map<HMODULE, String, SSH_TYPE, SSH_TYPE>* Helpers::dlls(nullptr);

	ssh_u Bits::total_set() const
	{
		if(hlp->is_cpu_caps(Helpers::SUPPORTS_POPCNT)) return __popcnt64(value);
		ssh_u count(0), total(total_bits());
		ssh_l tmp(value);
		for(ssh_u i = total; i; --i) { count += (tmp & 1); tmp >>= 1; }
		return count;
	}

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return hlp->proc_wndmsg(hWnd, msg, wParam, lParam);
	}

	Helpers::Helpers()
	{
		SSH_TRACE;
		SYSTEM_INFO	SysInfo;
		OSVERSIONINFO OSVersion;
		__int64 startTime, endTime, sampleDelta;

		MemStatus.dwLength = sizeof(MemStatus);
		GlobalMemoryStatus(&MemStatus);

		OSVersion.dwOSVersionInfoSize = sizeof(OSVersion);
		GetVersionEx(&OSVersion);
		GetSystemInfo(&SysInfo);

		osVersion.MajorVersion = OSVersion.dwMajorVersion;
		osVersion.MinorVersion = OSVersion.dwMinorVersion;

		DWORD ver1 = osVersion.MajorVersion;
		DWORD ver2 = osVersion.MinorVersion;

		if(OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
		{
			osVersion.Build = LOWORD(OSVersion.dwBuildNumber);
			platform = WINDOWS_95;

			if(ver2 == 0 && osVersion.Build > 950) platform = WINDOWS_95_SR2;
			else if(ver2 == 10) platform = WINDOWS_98;
			else if(ver2 > 10) platform = WINDOWS_ME;
		}
		else if(OSVersion.dwPlatformId == VER_PLATFORM_WIN32_NT)
		{
			osVersion.Build = OSVersion.dwBuildNumber;
			if(ver1 < 4) platform = WINDOWS_NT;
			else if(ver1 == 4) platform = WINDOWS_2K;
			else if(ver1 == 5) platform = WINDOWS_XP;
			else if(ver1 == 6 && ver2 == 0) platform = WINDOWS_VISTA;
			else if(ver1 == 6 && ver2 == 1) platform = WINDOWS_7;
			else platform = WINDOWS_FUTURE;
		}
		else
		{
			platform = WINDOWS_UNK;
			osVersion.Build = OSVersion.dwBuildNumber;
		}

		cpuFlags = asm_ssh_capability();

		startTime = __rdtsc();
		Sleep(100);
		endTime = __rdtsc();
		sampleDelta = endTime - startTime;
		processorSpeed = sampleDelta / 100;

		system_info(si, CSIDL_LOCAL_APPDATA);
	}

	void Helpers::system_info(String* arr, int csidl) const
	{
		SSH_TRACE;
		if(arr)
		{
			ssh_ws tmp[MAX_PATH];
			ssh_d sz(MAX_PATH);

			::GetTempPath(MAX_PATH, tmp);
			arr[siTempFolder] = slash_path(tmp);
			::GetCurrentDirectory(MAX_PATH, tmp);
			arr[siWorkFolder] = slash_path(tmp);
			::GetModuleFileName(::GetModuleHandle(nullptr), tmp, MAX_PATH);
			arr[siNameProg] = file_title(tmp);
			arr[siProgFolder] = file_path(tmp);
			if(SUCCEEDED(SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, 0, tmp))) arr[siUserFolder] = slash_path(tmp);
			if(SUCCEEDED(SHGetFolderPath(nullptr, csidl, nullptr, 0, tmp))) arr[siCustom] = slash_path(tmp);
			if(GetUserName(tmp, &sz)) arr[siUserName] = tmp;
			if(GetComputerName(tmp, &sz)) arr[siCompName] = tmp;
		}
	}

	bool Helpers::is_wrong_lex(const String& str, ssh_wcs errLexs) const
	{
		SSH_TRACE;
		ssh_u l(str.length());
		ssh_wcs err(errLexs ? errLexs : wrong_lexem);
		for(ssh_u i = 0; i < l; i++)
		{
			if(wcschr(err, str[i])) return true;
		}
		return false;
	}

	ssh_u Helpers::offset_line(const String& text, ssh_l ln) const
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

	String Helpers::num_volume(ssh_u num) const
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

	String Helpers::pathInRange(const String& path, ssh_u range) const
	{
		SSH_TRACE;
		String result(path);
		if(result.length() > range)
		{
			String strs[32];
			ssh_u pf(0), pl(split(L'\\', 0, result, strs, 32, L"") - 1), pll(pl);
			if(pl)
			{
				String resultF, resultL;
				if((strs[0].length() + strs[pl].length() + 5) <= range)
				{
					while(pf < pl)
					{
						String tmpF(resultF + strs[pf] + L'\\'), tmpL(resultL);
						if(!tmpL.is_empty()) tmpL = L'\\' + tmpL;
						tmpL = strs[pl] + tmpL;
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
				if(resultL.is_empty()) resultL = strs[pll];
				result = resultF + L"...\\" + resultL;
			}
		}
		return result;
	}

	void Helpers::make_path(const String& path, bool is_file) const
	{
		SSH_TRACE;
		ssh_ws dir[MAX_PATH];
		ssh_ws* _path(path.buffer());
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

	ssh_u Helpers::split(ssh_ws split, ssh_l skip, const String& src, String* dst, ssh_u count_dst, ssh_wcs def) const
	{
		SSH_TRACE;
		ssh_ws* _wcs(src.buffer());
		ssh_ws* t;
		ssh_u i(0), j;
		while(i < count_dst)
		{
			if((t = wcschr(_wcs, split))) *t = 0;
			if(skip <= 0) dst[i++] = (is_null(_wcs) ? def : _wcs);
			if(t) { *t = split; _wcs = t + 1; } else break;
			skip--;
		}
		j = i;
		// заполняем значениями по умолчанию
		for(; i < count_dst; i++) dst[i] = def;
		return j;
	}

	ssh_u Helpers::cnvValue(ssh_wcs str, ENUM_DATA* stk, ssh_u def) const
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

	String Helpers::cnvString(ssh_u flags, ENUM_DATA* stk, ssh_wcs def, bool enumerate) const
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

	bool Helpers::dlgSelectFolder(ssh_wcs title, String& folder, HWND hWnd) const
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
				folder = slash_path((ssh_wcs)(ssh_ws*)tmp);
				result = true;
			}
			pMalloc->Release();
		}

		return result;
	}

	ssh_u Helpers::dlgSaveOrOpenFile(bool bOpen, ssh_wcs title, ssh_wcs filter, ssh_wcs ext, String& folder, HWND hWnd, String* arr, ssh_u count) const
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

		if((bOpen ? ::GetOpenFileName(&ofn) : ::GetSaveFileName(&ofn)))
		{
			folder = _files;
			_files += (wcslen(_files) + 1);
			if(arr && *_files)
			{
				folder = slash_path(folder);
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

	bool Helpers::make_wnd(const DESC_WND& desc, bool is_show_wnd) const
	{
		SSH_TRACE;

		WNDCLASSEX wc;
		if(desc.className.is_empty()) desc.className = gen_name(L"");

		wc.cbSize = sizeof(WNDCLASSEX);
		wc.style = desc.stylesClass;
		wc.lpfnWndProc = desc.processingWnd ? desc.processingWnd : WndProc;
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
			if((desc.hWnd = CreateWindow(desc.className, desc.caption, desc.stylesWnd,
				desc.bar.x, desc.bar.y, desc.bar.w, desc.bar.h, desc.hWndParent, nullptr, desc.hInst, nullptr)))
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

	LRESULT Helpers::proc_wndmsg(HWND hWnd, UINT msg, WPARAM w, LPARAM l) const
	{
		switch(msg)
		{
			case WM_ERASEBKGND:
				return 1;
			case WM_KEYUP:
				keyboard->setKey((BYTE)w, LOWORD(l), false);
				break;
			case WM_KEYDOWN:
				keyboard->setKey((BYTE)w, LOWORD(l), true);
				break;
			case WM_CHAR:
				keyboard->setSym((BYTE)w, LOWORD(l));
				break;
			case WM_MOUSEWHEEL:
				mouse->setWheelStatus(HIWORD(w));
				break;
			case WM_LBUTTONDBLCLK:
			case WM_RBUTTONDBLCLK:
			case WM_MBUTTONDBLCLK:
				mouse->setDoubleClickStatus(msg == WM_MBUTTONDBLCLK, msg == WM_RBUTTONDBLCLK, msg == WM_LBUTTONDBLCLK);
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MOUSEMOVE:
			case WM_MBUTTONDOWN:
			case WM_MBUTTONUP:
				mouse->update(Pts<ssh_u>(LOWORD(l), HIWORD(l)), (long)w);
				break;
		}
		return ::DefWindowProc(hWnd, msg, w, l);
	}

	GUID Helpers::make_guid(ssh_wcs src) const
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
		_guid.Data4[0] = _wp[1];
		_guid.Data4[1] = _wp[0];
		_guid.Data4[2] = _up[5];
		_guid.Data4[3] = _up[4];
		_guid.Data4[4] = _up[3];
		_guid.Data4[5] = _up[2];
		_guid.Data4[6] = _up[1];
		_guid.Data4[7] = _up[0];
		return _guid;
	}

	String Helpers::make_guid(const GUID& guid) const
	{
		SSH_TRACE;
		ssh_ws buf[64];
		_swprintf_p(buf, 64, L"{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", guid.Data1, guid.Data2, guid.Data3, guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
		return buf;
	}

	void Helpers::remove_comments(String* lst, ssh_u count, bool is_simple)
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
					String path(lst[i]), npath(file_path_title(path) + L'+' + file_ext(path, true));
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

	void* Helpers::get_procedure(const String& dll, ssh_ccs proc, ssh_wcs _suffix)
	{
		HMODULE hdll;
		String module(file_path_title(dll));
		if(!dlls) dlls = new Map<HMODULE, String, SSH_TYPE, SSH_TYPE>(1000);
#ifdef _DEBUG
		if(_suffix) module += _suffix;
#endif
		module += (file_ext(dll, true));
		if(!(hdll = (*dlls)[module]))
		{
			if(!(hdll = LoadLibrary(module))) return nullptr;
			(*dlls)[module] = hdll;
		}
		return GetProcAddress(hdll, proc);
	}
}
