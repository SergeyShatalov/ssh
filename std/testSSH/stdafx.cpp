
#include "stdafx.h"

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

ENUM_DATA _stk[]=
{
	{L"ХУЙ", 1},
	{L"ПИЗДА", 2},
	{L"МАНДА", 4},
	{L"ЕБЛО", 8},
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
	ssh_u asm_ssh_shufb(ssh_u v);
}

vec3 m_points[16];
float m_weights[16];

void ColourSet(ssh_b const* rgba)
{
	for(int i = 0; i < 16; ++i)
	{
		// normalise coordinates to [0,1]
		float w = (float)rgba[4 * i + 3] / 255.0f;
		float x = (float)rgba[4 * i + 2] / 255.0f;
		float y = (float)rgba[4 * i + 1] / 255.0f;
		float z = (float)rgba[4 * i + 0] / 255.0f;
		// ensure there is always non-zero weight even for zero alpha
		// add the point
		m_points[i] = vec3(x, y, z);
		m_weights[i] = w;
	}
}


struct BC1
{
	ssh_w rgb[2];
	ssh_d bitmap;
};

struct BC2
{
	ssh_d bitmap[2];
	BC1 bc1;
};

struct BC3
{
	ssh_b alpha[2];
	ssh_b bitmap[6];
	BC1 bc1;
};

static int Unpack565(ssh_b const* packed, ssh_b* colour)
{
	// build the packed value
	int value = (int)packed[0] | ((int)packed[1] << 8);
	// get the components in the stored range
	ssh_b red = (ssh_b)((value >> 11) & 0x1f);
	ssh_b green = (ssh_b)((value >> 5) & 0x3f);
	ssh_b blue = (ssh_b)(value & 0x1f);

	// scale up to 8 bits
	colour[0] = (red << 3) | (red >> 2);
	colour[1] = (green << 2) | (green >> 4);
	colour[2] = (blue << 3) | (blue >> 2);
	colour[3] = 255;

	// return the value
	return value;
}

void DecompressColour(ssh_b* rgba, void const* block, bool isDxt1)
{
	// get the block bytes
	ssh_b const* bytes = reinterpret_cast< ssh_b const* >(block);

	// unpack the endpoints
	ssh_b codes[16];
	int a = Unpack565(bytes, codes);
	int b = Unpack565(bytes + 2, codes + 4);

	// generate the 2 midpoints
	for(int i = 0; i < 3; ++i)
	{
		int c = codes[i]; // r1 g1 b1
		int d = codes[4 + i]; // r2 g2 b2

		if(isDxt1 && a <= b)
		{
			codes[8 + i] = (ssh_b)((c + d) / 2);
			codes[12 + i] = 0;
		}
		else
		{
			codes[8 + i] = (ssh_b)((2 * c + d) / 3);
			codes[12 + i] = (ssh_b)((c + 2 * d) / 3);
		}
	}

	// fill in alpha for the intermediate values
	codes[11] = 255;
	codes[15] = (isDxt1 && a <= b) ? 0 : 255;

	// unpack the indices
	ssh_b indices[16];
	for(int i = 0; i < 4; ++i)
	{
		ssh_b* ind = indices + 4 * i;
		ssh_b packed = bytes[4 + i];

		ind[0] = packed & 0x3;
		ind[1] = (packed >> 2) & 0x3;
		ind[2] = (packed >> 4) & 0x3;
		ind[3] = (packed >> 6) & 0x3;
	}

	// store out the colours
	for(int i = 0; i < 16; ++i)
	{
		ssh_b offset = 4 * indices[i];
		for(int j = 0; j < 4; ++j)
			rgba[4 * i + j] = codes[offset + j];
	}
}

static void DecodeBC1(ssh_b *pColor, _In_ const BC1 *pBC, _In_ bool isbc1)
{
	static float s_Scale[] = { 1.f / 31.f, 1.f / 63.f, 1.f / 31.f, 1.f };
	// XMVECTOR clr0 = XMLoadU565(reinterpret_cast<const XMU565*>(&pBC->rgb[0]));
	//XMVECTOR clr1 = XMLoadU565(reinterpret_cast<const XMU565*>(&pBC->rgb[1]));
	//clr0 = XMVectorMultiply(clr0, s_Scale);
	//clr1 = XMVectorMultiply(clr1, s_Scale);
	//clr0 = XMVectorSwizzle<2, 1, 0, 3>(clr0);
	//clr1 = XMVectorSwizzle<2, 1, 0, 3>(clr1);
	//clr0 = XMVectorSelect(g_XMIdentityR3, clr0, g_XMSelect1110);
	//clr1 = XMVectorSelect(g_XMIdentityR3, clr1, g_XMSelect1110);
	//XMVECTOR clr2, clr3;
	if(isbc1 && (pBC->rgb[0] <= pBC->rgb[1]))
	{
		//clr2 = XMVectorLerp(clr0, clr1, 0.5f);
		//clr3 = XMVectorZero();  // Alpha of 0
	}
	else
	{
		//clr2 = XMVectorLerp(clr0, clr1, 1.f / 3.f);
		//clr3 = XMVectorLerp(clr0, clr1, 2.f / 3.f);
	}
	ssh_d dw = pBC->bitmap;
	for(size_t i = 0; i < 16; ++i, dw >>= 2)
	{
		switch(dw & 3)
		{
			//case 0: pColor[i] = clr0; break;
			//case 1: pColor[i] = clr1; break;
			//case 2: pColor[i] = clr2; break;

			case 3: dw = 0; break;
			//default: pColor[i] = clr3; break;
		}
	}
}

