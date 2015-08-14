
#include "stdafx.h"
#include "ssh_str.h"

namespace ssh
{
	ssh_wcs String::_empty = L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

	__regx_compile regx::_compile((__regx_compile)ssh_dll_proc(L"sshREGX.dll", "regex16_compile"));
	__regx_exec regx::_exec((__regx_exec)ssh_dll_proc(L"sshREGX.dll", "regex16_exec"));
	__regx_free regx::_free((__regx_free)ssh_dll_proc(L"sshREGX.dll", "regex_free"));

	const String& String::operator = (const Buffer<ssh_cs>& ccs)
	{
		ssh_u len(ccs.count());
		if(alloc(len, false))
		{
			MultiByteToWideChar(CP_ACP, 0, ccs, (int)len, buf, (int)len);
			buf[len] = 0;
			data()->update();
		}
		return *this;
	}

	String::String(ssh_ccs ccs, ssh_u len)
	{
		init();
		if(ccs)
		{
			ssh_u t(strlen(ccs));
			if(len > t) len = t;
			if(alloc(len, false))
			{
				MultiByteToWideChar(CP_ACP, 0, ccs, (int)len, buf, (int)len);
				buf[len] = 0;
				data()->update();
			}
		}
	}

	const String& String::add(ssh_wcs wcs, ssh_u len)
	{
		ssh_u l(length());
		if(alloc(l + len, true))
		{
			memcpy(buf + l, wcs, len * 2);
			buf[l + len] = 0;
			data()->update();
		}
		return *this;
	}

	bool String::alloc(ssh_u sz, bool is_copy)
	{
		if(sz)
		{
			STRING_BUFFER* buffer(data());
			ssh_d nsz((ssh_d)sz + 1);
			if(nsz > buffer->len_buf)
			{
				if(nsz < 8192) { nsz = ssh_pow2<ssh_d>(nsz, true) * 2; if(nsz < 32) nsz = 32; }
				// выделим память под новый буфер
				buffer = (STRING_BUFFER*)new ssh_b[sizeof(STRING_BUFFER) + nsz * sizeof(ssh_ws)];
				// скопировать старый, если необходимо
				if(is_copy && !is_empty()) memcpy(buffer->data(), buf, length() * 2);
				// очистить старый
				empty();
				// инициализируем новый
				buffer->len_buf = nsz;
				buf = buffer->data();
			}
			buffer->len = (ssh_d)sz;
			return true;
		}
		empty();
		return false;
	}

	String String::substr(ssh_u idx, ssh_u len) const
	{
		ssh_u l(length());
		if(idx > l) return String();
		if(len == -1) len = l;
		if((idx + len) > l) len = (l - idx);
		return String(buf + idx, len);
	}

	String String::add(ssh_wcs wcs1, ssh_u len1, ssh_wcs wcs2, ssh_u len2)
	{
		String ret(L'\0', len1 + len2);
		if(wcs1) SSH_MEMCPY(ret.buffer(), wcs1, len1 * 2);
		if(wcs2) SSH_MEMCPY(ret.buffer() + len1, wcs2, len2 * 2);
		return ret;
	}

	const String& String::replace(ssh_wcs _old, ssh_wcs _new)
	{
		ssh_u nOld(SSH_STRLEN(_old)), nNew(SSH_STRLEN(_new)), nLen(length()), nCount(0), nDstOffs(0), nSrcOffs(0), nDiff;
		ssh_ws* f(buf);
		// расчитать новый размер
		while((f = wcsstr(f, _old))) nCount++, f += nOld;
		nDiff = nNew - nOld;
		if(nNew > nOld) nDstOffs = nDiff; else nSrcOffs = -nDiff;
		ssh_u l(nDiff * nCount);
		// проверка на вместительность буфера
		if(alloc(nLen + l, true))
		{
			ssh_ws* _buf(buf);
			// непосредственно замена
			while((f = wcsstr(_buf, _old)))
			{
				l = (nLen - ((f + nSrcOffs) - buf));
				memmove(f + nDstOffs, f + nSrcOffs, l * 2);
				memcpy(f, _new, nNew * 2);
				_buf = f + nNew;
				nLen += nDiff;
			}
			buf[nLen] = 0;
			data()->update();
		}
		return *this;
	}

	const String& String::replace(ssh_ws _old, ssh_ws _new)
	{
		ssh_ws* ptr(buf);
		while(*ptr) { if(*ptr == _old) *ptr = _new; ptr++; }
		data()->update();
		return *this;
	}

	const String& String::remove(ssh_wcs wcs)
	{
		ssh_u nWcs(SSH_STRLEN(wcs)), nLen(length());
		ssh_ws* f(buf);
		while((f = wcsstr(f, wcs))) { nLen -= nWcs; SSH_MEMCPY(f, f + nWcs, ((nLen - (f - buf)) + 1) * 2); }
		data()->len = (ssh_d)nLen;
		data()->update();
		return *this;
	}

