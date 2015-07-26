
#include "stdafx.h"
#include "ssh_mail.h"

namespace ssh
{
	static ssh_cs _crlf[] = "\r\n";

	Mail::Command_Entry command_list[] =
	{
		{Mail::command_smtp_INIT, 20000, 220, L"SERVER_NOT_RESPONDING"},
		{Mail::command_smtp_EHLO, 20000, 250, L"COMMAND_EHLO"},
		{Mail::command_smtp_AUTHPLAIN, 20000, 235, L"COMMAND_AUTH_PLAIN"},
		{Mail::command_smtp_AUTHLOGIN, 20000, 334, L"COMMAND_AUTH_LOGIN"},
		{Mail::command_smtp_AUTHCRAMMD5, 20000, 334, L"COMMAND_AUTH_CRAMMD5"},
		{Mail::command_smtp_AUTHDIGESTMD5, 20000, 334, L"COMMAND_AUTH_DIGESTMD5"},
		{Mail::command_smtp_DIGESTMD5, 20000, 335, L"COMMAND_DIGESTMD5"},
		{Mail::command_smtp_USER, 20000, 334, L"UNDEF_XYZ_RESPONSE"},
		{Mail::command_smtp_PASSWORD, 20000, 235, L"BAD_LOGIN_PASS"},
		{Mail::command_smtp_MAILFROM, 20000, 250, L"COMMAND_MAIL_FROM"},
		{Mail::command_smtp_RCPTTO, 20000, 250, L"COMMAND_RCPT_TO"},
		{Mail::command_smtp_DATA, 10000, 354, L"COMMAND_DATA"},
		{Mail::command_smtp_DATABLOCK, 10000, 0, L"COMMAND_DATABLOCK"},
		{Mail::command_smtp_DATAEND, 30000, 250, L"MSG_BODY_ERROR"},
		{Mail::command_smtp_STARTTLS, 20000, 220, L"COMMAND_EHLO_STARTTLS"},
		{Mail::command_smtp_QUIT, 20000, 221, L"COMMAND_QUIT"},
		{Mail::command_pop_INIT, 20000, 1, L"SERVER_NOT_RESPONDING"},
		{Mail::command_pop_USER, 20000, 1, L"UNDEF_XYZ_RESPONSE"},
		{Mail::command_pop_PASSWORD, 20000, 1, L"BAD_LOGIN_PASS"},
		{Mail::command_pop_APOP, 20000, 1, L"COOMAND_APOP"},
		{Mail::command_pop_DELE, 20000, 1, L"COOMAND_DELE"},
		{Mail::command_pop_LIST, 20000, 1, L"COOMAND_LIST"},
		{Mail::command_pop_NOOP, 20000, 1, L"COOMAND_NOOP"},
		{Mail::command_pop_RETR, 20000, 1, L"COOMAND_RETR"},
		{Mail::command_pop_RSET, 20000, 1, L"COOMAND_RSET"},
		{Mail::command_pop_STAT, 20000, 1, L"COOMAND_STAT"},
		{Mail::command_pop_TOP, 20000, 1, L"COOMAND_TOP"},
		{Mail::command_pop_STLS, 20000, 1, L"COMMAND_STARTTLS"},
		{Mail::command_pop_QUIT, 20000, 1, L"COMMAND_QUIT"},
		{Mail::command_pop_CAPA, 20000, 1, L"COMMAND_CAPA"},
		{Mail::command_pop_UIDL, 20000, 1, L"COMMAND_UIDL"},
		{Mail::command_imap_INIT, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_LOGIN, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_AUTHENTICATE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_CLOSE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_LOGOUT, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_CREATE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_DELETE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_RENAME, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_SUBSCRIBE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_UNSUBSCRIBE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_LIST, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_LSUB, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_STATUS, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_APPEND, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_CHECK, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_EXPUNGE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_SEARCH, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_FETCH, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_STORE, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_COPY, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_UID, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_CAPABILITY, 20000, 1, L"COMMAND_"},
		{Mail::command_imap_NOOP, 20000, 1, L"COMMAND_"}
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
		if(!log->is_email_blocked())  SSH_LOG(mail->resp);
		SetEvent(mail->hEvent);
	}

