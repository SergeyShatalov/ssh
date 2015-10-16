
#include "stdafx.h"

#pragma pack(push, 1)

struct HEADER_TGA
{
	enum TypesTGA : ssh_b
	{
		INDEXED = 1,
		RGB = 2,
		GREY = 3,
		RLE = 8
	};
	enum FlagsTGA : ssh_b
	{
		MASK = 0x30,
		RIGHT = 0x10,
		UPPER = 0x20,
		ALPHA = 0x08
	};
	ssh_b	bIdLength;		//
	ssh_b	bColorMapType;	// тип цветовой карты ()
	ssh_b	bType;			// тип файла ()
	ssh_w	wCmIndex;		// тип индексов в палитре
	ssh_w	wCmLength;		// длина палитры
	ssh_b	bCmEntrySize;	// число бит на элемент палитры
	ssh_w	wXorg;			// 
	ssh_w	wYorg;			// 
	ssh_w	wWidth;			// ширина
	ssh_w	wHeight;		// высота
	ssh_b	bBitesPerPixel;	// бит на пиксель
	ssh_b	bFlags;			// 
};

#pragma pack(pop)

struct vector
{
	vector()
	{
		var[0] = -100;
		var[1] = -200;
		//var = -1000;
		v2[0].x = 11.11f;
		v2[0].y = 22.22f;
		v2[1].x = 111.11f;
		v2[1].y = 222.22f;
		v2[2].x = 1111.11f;
		v2[2].y = 2222.22f;
		v3.x = 111.111f;
		v3.y = 222.222f;
		v3.z = 333.333f;
	}
	ssh_u var[2];
	vec2 v2[3];
	vec3 v3;
};
class Temp3 : public Serialize
{
public:
	//Temp3() : str("Сергей Викторович"), xx(1), yy(2), zz(3) {}
	Temp3() {}
	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp3, xx, L"temp3", nullptr, 1)
			SCHEME_VAR(Temp3, xx, L"xx", 1, 0, L"1.0", nullptr)
			SCHEME_VAR(Temp3, yy, L"yy", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp3, zz, L"zz", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp3, str, L"str", 1, 0, L"", nullptr)
			SCHEME_OBJ_BEGIN(Temp3, vv, L"stk_vector", 1, 1)
				SCHEME_OBJ_BEGIN(vector, v2, L"vec2", 3, 2)
					SCHEME_OBJ_VAR1(vector, vv, vec2, v2[0], x, L"x", 1, 0, L"0.0", nullptr, 2)
					SCHEME_OBJ_VAR1(vector, vv, vec2, v2[0], y, L"y", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_END()
				SCHEME_OBJ_BEGIN(vector, v3, L"vec3", 1, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, x, L"x", 1, 0, L"0.0", nullptr, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, y, L"y", 1, 0, L"0.0", nullptr, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, z, L"z", 1, 0, L"0.0", nullptr, 3)
				SCHEME_OBJ_END()
			SCHEME_OBJ_VAR(vector, vv, var, L"var", 2, 0, L"0.0", nullptr, 1)
		SCHEME_END(Temp);
	}
	vector vv;
	String str;
	double xx;
	ssh_w yy;
	short zz;
};

class Temp2 : public Serialize
{
public:
	//Temp2() :x(1), y(2), z(3) {}
	//Temp2(float _x, ssh_u _y, short _z) :x(_x), y(_y), z(_z) {}
	Temp2() {}

	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp2, x, L"temp2", nullptr, 1)
			SCHEME_VAR(Temp2, x, L"x", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp2, y, L"y", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp2, z, L"z", 1, 0, L"1", nullptr)
			SCHEME_NOD(Temp2, tmp, L"tmp", nullptr, 1)
		SCHEME_END(Temp);
	}
	float x;
	ssh_u y;
	short z;
	Temp3 tmp;
};

enum ee
{
	ХУЙ, ПИЗДА, МАНДА, ЕБЛО
};

#define ssh_enum_tp

ENUM_DATA _stk[]=
{
	_E(ХУЙ),
	_E(ПИЗДА),
	_E(ЕБЛО),
	{nullptr, 0}
};

