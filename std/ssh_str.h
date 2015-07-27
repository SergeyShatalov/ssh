
#pragma once

namespace ssh
{
	template <typename T> class Range;
	template <typename T> class Buffer;

	ssh_u SSH ssh_hash(ssh_wcs wcs);

	class SSH String
	{
	public:
		enum Radix : int
		{
			_bin = 2,
			_oct = 8,
			_dec = 10,
			_hex = 16
		};
		// конструкторы
		String() { init(); }
		String(String&& str) { buf = str.buf; str.init(); }
		String(ssh_wcs wcs, ssh_u len = -1);
		String(ssh_ccs ccs, ssh_u len = -1);
		String(const Buffer<ssh_cs>& buf) { init(); *this = buf; }
		String(const String& str) { init(); *this = str; }
		String(ssh_ws ws, ssh_u rep) { init(); if(alloc(rep)) _wcsset(buf, ws); }
		template <typename T> String(T v, Radix r) { init(); fromNum(v, (int)r); }
		// деструктор
		~String() { empty(); }
		// привидение типа
		operator ssh_wcs() const { return buf; }
		operator bool() const { return compare(L"true"); }
		template <typename T> T toNum(ssh_u idx, int R = 10) const { return (T)_wcstoi64(buf + idx, nullptr, R); }
		template <typename T> void fromNum(T v, int R = 10) { ssh_ws tmp[128]; _i64tow((ssh_u)v, tmp, R); *this = tmp; }
		ssh_ws operator[](ssh_u idx) const { return get(idx); }
		// операторы сравнени€
		friend bool operator == (const String& str1, const String& str2) { return (str1.hash() == str2.hash()); }
		friend bool operator == (const String& str, ssh_wcs wcs) { return (SSH_STRCMP(str, wcs) == 0); }
		friend bool operator == (ssh_wcs wcs, const String& str) { return (SSH_STRCMP(wcs, str) == 0); }
		friend bool operator != (const String& str1, const String& str2) { return !(operator == (str1, str2)); }
		friend bool operator != (const String& str, ssh_wcs wcs) { return !(operator == (str, wcs)); }
		friend bool operator != (ssh_wcs wcs, const String& str) { return !(operator == (wcs, str)); }
		// операторы присваивани€
		const String& operator = (const String& str) { return make(str, str.length()); }
		const String& operator = (const Buffer<ssh_cs>& buf);
		const String& operator = (String&& str) { empty(); buf = str.buf; str.init(); return *this; }
		const String& operator = (ssh_ws ws) { return make((ssh_wcs)&ws, 1); }
		const String& operator = (ssh_wcs wcs) { return (wcs ? make(wcs, SSH_STRLEN(wcs)) : *this); }
		// операторы контакенции
		const String& operator += (const String& str) { return add(str, str.length()); }
		const String& operator += (ssh_ws ws) { return add((ssh_wcs)&ws, 1); }
		const String& operator += (ssh_wcs wcs) { return (wcs ? add(wcs, SSH_STRLEN(wcs)) : *this); }
		// дружественные операторы
		friend String operator + (ssh_ws ws, const String& str) { return String::add((ssh_wcs)&ws, 1, str, str.length()); }
		friend String operator + (ssh_wcs wcs, const String& str) { return String::add(wcs, SSH_STRLEN(wcs), str, str.length()); }
		friend String operator + (const String& str1, const String& str2) { return String::add(str1, str1.length(), str2, str2.length()); }
		friend String operator + (const String& str, ssh_ws ws) { return String::add(str, str.length(), (ssh_wcs)&ws, 1); }
		friend String operator + (const String& str, ssh_wcs wcs) { return String::add(str, str.length(), wcs, SSH_STRLEN(wcs)); }
		// методы
		ssh_ws* buffer() const { return buf; }
		ssh_u length() const { return data()->len; }
		ssh_ws get(ssh_u idx) const { return (idx >= length() ? L'0' : buf[idx]); }
		void set(ssh_u idx, ssh_ws ws) { if(idx < length()) buf[idx] = ws; }
		void empty() { if(!is_empty()) { delete data(); init(); } }
		bool is_empty() const { return (buf == (ssh_ws*)((ssh_cs*)_empty + sizeof(STRING_BUFFER)));; }
		bool compare(ssh_wcs wcs) const { return (_wcsicmp(buf, wcs) == 0); }
		ssh_u hash() const { return data()->hash; }
		// модификаци€
		ssh_cs* ccs(ssh_cs* ccs, int sz_ccs) const { WideCharToMultiByte(CP_ACP, 0, buf, (int)length() + 1, ccs, (int)sz_ccs, 0, 0); return ccs; }
		const String& load(ssh_u id);
		const String& lower() { _wcslwr(buf); return *this; }
		const String& upper() { _wcsupr(buf); return *this; }
		const String& reverse() { _wcsrev(buf); return *this; }
		const String& replace(ssh_wcs _old, ssh_wcs _new);
		const String& replace(ssh_ws _old, ssh_ws _new);
		const String& replace(ssh_wcs _old, ssh_wcs _new, ssh_u count);
		const String& remove(ssh_wcs wcs);
		const String& remove(ssh_ws ws);
		const String& remove(ssh_u idx, ssh_u len = -1);
		const String& fmt(ssh_wcs pattern, ...);
		const String& fmt(ssh_wcs pattern, va_list argList);
		const String& insert(ssh_u idx, ssh_wcs wcs);
		const String& insert(ssh_u idx, ssh_ws ws);
		const String& trim() { return trim(L" "); }
		const String& trim(ssh_wcs wcs) { trim_left(wcs); return trim_right(wcs); }
		const String& trim_left(ssh_wcs wcs);
		const String& trim_right(ssh_wcs wcs);
		// поиск
		ssh_l find(ssh_wcs wcs, ssh_u idx = 0) const { return (idx >= length() ? -1 : (ssh_l)(wcsstr(buf + idx, wcs) - buf)); }
		ssh_l find(ssh_ws ws, ssh_u idx = 0) const { return (idx >= length() ? -1 : (ssh_l)(wcschr(buf + idx, ws) - buf)); }
		ssh_l find_rev(ssh_ws ws) const { return (ssh_l)(wcsrchr(buf, ws) - buf); }
		String substr(ssh_u idx, ssh_u len = -1) const;
		String left(ssh_u idx) const { return substr(0, idx); }
	protected:
		struct STRING_BUFFER
		{
			// длина данных
			ssh_u len;
			// длина буфера
			ssh_u len_buf;
			// хэш
			ssh_u hash;
			// вернуть указатель на буфер
			ssh_ws* data() { return (ssh_ws*)(this + 1); }
			// пересчитать хэш
			void update() { hash = ssh_hash(data()); }
		};
		static String add(ssh_wcs wcs1, ssh_u len1, ssh_wcs wcs2, ssh_u len2);
		void init() { buf = (ssh_ws*)((ssh_cs*)_empty + sizeof(STRING_BUFFER)); }
		bool alloc(ssh_u size);
		const String& add(ssh_wcs wcs, ssh_u len);
		const String& make(ssh_wcs wcs, ssh_u len);
		STRING_BUFFER* data() const { return ((STRING_BUFFER*)buf) - 1; }
		ssh_ws* buf;
	private:
		static ssh_wcs _empty;
	};

