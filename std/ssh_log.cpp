
#include "stdafx.h"
#include "ssh_log.h"

namespace ssh
{
	bool StackTrace::is_disabled = true;

	Log::LOG::LOG()
	{
		_out = TypeOutput::File;
		string_begin = L"\r\n-------------------- Запуск сеанса [$us - $cm] - $nm ($dt - $tm) --------------------\r\n\r\n";
		string_finish = L"\r\n------------------------------ Завершение сеанса - $nm ($dt - $tm) ------------------------------\r\n\r\n";
		string_continue = L"\r\n------------------------------ Продолжение сеанса - $nm ($dt - $tm) ------------------------------\r\n\r\n";
		// вывод на экран
		screen_template =	L"\r\n-------------------- [$tp] --------------------\r\n"
							L"Функция: $fn\r\n\r\nФайл: $fl\r\n\r\nСтрока: $ln\r\n\r\nПричина: $ms\r\n\r\n\r\nПродолжить выполнение программы?\r\n\r\n"
							L"\r\n-------------------- [$tp] --------------------\r\n";
		// вывод в окно отладчика
		debug_template = L"[$tp] $DT-$tm\t$fn\t - \t($fl: $ln) - [$ms]\r\n";
		// вывод на почту
		email_address = L"ostrov_skal@mail.ru";
		email_host = L"smtp.mail.ru:465";
		email_flags = Mail::stSSL;
		email_template = L"[$tp] $DT-$tm\t$fn\t - \t($fl: $ln) - <$ms>\r\n";
		email_subject = L"Уведомление логгирования";
		email_max_msgs = 50;
		email_blocked = false;
		// вывод в файл
		file_path = hlp->get_system_info(Helpers::siProgFolder) + hlp->get_system_info(Helpers::siNameProg) + L".log";
		file_template = L"[$tp] $DT-$tm\t$fn  -  ($fl: $ln) - <$ms>\r\n";;
		file_flags = File::create_write;
		// вывод на хост
		host_name = L"localhost:11111";
		host_cert = L"e:\\SSL\\cer\\client.cer";
		host_pwd_cert = L"";
		host_template = L"[$tp] $fn - ($fl: $ln) - <$ms>";
		host_flags = 0;
		trace_template = L"$ms\r\n";
	}

	String Log::apply_template(ssh_wcs fn, ssh_wcs fl, int ln, int tp, ssh_wcs msg, String templ) const
	{
		String tmp;
		static ssh_wcs m_types[] = {L"INFO", L"ASSERT", L"EXCEPTION", L"TRACE"};
		static ssh_wcs rpl[] = {L"$DT", L"$fn", L"$ln", L"$fl", L"$ms", L"$tm", L"$dt", L"$us", L"$cm", L"$nm", L"$tp", nullptr};
		tmp.fmt(L"%s\1%s\1%i\1%s\1%s\1%s\1%s\1%s\1%s\1%s\1%s\1\1",
				Time::current().fmt(L"$d.$m.$y"), fn, ln, fl, msg, Time::current().fmt(L"$h:$nm:$s"),
				Time::current().fmt(L"$d $MN)+ $Y ($dW)"), hlp->get_system_info(Helpers::siUserName),
				hlp->get_system_info(Helpers::siCompName), hlp->get_system_info(Helpers::siNameProg), m_types[tp]);
		tmp.replace(L'\1', L'\0');
		return templ.replace(rpl, tmp);

	}

	static void ssh_terminate()
	{
		SSH_LOG(L"Аварийное завершение сеанса!\r\n");
	}

	static void __cdecl ssh_unexpected()
	{
		SSH_LOG(L"Аварийное завершение сеанса!\r\n");
		exit(3);
	}

	static void __cdecl ssh_signal(int numSignal)
	{
//		MemMgr::instance()->fault();
		signal(numSignal, SIG_DFL); // перепосылка сигнала
		exit(3);
	}