	Mail::Mail()
	{
		SSH_TRACE;
		recipients.setID(205);
		attach.setID(206);
		rx.set_pattern(0, LR"(^[a-z0-9\._-]+@[a-z0-9\._-]+\.[a-z]{2,4}$)", 0);
		rx.set_pattern(1, LR"((?m)^\d{3,3}[-|=|\s|\r\n])", 0);
		sock.setCallbacks(sock_receive, sock_connect, nullptr, nullptr, this);
		hEvent = CreateEvent(nullptr, true, false, nullptr);
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
		while((n = recipients.next())) header += cmd.fmt(L"%s %s%s", n->value.name, n->value.mail, (n->next ? L"," : L"\r\n"));
		header += cmd.fmt(L"Subject: %s\r\nMIME-Version: 1.0\r\n", cnv_rfc(subject));
		if(!attach.is_empty()) header += cmd.fmt(L"Content-Type: multipart/mixed; boundary=\"%s\"\r\n\r\n--%s\r\n", msg_id, msg_id);
		header += cmd.fmt(L"Content-type: text/%s; charset=\"%s\"\r\nContent-Transfer-Encoding: 8bit\r\n\r\n", (is_html ? L"html" : L"plain"), charset);
		return header;
	}

	bool Mail::check_keyword(ssh_wcs keyword)
	{
		return (rx.match(caps, cmd.fmt(L"(?im)[-\\s=]+%s[\\s=]+", keyword)) > 0);
	}
	
	void Mail::send_cmd(ssh_u command, ssh_u flags, const Buffer<ssh_cs>& data)
	{
		resp.empty();
		const Socket::SOCK* s(sock.get_sock(0));
		if(sock.send(s, data, data.count()))
		{
			if(sock.send(s, _crlf, 2))
			{
				recv_resp(command, flags);
				return;
			}
		}
		if(!log->is_email_blocked()) SSH_LOG(L"Ошибка при отправке!");
	}

	void Mail::send_cmd(ssh_u command, ssh_u flags, ssh_wcs fmt, ...)
	{
		String data;
		// формируем сообщение
		va_list	arglist;
		va_start(arglist, fmt);
		data.fmt(fmt, arglist);
		va_end(arglist);

		if(!log->is_email_blocked()) { SSH_LOG(data); }

		const Socket::SOCK* s(sock.get_sock(0));

		if(sock.send(s, ssh_cnv(cp_ansi, data, false)))
		{
			bool is(true);
			if((flags & Mail::add_crlf)) is = sock.send(s, _crlf, 2);
			if(is) { recv_resp(command, flags); return; }
		}
		if(!log->is_email_blocked()) SSH_LOG(L"Ошибка при отправке!");
	}

	void Mail::recv_resp(ssh_u command, ssh_u flags)
	{
		if(!(flags & Mail::no_resp))
		{
			resp.empty();
			ssh_l code(-1);
			while(code == -1)
			{
				if(WaitForSingleObject(hEvent, 20000) != WAIT_OBJECT_0) SSH_THROW(L"SERVER_NOT_RESPONSE");
				ResetEvent(hEvent);
				if(protocol == _pop3)
				{
					// pop3
					if(resp[0] == L'+')
					{
						if((flags & Mail::cont_resp))
						{
							if(resp.substr(resp.length() - 3) != L".\r\n") continue;
						}
						code = 1;
					}
					else code = 0;
				}
				else if(protocol == _imap)
				{
					if(resp[0] == L'+' || resp[0] == L'*')
					{
						if(resp.substr(resp.length() - 3) != L".\r\n") continue;
						code = 1;
					}
					else code = 0;
				}
				else if(protocol == _smtp) code = ((rx.match(resp, (ssh_u)1) > 0) ? resp.toNum<ssh_l>(rx.vec(0)) : 0);
				else SSH_THROW(L"UNKNOWN_PROTOCOL");
			}
			if(code != command_list[command].valid_code) SSH_THROW(command_list[command].error);
		}
	}

