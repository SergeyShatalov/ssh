
#pragma once

#include "ssh_str.h"

namespace ssh
{
	template <typename T> class Pts;
	template <typename T> class Range;
	template <typename T> class Bar;
	template <typename T> class Hash;
	template <typename T1, typename T2> class Box;

	template <typename T, ssh_u type> struct BaseNode { static void release(const T& t) { SSH_THROW(L""); } };
	template <typename T> struct BaseNode < T, SSH_TYPE > { static void release(const T& t) { t.~T(); } static T dummy() { return T(); } };
	template <typename T> struct BaseNode < T, SSH_PTR > { static void release(const T& t) { ::delete t; } static T dummy() { return nullptr; } };
	template <typename T> struct BaseNode < T, SSH_SPTR > { static void release(const T& t) { if(t) t->release(); } static T dummy() { return nullptr; } };

	class SSH Base
	{
		void* operator new(ssh_u sz);
	public:
		static List<Base*, SSH_SPTR>* objs()
		{
			static List<Base*, SSH_SPTR> obj(ID_LIST_BASE);
			return &obj;
		}
		// оператор выделения памяти под объект
		void* Base::operator new(ssh_u sz, Base** obj, const String& name, const String& type)
		{
			auto n(Base::objs()->find(name, type));
			if(n)
			{
				*obj = n->value;
				(*obj)->ref++;
				return nullptr;
			}
			*obj = (Base*)::operator new(sz);
			(*obj)->ref = 1;
			(*obj)->nm = new String(name);
			(*obj)->tp = new String(type);
			Base::objs()->add(*obj);
			return (void*)*obj;
		}
		// реализовать объект
		void release()
		{
			if(--ref == 0)
			{
				auto l(Base::objs());
				auto n(l->find(this));
				if(n)
				{
					l->remove(n);
					if(!l->root()) List<Base*, SSH_SPTR>::Node::get_MemArrayNode()->Reset();
				}
				delete this;
			}
		}
		// увеличить счётчик
		void add_ref() {ref++;}
		// вернуть имя
		const String& name() const {return *nm;}
		// вернуть тип
		const String& type() const {return *tp;}
	protected:
		// деструктор
		virtual ~Base() { SSH_DEL(nm); SSH_DEL(tp); }
		// число ссылок
		ssh_l ref;
		// имя
		String* nm;
	private:
		// имя типа
		String* tp;
	};

	class SSH Bits
	{
	public:
		// конструктор
		Bits() : value(0) {}
		// конструктор по значению
		Bits(ssh_l v) : value(v) {}
		// конструктор копии
		Bits(const Bits& src) : value(src.value) {}
		// операции
		// присваивание
		const Bits& operator = (const Bits& src) { value = src.value; return *this; }
		const Bits& operator = (const ssh_l src) { value = src; return *this; }
		// приведение
		operator ssh_l() const { return value; }
		// логические
		bool operator == (const ssh_l v) const { return (value == v); }
		bool operator != (const ssh_l v) const { return (value != v); }
		// функции
		// установить
		void set(const ssh_l v) { value = v; }
		// очистить
		void clear() { value = 0; }
		// добавить
		void add_flags(const ssh_l v) { value |= v; }
		// исключить
		void del_flags(const ssh_l v) { value &= ~v; }
		// установить бит
		void set_bit(ssh_l bit) { _bittestandset64(&value, bit); }
		// очистить бит
		void clear_bit(ssh_l bit) { _bittestandreset64(&value, bit); }
		// установить по признаку
		void set_bit(ssh_l bit, bool is) { is ? set_bit(bit) : clear_bit(bit); }
		// пустой ?
		bool is_empty() const { return (value == 0); }
		// проверка на бит
		bool test_bit(ssh_l bit) const { return (_bittest64(&value, bit) != 0); }
		// проверка на несколько бит
		bool test_flags(const ssh_l test) const { return ((value & test) == test); }
		// проверка на наличие хоть одного установленного
		bool test_any(const ssh_l test) const { return ((value & test) != 0); }
		// длина
		ssh_u total_bits() const { return 64; }
		// количество установленных
		ssh_u total_set() const;
		// значение
		ssh_l value;
	};

