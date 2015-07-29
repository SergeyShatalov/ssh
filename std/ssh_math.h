
#pragma once

#define OBOX_X1Y1Z1 0
#define OBOX_X2Y1Z1	1
#define OBOX_X1Y2Z1 2
#define OBOX_X2Y2Z1 3
#define OBOX_X1Y1Z2 4
#define OBOX_X2Y1Z2 5
#define OBOX_X1Y2Z2 6
#define OBOX_X2Y2Z2 7

#include "ssh_hlp.h"

namespace ssh
{
	inline long loop(long x, long y)
	{
		long z(y / (x * 2));
		long y1 = z * x;
		y -= ((y - y1) >= x) ? y1 * 2 : 0;
		return x - (y < x ? x - y : y - x);
	}

	inline int log2(int i)
	{
		int value = 0;
		while(i >>= 1) {value++;}
		return value;
	}

	inline float lerp(float f0, float f1, float t)
	{
		float s = 1.0f - t;
		return f0 * s + f1 * t;
	}

	inline float floor(float f)
	{
		return (float)_mm_cvtt_ss2si(_mm_set_ss(f));
	}

	inline int round(float f)
	{
		return _mm_cvt_ss2si(_mm_set_ss(f));
	}

	inline float frac(float f)
	{
		return f - ssh::floor(f);
	}

	class plane;
	class color;
	class mtx;
	class vec2;
	class vec3;
	class vec4;
	class quat;
	class ray;
	class sphere;
	class bbox;
	class obox;

	vec3 SSH ssh_vec3_mtx(const vec3& v, const mtx& m);
	vec4 SSH ssh_vec4_mtx(const vec4& v, const mtx& m);
	vec3 SSH ssh_mtx_vec3(const mtx& m, const vec3& v);
	vec4 SSH ssh_mtx_vec4(const mtx& m, const vec4& v);
	mtx SSH ssh_mtx_mtx(const mtx& m1, const mtx& m2);

	class SSH vec2
	{
	public:
		// конструкторы
		vec2() : x(0.0f), y(0.0f) {}
		vec2(float x, float y) : x(x), y(y) {}
		vec2(float* f) { x = f[0]; y = f[1];}
		vec2(const vec2& v) : x(v.x), y(v.y) {}
		// операции
		// унарные
		const vec2& operator + () const {return *this;}
		vec2 operator - () const {return vec2(-x, -y);}
		// бинарные
		vec2 operator + (const vec2& v) const {return vec2(x + v.x, y + v.y);}
		vec2 operator + (float f) const {return vec2(x + f, y + f);}
		vec2 operator - (const vec2& v) const {return vec2(x - v.x, y - v.y);}
		vec2 operator - (float f) const {return vec2(x - f, y - f);}
		vec2 operator * (const vec2& v) const {return vec2(x * v.x, y * v.y);}
		vec2 operator * (float f) const {return vec2(x * f, y * f);}
		vec2 operator / (const vec2& v) const {return vec2(x / v.x, y / v.y);}
		vec2 operator / (float f) const {f = 1.0f / f; return vec2(x * f, y * f);}
		const vec2& operator += (const vec2& v) {x += v.x ; y += v.y ; return *this;}
		const vec2& operator += (float f) {x += f ; y += f ; return *this;}
		const vec2& operator -= (const vec2& v) {x -= v.x ; y -= v.y ; return *this;}
		const vec2& operator -= (float f) {x -= f ; y -= f ; return *this;}
		const vec2& operator *= (const vec2& v) {x *= v.x ; y *= v.y ; return *this;}
		const vec2& operator *= (float f) {x *= f ; y *= f ; return *this;}
		const vec2& operator /= (const vec2& v) {x /= v.x ; y /= v.y ; return *this;}
		const vec2& operator /= (float f) {f = 1.0f / f; x *= f ; y *= f ; return *this;}
		friend vec2 operator + (float f, const vec2& v) {return vec2(f + v.x, f + v.y);}
		friend vec2 operator - (float f, const vec2& v) {return vec2(f - v.x, f - v.y);}
		friend vec2 operator * (float f, const vec2& v) {return vec2(f * v.x, f * v.y);}
		friend vec2 operator / (float f, const vec2& v) {return vec2(f / v.x, f / v.y);}
		// логические
		bool operator == (const vec2& v) const {return ((fabs(x - v.x) + fabs(y - v.y)) < SSH_EPSILON);}
		bool operator != (const vec2& v) const {return !(operator == (v));}
//		float operator[](ssh_u idx) const {return (idx < 2 ? flt[idx] : 0.0f);}
		// присваивание
		const vec2& operator = (const vec2& v) {x = v.x ; y = v.y ; return *this;}
		const vec2& operator = (float f) {x = f ; y = f ; return *this;}
		// специальные
		float length() const {return sqrt(x * x + y * y);}
		float lengthSq() const {return x * x + y * y;}
		float dot(const vec2& v) const {return x * v.x + y * v.y;}
		float cross(const vec2& v) const {return x * v.y - y * v.x;}
		bool is_identity() const {return (lengthSq() < SSH_EPSILON2);}
		const vec2& normalize() {float l(length()); l = (l > SSH_EPSILON ? 1.0f / l : 0.0f); x *= l; y *= l; return *this;}
		const vec2& floor(const vec2& v) {if(v.x < x) x = v.x; if(v.y < y) y = v.y; return *this;}
		const vec2& ceil(const vec2& v) { if(v.x > x) x = v.x; if(v.y > y) y = v.y; return *this;}
		const vec2& floor() {x = ssh::floor(x); y = ssh::floor(y); return *this;}
		vec2 middle(const vec2& v) const {return vec2((x + v.x) / 2.0f, (y + v.y) / 2.0f);}
		vec2 reflect(const vec2& v) const {return vec2(*this - (2.0f * dot(v) * v));}
		// приведение типа
		operator const float*() const {return (const float*)&x;}
		operator float*() const {return (float*)&x;}
		// члены
		float x;
		float y;
	};

