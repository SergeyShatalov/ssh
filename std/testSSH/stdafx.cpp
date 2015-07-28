
#include "stdafx.h"

class Tmp
{
public:
	Tmp() :y(0) {}
	int y;
};
class Temp : public Base
{
	SSH_DYNCREATE(Temp);
public:
	Temp() : x(0) {}
	virtual ~Temp() {}
	int x;
	virtual void save(const String& path, bool is_xml)  {}
protected:
	// начальная инициализация
	virtual void init() {}
	// сброс
	virtual void reset() {}
	// сформировать из памяти
	virtual void make(const Buffer<ssh_b>& buf) {}
};
int _tmain(int argc, _TCHAR* argv[])
{
	Singlton<Helpers> _hlp;
	Singlton<Log> _lg;
	try
	{
		Log::LOG _log;
		_log._out = Log::TypeOutput::Debug;
		_lg->init(&_log);
		Singlton<Gamepad> _gp;
		SSH_LOG(L"Привет!");
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