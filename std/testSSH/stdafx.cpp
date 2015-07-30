
#include "stdafx.h"

struct vector
{
	vector() : var(-100)
	{
		v2.x = 11.11f;
		v2.y = 22.22f;
		v3.x = 111.111f;
		v3.y = 222.222f;
		v3.z = 333.333f;
	}
	ssh_u var;
	vec2 v2;
	vec3 v3;
};
class Temp3 : public Serialize
{
public:
	Temp3() : str("Сергей Викторович"), xx(1), yy(2), zz(3) {}
	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp3, xx, L"temp3", nullptr, 1)
			SCHEME_VAR(Temp3, xx, L"xx", 1, 0, L"1.0", nullptr)
			SCHEME_VAR(Temp3, yy, L"yy", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp3, zz, L"zz", 1, 0, L"1", nullptr)
			SCHEME_VAR(Temp3, str, L"str", 1, 0, L"", nullptr)
			SCHEME_OBJ_BEGIN(Temp3, vv, L"stk_vector", 1, 1)
				SCHEME_OBJ_BEGIN(vector, v2, L"vec2", 1, 2)
					SCHEME_OBJ_VAR1(vector, vv, vec2, v2, x, L"x", 1, 0, L"0.0", nullptr, 2)
					SCHEME_OBJ_VAR1(vector, vv, vec2, v2, y, L"y", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_END()
				SCHEME_OBJ_BEGIN(vector, v3, L"vec3", 1, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, x, L"x", 1, 0, L"0.0", nullptr, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, y, L"y", 1, 0, L"0.0", nullptr, 3)
					SCHEME_OBJ_VAR1(vector, vv, vec3, v3, z, L"z", 1, 0, L"0.0", nullptr, 3)
				SCHEME_OBJ_END()
				SCHEME_OBJ_VAR(vector, vv, var, L"var", 1, 0, L"0.0", nullptr, 1)
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
	Temp2() :x(1), y(2), z(3) {}
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
class Temp : public Resource
{
	SSH_DYNCREATE(Temp);
public:
	Temp() : str(L"Шаталов"), x(100), d(123.123) { v.x = 22.0f; v.y = 15.0f; }
	Temp(ssh_wcs path) : x(0) { open(path); }
	virtual ~Temp() {}
	virtual void save(ssh_wcs path) override
	{
		saveXml(path, L"utf-8", (ssh_b*)this);
	}
	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp, x, L"temp", L"value", 1)
			SCHEME_VAR(Temp, x, L"x", 1, 0, L"1", nullptr)
			SCHEME_OBJ_BEGIN(Temp, v, L"vector", 1, 1)
				SCHEME_OBJ_VAR(vec2, v, x, L"x", 1, 0, L"0.0", nullptr, 1)
				SCHEME_OBJ_VAR(vec2, v, y, L"y", 1, 0, L"0.0", nullptr, 1)
			SCHEME_OBJ_END()
			SCHEME_OBJ_BEGIN(Temp, v3, L"vec3", 1, 2)
				SCHEME_OBJ_VAR(vec3, v3, x, L"x", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_VAR(vec3, v3, y, L"y", 1, 0, L"0.0", nullptr, 2)
				SCHEME_OBJ_VAR(vec3, v3, z, L"z", 1, 0, L"0.0", nullptr, 2)
			SCHEME_OBJ_END()
			SCHEME_VAR(Temp, d, L"temp_d", 1, 0, L"0.0", nullptr)
			SCHEME_VAR(Temp, str, L"string", 1, 0, L"Иванов", nullptr)
			SCHEME_NOD(Temp, tmp, L"tmp", nullptr, 1)
			SCHEME_END(Temp);
	}
protected:
	int x;
	Temp2 tmp;
	vec2 v;
	vec3 v3;
	String str;
	double d;
	// сформировать из памяти
	virtual void make(const Buffer<ssh_cs>& buf) override
	{
		openXml(buf, (ssh_b*)this);
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
//	void* _nm;
//	_nm = __unDNameHelper(NULL, (_This->_M_d_name) + 1, 0, UNDNAME_32_BIT_DECODE | UNDNAME_TYPE_ONLY)) == NULL)
//	String _tp;
//	_tp = typeid(argc).name();
	/*
	String _num(L"10f00");

	short _ii = _num;// .toNum<ssh_l>(_num, String::_dec);
	float _dd = 100.123f;
	void* pp(&_dd);
	*/
	Singlton<Helpers> _hlp;
	Singlton<Log> _lg;
	try
	{
		Log::LOG _log;
		_log._out = Log::TypeOutput::Debug;
		_lg->init(&_log);
		Singlton<Gamepad> _gp;
		SSH_LOG(L"Привет!");
		Temp* t;
		new(&t, L"serg") Temp();
		t->save(L"e:\\serg.xml");
		SSH_REL(t);
//		Xml _xml(L"e:\\1.xml");
//		_xml.save(L"e:\\1+.xml", L"utf-8");
 		return 0;
//		Mail mail_pop(L"imap.yandex.ru:143", L"ostrov-skal", MAIL_PASS, Mail::stTLS);
//		List<Mail::MAIL*> lst;
//		mail_pop.imap(L"X-Priority", &lst, false);
//		return 0;
		Mail mail_smtp(L"smtp.yandex.ru:25", L"ostrov-skal", MAIL_PASS, Mail::stTLS);
		mail_smtp.set_charset(L"koi8-r");
		mail_smtp.add_recipient(L"Шаталов Сергей", L"ostrov_skal@mail.ru");
		mail_smtp.add_recipient(L"Шаталов Сергей", L"ostrov-skal@yandex.ru");
		mail_smtp.set_sender(L"Влад", L"ostrov-skal@yandex.ru");
		mail_smtp.add_attach(L"e:\\1.jpg");
		mail_smtp.add_attach(L"e:\\2.jpg");
		mail_smtp.smtp(L"Новое сообщение", L"Моё третье собственноручно отправленное письмо!!!");
	}
	catch(const Exception& e) { e.add(L"главная процедура!"); }
	return 0;
	Singlton<Archive> _arh;
	String nm;
	if(File::is_exist(L"c:\\1.arh"))
	{
		_arh->open(L"c:\\1.arh", L"sergey");
	}
	else
	{
		_arh->make(L"c:\\1.arh", L"sergey", L"");
		_arh->add(L"c:\\ca.pem", L"ca");
		_arh->add(L"c:\\server.pem", L"server");
	}
	//_arh->rename(L"ca", L"ca+");
	_arh->remove(L"ca");
	nm = _arh->enumerate(true);
	while(!nm.is_empty())
	{
		nm = _arh->enumerate(false);
	}
	Buffer<ssh_b> buf(_arh->get(L"server"));
	//_arh->close();
	_arh->open(L"c:\\1.arh", L"sergey");
	return 0;
}