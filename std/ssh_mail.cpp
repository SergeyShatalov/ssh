
#include "stdafx.h"
#include "ssh_mail.h"

namespace ssh
{
	Mail::Command_Entry command_list[] =
	{
		{/* Mail::command_smtp_INIT, */220, L"SERVER_NOT_RESPONDING"},
		{/* Mail::command_smtp_EHLO, */250, L"COMMAND_EHLO"},
		{/* Mail::command_smtp_AUTHPLAIN, */235, L"COMMAND_AUTH_PLAIN"},
		{/* Mail::command_smtp_AUTHLOGIN, */334, L"COMMAND_AUTH_LOGIN"},
		{/* Mail::command_smtp_AUTHCRAMMD5, */334, L"COMMAND_AUTH_CRAMMD5"},
		{/* Mail::command_smtp_AUTHDIGESTMD5, */334, L"COMMAND_AUTH_DIGESTMD5"},
		{/* Mail::command_smtp_DIGESTMD5, */335, L"COMMAND_DIGESTMD5"},
		{/* Mail::command_smtp_USER, */334, L"UNDEF_XYZ_RESPONSE"},
		{/* Mail::command_smtp_PASSWORD, */235, L"BAD_LOGIN_PASS"},
		{/* Mail::command_smtp_MAILFROM, */250, L"COMMAND_MAIL_FROM"},
		{/* Mail::command_smtp_RCPTTO, */250, L"COMMAND_RCPT_TO"},
		{/* Mail::command_smtp_DATA, */354, L"COMMAND_DATA"},
		{/* Mail::command_smtp_DATABLOCK, */0, L"COMMAND_DATABLOCK"},
		{/* Mail::command_smtp_DATAEND, */250, L"MSG_BODY_ERROR"},
		{/* Mail::command_smtp_STARTTLS, */220, L"COMMAND_EHLO_STARTTLS"},
		{/* Mail::command_smtp_QUIT, */221, L"COMMAND_QUIT"}
	};

	static void sock_connect(Socket* sock, Socket::SOCK* s)
	{
		Mail* mail(sock->get_user_data<Mail*>());
		if(sock->is_start_tls()) SetEvent(mail->hEvent);
	}

