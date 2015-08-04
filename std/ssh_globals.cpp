
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
		if(singltons[index]) SSH_THROW(L"Синглтон с индексом %i уже существует!", index);
		singltons[index] = ptr;
	}

	long SSH ssh_cpu_caps()
	{
#define _EAX	0
#define _EBX	1
#define _ECX	2
#define _EDX	3

		int info[4]; // EAX, EBX, ECX и EDX.
		Bits<long> res(0);
		__cpuid(info, 1);
		Bits<int>* _info((Bits<int>*)&info[0]);

		if(_info[_ECX].testBit(0)) res.setBit(Helpers::SUPPORTS_SSE3);
		if(_info[_ECX].testBit(1)) res.setBit(Helpers::SUPPORTS_PCLMULQDQ);
		if(_info[_ECX].testBit(9)) res.setBit(Helpers::SUPPORTS_SSSE3);
		if(_info[_ECX].testBit(12)) res.setBit(Helpers::SUPPORTS_FMA);
		if(_info[_ECX].testBit(13)) res.setBit(Helpers::SUPPORTS_CMPXCHG16B);
		if(_info[_ECX].testBit(19)) res.setBit(Helpers::SUPPORTS_SSE4_1);
		if(_info[_ECX].testBit(20)) res.setBit(Helpers::SUPPORTS_SSE4_2);
		if(_info[_ECX].testBit(22)) res.setBit(Helpers::SUPPORTS_MOVBE);
		if(_info[_ECX].testBit(23)) res.setBit(Helpers::SUPPORTS_POPCNT);
		if(_info[_ECX].testBit(25)) res.setBit(Helpers::SUPPORTS_AES);
		if(_info[_ECX].testBit(28)) res.setBit(Helpers::SUPPORTS_AVX);
		if(_info[_ECX].testBit(30)) res.setBit(Helpers::SUPPORTS_RDRAND);

		if(_info[_EDX].testBit(15)) res.setBit(Helpers::SUPPORTS_CMOV);
		if(_info[_EDX].testBit(23)) res.setBit(Helpers::SUPPORTS_MMX);
		if(_info[_EDX].testBit(25)) res.setBit(Helpers::SUPPORTS_SSE);
		if(_info[_EDX].testBit(26)) res.setBit(Helpers::SUPPORTS_SSE2);

		if(_info[_EBX].testBit(3)) res.setBit(Helpers::SUPPORTS_BMI1);
		if(_info[_EBX].testBit(5)) res.setBit(Helpers::SUPPORTS_AVX2);
		if(_info[_EBX].testBit(8)) res.setBit(Helpers::SUPPORTS_BMI2);
		if(_info[_EBX].testBit(16)) res.setBit(Helpers::SUPPORTS_AVX512F);
		if(_info[_EBX].testBit(18)) res.setBit(Helpers::SUPPORTS_RDSEED);
		if(_info[_EBX].testBit(26)) res.setBit(Helpers::SUPPORTS_AVX512PF);
		if(_info[_EBX].testBit(27)) res.setBit(Helpers::SUPPORTS_AVX512ER);
		if(_info[_EBX].testBit(28)) res.setBit(Helpers::SUPPORTS_AVX512CD);
		return res;
	}

	ssh_u SSH ssh_rand(ssh_u begin, ssh_u end)
	{
		static ssh_u _genRnd(0);
		ssh_u tmp(_genRnd);
		if(!tmp) tmp = _time64((__time64_t*)tmp);
		tmp *= 1103515245;
		_genRnd = tmp;
		tmp = ((tmp >> 16) & 0x7fff);
		return begin + (tmp % ((end - begin) + 1));
	}

	static ssh_ccs base64_chars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

	static inline bool is_base64(ssh_cs c)
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

	Buffer<ssh_cs> SSH ssh_base64(const String& str, bool is_null)
	{
		ssh_l pos(str.find(L'='));
		ssh_u i(0), j(0), _c(str.length()), in_len(pos >= 0 ? pos : _c);
		ssh_cs char_array_4[4], char_array_3[3];
		ssh_u out_len(_c / 4 * 3 - (_c - in_len));
		Buffer<ssh_cs> ret(out_len + (is_null * 2));
		ssh_cs* ptr(ret);
		ssh_ws* buf(str.buffer());
		while(in_len-- && is_base64((ssh_cs)*buf))
		{
			char_array_4[i++] = (ssh_cs)*buf++;
			if(i == 4)
			{
				for(i = 0; i < 4; i++) char_array_4[i] = (ssh_cs)(strchr(base64_chars, char_array_4[i]) - base64_chars);
				*ptr++ = ((char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4));
				*ptr++ = (((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2));
				*ptr++ = (((char_array_4[2] & 0x3) << 6) + char_array_4[3]);
				i = 0;
			}
		}
		if(i)
		{
			for(j = i; j < 4; j++) char_array_4[j] = 0;
			for(j = 0; j < 4; j++) char_array_4[j] = (ssh_cs)(strchr(base64_chars, char_array_4[j]) - base64_chars);
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
			for(j = 0; (j < i - 1); j++) *ptr++ = char_array_3[j];
		}
		if(is_null) *(ssh_ws*)ptr = 0;
		return ret;
	}

	String SSH ssh_base64(ssh_wcs charset, const String& str)
	{
		return ssh_base64(ssh_cnv(charset, str, false));
	}

	String SSH ssh_base64(const Buffer<ssh_cs>& buf)
	{
		ssh_u i = 0, j = 0, _ret(0), pos(0), len(buf.count());
		ssh_u len_buf((len / 3 * 4) + ((len % 3) ? 4 : 0));
		String ret(L'\0', len_buf + 1);
		ssh_cs char_array_3[3], char_array_4[4];
		ssh_cs* ptr(buf);
		ssh_ws* _ws(ret.buffer());
		while(len--)
		{
			char_array_3[i++] = *ptr++;
			if(i == 3)
			{
				char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
				char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
				char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
				char_array_4[3] = char_array_3[2] & 0x3f;
				for(i = 0; i < 4; i++) *_ws++ = base64_chars[char_array_4[i]];
				i = 0;
			}
		}

		if(i)
		{
			for(j = i; j < 3; j++) char_array_3[j] = 0;
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;
			for(j = 0; (j < i + 1); j++) *_ws++ = base64_chars[char_array_4[j]];
			while((i++ < 3)) *_ws++ = '=';
		}
		return ret;
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
		Buffer<ssh_cs> out(str.length() * 2 + (is_null * 2));
		ssh_u in_c(str.length() * 2 + (is_null * 2));
		ssh_u out_c(out.count());
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
		return Buffer<ssh_cs>(out, BUFFER_COPY | BUFFER_RESET, _out - out);
	}
	
	String SSH ssh_cnv(ssh_wcs from, const Buffer<ssh_cs>& in, ssh_u offs)
	{
		iconv_t h;
		String out(L'\0', in.count() * 4);
		ssh_u in_c(in.count() - offs);
		ssh_u out_c(out.length());
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
		return String(out);
	}
}