	bool Mail::pop3(const String& x, List<Mail::MAIL*>* lst, bool is_del)
	{
		SSH_TRACE;
		try
		{
			if(x.is_empty() || !lst) return false;
			// ***** КОННЕКТ С СЕРВЕРОМ *****
			connect_pop3();
			// ***** ПОЛУЧЕНИЕ СПИСКА ПИСЕМ *****
			// определяем количество писем и их суммарнй размер в байтах
			send_cmd(command_pop_STAT, 0, L"STAT\r\n");
			ssh_u count(resp.toNum<ssh_u>(4, String::_dec));
			// проходим циклом по все письмам
			for(ssh_u i = 1; i < count; i++)
			{
				MAIL* m(nullptr);
				if(check_keyword(L"TOP"))
				{
					send_cmd(command_pop_TOP, Mail::cont_resp, L"TOP %i 0\r\n", i);
					if(!(m = parse_mail(resp, x, nullptr, false))) continue;
				}
				send_cmd(command_pop_RETR, Mail::cont_resp, L"RETR %i\r\n", i);
				if((m = parse_mail(resp, x, m, true)))
				{
					if(lst) lst->add(m);
				}
				else
				{
					delete m;
				}
			}
			say_quit();
		}
		catch(const Exception& e)
		{
			e.add(L"Не удалось принять письма по протоколу POP3!");
		}
		sock.close();
		return true;
	}

	void Mail::connect_pop3()
	{
		SSH_TRACE;
		String timestamp;
		protocol = Protocol::_pop3;
		sock.init(host, 0, sock_flags, L"", L"");
		recv_resp(command_pop_INIT, 0);
		ssh_l pos(resp.find(L" <"));
		if(pos >= 0) timestamp = resp.substr(pos + 1, resp.find_rev(L'>') - (pos + 1));
		send_cmd(command_pop_CAPA, 0, L"CAPA\r\n");
		caps = resp;
		if(check_keyword(L"STLS") && (sock_flags & Socket::OPENTLS)) start_tls();
		// проверяем на тип авторизации (USER APOP)
		if(check_keyword(L"APOP"))
		{
			String str(ssh_md5(timestamp + pass));
			send_cmd(command_pop_APOP, 0, L"APOP %s %s\r\n", login, str);
		}
		else if(check_keyword(L"USER"))
		{
			send_cmd(command_pop_USER, 0, L"USER %s\r\n", login);
			send_cmd(command_pop_PASSWORD, 0, L"PASS %s\r\n", pass);
		}
		else SSH_THROW(L"LOGIN_NOT_SUPPORTED");
	}

	bool Mail::imap(const String& cmd, List<Mail::MAIL*>* lst, bool is_del)
	{
		SSH_TRACE;
		try
		{
			connect_imap();
			return true;
		}
		catch(const Exception& e)
		{
			e.add(L"Не удалось принять письма по протоколу IMAP!");
		}
		sock.close();
		return true;
	}

	void Mail::connect_imap()
	{
		SSH_TRACE;
		String timestamp;
		protocol = Protocol::_pop3;
		sock.init(host, 0, sock_flags, L"", L"");
		recv_resp(command_imap_INIT, 0);
		ssh_l pos(resp.find(L" <"));
		send_cmd(command_imap_CAPABILITY, 0, L"CAPABILITY\r\n");
		caps = resp;
//		if(check_keyword(L"STLS") && (sock_flags & Socket::OPENTLS)) start_tls();
	}

