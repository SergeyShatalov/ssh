
#include "stdafx.h"
#include "ssh_globals.h"
#include <Common\ssh_ext.h>

namespace ssh
{
	typedef ssh_cs* (CALLBACK* __ext_undname)(ssh_cs* out, ssh_ccs name, int len_out, ssh_d flags);
	typedef void* (CALLBACK* __cnv_open)(ssh_wcs to, ssh_wcs from);
	typedef int (CALLBACK* __cnv_close)(void* h);
	typedef size_t(CALLBACK* __cnv_make)(void* cd, const char** inbuf, size_t* inbytesleft, char** outbuf, size_t* outbytesleft);

	static __ext_undname _und((__ext_undname)hlp->get_procedure(L"sshEXT", "ext_undname"));
	static __cnv_open _open((__cnv_open)hlp->get_procedure(L"sshCNV.dll", "iconv_open"));
	static __cnv_close _close((__cnv_close)hlp->get_procedure(L"sshCNV.dll", "iconv_close"));
	static __cnv_make _make((__cnv_make)hlp->get_procedure(L"sshCNV.dll", "iconv"));

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
		if(hlp->is_cpu_caps(Helpers::SUPPORTS_RDRAND)) is = (_rdrand64_step(&tmp) == 1);
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
		return Buffer<ssh_cs>(asm_ssh_from_base64(str.buffer(), str.length(), &len_buf, is_null * 2), len_buf, false);
	}

	String SSH ssh_base64(ssh_wcs charset, const String& str)
	{
		return ssh_base64(ssh_cnv(charset, str, false));
	}

	String SSH ssh_base64(const Buffer<ssh_cs>& buf)
	{
		return String(Buffer<ssh_ws>(asm_ssh_to_base64(buf, buf.count()), 1, false).to<ssh_ws>());
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

	Buffer<ssh_cs> SSH ssh_cnv(ssh_wcs to, const String& str, bool is_null)
	{
		iconv_t h;
		Buffer<ssh_cs> out((str.length() + is_null) * 2);
		ssh_u in_c(out.count()), out_c(in_c);
		ssh_ccs _in((ssh_ccs)str.buffer());
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
		ssh_u in_c(in.count() - offs);
		String out(L'\0', in_c);
		ssh_u out_c(out.length() * 2);
		ssh_ccs _in(in + offs);
		ssh_cs* _out((ssh_cs*)out.buffer());
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
		out.update();
		return out;
	}
}