	static void socket_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf)
	{
		SetEvent(log->hEventSocket);
	}

	void Log::sendSocket(const String& msg)
	{
		if(!sock.is_closed())
		{
			sock.send(0, msg);
			if(WaitForSingleObject(hEventSocket, 30000) != WAIT_OBJECT_0) sock.close();
			ResetEvent(hEventSocket);
		}
	}

	void Log::init(LOG* lg)
	{
		try
		{
			if(!lg) lg = &_log;
			_log._out = lg->_out;
			_log.string_begin = apply_template(L"", L"", 0, 0, L"", lg->string_begin);
			_log.string_finish = lg->string_finish;
			_log.string_continue = lg->string_continue;
			
			switch(_log._out)
			{
				case TypeOutput::Net:
					_log.host_name = lg->host_name;
					_log.host_flags = lg->host_flags;
					_log.host_cert = lg->host_cert;
					_log.host_pwd_cert = lg->host_pwd_cert;
					if((hEventSocket = CreateEvent(nullptr, true, false, nullptr)))
						sock.setCallbacks(socket_receive, nullptr, nullptr, nullptr, nullptr);
					break;
				case TypeOutput::Screen:
					_log.screen_template = lg->screen_template;
					break;
				case TypeOutput::File:
					_log.file_template = lg->file_template;
					file.open(lg->file_path, lg->file_flags);
					file.write(lg->string_begin, L"utf-8");
					break;
				case TypeOutput::Mail:
					_log.email_template = lg->email_template;
					_log.email_max_msgs = lg->email_max_msgs;
					_log.email_subject = lg->email_subject;
					_log.email_count_msgs = 0;
					_log.email_blocked = false;
					mail.set_host(lg->email_host, lg->email_flags);
					mail.set_login(lg->email_login, lg->email_pass);
					mail.set_charset(L"cp1251");
					mail.add_recipient(L"", lg->email_address);
					mail.set_sender(hlp->get_system_info(Helpers::siNameProg), lg->email_address);
					mail.set_message(L"Система логгирования для отладки программы.");
					_log.file_path = hlp->get_system_info(Helpers::siTempFolder) + hlp->gen_name(L"__MAIL__LOG__");
					file.open(_log.file_path, File::create_read_write);// | File::access_temp_remove);
					file.write(_log.string_begin, 0);
					break;
				case TypeOutput::Debug:
					_log.debug_template = lg->debug_template;
					OutputDebugString(_log.string_begin);
					break;
			}
		}
		catch(const Exception&)
		{
			file.close();
			sock.close();
			_log._out = TypeOutput::Null;
		}
		set_unexpected(ssh_unexpected);
		set_terminate(ssh_terminate);
		signal(SIGSEGV, ssh_signal);

		tracer.start();
		MemMgr::instance()->start();
	}

	void Log::close()
	{
		tracer.stop();
		tracer.output();
		MemMgr::instance()->stop();
		MemMgr::instance()->output();
		_log.string_finish = apply_template(L"", L"", 0, 0, L"", _log.string_finish);
		switch(_log._out)
		{
			case TypeOutput::Net:
				sendSocket(_log.string_finish);
				if(hEventSocket)
				{
					CloseHandle(hEventSocket);
					hEventSocket = nullptr;
				}
				break;
			case TypeOutput::Screen:
				break;
			case TypeOutput::File:
				if(!file.is_close()) file.write(_log.string_finish, L"utf-8");
				break;
			case TypeOutput::Mail:
				if(!file.is_close()) file.write(_log.string_finish, 0);
				send_email(L"");
				break;
			case TypeOutput::Debug:
				OutputDebugString(_log.string_finish);
				break;
		}
	}

	void Log::add(const String& msg)
	{
		String _msg(apply_template(L"", L"", 0, 0, msg, _log.trace_template));
		switch(_log._out)
		{
			case TypeOutput::Net:
				if(sock.is_closed())
				{
					sock.init(_log.host_name, 0, _log.host_flags, _log.host_cert, _log.host_pwd_cert);
					sendSocket(_log.string_begin);
				}
				sendSocket(_msg);
				break;
			case TypeOutput::File:
				if(!file.is_close()) file.write(_msg, L"utf-8");
				break;
			case TypeOutput::Mail:
				if(!file.is_close()) file.write(_msg, 0);
				_log.email_count_msgs++;
				if(_log.email_count_msgs >= _log.email_max_msgs && !is_email_blocked())
					send_email(apply_template(L"StackTrace", L"ssh_log.cpp", 161, 3, msg, _log.string_continue));
				break;
			case TypeOutput::Debug:
				OutputDebugString(_msg);
				break;
		}
	}

	void Log::add(TypeMessage type, ssh_wcs fn, ssh_wcs fl, int ln, ssh_wcs msg, ...)
	{
		if(!tracer.is_disabled)
		{
			tracer.stop();
			String msgArgs;
			// формируем сообщение
			va_list	arglist;
			va_start(arglist, msg);
			msgArgs.fmt(msg, arglist);
			va_end(arglist);
			msgArgs.replace(L'\r', L'.');
			msgArgs.replace(L'\n', L'.');
			// формируем сообщение на основании шаблона
			switch(_log._out)
			{
				case TypeOutput::Net:
					if(sock.is_closed())
					{
						sock.init(_log.host_name, 0, _log.host_flags, _log.host_cert, _log.host_pwd_cert);
						sendSocket(_log.string_begin);
					}
					sendSocket(apply_template(fn, fl, ln, type, msgArgs, _log.file_template));
					break;
				case TypeOutput::Screen:
					if(MessageBox(nullptr, apply_template(fn, fl, ln, type, msgArgs, _log.screen_template), hlp->get_system_info(Helpers::siNameProg), MB_ICONERROR | MB_YESNO) == IDNO) { exit(4); }
					break;
				case TypeOutput::File:
					if(!file.is_close()) file.write(apply_template(fn, fl, ln, type, msgArgs, _log.file_template), L"utf-8");
					break;
				case TypeOutput::Mail:
					if(!file.is_close()) file.write(apply_template(fn, fl, ln, type, msgArgs, _log.email_template), 0);
					_log.email_count_msgs++;
					if(_log.email_count_msgs >= _log.email_max_msgs && !is_email_blocked())
						send_email(apply_template(fn, fl, ln, type, msgArgs, _log.string_continue));
					break;
				case TypeOutput::Debug:
					OutputDebugString(apply_template(fn, fl, ln, type, msgArgs, _log.debug_template));
					break;
			}
			tracer.start();
		}
	}

	void Log::send_email(const String& ln)
	{
		if(!file.is_close())
		{
			_log.email_blocked = true;
			// читаем из файла
			file.set_pos(0, File::begin);
			String str(file.read(cp_utf, 0));
			file.close();
			// обнуляем файл
			file.open(_log.file_path, File::create_read_write);
			file.write(ln, 0);
			_log.email_count_msgs = 0;
			// отправка на почту
			mail.smtp(_log.email_subject, str);
			_log.email_blocked = false;
		}
	}

	void StackTrace::add(bool is, ssh_wcs func, ssh_wcs file, int line)
	{
		is_disabled = true;
		String tmp;
		if(!is) indent--;
		if(cdepth >= depth) remove_node();
		cdepth++;
		if(indent < 0) indent = 0;
		String _indent(L' ', indent * 2);
		add_node(tmp.fmt(L"%s%c%s()  -  (%s:%i)", _indent, (is ? L'+' : L'-'), func, file, line));
		if(is) indent++;
		is_disabled = false;
	}

	void StackTrace::output()
	{
#ifdef _DEBUG
		String tmp;
		log->add(tmp.fmt(L"\r\n\r\n-------------------------------------------------- Трассировка стека (%i вызовов) --------------------------------------------------\r\n\r\n", cdepth));
		auto n(root);
		while(n) { log->add(n->value); n = n->next; }
		log->add(L"\r\n\r\n--------------------------------------------------------- Трассировка стека -------------------------------------------------------\r\n\r\n");
		clear();
#endif
	}

	Exception::Exception(ssh_wcs fn, ssh_wcs fl, int ln, ssh_wcs msg, ...)
	{
		func = fn;
		file = fl;
		line = ln;

		va_list argList;
		va_start(argList, msg);
		message.fmt(msg, argList);
		va_end(argList);
	}

	void Exception::add(ssh_wcs msg, ...) const
	{
		String msgArgs;
		// формируем сообщение
		va_list	arglist;
		va_start(arglist, msg);
		msgArgs.fmt(msg, arglist);
		va_end(arglist);
		if(!msgArgs.is_empty()) msgArgs += L", ";
		log->add(Log::mException, func, file, line, msgArgs + message);
	}
}