void D3DXDecodeBC2(ssh_b *pColor, const BC2 *pBC2)
{
	DecodeBC1(pColor, &pBC2->bc1, false);
	DWORD dw = pBC2->bitmap[0];
	//for(size_t i = 0; i < 8; ++i, dw >>= 4) pColor[i] = XMVectorSetW(pColor[i], (float)(dw & 0xf) * (1.0f / 15.0f));
	dw = pBC2->bitmap[1];
	//for(size_t i = 8; i < 16; ++i, dw >>= 4) pColor[i] = XMVectorSetW(pColor[i], (float)(dw & 0xf) * (1.0f / 15.0f));
}

void D3DXDecodeBC3(ssh_b *pColor, const BC3 *pBC3)
{
	DecodeBC1(pColor, &pBC3->bc1, false);
	float fAlpha[8];
	fAlpha[0] = ((float)pBC3->alpha[0]) * (1.0f / 255.0f);
	fAlpha[1] = ((float)pBC3->alpha[1]) * (1.0f / 255.0f);
	if(pBC3->alpha[0] > pBC3->alpha[1])
	{
		for(size_t i = 1; i < 7; ++i) fAlpha[i + 1] = (fAlpha[0] * (7 - i) + fAlpha[1] * i) * (1.0f / 7.0f);
	}
	else
	{
		for(size_t i = 1; i < 5; ++i) fAlpha[i + 1] = (fAlpha[0] * (5 - i) + fAlpha[1] * i) * (1.0f / 5.0f);
		fAlpha[6] = 0.0f; fAlpha[7] = 1.0f;
	}
	DWORD dw = pBC3->bitmap[0] | (pBC3->bitmap[1] << 8) | (pBC3->bitmap[2] << 16);
//	for(size_t i = 0; i < 8; ++i, dw >>= 3) pColor[i] = XMVectorSetW(pColor[i], fAlpha[dw & 0x7]);
	dw = pBC3->bitmap[3] | (pBC3->bitmap[4] << 8) | (pBC3->bitmap[5] << 16);
//	for(size_t i = 8; i < 16; ++i, dw >>= 3) pColor[i] = XMVectorSetW(pColor[i], fAlpha[dw & 0x7]);
}

