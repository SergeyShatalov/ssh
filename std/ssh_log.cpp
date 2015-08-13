
#include "stdafx.h"
#include "ssh_log.h"
#include <new.h>
#include <stdlib.h>

namespace ssh
{
	bool StackTrace::is_disabled = true;

	static void __cdecl ssh_signal_handler(int numSignal)
	{
		SSH_FAULT(numSignal, (EXCEPTION_POINTERS*)_pxcptinfoptrs);
		exit(1);
	}

	static LONG WINAPI Win32UnhandledExceptionFilter(EXCEPTION_POINTERS* except)
	{
		SSH_FAULT(UNHANDLED_EXCEPTION, except);
		exit(-1);
	}

	static void __cdecl ssh_terminate_handler()
	{
		SSH_FAULT(TERMINATE_CALL, nullptr);
		exit(2);
	}

	static void __cdecl ssh_unexp_handler()
	{
		SSH_FAULT(UNEXPECTED_CALL, nullptr);
		exit(3);
	}

	static void __cdecl ssh_purecall_handler()
	{
		SSH_FAULT(PURE_CALL, nullptr);
		exit(4);
	}

	static void __cdecl ssh_security_handler(int code, void *x)
	{
		SSH_FAULT(SECURITY_ERROR, nullptr);
		exit(5);
	}

	static void __cdecl ssh_invalid_parameter_handler(ssh_wcs expression, ssh_wcs function, ssh_wcs file, unsigned int line, uintptr_t pReserved)
	{
		MemMgr::instance()->fault(INVALID_PARAMETER_ERROR, function, file, line, nullptr, expression);
		exit(6);
	}

	static int __cdecl ssh_new_handler(ssh_u size)
	{
		SSH_FAULT(NEW_OPERATOR_ERROR, nullptr);
		exit(6);
	}

	static void ssh_set_exceptionHandlers()
	{
		// установить фильтр исключений
		SetUnhandledExceptionFilter(Win32UnhandledExceptionFilter);
		// установить режимы отчета библиотеки времени выполнения
		//_CrtSetReportMode(_CRT_ERROR, 0);
		_CrtSetReportMode(_CRT_ASSERT, 0);

		set_terminate(ssh_terminate_handler);
		set_unexpected(ssh_unexp_handler);
		_set_purecall_handler(ssh_purecall_handler);
		_set_invalid_parameter_handler(ssh_invalid_parameter_handler);
		_set_new_handler(ssh_new_handler);
//		_set_security_error_handler(ssh_security_handler);
		signal(SIGABRT, ssh_signal_handler);
		signal(SIGINT, ssh_signal_handler);
		signal(SIGTERM, ssh_signal_handler);
		signal(SIGFPE, ssh_signal_handler);
		signal(SIGILL, ssh_signal_handler);
		signal(SIGSEGV, ssh_signal_handler);
	}