	template <typename T = ssh_u> class Pts
	{
	public:
		// конструкторы
		Pts() : x(0), y(0) {}
		Pts(const T* ptr) {x = ptr[0]; y = ptr[1];}
		Pts(const T& X, const T& Y) : x(X), y(Y) {}
		Pts(const Pts<T>& pt) : x(pt.x), y(pt.y) {}
		Pts(const POINT& pt) : x(pt.x), y(pt.y) {}
		Pts(const Bar<T> b) : x(b.x), y(b.y) {}
		// Операции
		Pts<T> operator - (const Pts<T>& p) const {return Pts<T>(x - p.x, y - p.y);}
		Pts<T> operator - (const T& i) const {return Pts<T>(x - i, y - i);}
		Pts<T> operator + (const Pts<T>& p) const {return Pts<T>(x + p.x, y + p.y);}
		Pts<T> operator + (const T& i) const {return Pts<T>(x + i, y + i);}
		Pts<T> operator * (const Pts<T>& p) const {return Pts<T>(x * p.x, y * p.y);}
		Pts<T> operator * (const T& i) const {return Pts<T>(x * i, y * i);}
		Pts<T> operator / (const Pts<T>& p) const {return Pts<T>(x / p.x, y / p.y);}
		Pts<T> operator / (const T& i) const {return Pts<T>(x / i, y / i);}
		Pts<T>& operator -= (const Pts<T>& p) {x -= p.x ; y -= p.y; return *this;}
		Pts<T>& operator -= (const T& i) {x -= i ; y -= i; return *this;}
		Pts<T>& operator += (const Pts<T>& p) {x += p.x ; y += p.y; return *this;}
		Pts<T>& operator += (const T& i) {x += i ; y += i; return *this;}
		Pts<T>& operator /= (const Pts<T>& p) {x /= p.x ; y /= p.y; return *this;}
		Pts<T>& operator /= (const T& i) {x /= i ; y /= i; return *this;}
		Pts<T>& operator *= (const Pts<T>& p) {x *= p.x ; y *= p.y; return *this;}
		Pts<T>& operator *= (const T& i) {x *= i ; y *= i; return *this;}
		// присваивание
		Pts<T>& operator = (const Bar<T>& r) {x = r.x ; y = r.y; return *this;}
		// внешние
		friend Pts<T> operator - (const T& i, const Pts<T>& p) {return Pts<T>(i - p.x, i - p.y);}
		friend Pts<T> operator + (const T& i, const Pts<T>& p) {return Pts<T>(i + p.x, i + p.y);}
		friend Pts<T> operator / (const T& i, const Pts<T>& p) {return Pts<T>(i / p.x, i / p.y);}
		friend Pts<T> operator * (const T& i, const Pts<T>& p) {return Pts<T>(i * p.x, i * p.y);}
		// Сравнение
		bool operator == (const Pts<T>& p) const {return (x == p.x && y == p.y);}
		bool operator != (const Pts<T>& p) const {return (x != p.x || y != p.y);}
		bool operator == (const Bar<T>& r) const {return (x == r.x && y == r.y);}
		bool operator != (const Bar<T>& r) const {return (x != r.x || y != r.y);}
		bool is_empty() const {return (x == 0 && y == 0);}
		// Приведение типов
		operator T*() const {return (T*)&x;}
		operator POINT*() const {return (POINT*)&x;}
		operator POINT() const {return (POINT&)x;}
		operator const POINT*() const {return (const POINT*)&x;}
		// Специальные
		Pts<T>& set(const T& X, const T& Y) {x = X ; y = Y; return *this;}
		T x, y;
	};