int _tmain(int argc, _TCHAR* argv[])
{
	BC3 _bc3_2;
	ssh_b rgba[] = { 10, 15, 20, 16, 25, 30, 35, 32, 40, 45, 50, 48, 55, 60, 65, 64,
					70, 75, 80, 80, 85, 90, 95, 96, 100, 105, 110, 112, 115, 120, 125, 128,
					130, 135, 140, 144, 0, 0, 0, 160, 160, 165, 170, 176, 175, 180, 185, 192,
					190, 195, 200, 208, 205, 210, 215, 224, 220, 225, 230, 240, 235, 240, 245, 255 };
	float f_1 = 1.0f;
	ColourSet(rgba);
	Singlton<Log> _lg;
	try
	{
		ssh_u resss = asm_ssh_shufb(0x0102030405060708);
		Log::LOG _log;
		_log._out = Log::TypeOutput::Debug;
		_lg->init(&_log);
		String trans = ssh_translate(L"Шаталов Сергей Викторович - дата рождения - 06.06.1979, место рожденья - СССР, СОАССР, г.Орджоникидзе, ул. Владикавказская, 32/2 кв.79", true);
		String trans1 = ssh_translate(trans, false);
		Image* img;
		new(&img, L"image") Image(Image::TypesMap::TextureMap, FormatsMap::rgba8);
		img->set_map(L"e:\\11.gif", 0, 0);
		img->save(L"e:\\gifi", ImgCnv::Types::tga, FormatsMap::rgba8, 0);
		img->release();
		return 0;
		img->save(L"e:\\rgb5a1", ImgCnv::Types::bmp, FormatsMap::rgb5a1, 0);
		img->save(L"e:\\rgb8", ImgCnv::Types::bmp, FormatsMap::rgb8, 0);
		img->save(L"e:\\rgba8", ImgCnv::Types::bmp, FormatsMap::rgba8, 0);
		img->save(L"e:\\rgba4", ImgCnv::Types::bmp, FormatsMap::rgba4, 0);
		img->save(L"e:\\bgra8", ImgCnv::Types::bmp, FormatsMap::bgra8, 0);
		img->save(L"e:\\r5g6b5", ImgCnv::Types::bmp, FormatsMap::r5g6b5, 0);
		img->save(L"e:\\bgr8", ImgCnv::Types::bmp, FormatsMap::bgr8, 0);
		img->save(L"e:\\a8", ImgCnv::Types::bmp, FormatsMap::a8, 0);
		return 0;
		ssh_u is_c = ssh_system_info(SystemInfo::siCpuCaps, CpuCaps::SUPPORTS_BMI1);
		ssh_u h1 = ssh_hash(L"bmp");
		ssh_u h2 = ssh_hash(L"bfs");
		ssh_u h3 = ssh_hash(L"fse");
		ssh_u h4 = ssh_hash(L"jpg");
		ssh_u h5 = ssh_hash(L"tga");
		ssh_u h6 = ssh_hash(L"dds");
		ssh_u h7 = ssh_hash(L"gif");
		Map<String, String, SSH_TYPE, SSH_TYPE>* m;
		Array<Map<String, String, SSH_TYPE, SSH_TYPE>, SSH_TYPE> arr(550, 10, 20);
		MySql _sql(L"localhost", L"root", L"", L"sergey");
		_sql.del_table(L"shatalov");
		MySql::FIELD flds[] =
		{
			{L"ID", MySql::TypesField::INT, MySql::NOT_NULL | MySql::AUTO_INCREMENT | MySql::PRIMARY | MySql::KEY, 0, nullptr, L"идентификатор"},
			{ L"бит", MySql::TypesField::BIT, MySql::_NULL, 0, nullptr, L"тип данных - BIT" },
			{ L"_enum", MySql::TypesField::ENUM, MySql::NOT_NULL, 0, L"serg", L"тип данных - ENUM", 0, 0, L"max,vlad,serg,olga" },
			{ L"_varchar", MySql::TypesField::VARCHAR, MySql::NOT_NULL | MySql::KEY, 255, L"Шаталов", L"тип данных - VARCHAR", 0, 10 },
			{ L"_timestamp", MySql::TypesField::TIMESTAMP, MySql::_NULL, 1, nullptr, L"тип данных - TIMESTAMP" },
			{ L"_double", MySql::TypesField::DOUBLE, MySql::UNSIGNED, 10, nullptr, L"тип данных - DOUBLE", 5 },
			{ L"_float", MySql::TypesField::FLOAT, MySql::UNSIGNED, 10, L"1.0", L"тип данных - FLOAT", 6 },
			{ L"_decimal", MySql::TypesField::DECIMAL, MySql::UNSIGNED, 20, L"100.0", L"тип данных - DECIMAL", 10 },
			{ L"_char", MySql::TypesField::CHAR, MySql::_NULL, 10, L"c", L"тип данных - CHAR" },
			{ L"_tinyint", MySql::TypesField::TINYINT, MySql::NOT_NULL, 0, L"10", L"тип данных - TINYINT" },
			{ L"_smallint", MySql::TypesField::SMALLINT, MySql::NOT_NULL, 0, L"20", L"тип данных - SMALLINT" },
			{ L"_mediumint", MySql::TypesField::MEDIUMINT, MySql::NOT_NULL, 0, L"30", L"тип данных - MEDIUMINT" },
			{ L"_bigint", MySql::TypesField::BIGINT, MySql::NOT_NULL, 0, L"40", L"тип данных - BIGINT" },
			{ L"_int", MySql::TypesField::INT, MySql::NOT_NULL, 0, L"50", L"тип данных - INT" },
			{ L"_year", MySql::TypesField::YEAR, MySql::_NULL, 2, nullptr, L"тип данных - YEAR" },
			{ L"_set", MySql::TypesField::SET, MySql::NOT_NULL, 0, L"max", L"тип данных - SET", 0, 0, L"max,vlad,serg,olga" },
			{ L"_tinyblob", MySql::TypesField::TINYBLOB, MySql::_NULL, 0, nullptr, L"тип данных - TINYBLOB" },
			{ L"_mediumblob", MySql::TypesField::MEDIUMBLOB, MySql::_NULL, 0, nullptr, L"тип данных - MEDIUMBLOB" },
			{ L"_longblob", MySql::TypesField::LONGBLOB, MySql::_NULL, 0, nullptr, L"тип данных - LONGBLOB" },
			{ L"_blob", MySql::TypesField::BLOB, MySql::_NULL, 0, nullptr, L"тип данных - BLOB" },
			{ L"_tinytext", MySql::TypesField::TINYTEXT, MySql::_NULL, 0, nullptr, L"тип данных - TINYTEXT" },
			{ L"_mediumtext", MySql::TypesField::MEDIUMTEXT, MySql::_NULL, 0, nullptr, L"тип данных - MEDIUMTEXT" },
			{ L"_longtext", MySql::TypesField::LONGTEXT, MySql::_NULL, 0, nullptr, L"тип данных - LONGTEXT" },
			{ L"_text", MySql::TypesField::TEXT, MySql::_NULL, 0, nullptr, L"тип данных - TEXT" },
			{ L"_date", MySql::TypesField::DATE, MySql::_NULL, 0, nullptr, L"тип данных - DATE" },
			{ L"_time", MySql::TypesField::TIME, MySql::_NULL, 0, nullptr, L"тип данных - TIME" },
			{ L"_datetime", MySql::TypesField::DATETIME, MySql::_NULL, 0, nullptr, L"тип данных - DATETIME" },
			{ nullptr },
		};
		Time tt(Time::current());
		int y = tt.year();
		Time t(1970, 1, 1, 4, 0, 0);
		y = t.year();
		//		_sql.query(L"CREATE TABLE `shatalov` (`id` int UNSIGNED NOT NULL AUTO_INCREMENT, PRIMARY KEY `idx_id`(`id`), `str` varchar(255) NOT NULL UNIQUE DEFAULT 'serg', FULLTEXT KEY `idx_str`(`str`(10)))");
		_sql.add_table(flds, L"shatalov", L"Таблица для проверки всех типов данных mysql!");
		_sql.insert(L"shatalov", L"_varchar,_timestamp,_enum,_double,_float,_decimal,_char,_tinyint,_smallint,_mediumint,_bigint,_int,_year,_set,_tinyblob,_mediumblob,_longblob,_blob,_tinytext,_mediumtext,_longtext,_text,бит,_date,_time,_datetime",
					L"Сергей,2015-10-10 11:12:13,max,25.10,11.234,100.25678,ccc,1,2,3,4,5,16,vlad,aaa,bbb,ccc,ddd,qqq,www,eee,rrr,0,2010-08-08,01:02:03,2008-05-05 03:04:05");
		arr = _sql.select(L"*", L"shatalov", nullptr);
		for(ssh_u i = 0; i < arr.size(); i++)
		{
			m = &arr[i];
			//Time t = Time::parse(L"$y-$m-$d", (*m)[L"_date"]);
		}
		return 0;
		ssh_u res = _sql.remove(L"shatalov");
		String tmp;
		ssh_wcs _tmp[] = { L"Сергей", L"Максим", L"Владислав", L"Ольга", L"Мирослав", L"Виктор", L"Владимир", L"Алан", L"Светлана", L"Георгий", L"Иван", L"Василий"};
		for(ssh_u i = 0; i < 10; i++)
		{
			res = _sql.insert(L"shatalov", L"f1,f2,f3, f4", tmp.fmt(L"%i,%i,%s,%i", i, i*10, _tmp[i], i*20));
		}
		_sql.del_table(L"new_table");
	}
	catch(const Exception& e)
	{
		e.add(L"");
	}
	return 0;
	try
	{
		Log::LOG _log;
		_log._out = Log::TypeOutput::File;
		_lg->init(&_log);
		Singlton<Gamepad> _gp;
		SSH_LOG(L"Привет!!!!");
		Log::LOG _log1;
		_log1._out = Log::TypeOutput::Mail;
		_log1.email_login = L"ostrov_skal";
		_log1.email_pass = MAIL_PASS;
		_lg->init(&_log1);
		SSH_LOG(L"Привет!");
		Mail mail_smtp(L"smtp.yandex.ru:25", L"ostrov-skal", MAIL_PASS, Mail::stTLS);
		mail_smtp.set_charset(L"koi8-r");
		mail_smtp.add_recipient(L"Шаталов Сергей", L"ostrov_skal@mail.ru");
		mail_smtp.add_recipient(L"Шаталов Сергей", L"ostrov-skal@yandex.ru");
		mail_smtp.set_sender(L"Влад", L"ostrov-skal@yandex.ru");
//		mail_smtp.add_attach(L"e:\\1.jpg");
		//mail_smtp.add_attach(L"e:\\2.jpg");
		mail_smtp.smtp(L"Привет!", L"новое письмецо!!!");
		return 0;
	}
	catch(const Exception& e) { e.add(L"главная процедура!"); }
	return 0;
}

