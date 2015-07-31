
#include "stdafx.h"

#include "ssh_str.h"
#include "ssh_buf.h"
#include "ssh_hlp.h"

namespace ssh
{
	ssh_wcs String::_empty = L"\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
	__regx_compile regx::_compile((__regx_compile)hlp->get_procedure(L"sshREGX.dll", "regex16_compile"));
	__regx_exec regx::_exec((__regx_exec)hlp->get_procedure(L"sshREGX.dll", "regex16_exec"));
	__regx_free regx::_free((__regx_free)hlp->get_procedure(L"sshREGX.dll", "regex_free"));

	const String& String::operator = (const Buffer<ssh_cs>& ccs)
	{
		ssh_u len(ccs.count());
		if(alloc(len))
		{
			MultiByteToWideChar(CP_ACP, 0, ccs, (int)len, buf, (int)len);
			data()->update();
		}
		return *this;
	}

	String::String(ssh_wcs wcs, ssh_u len)
	{
		init();
		if(wcs)
		{
			ssh_u t(SSH_STRLEN(wcs));
			make(wcs, len > t ? t : len);
		}
	}
	
	String::String(ssh_ccs ccs, ssh_u len)
	{
		init();
		if(ccs)
		{
			ssh_u t(strlen(ccs));
			if(len > t) len = t;
			if(alloc(len))
			{
				MultiByteToWideChar(CP_ACP, 0, ccs, (int)len, buf, (int)len);
				data()->update();
			}
		}
	}

	const String& String::add(ssh_wcs wcs, ssh_u len)
	{
		STRING_BUFFER* sb(data());
		if((sb->len + len) < sb->len_buf)
		{
			// в старый буфер все помещается
			SSH_MEMCPY(buf + sb->len, wcs, len * 2);
			buf[sb->len + len] = 0;
			sb->len += len;
			sb->update();
		}
		else
		{
			// перевыделение буфера
			ssh_u l(length());
			Buffer<ssh_ws> ptr(buf, true, l);
			if(alloc(l + len))
			{
				SSH_MEMCPY((ssh_ws*)SSH_MEMCPY(buf, ptr, ptr.size()) + l, wcs, len * 2);
				data()->update();
			}
		}
		return *this;
	}
	
	const String& String::make(ssh_wcs wcs, ssh_u len)
	{
		if(alloc(len))
		{
			SSH_MEMCPY(buf, wcs, len * 2);
			data()->update();
		}
		return *this;
	}

	bool String::alloc(ssh_u sz)
	{
		if(sz)
		{
			STRING_BUFFER* buffer(data());
			ssh_u nsz(sz + 1);
			if(nsz > buffer->len_buf)
			{
				if(nsz < 8192)
				{
					ssh_d idx;
					_BitScanReverse64(&idx, nsz);
					idx++;
					nsz = (ssh_u)(1 << idx);
					if(nsz < 32) nsz = 32;
				}
				empty();
				buffer = (STRING_BUFFER*)new ssh_ws[sizeof(STRING_BUFFER) / 2 + nsz];
				buffer->len_buf = nsz;
				buf = buffer->data();
			}
			buffer->len = sz;
			buf[sz] = 0;
			return true;
		}
		init();
		return false;
	}

	String String::substr(ssh_u idx, ssh_u len) const
	{
		SSH_TRACE;
		ssh_u l(length());
		if(idx > l) return String();
		if(len == -1) len = l;
		if((idx + len) > l) len = (l - idx);
		return String(buf + idx, len);
	}

	String String::add(ssh_wcs wcs1, ssh_u len1, ssh_wcs wcs2, ssh_u len2)
	{
		if(wcs1 && wcs2)
		{
			Buffer<ssh_ws> ptr(len1 + len2 + 1);
			SSH_MEMCPY((ssh_ws*)SSH_MEMCPY(ptr, wcs1, len1 * 2) + len1, wcs2, len2 * 2);
			ptr[len1 + len2] = 0;
			return String(ptr, len1 + len2);
		}
		return String();
	}

	const String& String::replace(ssh_wcs _old, ssh_wcs _new)
	{
		SSH_TRACE;
		ssh_u nOld(SSH_STRLEN(_old)), nNew(SSH_STRLEN(_new)), nLen(length()), pos(0), nCount(0);
		ssh_ws* f(buf);
		//ssh_l p;
		Buffer<ssh_ws> ptr(buf, true, nLen + 1);
		// расчитать новый размер
		while((f = wcsstr(f, _old))) nCount++, f += nOld;
		nLen -= (nOld - nNew) * nCount;
		if(alloc(nLen))
		{
			ssh_ws *tmp(buf), *pptr(ptr);
			while((f = wcsstr(pptr, _old)))
			{
				ssh_l l(f - pptr);
				SSH_MEMCPY((ssh_ws*)SSH_MEMCPY(tmp, pptr, l * 2) + l, _new, nNew * 2);
				pptr += (l + nOld);
				tmp += (l + nNew);
			}
			SSH_STRCPY(tmp, pptr);
			data()->update();
		}
		return *this;
	}
	