	template <typename T = ssh_u> class Range
	{
	public:
		// Конструкторы
		Range() : w(0), h(0) {}
		Range(const T* ptr) { w = ptr[0]; h = ptr[1];}
		Range(const T& W, const T& H) : w(W), h(H) {}
		Range(const Range<T>& s) : w(s.w), h(s.h) {}
		Range(const SIZE& s) : w(s.cx), h(s.cy) {}
		Range(const Bar<T>& r) : w(r.w), h(r.h) {}
		// Операции
		Range<T> operator - (const Range<T>& s) const {return Range<T>(w - s.w, h - s.h);}
		Range<T> operator - (const T& i) const {return Range<T>(w - i, h - i);}
		Range<T> operator + (const Range<T>& s) const {return Range<T>(w + s.w, h + s.h);}
		Range<T> operator + (const T& i) const {return Range<T>(w + i, h + i);}
		Range<T> operator * (const Range<T>& s) const {return Range<T>(w * s.w, h * s.h);}
		Range<T> operator * (const T& i) const {return Range<T>(w * i, h * i);}
		Range<T> operator / (const Range<T>& s) const {return Range<T>(w / s.w, h / s.h);}
		Range<T> operator / (const T& i) const {return Range<T>(w / i, h / i);}
		Range<T>& operator += (const Range<T>& s) {w += s.w ; h += s.h; return *this;}
		Range<T>& operator += (const T& s) {w += s ; h += s; return *this;}
		Range<T>& operator -= (const Range<T>& s) {w -= s.w ; h -= s.h; return *this;}
		Range<T>& operator -= (const T& s) {w -= s ; h -= s; return *this;}
		Range<T>& operator /= (const Range<T>& s) {w /= s.w ; h /= s.h; return *this;}
		Range<T>& operator /= (const T& s) {w /= s ; h /= s; return *this;}
		Range<T>& operator *= (const Range<T>& s) {w *= s.w ; h *= s.h; return *this;}
		Range<T>& operator *= (const T& s) {w *= s ; h *= s; return *this;}
		// Присваивание
		Range<T>& operator = (const Bar<T>& r) {w = r.w ; h = r.h; return *this;}
		// внешние
		friend Range<T> operator - (const T& i, const Range<T>& p) {return Range<T>(i - p.w, i - p.h);}
		friend Range<T> operator + (const T& i, const Range<T>& p) {return Range<T>(i + p.w, i + p.h);}
		friend Range<T> operator * (const T& i, const Range<T>& p) {return Range<T>(i * p.w, i * p.h);}
		friend Range<T> operator / (const T& i, const Range<T>& p) {return Range<T>(i / p.w, i / p.h);}
		// Операции
		bool operator == (const Range<T>& s) const {return (w == s.w && h == s.h);}
		bool operator == (const Bar<T>& r) const {return (w == r.w && h == r.h);}
		bool operator != (const Range<T>& s) const {return (w != s.w || h != s.h);}
		bool operator != (const Bar<T>& r) const {return (w != r.w || h != r.h);}
		// Приведение типов
		operator T*() const {return (T*)&w;}
		operator SIZE*() const {return (SIZE*)&w;}
		operator SIZE() const {return (SIZE&)w;}
		operator const SIZE*() const {return (const SIZE*)&w;}
		// Атрибуты
		Range<T>& null() {w = h = 0; return *this;}
		bool is_null() const {return (w == 0 && h == 0);}
		Range<T>& set(const T& W, const T& H) {w = W ; h = H; return *this;}
		T w, h;
	};

