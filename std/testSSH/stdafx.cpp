
#include "stdafx.h"

size_t _Hash(ssh_wcs _First, size_t _Count)
{
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;
	size_t _Val = _FNV_offset_basis;
	for(size_t _Next = 0; _Next < _Count; ++_Next)
	{	// fold in another byte
		_Val ^= (size_t)_First[_Next];
		_Val *= _FNV_prime;
	}
	_Val ^= _Val >> 32;
	return (_Val);
}

class Temp : public Resource
{
	SSH_DYNCREATE(Temp);
public:
	Temp() : x(100), d(123.123) { v.x = 22.0f; v.y = 15.0f; }
	Temp(ssh_wcs path) : x(0) { open(path); }
	virtual ~Temp() {}
	virtual void save(ssh_wcs path) override
	{
		saveXml(path, L"utf-8", (ssh_b*)this);
	}
	virtual SCHEME* get_scheme() const override
	{
		SCHEME_BEGIN(Temp)
			SCHEME_NOD(Temp, x, L"sergey")
			SCHEME_VAR(Temp, x, L"sergey_x", 0, 1, L"1", nullptr, 0)
			SCHEME_OBJ(Temp, v, L"vector", 1)
				SCHEME_OBJ_VAR(vec2, v, x, L"vec_x", SC_OBJ, 1, L"0.0", nullptr, 1)
				SCHEME_OBJ_VAR(vec2, v, y, L"vec_y", SC_OBJ, 1, L"0.0", nullptr, 1)
			SCHEME_VAR(Temp, d, L"sergey_d", 0, 1, L"0.0", nullptr, 0)
			SCHEME_END(Temp);
	}
protected:
	int x;
	vec2 v;
	double d;
	// сформировать из памяти
	virtual void make(const Buffer<ssh_cs>& buf) override
	{
		openXml(buf, (ssh_b*)this);
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	String _num(L"1000");

	double _ii = _num.toNum<double>(_num, String::_dbl);
	float _dd = 100.123f;
	void* pp(&_dd);
	//	offsetof(Temp, x);
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
		Xml _xml(L"e:\\1.xml");
		_xml.save(L"e:\\1+.xml", L"utf-8");
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