	void Mail::smtp(const String& subject, const String& body, bool is_html, bool is_notify)
	{
		SSH_TRACE;
		try
		{
			// ***** КОННЕКТ С СЕРВЕРОМ *****
			connect_smtp();
			// ***** РАСЧЕТ РАЗМЕРА ВЛОЖЕНИЙ *****
			ssh_u totalSize(0);
			auto n(attach.root());
			while((n = attach.next()))
			{
				try
				{
					File f(n->value, File::open_read);
					totalSize += f.length();
					f.close();
				}
				catch(const Exception&) {}
				if(totalSize > 5242880) SSH_THROW(L"Слишком большой размер вложений <%i> для отправки по почте!", totalSize);
			}
			// ***** ОТПРАВКА ПОЧТЫ *****
			if(recipients.is_empty()) SSH_THROW(L"UNDEF_RECIPIENTS");
			if(sender.mail.is_empty()) SSH_THROW(L"UNDEF_MAIL_FROM");
			send_cmd(command_smtp_MAILFROM, 0, L"MAIL FROM: %s\r\n", sender.mail);
			auto nn(recipients.root());
			while((nn = recipients.next())) send_cmd(command_smtp_RCPTTO, 0, L"RCPT TO: %s\r\n", nn->value.mail);
			// DATA <CRLF>
			send_cmd(command_smtp_DATA, 0, L"DATA\r\n");
			send_cmd(command_smtp_DATABLOCK, Mail::no_resp, headers(subject, is_html, is_notify).buffer());
			send_cmd(command_smtp_DATABLOCK, Mail::no_resp, ssh_cnv(charset, body, false));
			n = attach.root();
			while((n = attach.next()))
			{
				String name(n->value);
				ssh_l pos;
				try
				{
					if((pos = name.find_rev(L'\\')) < 0) continue;
					File f(name, File::open_read);
					name.fmt(L"=?%s?B?%s?=", charset, ssh_to_base64(name.substr(pos + 1), true).to<ssh_ws>());
					send_cmd(command_smtp_DATABLOCK, Mail::no_resp, L"--%s\r\nContent-Type: application/x-msdownload; name=\"%s\"\r\nContent-Transfer-Encoding: base64\r\nContent-Disposition: attachment; filename=\"%s\"\r\n\r\n", msg_id, name, name);
					send_cmd(command_smtp_DATABLOCK, Mail::add_crlf | Mail::no_resp, ssh_to_base64(f.read<ssh_cs>(), false));
					f.close();
				}
				catch(const Exception& e) { e.add(L"Не удалось открыть файл вложений при отправке электронной почты!"); }
			}
			if(!attach.is_empty()) send_cmd(command_smtp_DATABLOCK, Mail::no_resp, L"\r\n--%s--\r\n", msg_id);
			send_cmd(command_smtp_DATAEND, 0, L"\r\n.\r\n");
			say_quit();
		}
		catch(const Exception& e)
		{
			if(!log->is_email_blocked()) e.add(L"Не удалось отправить письмо по протоколу SMTP!");
		}
		sock.close();
	}

