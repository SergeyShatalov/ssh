
#include "stdafx.h"

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
		Temp* tmp;
		Temp* tmp1;
		Temp* tmp2;
		new(&tmp, L"sergey") Temp();
		new(&tmp1, L"sergey") Temp();
		new(&tmp2, L"sergey1") Temp();
		SSH_REL(tmp);
		SSH_REL(tmp1);
		SSH_REL(tmp2);
		char* buf(new char[10]);
		delete[] buf;
		Buffer<ssh_cs> _csss;
		_csss = Buffer < ssh_cs>(100);
		String timestamp = L"<1896.697170952@dbc.mtview.ca.us>vasinpass";
		timestamp = ssh_md5(timestamp);

		String _serg(L"windows-1251");
		Buffer<ssh_cs> out(ssh_cnv(L"windows-1251", _serg, true));
		regx rx;
		File _x(L"e:\\1.xml", File::open_read);
		String eml(_x.read(L"windows-1251", 0));
		String cmd;
		String charset;
		// xml
		// 1. <tag>
		// 2. </tag>
		// 3. <tag attr1=val1 ... />
		// 3. <tag attr1=val1 ... >
		eml = L"(19+*(29+(30+(40+1)+20)+30)+40)";
		eml = L"<div>текс в див<div>а это вложенный див</div></div><div>а это другой див</div>";
		if(rx.match(eml, LR"serg((?:<(div)>)(?:(?:(?!</?\1>).)|(?R))*(?:</\1>))serg") > 0)
		{
			for(ssh_l i = 1; i < rx.count(); i++)
			{
				eml = rx.substr(i);
			}
		}
//		if(rx.match(eml, LR"((?mUs)<([/\w\d_-]+)(?:\s+([\w\d_-]+)\s*=\s*["]*([\w\d_\s+,]*)["]*\s+)([/]*)>)") > 0)
		//eml = LR"(<elem path="startScreen.fse" name="startScreen" src="startScreen.bmp" type="Fse"/>)";
		eml = LR"(</elem a-"1212jdjd,-0=0@#$%^&&b" b="a"//>)";
		//eml = LR"(</elem>)";
		//eml = LR"(<elem path="startScreen.fse" name="startScreen" src="startScreen.bmp" type="Fse">)";
		if(rx.match(eml, LR"serg(<([/]{0,1})([\w\d_-]+)(?:\s+(.*?[/]*?))\s*([/]{0,1})>)serg") > 0)
		{
			for(ssh_l i = 1; i < rx.count(); i++)
			{
				eml = rx.substr(i);
			}
			eml = rx.substr(3);
			ssh_l idx(0);
			rx.match(eml, LR"serg(([\w\d_-]+)\s*=\s*"(.*?)")serg", idx);
			for(ssh_l i = 1; i < rx.count(); i++)
			{
				eml = rx.substr(i);
			}
		}
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

//		Xml xml(L"c:\\1.xml", Xml::_utf16be);
//		xml.save(L"c:\\o.xml");
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