	const String& String::remove(ssh_ws ws)
	{
		ssh_ws* ptr(buf), *rem(buf);
		while(*ptr) { if(*ptr != ws) *rem++ = *ptr; ptr++; }
		*rem = 0;
		data()->len -= (ssh_d)(ptr - rem);
		data()->update();
		return *this;
	}

	const String& String::remove(ssh_u idx, ssh_u len)
	{
		ssh_u l(length());
		if(idx < l)
		{
			if(len == -1) len = l;
			if((idx + len) > l) len = (l - idx);
			ssh_u ll(idx + len);
			SSH_MEMCPY(buf + idx, buf + ll, ((l - ll) + 1) * 2);
			data()->len -= (ssh_d)len;
			data()->update();
		}
		return *this;
	}

	const String& String::insert(ssh_u idx, ssh_wcs wcs)
	{
		ssh_u len(length()), nWcs(SSH_STRLEN(wcs));
		if(idx < len && alloc(len + nWcs, true))
		{
			memmove(buf + idx + nWcs, buf + idx, ((len - idx) + 1) * 2);
			memcpy(buf + idx, wcs, nWcs * 2);
			data()->update();
		}
		return *this;
	}

	const String& String::insert(ssh_u idx, ssh_ws ws)
	{
		ssh_u len(length());
		if(idx < len && alloc(len + 1, true))
		{
			memmove(buf + idx + 1, buf + idx, ((len - idx) + 1) * 2);
			buf[idx] = ws;
			data()->update();
		}
		return *this;
	}

	const String& String::fmt(ssh_wcs pattern, ...)
	{
		va_list argList;
		va_start(argList, pattern);
		fmt(pattern, argList);
		va_end(argList);
		return *this;
	}

	const String& String::fmt(ssh_wcs pattern, va_list argList)
	{
		int sz(_vscwprintf(pattern, argList));
		if(alloc(sz, false))
		{
			vswprintf(buf, sz + 1, pattern, argList);
			buf[sz] = 0;
			data()->update();
		}
		return *this;
	}

	const String& String::replace(ssh_wcs* _old, ssh_wcs _new)
	{
		ssh_u idx(0);
		while(_old[idx]) replace(_old[idx++], _new), _new += (wcslen(_new) + 1);
		return *this;
	}

	const String& String::load(ssh_u id)
	{
		HINSTANCE hInst(::GetModuleHandle(nullptr));
		HRSRC h(::FindResourceW(hInst, MAKEINTRESOURCE((((id >> 4) + 1) & static_cast<WORD>(~0))), RT_STRING));
		ssh_u len(::SizeofResource(hInst, h) / sizeof(ssh_ws));
		if(alloc(len, false))
		{
			::LoadString(hInst, (UINT)id, buf, (int)len);
			buf[len] = 0;
			data()->update();
		}
		return *this;
	}

	const bool is_chars(ssh_ws* ws, ssh_wcs wcs, ssh_u ln)
	{
		for(ssh_u i = 0; i < ln; i++)
		{
			if(*ws == wcs[i]) return true;
		}
		return (*ws == L' ');
	}

	const String& String::trim_left(ssh_wcs wcs)
	{
		ssh_u len(length()), ln(wcslen(wcs));
		ssh_ws* _ws(buf);
		while(is_chars(_ws, wcs, ln)) { len--; _ws++; }
		memcpy(buf, _ws, (len + 1) * 2);
		data()->len = (ssh_d)len;
		data()->update();
		return *this;
	}

	const String& String::trim_right(ssh_wcs wcs)
	{
		ssh_u len(length()), ln(wcslen(wcs));
		ssh_ws* _ws(buf + len - 1);
		while(is_chars(_ws, wcs, ln)) { len--; _ws--; }
		buf[len] = 0;
		data()->len = (ssh_d)len;
		data()->update();
		return *this;
	}

	String regx::substr(ssh_l idx)
	{
		String ret;
		if(idx < result && idx >= 0 && len(idx))
		{
			ssh_u offs(vector[idx * 2 + 1]);
			ssh_ws ws(subj[offs]);
			subj[offs] = L'\0';
			ret = (subj + vector[idx * 2]);
			subj[offs] = ws;
		}
		return ret;
	}

	void regx::replace(String& subject, ssh_wcs repl, ssh_u idx_ptrn, ssh_l idx)
	{
		ssh_l nWcs(wcslen(repl));
		while(match(subject, idx_ptrn, idx) > 0)
		{
			idx = vector[0];
			subject.remove(idx, vector[1] - idx);
			subject.insert(idx, repl);
			idx += nWcs;
		}
	}
}