	class SSH vec3
	{
	public:
		// конструкторы
		vec3() : x(0.0f), y(0.0f), z(0.0f) {}
		vec3(float f) : x(f), y(f), z(f) {}
		vec3(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
		vec3(float* f) { x = f[0]; y = f[1]; z = f[2];}
		vec3(const vec2& v) : x(v.x), y(v.y), z(0.0f) {}
		vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}
		// операции
		// унарные
		const vec3& operator + () const {return *this;}
		vec3 operator - () const {return vec3(-x, -y, -z);}
		// бинарные
		vec3 operator + (const vec3& v) const {return vec3(x + v.x, y + v.y, z + v.z);}
		vec3 operator + (float f) const {return vec3(x + f, y + f, z + f);}
		vec3 operator - (const vec3& v) const {return vec3(x - v.x, y - v.y, z - v.z);}
		vec3 operator - (float f) const {return vec3(x - f, y - f, z - f);}
		vec3 operator * (const vec3& v) const {return vec3(x * v.x, y * v.y, z * v.z);}
		vec3 operator * (const mtx& m) const;
		vec3 operator * (float f) const {return vec3(x * f, y * f, z * f);}
		vec3 operator / (const vec3& v) const {return vec3(x / v.x, y / v.y, z / v.z);}
		vec3 operator / (float f) const {f = 1.0f / f; return vec3(x * f, y * f, z * f);}
		const vec3& operator += (const vec3& v) {x += v.x ; y += v.y ; z += v.z ; return *this;}
		const vec3& operator += (float f) {x += f ; y += f ; z += f ; return *this;}
		const vec3& operator -= (const vec3& v) {x -= v.x ; y -= v.y ; z -= v.z ; return *this;}
		const vec3& operator -= (float f) {x -= f ; y -= f ; z -= f ; return *this;}
		const vec3& operator *= (const vec3& v) {x *= v.x ; y *= v.y ; z *= v.z ; return *this;}
		const vec3& operator *= (const mtx& m);
		const vec3& operator *= (float f) {x *= f ; y *= f ; z *= f ; return *this;}
		const vec3& operator /= (const vec3& v) {x /= v.x ; y /= v.y ; z /= v.z ; return *this;}
		const vec3& operator /= (float f) {f = 1.0f / f; x *= f ; y *= f ; z *= f ; return *this;}
		friend vec3 operator + (float f, const vec3& v) {return vec3(f + v.x, f + v.y, f + v.z);}
		friend vec3 operator - (float f, const vec3& v) {return vec3(f - v.x, f - v.y, f - v.z);}
		friend vec3 operator * (float f, const vec3& v) {return vec3(f * v.x, f * v.y, f * v.z);}
		friend vec3 operator / (float f, const vec3& v) {return vec3(f / v.x, f / v.y, f / v.z);}
		friend vec3 operator * (const mtx& m, const vec3& v);
		friend vec3 operator * (const quat& q, const vec3& v);
		// логические
		bool operator == (const vec3& v) const { return ((fabs(x - v.x) + fabs(y - v.y) + fabs(z - v.z)) < SSH_EPSILON); }
		bool operator != (const vec3& v) const {return (!operator == (v));}
		bool operator > (const vec3& v) const {return (x > v.x && y > v.y && z > v.z);}
		bool operator < (const vec3& v) const {return (x < v.x && y < v.y && z < v.z);}
		// присваивание
		const vec3& operator = (const vec3& v) {x = v.x ; y = v.y ; z = v.z; return *this;}
		const vec3& operator = (float f) {x = f ; y = f ; z = f; return *this;}
		float operator[](ssh_u idx) const { return (idx < 3 ? flt[idx] : 0.0f); }
		// специальные
		float length() const {return sqrt(x * x + y * y + z * z);}
		float lengthSq() const {return x * x + y * y + z * z;}
		float dot(const vec3& v) const {return x * v.x + y * v.y + z * v.z;}
		bool is_identity() const { return (lengthSq() < SSH_EPSILON2); }
		const vec3& normalize() { float l(length()); l = (l > SSH_EPSILON ? 1.0f / l : 0.0f); x *= l; y *= l; z *= l; return *this; }
		const vec3& floor(const vec3& v) {if(v.x < x) x = v.x; if(v.y < y) y = v.y; if(v.z < z) z = v.z; return *this;}
		const vec3& ceil(const vec3& v) {if(v.x > x) x = v.x; if(v.y > y) y = v.y; if(v.z > z) z = v.z; return *this;}
		const vec3& set(float X, float Y, float Z) {x = X; y = Y; z = Z; return *this;}
		const vec3& floor() {x = ssh::floor(x); y = ssh::floor(y); z = ssh::floor(z); return *this;}
		vec3 cross(const vec3& v) const {return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);}
		vec3 middle(const vec3& v) const {return vec3((x + v.x) / 2.0f, (y + v.y) / 2.0f, (z + v.z) / 2.0f);}
		vec3 reflect(const vec3& v) const {return vec3(*this - (2.0f * dot(v) * v));}
		vec3 lerp(const vec3& v, float t) const {float s = 1.0f - t ; return vec3(x * s + t * v.x, y * s + t * v.y, z * s + t * v.z);}
		// приведение типа
		operator const float*() const {return (const float*)&x;}
		operator float*() const {return (float*)&x;}
		// члены
		union
		{
			struct  {float x, y, z;};
			float flt[3];
		};
	};

	__declspec(align(16)) class SSH vec4
	{
	public:
		// конструкторы
		vec4() {identity();}
		vec4(float f) : x(f), y(f), z(f), w(f) {}
		vec4(float X, float Y, float Z, float W) : x(X), y(Y), z(Z), w(W) {}// 0 - vector 1 - point
		vec4(float* f) { x = f[0]; y = f[1]; z = f[2]; w = f[3];}
		vec4(const vec2& v) : x(v.x), y(v.y), z(1.0f), w(0.0f) {}
		vec4(const vec3& v) : x(v.x), y(v.y), z(v.z), w(0.0f) {}
		vec4(const vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
		// операции
		// унарные
		const vec4& operator + () const {return *this;}
		vec4 operator - () const {return vec4(-x, -y, -z, -w);}
		// бинарные
		vec4 operator + (const vec4& v) const {return vec4(x + v.x, y + v.y, z + v.z, w + v.w);}
		vec4 operator + (float f) const {return vec4(x + f, y + f, z + f, w + f);}
		vec4 operator - (const vec4& v) const {return vec4(x - v.x, y - v.y, z - v.z, w - v.w);}
		vec4 operator - (float f) const {return vec4(x - f, y - f, z - f, w - f);}
		vec4 operator * (const vec4& v) const {return vec4(x * v.x, y * v.y, z * v.z, w * v.w);}
		vec4 operator * (const mtx& m) const;
		vec4 operator * (float f) const {return vec4(x * f, y * f, z * f, w * f);}
		vec4 operator / (const vec4& v) const {return vec4(x / v.x, y / v.y, z / v.z, w / v.w);}
		vec4 operator / (float f) const {f = 1.0f / f; return vec4(x * f, y * f, z * f, w * f);}
		const vec4& operator += (const vec4& v) {x += v.x ; y += v.y ; z += v.z ; w += v.w ; return *this;}
		const vec4& operator += (float f) {x += f ; y += f ; z += f ; w += f ; return *this;}
		const vec4& operator -= (const vec4& v) {x -= v.x ; y -= v.y ; z -= v.z ; w -= v.w ; return *this;}
		const vec4& operator -= (float f) {x -= f ; y -= f ; z -= f ; w -= f ; return *this;}
		const vec4& operator *= (const vec4& v) {x *= v.x ; y *= v.y ; z *= v.z ; w *= v.w ; return *this;}
		const vec4& operator *= (const mtx& m);
		const vec4& operator *= (float f) {x *= f ; y *= f ; z *= f ; w *= f ; return *this;}
		const vec4& operator /= (const vec4& v) {x /= v.x ; y /= v.y ; z /= v.z ; w /= v.w ; return *this;}
		const vec4& operator /= (float f) {f = 1.0f / f; x *= f ; y *= f ; z *= f ; w *= f ; return *this;}
		friend vec4 operator + (float f, const vec4& v) {return (f + v.x, f + v.y, f + v.z, f + v.w);}
		friend vec4 operator - (float f, const vec4& v) {return vec4(f - v.x, f - v.y, f - v.z, f - v.w);}
		friend vec4 operator * (float f, const vec4& v) {return vec4(f * v.x, f * v.y, f * v.z, f * v.w);}
		friend vec4 operator * (const mtx& m, const vec4& v);
		friend vec4 operator / (float f, const vec4& v) {return vec4(f / v.x, f / v.y, f / v.z, f / v.w);}
		// логические
		bool operator == (const vec4& v) const { return ((fabs(x - v.x) + fabs(y - v.y) + fabs(z - v.z) + fabs(w - v.w)) < SSH_EPSILON); }
		bool operator != (const vec4& v) const {return !(operator == (v));}
		// присваивание
		const vec4& operator = (const vec4& v) {x = v.x ; y = v.y ; z = v.z ; w = v.w ; return *this;}
		const vec4& operator = (float f) {x = f ; y = f ; z = f ; w = 0.0f ; return *this;}
		float operator[](ssh_u idx) const { return (idx < 4 ? flt[idx] : 0.0f); }
		// специальные
		void identity() {x = y = z = w = 0.0f;}
		float length() const {return sqrt(x * x + y * y + z * z + w * w);}
		float lengthSq() const {return x * x + y * y + z * z + w * w;}
		float dot(const vec4& v) const {return x * v.x + y * v.y + z * v.z + w * v.w;}
		bool is_identity() const { return(lengthSq() < SSH_EPSILON2); }
		const vec4& normalize() { float l(length()); l = (l > SSH_EPSILON ? 1.0f / l : 0.0f); x *= l; y *= l; z *= l; w *= l; return *this; }
		const vec4& floor(const vec4& v) {if(v.x < x) x = v.x; if(v.y < y) y = v.y; if(v.z < z) z = v.z; w = 0.0f; return *this;}
		const vec4& ceil(const vec4& v) {if(v.x > x) x = v.x; if(v.y > y) y = v.y; if(v.z > z) z = v.z; w = 0.0f; return *this;}
		const vec4& set(float X, float Y, float Z, float W) {x = X ; y = Y ; z = Z ; w = W ; return *this;}
		vec4 cross(const vec4& v) const {return vec4(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x, w);}
		vec4 middle(const vec4& v) const {return vec4((x + v.x) / 2.0f, (y + v.y) / 2.0f, (z + v.z) / 2.0f, w);}
		vec4 reflect(const vec4& v) const {return vec4(*this - (2.0f * dot(v) * v));}
		// приведение типа
		operator const float*() const {return (const float*)&x;}
		operator float*() const {return (float*)&x;}
		// члены
		union
		{
			struct {float x, y, z, w;};
			__m128 xmm;
			float flt[4];
		};
	};

	__declspec(align(16)) class SSH mtx
	{
	public:
		// конструкторы
		mtx() {identity();}
		mtx(float _f11, float _f12, float _f13, float _f14, float _f21, float _f22, float _f23, float _f24, float _f31, float _f32, float _f33, float _f34, float _f41, float _f42, float _f43, float _f44);
		mtx(__m128 _1, __m128 _2, __m128 _3, __m128 _4) { xmm[0] = _1; xmm[1] = _2; xmm[2] = _3; xmm[3] = _4; }
		mtx(float* mm) {SSH_MEMCPY(*this, mm, sizeof(mtx));}
		mtx(const mtx& mm) {*this = mm;}
		mtx(const mtx& trans, const mtx& rotx, const mtx& roty, const mtx& rotz) {*this = rotx * roty; *this *= rotz * trans;}
		mtx(const quat& q) {fromQuat(q);}
		// операции
		const mtx& operator + () const {return *this;}
		// бинарные
		mtx operator - () const {return mtx(-_11, -_12, -_13, -_14, -_21, -_22, -_23, -_24, -_31, -_32, -_33, -_34, -_41, -_42, -_43, -_44);}
		mtx operator + (float f) const
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_add_ps(xmm[0], a), _mm_add_ps(xmm[1], a), _mm_add_ps(xmm[2], a), _mm_add_ps(xmm[3], a));
		}
		mtx operator - (float f) const
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_sub_ps(xmm[0], a), _mm_sub_ps(xmm[1], a), _mm_sub_ps(xmm[2], a), _mm_sub_ps(xmm[3], a));
		}
		mtx operator * (float f) const
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_mul_ps(xmm[0], a), _mm_mul_ps(xmm[1], a), _mm_mul_ps(xmm[2], a), _mm_mul_ps(xmm[3], a));
		}
		mtx operator / (float f) const
		{
			__m128 a(_mm_rcp_ss(_mm_set_ss(f))) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_mul_ps(xmm[0], a), _mm_mul_ps(xmm[1], a), _mm_mul_ps(xmm[2], a), _mm_mul_ps(xmm[3], a));
		}
		mtx operator + (const mtx& m) const
		{
			return mtx(_mm_add_ps(xmm[0], m.xmm[0]), _mm_add_ps(xmm[1], m.xmm[1]), _mm_add_ps(xmm[2], m.xmm[2]), _mm_add_ps(xmm[3], m.xmm[3]));
		}
		mtx operator - (const mtx& m) const
		{
			return mtx(_mm_sub_ps(xmm[0], m.xmm[0]), _mm_sub_ps(xmm[1], m.xmm[1]), _mm_sub_ps(xmm[2], m.xmm[2]), _mm_sub_ps(xmm[3], m.xmm[3]));
		}
		mtx operator / (const mtx& m) const
		{
			return mtx(_mm_div_ps(xmm[0], m.xmm[0]), _mm_div_ps(xmm[1], m.xmm[1]), _mm_div_ps(xmm[2], m.xmm[2]), _mm_div_ps(xmm[3], m.xmm[3]));
		}
		const mtx& operator += (float f)
		{
			__m128 a(_mm_set_ss(f)); a = _mm_shuffle_ps(a, a, 0);
			xmm[0] = _mm_add_ps(xmm[0], a); xmm[1] = _mm_add_ps(xmm[1], a);
			xmm[2] = _mm_add_ps(xmm[2], a); xmm[3] = _mm_add_ps(xmm[3], a);
			return *this;
		}
		const mtx& operator -= (float f)
		{
			__m128 a(_mm_set_ss(f)); a = _mm_shuffle_ps(a, a, 0);
			xmm[0] = _mm_sub_ps(xmm[0], a); xmm[1] = _mm_sub_ps(xmm[1], a);
			xmm[2] = _mm_sub_ps(xmm[2], a); xmm[3] = _mm_sub_ps(xmm[3], a);
			return *this;
		}
		const mtx& operator *= (float f)
		{
			__m128 a(_mm_set_ss(f)); a = _mm_shuffle_ps(a, a, 0);
			xmm[0] = _mm_mul_ps(xmm[0], a); xmm[1] = _mm_mul_ps(xmm[1], a);
			xmm[2] = _mm_mul_ps(xmm[2], a); xmm[3] = _mm_mul_ps(xmm[3], a);
			return *this;
		}
		const mtx& operator /= (float f)
		{
			__m128 a(_mm_rcp_ss(_mm_set_ss(f))); a = _mm_shuffle_ps(a, a, 0);
			xmm[0] = _mm_mul_ps(xmm[0], a); xmm[1] = _mm_mul_ps(xmm[1], a);
			xmm[2] = _mm_mul_ps(xmm[2], a); xmm[3] = _mm_mul_ps(xmm[3], a);
			return *this;
		}
		const mtx& operator += (const mtx& m)
		{
			xmm[0] = _mm_add_ps(xmm[0], m.xmm[0]); xmm[1] = _mm_add_ps(xmm[1], m.xmm[1]);
			xmm[2] = _mm_add_ps(xmm[2], m.xmm[2]); xmm[3] = _mm_add_ps(xmm[3], m.xmm[3]);
			return *this;
		}
		const mtx& operator -= (const mtx& m)
		{
			xmm[0] = _mm_sub_ps(xmm[0], m.xmm[0]); xmm[1] = _mm_sub_ps(xmm[1], m.xmm[1]);
			xmm[2] = _mm_sub_ps(xmm[2], m.xmm[2]); xmm[3] = _mm_sub_ps(xmm[3], m.xmm[3]);
			return *this;
		}
		const mtx& operator /= (const mtx& m)
		{
			xmm[0] = _mm_div_ps(xmm[0], m.xmm[0]); xmm[1] = _mm_div_ps(xmm[1], m.xmm[1]);
			xmm[2] = _mm_div_ps(xmm[2], m.xmm[2]); xmm[3] = _mm_div_ps(xmm[3], m.xmm[3]);
			return *this;
		}
		mtx operator * (const mtx& m) const
		{
			return mtx(ssh_mtx_mtx(*this, m));
		}
		const mtx& operator *= (const mtx& m)
		{
			return (*this = ssh_mtx_mtx(*this, m));
		}
		friend mtx operator + (float f,  const mtx& m)
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_add_ps(a, m.xmm[0]), _mm_add_ps(a, m.xmm[1]), _mm_add_ps(a, m.xmm[2]), _mm_add_ps(a, m.xmm[3]));
		}
		friend mtx operator - (float f,  const mtx& m)
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_sub_ps(a, m.xmm[0]), _mm_sub_ps(a, m.xmm[1]), _mm_sub_ps(a, m.xmm[2]), _mm_sub_ps(a, m.xmm[3]));
		}
		friend mtx operator * (float f,  const mtx& m)
		{
			__m128 a(_mm_set_ss(f)) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_mul_ps(a, m.xmm[0]), _mm_mul_ps(a, m.xmm[1]), _mm_mul_ps(a, m.xmm[2]), _mm_mul_ps(a, m.xmm[3]));
		}
		friend mtx operator / (float f,  const mtx& m)
		{
			__m128 a(_mm_rcp_ss(_mm_set_ss(f))) ; a = _mm_shuffle_ps(a, a, 0);
			return mtx(_mm_mul_ps(a, m.xmm[0]), _mm_mul_ps(a, m.xmm[1]), _mm_mul_ps(a, m.xmm[2]), _mm_mul_ps(a, m.xmm[3]));
		}
		const mtx& operator = (const mtx& m) {SSH_MEMCPY((void*)this, (void*)&m, sizeof(mtx)) ; return *this;}
		float operator [] (ssh_u idx) const { return (idx < 16 ? m[idx] : 0.0f); }
		bool operator == (const mtx& m) {return (memcmp(*this, &m, sizeof(mtx)) == 0);}
		bool operator != (const mtx& m) {return !(operator == (m));}
		float MINOR(const mtx& m, int r0, int r1, int r2, int c0, int c1, int c2) const;
		// определитель
		float determinant() const;
		mtx adjoint() const;
		// инвертировать
		const mtx& inverse() {*this = mtx(adjoint() * (1.0f / determinant())); return *this;}
		// матрица вида
		const mtx& view(const vec3& pos, const vec3& at, const vec3& up);
		// теневая
		const mtx& shadow(const vec4& l, const plane& pln);
		// из кватерниона
		const mtx& fromQuat(const quat& q);
		// мировая матрица
		const mtx& world(const vec3& position, const vec3& scale, const quat& orientation);
		// отражения
		const mtx& reflect(const plane& pl);
		// ортографическая матрица
		const mtx& ortho(float w, float h, float zn, float zf);
		// перспективная матрица
		const mtx& perspective(float w, float h, float zn, float zf);
		// перспективная матрица из угла камеры
		const mtx& perspectiveFov(float fovy, float aspect, float zn, float zf);
		// перевернуть
		const mtx& transpose() {*this = mtx(_11, _21, _31, _41, _12, _22, _32, _42, _13, _23, _33, _43, _14, _24, _34, _44) ; return *this;}
		// единичная
		const mtx& identity() {SSH_MEMZERO(this, sizeof(mtx)) ; _11 = 1.0f; _22 = 1.0f; _33 = 1.0f; _44 = 1.0f; return *this;}
		// матрица позиции
		const mtx& make_translate(const vec3& v) {identity() ; _41 = v.x; 	_42 = v.y; _43 = v.z ; return *this;}
		// матрица масштабирования
		const mtx& make_scale(const vec3& v) {identity() ; _11 = v.x; 	_22 = v.y; 	_33 = v.z ; return *this;}
		// создать матрицу поворота из углов Эйлера
		const mtx& set_rotateEuler(float yaw, float pitch, float roll);
		// из произвольной оси и угла
		const mtx& set_rotateAngleAxis(const vec3& v, float angle);
		// установить позицию
		const mtx& set_translate(const vec3& v) {_41 = v.x ; _42 = v.y ; _43 = v.z ; return *this;}
		// установить масштаб
		const mtx& set_scale(const vec3& v) {_11 = v.x ; _22 = v.y ; _33 = v.z ; return *this;}
		// убрать позицию
		const mtx& no_trans() {_41 = 0.0f ; _42 = 0.0f ; _43 = 0.0f ; return *this;}
		// вернуть смещение
		vec3 get_translate() const {return vec3(_41, _42, _43);}
		// вернуть масштаб
		vec3 get_scale() const {return vec3(_11, _22, _33);}
		// вернуть углы Эйлера
		vec3 get_rotateEuler() const;
		const mtx& from3dsMax(float* f);
		// приведение типа
		operator const float*() const {return (const float*)m;}
		operator float*() const {return (float*)m;}
		// члены
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			__m128 xmm[4];
			float m[16];
		};
	};

	__declspec(align(16)) class SSH quat
	{
	public:
		// конструкторы
		quat() {identity();}
		quat(float X, float Y, float Z, float W = 1.0f) : x(X), y(Y), z(Z), w(W) {}
		quat(float* f) { x = f[0]; y = f[1]; z = f[2]; w = f[3];}
		quat(const vec3& v, float f) {rotateAxis(v, f);}
		quat(const vec4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
		quat(const quat& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
		quat(const mtx& m) {fromMatrix(m);}
		// операции
		const quat& operator + () const {return *this;}
		quat operator - () const {return quat(-x, -y, -z, -w);}
		// бинарные
		quat operator + (const quat& q) const {return quat(x + q.x, y + q.y, z + q.z, w + q.w);}
		quat operator + (float f) const {return quat(x + f, y + f, z + f, w + f);}
		quat operator - (const quat& q) const {return quat(x - q.x, y - q.y, z - q.z, w - q.w);}
		quat operator - (float f) const {return quat(x - f, y - f, z - f, w - f);}
		quat operator * (const quat& q) const;
		quat operator * (float f) const {return quat(x * f, y * f, z * f, w * f);}
		quat operator / (const quat& q) const {return quat(x / q.x, y / q.y, z / q.z, w / q.w);}
		quat operator / (float f) const {f = 1.0f / f; return quat(x * f, y * f, z * f, w * f);}
		const quat& operator += (const quat& q) {x += q.x ; y += q.y ; z += q.z ; w += q.w; return *this;}
		const quat& operator += (float f) {x += f ; y += f ; z += f ; w += f; return *this;}
		const quat& operator -= (const quat& q) {x -= q.x ; y -= q.y ; z -= q.z ; w -= q.w; return *this;}
		const quat& operator -= (float f) {x -= f ; y -= f ; z -= f ; w -= f; return *this;}
		const quat& operator *= (const quat& q);
		const quat& operator *= (float f) {x *= f ; y *= f ; z *= f ; w *= f; return *this;}
		const quat& operator /= (const quat& q) {x /= q.x ; y /= q.y ; z /= q.z ; w /= q.w; return *this;}
		const quat& operator /= (float f) {f = 1.0f / f; x *= f ; y *= f ; z *= f ; w *= f; return *this;}
		friend quat operator + (float f, const quat& q) {return quat(f + q.x, f + q.y, f + q.z, f + q.w);}
		friend quat operator - (float f, const quat& q) {return quat(f - q.x, f - q.y, f - q.z, f - q.w);}
		friend quat operator * (float f, const quat& q) {return quat(f * q.x, f * q.y, f * q.z, f * q.w);}
		friend quat operator / (float f, const quat& q) {return quat(f / q.x, f / q.y, f / q.z, f / q.w);}
		// логические
		bool operator == (const quat& q) const { return ((fabs(x - q.x) + fabs(y - q.y) + fabs(z - q.z) + fabs(w - q.w)) < SSH_EPSILON); }
		bool operator != (const quat& q) const {return !(operator == (q));}
		// присваивание
		const quat& operator = (const quat& q) {x = q.x ; y = q.y ; z = q.z ; w = q.w; return *this;}
		// специальные
		bool is_identity() const { return (fabs(x - y - z) < SSH_EPSILON2 && fabs(w - 1) < SSH_EPSILON2); }
		float dot(const quat& q) const {return w * q.w + x * q.x + y * q.y + z * q.z;}
		float length() const {return sqrtf(x * x + y * y + z * z + w * w);}
		float lengthSq() const {return (x * x + y * y + z * z + w * w);}
		float roll() const {return atan2(2 * (x * y + w * z), w * w + x * x - y * y - z * z);}
		float pitch() const {return atan2(2 * (y * z + w * x), w * w - x * x - y * y + z * z);}
		float yaw() const {return asin(-2 * (x * z - w * y));}
		const quat& set(float X, float Y, float Z, float W) {x = X ; y = Y ; z = Z ; w = W; return *this;}
		const quat& identity() {x = 0.0f; y = 0.0f; z = 0.0f; w = 1.0f; return *this;}
		vec3 get_rotate() const {return vec3(yaw(), pitch(), roll());}
		void angleAxis(vec3& axis, float& angle) const;
		quat slerp(const quat& q, float t, bool shortestPath) const;
		const quat& rotateAxis(const vec3& v, float theta);
		//Преобразование сферических координат в кватернион
		const quat& fromSpherical(float latitude, float longitude, float angle);
		const quat& fromMatrix(const mtx& mm);
		vec3 xAxis() const;
		vec3 yAxis() const;
		vec3 zAxis() const;
		const quat& inverse();
		const quat& exp();
		const quat& ln();
		const quat& angles(float yaw, float pitch, float roll);
		quat nlerp(const quat& q, float t, bool shortestPath) const;
		quat squad(const quat& q1, const quat& q2, const quat& q3, float t, bool shortestPath) const;
		const quat& normalize() {float factor(1.0f / sqrt(lengthSq())); *this *= factor; return *this;}
		// приведение типа
		operator const float*() const {return (const float*)&x;}
		operator float*() const {return (float*)&x;}
		// члены
		union
		{
			struct {float x, y, z, w;};
			__m128 xmm;
			float flt[4];
		};
	};

	__declspec(align(16)) class SSH dual_quat
	{
	public:
		// конструкторы
		dual_quat() {make(quat(), vec3());}
		dual_quat(float* f) {make(f, &f[4]);}
		dual_quat(float* q, float* v) {make(q, v);}
		dual_quat(const quat& q) {make(q, vec3(0, 0, 0));}
		dual_quat(const dual_quat& qq) {SSH_MEMCPY(this, &qq, sizeof(dual_quat));}
		dual_quat(const quat& q, const vec3& v) {make(q, v);}
		dual_quat(const mtx& m) {make(quat(m), m.get_translate());}
		// формирование
		const dual_quat& make(float* qq, float* t)
		{
			flt[0] = qq[0]; flt[1] = qq[1]; flt[2] = qq[2]; flt[3] = qq[3];
			flt[4] = -0.5f * ( t[0] * qq[1] + t[1] * qq[2] + t[2] * qq[3]);
			flt[5] = 0.5f  * ( t[0] * qq[0] + t[1] * qq[3] - t[2] * qq[2]);
			flt[6] = 0.5f  * (-t[0] * qq[3] + t[1] * qq[0] + t[2] * qq[1]);
			flt[7] = 0.5f  * ( t[0] * qq[2] - t[1] * qq[1] + t[2] * qq[0]);

			return *this;
		}
		// логические
		bool operator == (const dual_quat& qq) const
		{
			return ((fabsf(flt[0] - qq.flt[0]) + fabsf(flt[1] - qq.flt[1]) + fabsf(flt[2] - qq.flt[2]) + 
					 fabsf(flt[3] - qq.flt[3]) + fabsf(flt[4] - qq.flt[4]) + fabsf(flt[5] - qq.flt[5]) + 
					 fabsf(flt[6] - qq.flt[6]) + fabsf(flt[7] - qq.flt[7])) < SSH_EPSILON);
		}
		bool operator != (const dual_quat& qq) const {return !(operator == (qq));}
		// приведение типа
		operator const quat() const {return quat((float*)flt);}
		operator const vec3() const
		{
			return vec3(2.0f * (-flt[4] * flt[1] + flt[5] * flt[0] - flt[6] * flt[3] + flt[7] * flt[2]),
						2.0f * (-flt[4] * flt[2] + flt[5] * flt[3] + flt[6] * flt[0] - flt[7] * flt[1]),
						2.0f * (-flt[4] * flt[3] - flt[5] * flt[2] + flt[6] * flt[1] + flt[7] * flt[0]));
		}
		operator const float*() const {return (const float*)flt;}
		operator float*() const {return (float*)flt;}
		// члены
		union
		{
			float flt[8];
			__m128 xmm[2];
		};
	};

	class SSH plane
	{
	public:
		enum Side
		{
			NO_SIDE,
			POSITIVE_SIDE,
			NEGATIVE_SIDE
		};
		// коснтрукторы
		plane() : x(0), y(0), z(0), d(0) {}
		plane(float X, float Y, float Z, float D) : x(X), y(Y), z(Z), d(D) {}
		plane(float* f) { x = f[0]; y = f[1]; z = f[2]; d = f[3];}
		plane(const plane& p) : x(p.x), y(p.y), z(p.z), d(p.d) {}
		plane(const vec4& v) : x(v.x), y(v.y), z(v.z), d(v.w) {}
		plane(const vec3& v, float dist) : x(v.x), y(v.y), z(0.0f), d(dist) {}
		plane(const vec3& point, const vec3& normal) : x(normal.x), y(normal.y), z(normal.z), d(point.x * normal.x + point.y * normal.y + point.z * normal.z) {}
		plane(const vec3& v1, const vec3& v2, const vec3& v3);
		plane operator * (const mtx& mm) const;
		const plane& normalize() {vec.normalize(); return *this;}
		Side side(const vec3& v) const;
		// операции
		// унарные
		const plane& operator + () const {return *this;}
		plane operator - () const {return plane(x, y, z, -d);}
		// бинарные
		friend plane operator * (const mtx& mm, const plane& p);
		// логические
		bool operator == (const plane& p) const { return ((fabs(x - p.x) + fabs(y - p.y) + fabs(z - p.z) + fabs(d - p.d)) < SSH_EPSILON); }
		bool operator != (const plane& p) const { return !(operator == (p)); }
		// присваивание
		const plane& operator = (const plane& p) {x = p.x ; y = p.y ; z = p.z ; d = p.d; return *this;}
		// приведение типов
		operator float*() const {return (float*)&x;}
		operator const float*() const {return (const float*)&x;}
		// специальные
		float dot(const vec4& v) const {return x * v.x + y * v.y + z * v.z + d * v.w;}
		float dotNormal(const vec3& v) const {return x * v.x + y * v.y + z * v.z;}
		float distance(const vec3& v) const {return normal.dot(v) + scalar;}
		union
		{
			struct {vec3 normal; float scalar;};
			struct {vec4 vec;};
			struct {float x, y, z, d;};
			float flt[4];
		};
	};

	class SSH color
	{
	public:
		// конструкторы
		color() : r(0), g(0), b(0), a(0) {}
		color(float red, float green, float blue, float alpha) : r(red), g(green), b(blue), a(alpha) {saturate();}
		color(long rgba) {BGRA(rgba);}
		color(const color& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		color(float* f) { r = f[0]; g = f[1]; b = f[2]; a = f[3];}
		// операции
		// арифметические
		color operator + (const color& c) const {return color(r + c.r, g + c.g, b + c.b, a + c.a);}
		color operator - (const color& c) const {return color(r - c.r, g - c.g, b - c.b, a - c.a);}
		color operator * (float f) const {return color(r * f, g * f, b * f, a * f);}
		color operator * (const color& c) const {return color(r * c.r, g * c.g, b * c.b, a * c.a);}
		color operator / (const color& c) const {return color(r / c.r, g / c.g, b / c.b, a / c.a);}
		color operator / (float f) const {f = 1.0f / f; return color(r * f, g * f, b * f, a * f);}
		const color& operator += (const color& c) {r += c.r ; g += c.g ; b += c.b ; a += c.a; return *this;}
		const color& operator -= (const color& c) {r -= c.r ; g -= c.g ; b -= c.b ; a -= c.a; return *this;}
		const color& operator *= (float f) {r *= f ; g *= f ; b *= f ; a *= f; return *this;}
		const color& operator /= (float f) {f = 1.0f / f; r *= f ; g *= f ; b *= f ; a *= f; return *this;}
		// логические
		float operator [] (ssh_u idx) const { return (idx < 4 ? flt[idx] : 0.0f); }
		bool operator == (const color& c) const { return ((fabs(r - c.r) + fabs(g - c.g) + fabs(b - c.b) + fabs(a - c.a)) < SSH_EPSILON); }
		bool operator != (const color& c) const {return !(*this == c);}
		friend color operator * (float f, const color& c) {return color(f * c.r, f * c.g, f * c.b, f * c.a);}
		// приведение
		operator const float*() const {return (const float*)&r;}
		operator float*() const {return (float*)&r;}
		// специальные
		void HSB(float hue, float saturation, float brightness);
		void RGBA(long val);
		void BGRA(long val);
		long RGBA() const;
		long BGRA() const;
		const color& saturate();
		// члены
		union
		{
			struct {float r, g, b, a;};
			float flt[4];
		};
	};

	class SSH sphere
	{
	public:
		sphere() {identity();}
		sphere(const vec3& v, float f) : c(v), r(f) {}
		sphere(const sphere& s) : c(s.c), r(s.r) {}
		sphere(float* f) {SSH_MEMCPY(flt, f, sizeof(sphere));}
		sphere operator + (const sphere& s) const;
		const sphere& operator += (const sphere& s) {contactSphere(s, c, r);return *this;}
		void identity() {r = 0.0f ; c.x = 0.0f ; c.y = 0.0f ; c.z = 0.0f;}
		bool is_identity() const { return (r < SSH_EPSILON && fabs(c.x - c.y - c.z) < SSH_EPSILON); }
		// установить радиус
		void setRadius(float f) {r = f;}
		// установить центр
		void setCenter(const vec3& v) {c = v;}
		// вернуть радиус
		float radius() const {return r;}
		// вернуть центр
		const vec3& center() const {return c;}
		// проверки на пересечения
		bool intersects(const bbox& box) const;
		bool intersects(const sphere& s) const;
		bool intersects(const plane& plane) const;
		bool intersects(const vec3& v) const;
		union
		{
			struct {vec3 c; float r;};
			float flt[4];
		};
	protected:
		void contactSphere(const sphere& s, vec3& v, float& f) const;
	};

	class SSH bbox
	{
	public:
		bbox() {setMinimum(-1, -1, -1) ; setMaximum(1, 1, 1);}
		bbox(const vec3& min, const vec3& max) {setExtents(min, max);}
		bbox(float x1, float y1, float z1, float x2, float y2, float z2) {setExtents(x1, y1, z1, x2, y2, z2);}
		bbox(const bbox& box) {setExtents(box.mn, box.mx);}
		bbox(float* box) { setExtents(box[0], box[1], box[2], box[3], box[4], box[5]);}
		void setMinimum(const vec3& vec) {mn = vec; updateCorners();}
		void setMinimum(float x, float y, float z) {mn.set(x, y, z); updateCorners();}
		void setMaximum(const vec3& vec) {mx = vec; updateCorners();}
		void setMaximum(float x, float y, float z) {mx.set(x, y, z); updateCorners();}
		void setExtents(const vec3& min, const vec3& max) {mn = min ; mx = max; updateCorners();}
		void setExtents(float x1, float y1, float z1, float x2, float y2, float z2) {mn.x = x1; mn.y = y1; mn.z = z1; mx.x = x2; mx.y = y2; mx.z = z2; updateCorners();}
		void scale(const vec3& s) {setExtents(mn * s, mx * s);}
		bool intersects(const vec3& v) const {return(v.x >= mn.x && v.x <= mx.x && v.y >= mn.y && v.y <= mx.y && v.z >= mn.z && v.z <= mx.z);}
		const vec3& minimum() const {return mn;}
		const vec3& maximum() const {return mx;}
		bool intersects(const bbox& b2) const;
		bool intersects(const sphere& s) const;
		bbox intersection(const bbox& b2) const;
		void transform(const mtx& matrix);
		void merge(const bbox& b2);
		void merge(const vec3& point);
		float volume() const {vec3 diff(mx - mn); return diff.x * diff.y * diff.z;}
		vec3 center() const {return vec3((mx + mn) / 2.0f);}
		// вернуть массив углов бокса
		//			       1-----2
		//			      /|    /|
		//			     / |   / |
		//			    5-----4  |
		//			    |  0--|--3
		//			    | /   | /
		//			    |/    |/
		//			    6-----7
		const vec3* getAllCorners() const {return (const vec3*)corners;}
		vec3 mn;
		vec3 mx;
		vec3 corners[8];
	protected:
		void updateCorners();
	};

	class SSH obox
	{
	public:
		obox() {identity();}
		obox(const obox& b) : x1y1z1(b.x1y1z1), x2y1z1(b.x2y1z1), x1y2z1(b.x1y2z1), x2y2z1(b.x2y2z1), x1y1z2(b.x1y1z2), x2y1z2(b.x2y1z2), x1y2z2(b.x1y2z2), x2y2z2(b.x2y2z2) {}
		obox(const vec3& _x1y1z1, const vec3& _x2y1z1, const vec3& _x1y2z1, const vec3& _x2y2z1, const vec3& _x1y1z2, const vec3& _x2y1z2, const vec3& _x1y2z2, const vec3& _x2y2z2);
		obox(const bbox& bbox);
		obox(float* b);
		void identity() {SSH_MEMZERO(this, sizeof(obox));}
		bool intersects(const bbox& b) const;
		bool intersects(const obox& b) const;
		bool intersects(const sphere& s) const;
		bool intersects(const vec3& v) const;
		obox transform(const mtx& m) const;;
		vec3 center() const;
		//			       5-----6
		//			      /|    /|
		//			     / |   / |
		//			    1-----2  |
		//			    |  7--|--8
		//			    | /   | /
		//			    |/    |/
		//			    3-----4
		vec3 x1y1z1;// 1
		vec3 x2y1z1;// 2
		vec3 x1y2z1;// 3
		vec3 x2y2z1;// 4
		vec3 x1y1z2;// 5
		vec3 x2y1z2;// 6
		vec3 x1y2z2;// 7
		vec3 x2y2z2;// 8
	};

	class SSH ray
	{
	public:
		ray() {}
		ray(const vec3& position, const vec3& direction) : pos(position), dir(direction) {}
		ray(const ray& r) : pos(r.pos), dir(r.dir) {}
		vec3 operator * (float t) const {return point(t);}
		void setPosition(const vec3& position) {pos = position;}
		void setDirection(const vec3& direction) {dir = direction;}
		bool intersects(const sphere& s, float* f) const;
		bool intersects(const bbox& box, float* f) const;
		bool intersects(const plane& p, float* f) const;
		const vec3& position() const {return pos;}
		const vec3& direction() const {return dir;}
		vec3 point(float t) const {return vec3(pos + (dir * t));}
		vec3 pos;
		vec3 dir;
	};

	class SSH Angle
	{
	public:
		Angle() :angle(0.0f) {}
		Angle(float v) : angle(v) {}
		float Degree() const {return (float)((SSH_PI / 180.0f) * angle);}
		float Radian() const {return (float)((180.0f / SSH_PI) * angle);}
		float angle;
	};

	class SSH Half
	{
		union Bits
		{
			float			f;
			long			si;
			unsigned long	ui;
		};

		static int const shift = 13;
		static int const shiftSign = 16;
		static long const infN = 0x7F800000; // flt32 infinity
		static long const maxN = 0x477FE000; // max flt16 normal as a flt32
		static long const minN = 0x38800000; // min flt16 normal as a flt32
		static long const signN = 0x80000000; // flt32 sign bit

		static long const infC = infN >> shift;
		static long const nanN = (infC + 1) << shift; // minimum flt16 nan as a flt32
		static long const maxC = maxN >> shift;
		static long const minC = minN >> shift;
		static long const signC = signN >> shiftSign; // flt16 sign bit

		static long const mulN = 0x52000000; // (1 << 23) / minN
		static long const mulC = 0x33800000; // minN / (1 << (23 - shift))

		static long const subC = 0x003FF; // max flt32 subnormal down shifted
		static long const norC = 0x00400; // min flt32 normal down shifted

		static long const maxD = infC - maxC - 1;
		static long const minD = minC - subC - 1;

	public:

		static ssh_w compress(float value)
		{
			Bits v, s;
			v.f = value;
			unsigned long sign = v.si & signN;
			v.si ^= sign;
			sign >>= shiftSign; // logical shift
			s.si = mulN;
			s.si = (long)(s.f * v.f); // correct subnormals
			v.si ^= (s.si ^ v.si) & -(minN > v.si);
			v.si ^= (infN ^ v.si) & -((infN > v.si) & (v.si > maxN));
			v.si ^= (nanN ^ v.si) & -((nanN > v.si) & (v.si > infN));
			v.ui >>= shift; // logical shift
			v.si ^= ((v.si - maxD) ^ v.si) & -(v.si > maxC);
			v.si ^= ((v.si - minD) ^ v.si) & -(v.si > subC);
			return (ssh_w)(v.ui | sign);
		}

		static float decompress(ssh_w value)
		{
			Bits v;
			v.ui = value;
			long sign = v.si & signC;
			v.si ^= sign;
			sign <<= shiftSign;
			v.si ^= ((v.si + minD) ^ v.si) & -(v.si > subC);
			v.si ^= ((v.si + maxD) ^ v.si) & -(v.si > maxC);
			Bits s;
			s.si = mulC;
			s.f *= v.si;
			long mask = -(norC > v.si);
			v.si <<= shift;
			v.si ^= (s.si ^ v.si) & mask;
			v.si |= sign;
			return v.f;
		}
	};

	inline vec3 vec3::operator * (const mtx& m) const
	{
		return vec3(ssh_vec3_mtx(*this, m));
	}

	inline const vec3& vec3::operator *= (const mtx& m)
	{
		return (*this = ssh_vec3_mtx(*this, m));
	}

	inline vec3 operator * (const mtx& m, const vec3& v)
	{
		return vec3(ssh_mtx_vec3(m, v));
	}

	inline vec4 vec4::operator * (const mtx& m) const
	{
		return vec4(ssh_vec4_mtx(*this, m));
	}

	inline const vec4& vec4::operator *= (const mtx& m)
	{
		return (*this = ssh_vec4_mtx(*this, m));
	}

	inline vec4 operator * (const mtx& m, const vec4& v)
	{
		return vec4(ssh_mtx_vec4(m, v));
	}

	inline const mtx& mtx::set_rotateEuler(float yaw, float pitch, float roll)
	{
		quat q;
		return fromQuat(q.angles(yaw, pitch, roll));
	}
	
	inline const mtx& mtx::set_rotateAngleAxis(const vec3& v, float angle)
	{
		quat q;
		return fromQuat(q.rotateAxis(v, angle));
	}

	inline vec3 mtx::get_rotateEuler() const
	{
		quat q(*this);
		return q.get_rotate();
	}

	inline bool sphere::intersects(const sphere& s) const
	{
		float rr(s.r + r); rr *= rr;
		return ((s.c - c).lengthSq() <= rr);
	}
	inline bool sphere::intersects(const plane& plane) const
	{
		vec3 v((float*)&plane);
		return (fabs(v.dot(c)) <= r);
	}
	inline bool sphere::intersects(const vec3& v) const
	{
		return ((v - c).lengthSq() <= (r * r));
	}
}