	const String& String::replace(ssh_ws _old, ssh_ws _new)
	{
		SSH_TRACE;
		ssh_ws* ptr(buf);
		while(*ptr) { if(*ptr == _old) *ptr = _new; ptr++; }
		data()->update();
		return *this;
	}
	
	const String& String::remove(ssh_wcs wcs)
	{
		SSH_TRACE;
		ssh_u nWcs(SSH_STRLEN(wcs)), nLen(length()), pos(0);
		ssh_ws* f;
		while((f = wcsstr(buf + pos, wcs)))
		{
			pos = (f - buf);
			nLen -= nWcs;
			SSH_MEMCPY(f, f + nWcs, (nLen - pos)* 2);
		}
		buf[nLen] = 0;
		data()->len = nLen;
		data()->update();
		return *this;
	}
	
	const String& String::remove(ssh_ws ws)
	{
		SSH_TRACE;
		ssh_ws* ptr(buf), *rem(buf);
		while(*ptr) { if(*ptr != ws) *rem++ = *ptr; ptr++; }
		*rem = 0;
		data()->len -= (ptr - rem);
		data()->update();
		return *this;
	}
	
	const String& String::remove(ssh_u idx, ssh_u len)
	{
		SSH_TRACE;
		ssh_u l(length());
		if(idx < l)
		{
			if(len == -1) len = l;
			if((idx + len) > l) len = (l - idx);
			ssh_u ll(idx + len);
			SSH_MEMCPY(buf + idx, buf + ll, ((l - ll) + 1) * 2);
			data()->len -= len;
			data()->update();
		}
		return *this;
	}
	
	const String& String::insert(ssh_u idx, ssh_wcs wcs)
	{
		SSH_TRACE;
		ssh_u len(length()), nWcs(SSH_STRLEN(wcs));
		if(idx < len)
		{
			Buffer<ssh_ws> ptr(buf, true, len);
			if(alloc(len + nWcs))
			{
				SSH_MEMCPY((ssh_ws*)SSH_MEMCPY((ssh_ws*)SSH_MEMCPY(buf, ptr, idx * 2) + idx, wcs, nWcs * 2) + nWcs, &ptr[idx], (len - idx) * 2);
				data()->update();
			}
		}
		return *this;
	}
	
	const String& String::insert(ssh_u idx, ssh_ws ws)
	{
		SSH_TRACE;
		ssh_u len(length());
		if(idx < len)
		{
			Buffer<ssh_ws> ptr(buf, true, len);
			if(alloc(len + 1))
			{
				SSH_MEMCPY((ssh_ws*)SSH_MEMCPY(buf, ptr, idx * 2) + idx + 1, &ptr[idx], (len - idx) * 2);
				buf[idx] = ws;
				data()->update();
			}
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
		if(alloc(sz))
		{
			vswprintf(buf, sz + 1, pattern, argList);
			data()->update();
		}
		return *this;
	}

	const String& String::replace(ssh_wcs* _old, ssh_wcs _new)
	{
		SSH_TRACE;
		ssh_u idx(0);
		ssh_wcs o;
		while((o = _old[idx++]))
		{
			replace(o, _new);
			_new += (wcslen(_new) + 1);
		}
		return *this;
	}

	const String& String::load(ssh_u id)
	{
		SSH_TRACE;
		HINSTANCE hInst(::GetModuleHandle(nullptr));
		HRSRC h(::FindResourceW(hInst, MAKEINTRESOURCE((((id >> 4) + 1) & static_cast<WORD>(~0))), RT_STRING));
		if(alloc(::SizeofResource(hInst, h)))
		{
			::LoadString(hInst, (UINT)id, buf, (int)length());
			data()->len = wcslen(buf);
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
		ssh_ws* _ws(buffer());
		while(is_chars(_ws, wcs, ln)) { len--; _ws++; }
		memcpy(buf, _ws, len * 2);
		buf[len] = 0;
		data()->len = len;
		data()->update();
		return *this;
	}
	
	const String& String::trim_right(ssh_wcs wcs)
	{
		ssh_u len(length()), ln(wcslen(wcs));
		ssh_ws* _ws(buffer() + len - 1);
		while(is_chars(_ws, wcs, ln)) { len--; _ws--; }
		buf[len] = 0;
		data()->len = len;
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
