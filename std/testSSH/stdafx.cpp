
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
	ssh_u asm_ssh_shufb(const Range<int>& rn_dst, const Bar<int>& rn_src, void* buf, ssh_d color);
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
		asm_ssh_shufb(rn, atl->ixywh, buf, colors[i & 7]);
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

inline namespace _1
{
	typedef int _tp;
}

inline namespace _2
{
	typedef long _tp;
}

int _tmain(int argc, _TCHAR* argv[])
{
	Xml xml;
	HXML h(xml.add_node(xml.root(), L"fff", L""));
	int ii(1);
	xml.set_attr(h, L"11", ii);
	/*
	for(int i = 0; i < 108; i++)
	{
		atls += new Atl(ssh_rand(5, 205), ssh_rand(5, 205));
	}
	Range<int> rn(2048, 2048);
	packed_atlas(rn);
	save_atlas(L"e:\\atlas1.tga", rn);
//	Range<int> rn1(2048, 2048);
//	packed_atlas_old(rn1);
//	save_atlas(L"e:\\atlas2.tga", rn1);
	return 0;
	*/
	ENUM_DATA* _s = _stk;
	Singlton<Log> _lg;
	try
	{
		//ssh_u resss = asm_ssh_shufb(0x0102030405060708);
		Log::LOG _log;
		_log._out = Log::TypeOutput::Debug;
		_lg->init(&_log);
		Image* img;
		new(&img, L"image") Image(Image::TypesMap::TextureMap, FormatsMap::rgba8);
		img->set_font(L"font", L"Arial", nullptr, -16, 0);
		return 0;
		img->save(L"e:\\bc1", ImgCnv::Types::dds, FormatsMap::bc1, 0);
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