	void Mail::connect_smtp()
	{
		SSH_TRACE;
		protocol = Protocol::_smtp;
		sock.init(host, 0, sock_flags, L"", L"");
		recv_resp(command_smtp_INIT, 0); // просто принять ответ
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
				cmd = login + L'\1' + login + L'\1' + pass;
				cmd.replace(L'\1', L'\0');
				send_cmd(command_smtp_AUTHPLAIN, 0, L"AUTH PLAIN %s", ssh_to_base64(cmd, true).to<ssh_ws>());
			}
			else if(check_keyword(L"LOGIN"))
			{
				send_cmd(command_smtp_AUTHLOGIN, 0, L"AUTH LOGIN");
				send_cmd(command_smtp_USER, 0, ssh_to_base64(login, false));
				send_cmd(command_smtp_PASSWORD, 0, ssh_to_base64(pass, false));
			}
			else SSH_THROW(L"LOGIN_NOT_SUPPORTED");
		}
	}

	void Mail::say_hello()
	{
		SSH_TRACE;
		send_cmd(command_smtp_EHLO, 0, L"EHLO %s\r\n", hlp->get_system_info(Helpers::siCompName));
		caps = resp;
	}

	void Mail::say_quit()
	{
		SSH_TRACE;
		send_cmd((protocol == _smtp ? command_smtp_QUIT : (protocol == _pop3 ? command_pop_QUIT : command_pop_QUIT)), 0, L"QUIT\r\n");
	}

	void Mail::start_tls()
	{
		SSH_TRACE;
		if(protocol == _smtp)
		{
			if(!check_keyword(L"STARTTLS")) SSH_THROW(L"STARTTLS_NOT_SUPPORTED");
			send_cmd(command_smtp_STARTTLS, 0, L"STARTTLS\r\n");
		}
		else if(protocol == _pop3)
		{
			send_cmd(command_pop_STLS, 0, L"STLS\r\n");
		}
		else
		{

		}
		sock.startTLS();
		if(WaitForSingleObject(hEvent, 10000) != WAIT_OBJECT_0) SSH_THROW(L"SERVER_NOT_RESPONSE");
		ResetEvent(hEvent);
	}

	void Mail::default(bool is_recipient, bool is_attach)
	{
		SSH_TRACE;
		if(is_recipient) recipients.free();
		if(is_attach) attach.free();

		login = L"ostrov_skal";
		pass = L"IfnfkjdCthutq";
		x_ostrov.name = L"";
		x_ostrov.mail = L"";
		charset = L"windows-1251";
		sender = makeNameMail(L"Шаталов Сергей", L"ostrov_skal@mail.ru");
		reply_to = makeNameMail(L"", L"ostrov_skal@mail.ru");
		x_msg = L"Создано Шаталовым С.В. в системе stdSSH";
		host = L"smtp.mail.ru:465";
		sock_flags = (Socket::OPENSSL | Socket::METHOD_SSLv23);

		msg_id = hlp->gen_name(L"__MESSAGE__ID__");
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
		return cmd.fmt(L"=?%s?B?%s?=", charset, str.is_empty() ? L"" : ssh_to_base64(ssh_cnv(charset, str, false), true).to<ssh_ws>());
	}

	Mail::MAIL_NAME Mail::makeNameMail(const String& name, const String& mail)
	{
		SSH_TRACE;
		return{cnv_rfc(name), ((rx.match(mail, (ssh_u)0) > 0) ? L'<' + mail + L'>' : L"")};
	}

	String Mail::decode_string(String charset, const String& subj, bool is_base64)
	{
		Buffer<ssh_cs> buf(is_base64 ? ssh_from_base64(ssh_cnv(cp_ansi, subj, false)) : ssh_cnv(cp_ansi, subj, false));
		return ssh_cnv(charset, buf, 0);
	}

	Mail::MAIL* Mail::parse_mail(const String& mail, const String& x, MAIL* m, bool is_body)
	{
		SSH_TRACE;
		// парсер - xcmd, subject, charset, date, from, x-mailer, body, attached(file_name, body)
		regx rx;
		MAIL* stk(nullptr);
		if(!m)
		{
			// проверяем - это письмо - то что нам нужно?
			// xcmd
			if(rx.match(mail, cmd.fmt(LR"((?im)%s:(.+)$)", x)) <= 0) return nullptr;
			m = stk = new MAIL;
			stk->xcmd = rx.substr(1);
			// date
			if(rx.match(mail, LR"((?m)Date:[,\D]*(\d{1,2})\s+(\w\w\w\s)\s*([\d]+)\s+(\d{1,2}):(\d{1,2}):(\d{1,2}))") > 0)
			{
				static String months(L"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ");
				int month((int)months.find(rx.substr(2)));
				if(month >= 0) month = (month / 4) + 1; else month = 0;
				stk->gmt_date = Time(mail.toNum<int>(rx.vec(3)), (int)month, mail.toNum<int>(rx.vec(1)),
									 mail.toNum<int>(rx.vec(4)), mail.toNum<int>(rx.vec(5)), mail.toNum<int>(rx.vec(6)));
			}
			if(rx.match(mail, LR"(From:\s+(?:(?:\s*)|(["А-Яа-я\w\d\.\s_-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))\s*<([\w\d\._-]+@[\w\d\._-]+\.[\w]{2,4})>\r\n)") == 5)
			{
				// вся строка, имя, кодировка, имя, адрес
				if(rx.vec(2) != -1) stk->sender.name = decode_string(rx.substr(2), rx.substr(3));
				else if(rx.vec(1) != -1) stk->sender.name = rx.substr(1).trim(L"\"");
				stk->sender.mail = rx.substr(4);
			}
			// x_mailer
			if(rx.match(mail, LR"((?m)X-Mailer: (?:([\w\d\.="\s-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))$)") > 0)
				stk->xmailer = rx.vec(1) != -1 ? rx.substr(1) : decode_string(rx.substr(2), rx.substr(3));
			// subject
			if(rx.match(mail, LR"((?m)Subject: (?:([\w\d\.="\s-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))$)") > 0)
				stk->subject = rx.vec(1) != -1 ? rx.substr(1) : decode_string(rx.substr(2), rx.substr(3));
		}
		if(is_body)
		{
			String msg_id;
			if(rx.match(mail, LR"((?mi)Content-Type:.*multipart/mixed;.*boundary="(.*?))") > 0)
				msg_id = rx.substr(1);
			/*
			// Content - type: text / plain; charset = "cp1251"
			// Content - Transfer - Encoding: 8bit
			//
			// текст письма
			*/
			ssh_l idx(0);
			if(rx.match(mail, LR"((?mis)Content-Type:.*text/([\w\d-]+);.*charset=["]?([\w\d-]+)["]?;.*Content-Transfer-Encoding: ([\d\w-]+))") > 0)
			{
				String charset(rx.substr(2).lower());
				m->body_type = rx.substr(1).lower();
				String coding_body(rx.substr(3).lower());
				idx = rx.vec(3, 1);
				if(rx.match(mail, cmd.fmt(LR"((?mis)(?:\r\n\r\n(.*)|(?:(--%s\r\n)|(\r\n\.\r\n)))", msg_id), 0, idx) > 0)
				{
					m->body = decode_string(charset, rx.substr(1), coding_body == L"base64");
					idx = rx.vec(0, 1);
				}
			}
			/*
			// --__MESSAGE__ID__10001FCB31950E475
			// Content - Type: application / x - msdownload; name = "=?UTF-8?B?MS5qcGc=?="
			// Content - Transfer - Encoding: base64
			// Content - Disposition : attachment; filename = "=?UTF-8?B?MS5qcGc=?="
			// вложение
			// --__MESSAGE__ID__10001FCB31950E475
			*/
			if(!msg_id.is_empty())
			{
				rx.set_pattern(0, cmd.fmt(	LR"((?ism)--%s.*"
											L"Content-Type: application/x-msdownload;.*"
											L"name=["]?(?:(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)=\?=)|([\w\d_\.=-]+))["]?.*"
											L"Content-Transfer-Encoding: ([\w\d-]+).*"
											L"Content-Disposition: attachment;.*"
											L"filename=["]?(?:(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)=\?=)|([\w\d_\.=-]+))["]?\r\n\r\n"
											L"(.*?)\r\n")", msg_id), 0);
				while(rx.match(mail, (ssh_u)0, idx) == 7)
				{
					MAIL::ATTACH* attach = new MAIL::ATTACH;
					// name
					attach->name = decode_string(rx.substr(1), rx.substr(2));
					// bits
					attach->bits = rx.substr(3);
					// filename
					attach->filename = decode_string(rx.substr(4), rx.substr(5));
					// file_body
					attach->obj = ssh_from_base64(rx.substr(6));
					// add mail
					m->attached.add(attach);
					// correct idx
					idx += rx.vec(0, 1);
				}
			}
		}
		return m;
	}
}