	class SSH regx
	{
	public:
		regx() : result(0), re(nullptr), subj(nullptr) { memset(patterns, 0, sizeof(patterns)); }
		regx(ssh_wcs pattern) : regx() { re = compile(pattern); }
		~regx()
		{
			if(re) regex_free(re);
			for(ssh_u i = 0; i < 32; i++)
				if(patterns[i]) regex_free(patterns[i]);
		}
		// запомнить паттерн в массиве
		bool set_pattern(ssh_u idx, ssh_wcs pattern)
		{
			if(idx < 32) return ((patterns[idx] = compile(pattern)) != nullptr);
			return false;
		}
		// найти совпадени€ без компил€ции паттерна
		ssh_l match(ssh_wcs subject, ssh_u idx_ptrn = -1, ssh_l idx = 0)
		{
			subj = (ssh_ws*)subject;
			return (result = regex16_exec((idx_ptrn == -1 ? re : patterns[idx_ptrn]), subject, wcslen(subject), idx, 0, vector, 256));
		}
		// найти совпадени€ с компил€цией паттерна
		ssh_l match(ssh_wcs subject, ssh_wcs pattern, ssh_l idx = 0)
		{
			return ((re = compile(pattern)) ? match(subject, -1, idx) : 0);
		}
		// вернуть подстроку по результатам последней операции
		String substr(ssh_l idx)
		{
			String ret;
			if(idx < result && idx >= 0 && vec(idx, 0) != -1)
			{
				ssh_u offs(vector[idx * 2 + 1]);
				ssh_ws ws(subj[offs]);
				subj[offs] = L'\0';
				ret = (subj + vector[idx * 2]);
				subj[offs] = ws;
			}
			return ret;
		}
		// заменить без компил€ции паттерна
		void replace(String& subject, ssh_wcs repl, ssh_u idx_ptrn = -1, ssh_l idx = 0)
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
		// заменить с компил€цией паттерна
		void replace(String& subject, ssh_wcs pattern, ssh_wcs repl, ssh_l idx = 0)
		{
			if((re = compile(pattern))) replace(subject, repl, -1, idx);
		}
		// вернуть количество найденных совпадений
		ssh_l count() const { return result; }
		// вернуть индекс в массике совпадений
		ssh_l vec(ssh_l idx, int offs = 0) const { return ((idx < result && idx >= 0) ? vector[idx * 2 + offs] : -1); }
	protected:
		// компилировать
		regex16* compile(ssh_wcs pattern)
		{
			result = 0;
			if(re) { regex_free(re); re = nullptr; }
			return regex16_compile(pattern, 0);
		}
		ssh_ws* subj;
		// найденные позиции
		ssh_l vector[256];
		// всего найденных
		ssh_l result;
		// временный паттерн
		regex16* re;
		// откомрилированные паттерны
		regex16* patterns[32];
	};
}