class Temp : public Resource
{
	SSH_DYNCREATE(Temp);
public:
	Temp() { _cs = 35; }
	/*
	Temp()
	{
		_wcs = L"Sergey";
		_ccs = "Vlad";
		_ws[0] = L'С'; _ws[1] = L'е'; _ws[2] = L'р'; _ws[3] = L'г'; _ws[4] = L'е'; _ws[5] = L'й'; _ws[6] = L' '; _ws[7] = L'Ш'; _ws[8] = L'С'; _ws[9] = L'!';
//		_cs[0] = 'Ш'; _cs[1] = 'а'; _cs[2] = 'т'; _cs[3] = 'а'; _cs[4] = 'л'; _cs[5] = 'о'; _cs[6] = 'в'; _cs[7] = '!'; _cs[8] = '!'; _cs[9] = '!';
		tmp[0] = Temp2(1, 2, 3);
		tmp[1] = Temp2(10, 20, 30);
		str[0] = L"Shatalov"; str[1] = L"Петров"; str[2] = L"Иванов"; x[0] = 14; x[1] = 15; v[0].x = 22.0f; v[0].y = 15.0f; v[1].x = 222.0f; v[1].y = 155.0f; d[0] = 1.1; d[1] = 2.2; d[2] = 3.3;
	}
	*/
	Temp(ssh_wcs path) { open(path); }
	virtual void save(ssh_wcs path, bool is_xml) override
	{
		Serialize::save(path, this, is_xml);
	}
	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp, x, L"temp", L"value", 1)
			SCHEME_VAR(Temp, d, L"temp_d", 3, 0, L"0.0", nullptr)
			SCHEME_VAR(Temp, _wcs, L"wcs", 1, 0, L"И", nullptr)
			SCHEME_VAR(Temp, _ccs, L"ccs", 1, 0, L"И", nullptr)
			SCHEME_OBJ_BEGIN(Temp, v, L"vector", 3, 1)
				SCHEME_OBJ_VAR(vec2, v[0], x, L"x", 1, 0, L"0.0", nullptr, 1)
				SCHEME_OBJ_VAR(vec2, v[0], y, L"y", 1, 0, L"0.0", nullptr, 1)
			SCHEME_OBJ_END()
			SCHEME_VAR(Temp, x, L"x", 2, SC_FLAGS, L"unknown", _stk)
			SCHEME_OBJ_BEGIN(Temp, v3, L"vec3", 1, 2)
				SCHEME_OBJ_VAR(vec3, v3, x, L"x", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_VAR(vec3, v3, y, L"y", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_VAR(vec3, v3, z, L"z", 1, 0, L"0.0", nullptr, 2)
			SCHEME_OBJ_END()
			SCHEME_VAR(Temp, str, L"string", 3, SC_BASE64, L"Иванов", nullptr)
			SCHEME_VAR(Temp, _ws, L"ws", 10, 0, L"И", nullptr)
			SCHEME_VAR(Temp, _cs, L"cs", 1, 0, L"И", nullptr)
			SCHEME_NOD(Temp, tmp, L"tmp", nullptr, 2)
			SCHEME_END(Temp);
	}
	double d[3];
	virtual ~Temp() {}
protected:
	ssh_wcs _wcs;
	ssh_ccs _ccs;
	ssh_ws _ws[10];
	char _cs;
	int x[2];
	Temp2 tmp[2];
	vec2 v[3];
	vec3 v3;
	String str[3];
	// сформировать из памяти
	virtual void make(const Buffer<ssh_cs>& buf) override
	{
		Serialize::open(buf, this, true);
	}
};

extern "C"
{
	ssh_u asm_ssh_shufb();
}

struct Atl
{
	Atl() { is = true; }
	Atl(int w, int h) { is = true; ixywh.set(0, 0, w, h); }
	Bar<int> ixywh;
	bool is;
};

static Array<Atl*, SSH_PTR> atls;