	template <typename T = ssh_u> class Bar
	{
	public:
		// Конструкторы
		Bar() : x(0), y(0), w(0), h(0) {}
		Bar(const T* ptr) { x = ptr[0]; y = ptr[1]; w = ptr[2]; h = ptr[3];}
		Bar(const T& X, const T& Y, const T& W, const T& H) : x(X), y(Y), w(W), h(H) {}
		Bar(const RECT& r) : x(r.left), y(r.top), w(r.right - x), h(r.bottom - y) {}
		Bar(const Bar<T>& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}
		Bar(const Pts<T>& pt, const Range<T>& s) : x(pt.x), y(pt.y), w(s.w), h(s.h) {}
		Bar(const Range<T>& sz) : x(0), y(0), w(sz.w), h(sz.h) {}
		Bar(const Pts<T>& pt) : x(pt.x), y(pt.y), w(0), h(0) {}
		// Операции
		Bar<T> operator - (const Bar<T>& r) const {return Bar<T>(x - r.x, y - r.y, w - r.w, h - r.h);}
		Bar<T> operator - (const T& i) const {return Bar<T>(x - i, y - i, w - i, h - i);}
		Bar<T> operator - (const Pts<T>& p) const {return Bar<T>(x - p.x, y - p.y, w - p.x, h - p.y);}
		Bar<T> operator - (const Range<T>& s) const {return Bar<T>(x - s.w, y - s.h, w - s.w, h - s.h);}
		Bar<T> operator + (const Bar<T>& r) const {return Bar<T>(x + r.x, y + r.y, w + r.w, h + r.h);}
		Bar<T> operator + (const Pts<T>& p) const {return Bar<T>(x + p.x, y + p.y, w + p.x, h + p.y);}
		Bar<T> operator + (const T& i) const {return Bar<T>(x + i, y + i, w + i, h + i);}
		Bar<T> operator + (const Range<T>& s) const {return Bar<T>(x + s.w, y + s.h, w + s.w, h + s.h);}
		Bar<T> operator / (const Bar<T>& r) const {return Bar<T>(x / r.x, y / r.y, w / r.w, h / r.h);}
		Bar<T> operator / (const T& i) const {return Bar<T>(x / i, y / i, w / i, h / i);}
		Bar<T> operator / (const Pts<T>& p) const {return Bar<T>(x / p.x, y / p.y, w / p.x, h / p.y);}
		Bar<T> operator / (const Range<T>& s) const {return Bar<T>(x / s.w, y / s.h, w / s.w, h / s.h);}
		Bar<T> operator * (const Bar<T>& r) const {return Bar<T>(x * r.x, y * r.y, w * r.w, h * r.h);}
		Bar<T> operator * (const T& i) const {return Bar<T>(x * i, y * i, w * i, h * i);}
		Bar<T> operator * (const Pts<T>& p) const {return Bar<T>(x * p.x, y * p.y, w * p.x, h * p.y);}
		Bar<T> operator * (const Range<T>& s) const {return Bar<T>(x * s.w, y * s.h, w * s.w, h * s.h);}
		const Bar<T>& operator -= (const Bar<T>& r) {x -= r.x ; y -= r.y ; w -= r.w ; h -= r.h; return *this;}
		const Bar<T>& operator -= (const Pts<T>& p) {x -= p.x ; y -= p.y; w -= p.x ; h -= p.y ; return *this;}
		const Bar<T>& operator -= (const T& i) {x -= i ; y -= i ; w -= i ; h -= i ; return *this;}
		const Bar<T>& operator -= (const Range<T>& s) {x -= s.w; y -= s.h ; w -= s.w ; h -= s.h; return *this;}
		const Bar<T>& operator += (const Bar<T>& r) {x += r.x ; y += r.y ; w += r.w ; h += r.h; return *this;}
		const Bar<T>& operator += (const Pts<T>& p) {x += p.x ; y += p.y; w += p.x ; h += p.y ; return *this;}
		const Bar<T>& operator += (const T& i) {x += i ; y += i ; w += i; h += i; return *this;}
		const Bar<T>& operator /= (const Bar<T>& r) {x /= r.x; y /= r.y; w /= r.w; h /= r.h; return *this;}
		const Bar<T>& operator /= (const T& i) {x /= i; y /= i; w /= i; h /= i; return *this;}
		const Bar<T>& operator /= (const Pts<T>& p) {x /= p.x; y /= p.y; w / p.x; h /= p.y; return *this;}
		const Bar<T>& operator /= (const Range<T>& s) {x /= s.w ; y /= s.h ; w /= s.w; h /= s.h; return *this;}
		const Bar<T>& operator *= (const Bar<T>& r) {x *= r.x; y *= r.y; w *= r.w; h *= r.h; return *this;}
		const Bar<T>& operator *= (const T& i) {x *= i; y *= i; w *= i; h *= i; return *this;}
		const Bar<T>& operator *= (const Pts<T>& p) {x *= p.x; y *= p.y; w *= p.x ; h *= p.y ; return *this;}
		const Bar<T>& operator *= (const Range<T>& s) {x *= s.w ; y *= s.h ; w *= s.w; h *= s.h; return *this;}
		// внешние
		friend Bar<T> operator - (const Pts<T>& p, const Bar<T>& r) {return Bar<T>(p.x - r.x, p.y - r.y, p.x - r.w, p.y - r.h);}
		friend Bar<T> operator - (const T& i, const Bar<T>& r) {return Bar<T>(i - r.x, i - r.y, i - r.w, i - r.h);}
		friend Bar<T> operator - (const Range<T>& s, const Bar<T>& r) {return Bar<T>(s.w - r.x, s.h - r.y, s.w - r.w, s.h - r.h);}
		friend Bar<T> operator + (const Pts<T>& p, const Bar<T>& r) {return Bar<T>(p.x + r.x, p.y + r.y, p.x + r.w, p.y + r.h);}
		friend Bar<T> operator + (const T& i, const Bar<T>& r) {return Bar<T>(i + r.x, i + r.y, i + r.w, i + r.h);}
		friend Bar<T> operator + (const Range<T>& s, const Bar<T>& r) {return Bar<T>(s.w + r.x, s.h + r.y, s.w + r.w, s.h + r.h);}
		friend Bar<T> operator / (const Pts<T>& p, const Bar<T>& r) {return Bar<T>(p.x / r.x, p.y / r.y, p.x / r.w, p.y / r.h);}
		friend Bar<T> operator / (const T& i, const Bar<T>& r) {return Bar<T>(i / r.x, i / r.y, i / r.w, i / r.h);}
		friend Bar<T> operator / (const Range<T>& s, const Bar<T>& r) {return Bar<T>(s.w / r.x, s.h / r.y, s.w / r.w, s.h / r.h);}
		friend Bar<T> operator * (const Pts<T>& p, const Bar<T>& r) {return Bar<T>(p.x * r.x, p.y * r.y, p.x * r.w, p.y * r.h);}
		friend Bar<T> operator * (const T& i, const Bar<T>& r) {return Bar<T>(i * r.x, i * r.y, i * r.w, i * r.h);}
		friend Bar<T> operator * (const Range<T>& s, const Bar<T>& r) {return Bar<T>(s.w * r.x, s.h * r.y, s.w * r.w, s.h * r.h);}
		// Сравнение
		bool operator == (const Pts<T>& p) const {return (x == p.x && y == p.y);}
		bool operator == (const Bar<T>& r) const {return (x == r.x && y == r.y && w == r.w && h == r.h);}
		bool operator == (const Range<T>& s) const {return (w == s.w && h == s.h);}
		bool operator != (const Pts<T>& p) const {return (x != p.x || y != p.y);}
		bool operator != (const Bar<T>& r) const {return (x != r.x || y != r.y || w != r.w || h != r.h);}
		bool operator != (const Range<T>& s) const {return (w != s.w || h != s.h);}
		// Присваивание
		Bar<T>& operator = (const Bar<T>& r) {x = r.x ; y = r.y ; w = r.w ; h = r.h; return *this;}
		Bar<T>& operator = (const RECT& r) {x = r.left ; y = r.top ; w = r.right - x ; h = r.bottom - y; return *this;}
		Bar<T>& operator = (const Range<T>& s) {w = s.w ; h = s.h; return *this;}
		Bar<T>& operator = (const Pts<T>& p) {x = p.x ; y = p.y; return *this;}
		// Приведение типов
		operator const T*() const {return (const T*)&x;}
		operator T*() {return (T*)&x;}
		operator RECT*() const {static RECT r; r.left = x ; r.top = y ; r.bottom = y + h ; r.right = x + w; return &r;}
		operator const RECT*() const {static RECT r; r.left = x ; r.top = y ; r.bottom = y + h ; r.right = x + w; return &r;}
		operator Range<T>() const {return Range<T>(w, h);}
		operator Pts<T>() const {return Pts<T>(x, y);}
		// Специальные
		const Bar<T>& set(const T& X, const T& Y, const T& W, const T& H) {x = X ; y = Y ; w = W ; h = H; return *this;}
		const Bar<T>& set(const Pts<T>& pt, const Range<T>& sz) {x = pt.x ; y = pt.y ; w = sz.w ; h = sz.h; return *this;}
		const Bar<T>& empty() {x = y = w = h = 0; return *this;}
		Pts<T> centerPt() const {return Pts<T>(x + (w / 2), y + (h / 2));}
		bool is_empty() const {return (w <= 0 || h <= 0);}
		bool ptInRc(const Pts<T>& pt) const {return (::PtInRect(*this, pt) != 0);}
		bool rcInRc(const Bar<T>& rc, RECT* dst) const
		{
			RECT r1, r2;
			::SetRect(&r1, x, y, x + w, y + h);
			::SetRect(&r2, rc.x, rc.y, rc.x + rc.w, rc.y + rc.h);
			return (::IntersectRect(dst, &r1, &r2) != 0);
		}
		const Bar<T>& inflate(const T& X, const T& Y, const T& W, const T& H) {x += X ; y += Y ; w += W ; h += H; return *this;}
		const T right() const {return (x + w);}
		const T bottom() const {return (y + h);}
		union
		{
			struct
			{
				Pts<T> point;
				Range<T> range;
			};
			struct
			{
				T x;
				T y;
				T w;
				T h;
			};
		};
	};