	Log::LOG::LOG()
	{
		_out = TypeOutput::File;
		string_begin = L"\r\n-------------------- Запуск сеанса [$us - $cm] - $nm ($dt - $tm) --------------------\r\n\r\n";
		string_finish = L"\r\n------------------------------ Завершение сеанса - $nm ($dt - $tm) ------------------------------\r\n\r\n";
		string_continue = L"\r\n------------------------------ Продолжение сеанса - $nm ($dt - $tm) ------------------------------\r\n\r\n";
		// вывод на экран
		screen_template = L"\r\n-------------------- [$tp] --------------------\r\n"
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
		email_max_msgs = 200;
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

	String Log::apply_template(ssh_wcs fn, ssh_wcs fl, int ln, int tp, ssh_wcs msg, ssh_wcs tpl) const
	{
		String tmp;
		String templ(tpl);
		static ssh_wcs m_types[] = {L"INFO", L"ASSERT", L"EXCEPTION", L"TRACE"};
		static ssh_wcs rpl[] = {L"$DT", L"$fn", L"$ln", L"$fl", L"$ms", L"$tm", L"$dt", L"$us", L"$cm", L"$nm", L"$tp", nullptr};
		tmp.fmt(L"%s\1%s\1%i\1%s\1%s\1%s\1%s\1%s\1%s\1%s\1%s\1\1",
				Time::current().fmt(L"$d.$m.$y"), fn, ln, fl, msg, Time::current().fmt(L"$h:$nm:$s"),
				Time::current().fmt(L"$d $MN)+ $Y ($dW)"), hlp->get_system_info(Helpers::siUserName),
				hlp->get_system_info(Helpers::siCompName), hlp->get_system_info(Helpers::siNameProg), m_types[tp]);
		tmp.replace(L'\1', L'\0');
		return templ.replace(rpl, tmp);
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
			shutdown();

			if(!lg) lg = &_log;
			_log._out = lg->_out;
			_log.string_begin = apply_template(L"", L"", 0, 0, L"", lg->string_begin);
			_log.string_continue = apply_template(L"", L"", 0, 0, L"", lg->string_continue);
			_log.string_finish = lg->string_finish;
			
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
					file.write(_log.string_begin, L"utf-8");
					break;
				case TypeOutput::Mail:
					_log.email_template = lg->email_template;
					_log.email_max_msgs = lg->email_max_msgs;
					_log.email_subject = lg->email_subject;
					_log.email_host = lg->email_host;
					_log.email_flags = lg->email_flags;
					_log.email_login = lg->email_login;
					_log.email_pass = lg->email_pass;
					_log.email_address = lg->email_address;
					_log.email_count_msgs = 0;
					_log.email_blocked = false;
					_log.file_path = hlp->get_system_info(Helpers::siTempFolder) + hlp->gen_name(L"__MAIL__LOG__");
					file.open(_log.file_path, File::create_read_write);// | File::access_temp_remove);
					file.write(_log.string_begin, cp_utf);
					break;
				case TypeOutput::Debug:
					_log.debug_template = lg->debug_template;
					OutputDebugString(_log.string_begin);
					break;
			}
		}
		catch(const Exception&)
		{
			_log._out = TypeOutput::Null;
			shutdown();
		}
		// установть обработчики нестандартных исключительных ситуаций
		ssh_set_exceptionHandlers();
		// старт трассировщика
		tracer.start();
		// старт менеджера памяти
		MemMgr::instance()->start();
	}

	void Log::shutdown()
	{
		tracer.stop();
		MemMgr::instance()->stop();
		_log.string_finish = apply_template(L"", L"", 0, 0, L"", _log.string_finish);
		switch(_log._out)
		{
			case TypeOutput::Net:
				sendSocket(_log.string_finish);
				break;
			case TypeOutput::Screen:
				break;
			case TypeOutput::File:
				if(!file.is_close()) file.write(_log.string_finish, L"utf-8");
				break;
			case TypeOutput::Mail:
				if(!file.is_close()) file.write(_log.string_finish, cp_utf);
				send_email(L"");
				break;
			case TypeOutput::Debug:
				OutputDebugStringW(_log.string_finish);
				break;
		}
		_log._out = TypeOutput::Null;
		if(hEventSocket)
		{
			CloseHandle(hEventSocket);
			hEventSocket = nullptr;
		}
		file.close();
		sock.close();
	}

	void Log::close()
	{
		tracer.stop();
		tracer.output();
		MemMgr::instance()->stop();
		MemMgr::instance()->output();
		shutdown();
	}

	void Log::add(ssh_wcs msg)
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
				if(!_log.email_blocked)
				{
					if(!file.is_close()) file.write(_msg, cp_utf);
					_log.email_count_msgs++;
					if(_log.email_count_msgs >= _log.email_max_msgs)
						send_email(_log.string_continue);
				}
				break;
			case TypeOutput::Debug:
				OutputDebugStringW(_msg);
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
					if(!_log.email_blocked)
					{
						if(!file.is_close()) file.write(apply_template(fn, fl, ln, type, msgArgs, _log.email_template), cp_utf);
						_log.email_count_msgs++;
						if(_log.email_count_msgs >= _log.email_max_msgs)
							send_email(_log.string_continue);
					}
					break;
				case TypeOutput::Debug:
					OutputDebugStringW(apply_template(fn, fl, ln, type, msgArgs, _log.debug_template));
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
			file.write(ln, cp_utf);
			_log.email_count_msgs = 0;
			// отправка на почту
			Buffer<Mail> m(new Mail(_log.email_host, _log.email_login, _log.email_pass, _log.email_flags), 1, false);
			m->set_charset(L"cp1251");
			m->add_recipient(L"", _log.email_address);
			m->set_sender(hlp->get_system_info(Helpers::siNameProg), _log.email_address);
			m->set_message(L"Система логгирования для отладки программы.");
			m->smtp(_log.email_subject, str);
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