static bool packed_atlas(Range<int>& rn)
{
	int wmax(0), hmax(0), i, j;
	static Bar<int> bars[4];
	// массив свободных узлов
	Array<Bar<int>, SSH_TYPE> nodes;
	// базовый свободный узел
	nodes += Bar<int>(rn);
	// 1. сортируем по высоте
	for(i = 0; i < atls.size(); i++)
	{
		int h1(atls[0]->ixywh.h), _i(i);
		for(j = i + 1; j < atls.size(); j++)
		{
			int h2(atls[j]->ixywh.h);
			if(h2 > h1) { h1 = h2; _i = j; }
		}
		if(_i != i) ssh_swap<Atl*>(atls[_i], atls[i]);
	}
	// 2. отсортировать по ширине
	for(i = 0; i < atls.size(); i++)
	{
		int w1(atls[i]->ixywh.w), _i(i);
		for(j = i + 1; j < atls.size(); j++)
		{
			int w2(atls[j]->ixywh.w);
			if(w2 > w1) { w1 = w2; _i = j; }
		}
		if(_i != i) ssh_swap<Atl*>(atls[_i], atls[i]);
	}
	// 3. упаковать
	for(j = 0; j < atls.size(); j++)
	{
		Atl* atl(atls[j]);
		Bar<int>* barA(&atl->ixywh);
		int aw(barA->w + 1), ah(barA->h + 1);
		for(i = 0; i < nodes.size(); i++)
		{
			Bar<int>* barN(&nodes[i]);
			if(aw <= barN->w && ah <= barN->h)
			{
				barA->x = barN->x; barA->y = barN->y;
				// формируем 4 дочерних узла
				bars[0].set(barN->x, barN->y + ah, barN->w, barN->h - ah);
				bars[1].set(barN->x + aw, barN->y, barN->w - aw, ah);
				bars[2].set(barN->x + aw, barN->y, barN->w - aw, barN->h);
				bars[3].set(barN->x, barN->y + ah, aw, barN->h - ah);
				// ищем минимальный по диагонали
				int idx = 0;
				int diag(bars[idx].w * bars[idx].w + bars[idx].h * bars[idx].h);
				for(int ii = 1; ii < 4; ii++)
				{
					int tmp(bars[ii].w * bars[ii].w + bars[ii].h * bars[ii].h);
					if(tmp < diag) { idx = ii; diag = tmp; }
				}
				// добавляем найденный вариант
				idx = (idx > 1 ? 2 : 0);
				nodes.remove(i, 1);
				nodes += bars[idx];
				nodes += bars[idx + 1];
				wmax = ssh_max<int>(wmax, atl->ixywh.right());
				hmax = ssh_max<int>(hmax, atl->ixywh.bottom());
				atl = nullptr;
				break;
			}
		}
		if(atl) return false;
	}
//	rn.set(ssh_pow2<int>(wmax, false), ssh_pow2<int>(hmax, false));
	rn.set(wmax, hmax);
	return true;
}


void save_atlas(ssh_wcs path, const Range<int>& rn)
{
	ssh_d colors[] = {0xf4ff4fff, 0xff00ff00, 0x004fff00, 0x6fff0000, 0x00f700ff, 0x00f4ff5f, 0xf400f5ff, 0x00ff0000};
	buf_cs buf(rn.w * rn.h * 4);
	memset(buf, 0, rn.w * rn.h * 4);
	for(int i = 0; i < atls.size(); i++)
	{
		Atl* atl(atls[i]);
//		asm_ssh_shufb(rn, atl->ixywh, buf, colors[i & 7]);
	}
	File f(path, File::create_write);
	// заголовок
	HEADER_TGA head{ 0, 0, HEADER_TGA::RGB, 0, 0, 0, 0, 0, rn.w, rn.h, 32, HEADER_TGA::UPPER | HEADER_TGA::ALPHA };
	// записываем заголовок
	f.write(&head, sizeof(HEADER_TGA));
	// записываем пиксели
	f.write(buf, buf.size());
	f.close();
}

int _tmain(int argc, _TCHAR* argv[])
{
	Singlton<Log> _lg;
	try
	{
		Log::LOG _log;
		_log._out = Log::TypeOutput::File;
		_lg->init(_log);
		Image* img;
		asm_ssh_shufb();
		new(&img, L"image") Image(Image::TypesMap::TextureMap, FormatsMap::rgba8);
		int w = 512, h = 512;
		int ww = 1536, hh = 2048;
		img->set_empty(Range<int>(w, h), 0);
		Xml xml(L"e:\\mod.xml");
		HXML hroot(xml.node(xml.root(), L"modify"));
		ImgMod(&xml, xml.node(hroot, L"mod1"), img).apply(img->get_map(0));
		ImgMod(&xml, xml.node(hroot, L"mod2"), img).apply(img->get_map(0));
		//asm_ssh_copy(Bar<int>(0, 0, ww, hh), Range<int>(ww, hh), img->get_map(0)->pixels(), img->get_map(1)->pixels(), Bar<int>(0, 0, w, h), Range<int>(w, h), &mod);
		img->save(L"e:\\mod.tga", ImgCnv::Types::tga, FormatsMap::rgba8, 0);
		img->release();
		return 0;
	}
	catch(const Exception& e) { e.add(L"главная процедура!"); }
	return 0;
}