	static void sock_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf)
	{
		Mail* mail(sock->get_user_data<Mail*>());
		// реализовать процесс накопления
		mail->resp += String(buf);
		SSH_LOG(mail->resp);
		SetEvent(mail->hEvent);
	}

	Mail::Mail()
	{
		SSH_TRACE;
		rx.set_pattern(0, LR"(^[a-z0-9\._-]+@[a-z0-9\._-]+\.[a-z]{2,4}$)");
		rx.set_pattern(1, LR"((?m)^\d{3,3}[-|=|\s|\r\n])");
		sock.setCallbacks(sock_receive, sock_connect, nullptr, nullptr, this);
		hEvent = CreateEventW(nullptr, true, false, nullptr);
		default(true, true);
	}

	Mail::~Mail()
	{
		SSH_TRACE;
		sock.resetCallbacks();
		if(hEvent)
		{
			CloseHandle(hEvent);
			hEvent = nullptr;
		}
		sock.close();
	}

	String Mail::headers(const String& subject, bool is_html, bool is_notify)
	{
		SSH_TRACE;
		static ssh_ws month[][4] = {L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec"};
		String header;
		Time t(Time::current());
		struct tm* g(t.gmt());
		header.fmt(L"Date: %d %s %d %d:%d:%d\r\n", g->tm_mday, month[g->tm_mon], g->tm_year + 1900, g->tm_hour, g->tm_min, g->tm_sec);
		header += cmd.fmt(L"From: %s %s\r\n", sender.name, sender.mail);
		if(!x_msg.is_empty()) header += cmd.fmt(L"X-Mailer: %s\r\n", cnv_rfc(x_msg));
		if(!reply_to.mail.is_empty()) header += cmd.fmt(L"Reply-To: %s %s\r\n", reply_to.name, reply_to.mail);
		if(!x_ostrov.name.is_empty()) header += cmd.fmt(L"%s: %s\r\n", x_ostrov.name, x_ostrov.mail);
		if(is_notify) header += cmd.fmt(L"Disposition-Notification-To: %s\r\n", (reply_to.mail.is_empty() ? sender.mail : reply_to.mail));
		header += L"To: ";
		auto n(recipients.root());
		while(n) { header += cmd.fmt(L"%s %s%s", n->value.name, n->value.mail, (n->next ? L"," : L"\r\n")); n = n->next; }
		header += cmd.fmt(L"Subject: %s\r\nMIME-Version: 1.0\r\n", cnv_rfc(subject));
		if(!attach.is_empty()) header += cmd.fmt(L"Content-Type: multipart/mixed; boundary=\"%s\"\r\n\r\n--%s\r\n", msg_id, msg_id);
		header += cmd.fmt(L"Content-type: text/%s; charset=\"%s\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n", (is_html ? L"html" : L"plain"), charset);
		return header;
	}

	bool Mail::check_keyword(ssh_wcs keyword)
	{
		return (rx.match(caps, (ssh_wcs)cmd.fmt(L"(?im)[-\\s=]+%s[\\s=]+", keyword), (ssh_l)0) > 0);
	}

	void Mail::_send_cmd(ssh_u command, ssh_wcs data, ssh_u flags)
	{
		SSH_LOG(data);

		const Socket::SOCK* s(sock.get_sock(0));

		if(sock.send(s, ssh_cnv(cp_ansi, data, false)))
		{
			static ssh_cs _crlf[] = "\r\n";
			bool is(true);
			if((flags & Mail::add_crlf)) is = sock.send(s, _crlf, 2);
			if(is) { recv_resp(command, flags); return; }
		}
		SSH_LOG(L"Ошибка при отправке!");
	}
		
	void Mail::send_cmd(ssh_u command, ssh_wcs fmt, ssh_u flags, ...)
	{
		String data;
		// формируем сообщение
		va_list	arglist;
		va_start(arglist, flags);
		data.fmt(fmt, arglist);
		va_end(arglist);
		_send_cmd(command, data, flags);
	}

	void Mail::recv_resp(ssh_u command, ssh_u flags)
	{
		if(!(flags & Mail::no_resp))
		{
			resp.empty();
			if(WaitForSingleObject(hEvent, 20000) != WAIT_OBJECT_0) SSH_THROW(L"SERVER_NOT_RESPONSE");
			ResetEvent(hEvent);
			ssh_l code((rx.match(resp, (ssh_u)1) > 0) ? resp.toNum<ssh_l>(rx.vec(0), String::_dec) : 0);
			if(code != command_list[command].valid_code) SSH_THROW(command_list[command].error);
		}
	}

	void Mail::smtp(const String& subject, const String& body, bool is_html, bool is_notify)
	{
		SSH_TRACE;
		try
		{
			// ***** КОННЕКТ С СЕРВЕРОМ *****
			sock.init(host, 0, sock_flags);
			recv_resp(command_smtp_INIT); // просто принять ответ
			say_hello();
			if(sock_flags == Socket::OPENTLS)
			{
				start_tls();
				say_hello();
			}
			if(check_keyword(L"AUTH"))
			{
				if(check_keyword(L"PLAIN"))
				{
					cmd = ssh_base64(ssh_cnv(cp_ansi, login, true));
					cmd = L"AUTH PLAIN " + cmd + cmd + ssh_base64(cp_ansi, pass) + L"\r\n";
					_send_cmd(command_smtp_AUTHPLAIN, cmd);
				}
				else if(check_keyword(L"LOGIN"))
				{
					_send_cmd(command_smtp_AUTHLOGIN, L"AUTH LOGIN");
					_send_cmd(command_smtp_USER, ssh_base64(cp_ansi, login), Mail::add_crlf);
					_send_cmd(command_smtp_PASSWORD, ssh_base64(cp_ansi, pass), Mail::add_crlf);
				}
				else SSH_THROW(L"LOGIN_NOT_SUPPORTED");
			}
			// ***** ОТПРАВКА ПОЧТЫ *****
			if(recipients.is_empty()) SSH_THROW(L"Отсутствуют адресат(ы) отправителя!");
			if(sender.mail.is_empty()) SSH_THROW(L"Отсутствует отправитель письма!");
			send_cmd(command_smtp_MAILFROM, L"MAIL FROM: %s\r\n", 0, sender.mail);
			auto nn(recipients.root());
			while(nn) { send_cmd(command_smtp_RCPTTO, L"RCPT TO: %s\r\n", 0, nn->value.mail); nn = nn->next; }
			// DATA <CRLF>
			_send_cmd(command_smtp_DATA, L"DATA\r\n");
			_send_cmd(command_smtp_DATABLOCK, headers(subject, is_html, is_notify).buffer(), Mail::no_resp);
			_send_cmd(command_smtp_DATABLOCK, String(ssh_cnv(charset, body, false)), Mail::add_crlf | Mail::no_resp);
			auto n(attach.root());
			while(n)
			{
				String name(n->value);
				ssh_l pos;
				try
				{
					if((pos = name.find_rev(L'\\')) < 0) continue;
					File f(name, File::open_read);
					name.fmt(L"=?%s?B?%s?=", charset, ssh_base64(charset, name.substr(pos + 1)));
					send_cmd(command_smtp_DATABLOCK, L"--%s\r\nContent-Type: application/x-msdownload; name=\"%s\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"%s\"\r\n\r\n", Mail::no_resp, msg_id, name, name);
					_send_cmd(command_smtp_DATABLOCK, ssh_base64(f.read<ssh_cs>()), Mail::add_crlf | Mail::no_resp);
					f.close();
				}
				catch(const Exception& e) { e.add(L"Не удалось открыть файл вложений при отправке электронной почты!"); }
				n = n->next;
			}
			if(!attach.is_empty()) send_cmd(command_smtp_DATABLOCK, L"\r\n--%s--\r\n", Mail::no_resp, msg_id);
			_send_cmd(command_smtp_DATAEND, L"\r\n.\r\n");
			_send_cmd(command_smtp_QUIT, L"QUIT\r\n");
		}
		catch(const Exception& e)
		{
			e.add(L"Не удалось отправить письмо по протоколу SMTP!");
		}
		sock.close();
	}

	void Mail::say_hello()
	{
		SSH_TRACE;
		_send_cmd(command_smtp_EHLO, L"EHLO " + ssh_system_paths(SystemInfo::siCompName), Mail::add_crlf);
		caps = resp;
	}

	void Mail::start_tls()
	{
		SSH_TRACE;
		if(!check_keyword(L"STARTTLS")) SSH_THROW(L"STARTTLS_NOT_SUPPORTED");
		_send_cmd(command_smtp_STARTTLS, L"STARTTLS\r\n");
		sock.startTLS();
		if(WaitForSingleObject(hEvent, 10000) != WAIT_OBJECT_0) SSH_THROW(L"SERVER_NOT_RESPONSE");
		ResetEvent(hEvent);
	}

	void Mail::default(bool is_recipient, bool is_attach)
	{
		SSH_TRACE;
		if(is_recipient) recipients.reset();
		if(is_attach) attach.reset();

		login = L"ostrov_skal";
		pass = L"IfnfkjdCthutq";
		x_ostrov.name = L"";
		x_ostrov.mail = L"";
		charset = L"windows-1251";
		sender = makeNameMail(L"Шаталов Сергей", L"ostrov_skal@mail.ru");
		reply_to.name = sender.name;
		reply_to.mail = sender.mail;
		x_msg = L"Создано Шаталовым С.В. в системе stdSSH";
		host = L"smtp.mail.ru:465";
		sock_flags = (Socket::OPENSSL | Socket::METHOD_SSLv23);

		msg_id = ssh_gen_name(L"__MESSAGE__ID__");
	}

	void Mail::set_host(const String& hst, int type)
	{
		SSH_TRACE;
		host = hst;
		sock_flags = 0;
		if(type == SecurityType::stSSL) sock_flags = (Socket::OPENSSL | Socket::METHOD_SSLv23);
		else if(type == SecurityType::stTLS) sock_flags = (Socket::OPENTLS | Socket::METHOD_SSLv23);
	}

	String Mail::cnv_rfc(const String& str)
	{
		SSH_TRACE;
		return cmd.fmt(L"=?%s?B?%s?=", charset, ssh_base64(charset, str));
	}

	Mail::MAIL_NAME Mail::makeNameMail(const String& name, const String& mail)
	{
		SSH_TRACE;
		return{cnv_rfc(name), ((rx.match(mail, (ssh_u)0) > 0) ? L'<' + mail + L'>' : L"")};
	}
}