	template <typename T1 = long, typename T2 = float> class Box
	{
	public:
		// конструкторы
		Box<T1, T2>() : x(0), y(0), w(0), h(0), n(0), f(0) {}
		Box<T1, T2>(const Bar<T1>& r, const Range<T2>& s) : x(r.x), y(r.y), w(r.w), h(r.h), n(s.w), f(s.h) {}
		Box<T1, T2>(const T1& X, const T1& Y, const T1& W, const T1& H, const T2& N, const T2& F) : x(X), y(Y), w(W), h(H), n(N), f(F) {}
		Box<T1, T2>(const Box<T1, T2>& b) : x(b.x), y(b.y), w(b.w), h(b.h), n(b.n), f(b.f) {}
		// операторы
		// логические
		bool operator == (const Box<T1, T2>& b) const {return (x == b.x && y == b.y && w == b.w && h == b.h && n == b.n && f == b.f);}
		bool operator != (const Box<T1, T2>& b) const {return (x != b.x || y != b.y || w != b.w || h != b.h || n != b.n || f != b.f);}
		// присваивания
		const Box<T1, T2>& operator = (const Box<T1, T2>& b) {SSH_MEMCPY(*this, &b, sizeof(Box<T1, T2>)); return *this;}
		const Box<T1, T2>& operator = (const Bar<T1>& r) {x = r.x ; y = r.y ; w = r.w ; h = r.h; return *this;}
		const Box<T1, T2>& operator = (const Range<T2>& s) {n = s.w ; f = s.h; return *this;}
		// арифметические
		Box<T1, T2> operator + (const Bar<T1>& r) const {return Box<T1, T2>(x + r.x, y + r.y, w + r.w, h + r.h, n, f);}
		Box<T1, T2> operator + (const Range<T2>& s) const {return Box<T1, T2>(x, y, w, h, n + s.w, f + s.h);}
		Box<T1, T2> operator - (const Bar<T1>& r) const {return Box<T1, T2>(x - r.x, y - r.y, w - r.w, h - r.h, n, f);}
		Box<T1, T2> operator - (const Range<T2>& s) const {return Box<T1, T2>(x, y, w, h, n - s.w, f - s.h);}
		const Box<T1, T2>& operator += (const Bar<T1>& r) {x += r.x ; y += r.y ; w += r.w ; h += r.h; return this;}
		const Box<T1, T2>& operator += (const Range<T2>& s) {n += s.w ; f += s.h; return this;}
		const Box<T1, T2>& operator -= (const Bar<T1>& r) {x -= r.x ; y -= r.y ; w -= r.w ; h -= r.h; return this;}
		const Box<T1, T2>& operator -= (const Range<T2>& s) {n -= s.w ; f -= s.h; return this;}
		// приведение типа
		operator Bar<T1>*() const {return Bar<T1>(x, y, w, h);}
		operator const Bar<T1>*() const {return Bar<T1>(x, y, w, h);}
		operator Range<T2>*() const {return Range<T2>(n, f);}
		operator const Range<T2>*() const {return Range<T2>(n, f);}
		// специальные
		bool is_null() const {return (x == 0 && y == 0 && w == 0 && h == 0 && n == 0 && f == 0);}
		bool is_empty() const {return (w == 0 && h == 0 && n == 0 && f == 0);}
		T1 x, y, w, h;
		T2 n, f;
	};

	template<ssh_u i> class HashCompiletime
	{
	public:
		static __forceinline ssh_u make(const ssh_u hash, const ssh_u len, ssh_wcs str)
		{
			return HashCompiletime<i - 1>::make(hash ^ ((hash << 5) + str[len - i] + (hash >> 2)), len, str);
		}
	};

	template <> class HashCompiletime<0>
	{
	public:
		static __forceinline ssh_u make(const ssh_u hash, const ssh_u len, ssh_wcs str)
		{
			return hash ^ ((hash << 5) + str[len] + (hash >> 2));
		}
	};

	template<ssh_u N> __forceinline ssh_u HashCT(const wchar_t(&str)[N])
	{
		return HashCompiletime<N - 2>::make(1315423911, N - 2, str);
	}
}
