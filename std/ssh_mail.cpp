
#include "stdafx.h"
#include "ssh_mail.h"

namespace ssh
{
	static ssh_cs _crlf[] = "\r\n";

	Mail::Command_Entry command_list[] =
	{
		{Mail::command_smtp_INIT, 220, L"SERVER_NOT_RESPONDING"},
		{Mail::command_smtp_EHLO, 250, L"COMMAND_EHLO"},
		{Mail::command_smtp_AUTHPLAIN, 235, L"COMMAND_AUTH_PLAIN"},
		{Mail::command_smtp_AUTHLOGIN, 334, L"COMMAND_AUTH_LOGIN"},
		{Mail::command_smtp_AUTHCRAMMD5, 334, L"COMMAND_AUTH_CRAMMD5"},
		{Mail::command_smtp_AUTHDIGESTMD5, 334, L"COMMAND_AUTH_DIGESTMD5"},
		{Mail::command_smtp_DIGESTMD5, 335, L"COMMAND_DIGESTMD5"},
		{Mail::command_smtp_USER, 334, L"UNDEF_XYZ_RESPONSE"},
		{Mail::command_smtp_PASSWORD, 235, L"BAD_LOGIN_PASS"},
		{Mail::command_smtp_MAILFROM, 250, L"COMMAND_MAIL_FROM"},
		{Mail::command_smtp_RCPTTO, 250, L"COMMAND_RCPT_TO"},
		{Mail::command_smtp_DATA, 354, L"COMMAND_DATA"},
		{Mail::command_smtp_DATABLOCK, 0, L"COMMAND_DATABLOCK"},
		{Mail::command_smtp_DATAEND, 250, L"MSG_BODY_ERROR"},
		{Mail::command_smtp_STARTTLS, 220, L"COMMAND_EHLO_STARTTLS"},
		{Mail::command_smtp_QUIT, 221, L"COMMAND_QUIT"},
		{Mail::command_pop_INIT, 1, L"SERVER_NOT_RESPONDING"},
		{Mail::command_pop_USER, 1, L"UNDEF_XYZ_RESPONSE"},
		{Mail::command_pop_PASSWORD, 1, L"BAD_LOGIN_PASS"},
		{Mail::command_pop_APOP, 1, L"COOMAND_APOP"},
		{Mail::command_pop_DELE, 1, L"COOMAND_DELE"},
		{Mail::command_pop_LIST, 1, L"COOMAND_LIST"},
		{Mail::command_pop_NOOP, 1, L"COOMAND_NOOP"},
		{Mail::command_pop_RETR, 1, L"COOMAND_RETR"},
		{Mail::command_pop_RSET, 1, L"COOMAND_RSET"},
		{Mail::command_pop_STAT, 1, L"COOMAND_STAT"},
		{Mail::command_pop_TOP, 1, L"COOMAND_TOP"},
		{Mail::command_pop_STLS, 1, L"COMMAND_STARTTLS"},
		{Mail::command_pop_QUIT, 1, L"COMMAND_QUIT"},
		{Mail::command_pop_CAPA, 1, L"COMMAND_CAPA"},
		{Mail::command_pop_UIDL, 1, L"COMMAND_UIDL"},
		{Mail::command_imap_INIT, 1, L"COMMAND_"},
		{Mail::command_imap_LOGIN, 1, L"COMMAND_"},
		{Mail::command_imap_AUTHENTICATE, 1, L"COMMAND_AUTHENTICATE"},
		{Mail::command_imap_CLOSE, 1, L"COMMAND_CLOSE"},
		{Mail::command_imap_LOGOUT, 1, L"COMMAND_LOGOUT"},
		{Mail::command_imap_CREATE, 1, L"COMMAND_CREATE"},
		{Mail::command_imap_DELETE, 1, L"COMMAND_DELETE"},
		{Mail::command_imap_RENAME, 1, L"COMMAND_RENAME"},
		{Mail::command_imap_SUBSCRIBE, 1, L"COMMAND_SUBSCRIBE"},
		{Mail::command_imap_UNSUBSCRIBE, 1, L"COMMAND_"},
		{Mail::command_imap_LIST, 1, L"COMMAND_"},
		{Mail::command_imap_LSUB, 1, L"COMMAND_"},
		{Mail::command_imap_STATUS, 1, L"COMMAND_"},
		{Mail::command_imap_APPEND, 1, L"COMMAND_"},
		{Mail::command_imap_CHECK, 1, L"COMMAND_"},
		{Mail::command_imap_EXPUNGE, 1, L"COMMAND_"},
		{Mail::command_imap_SEARCH, 1, L"COMMAND_"},
		{Mail::command_imap_FETCH, 1, L"COMMAND_"},
		{Mail::command_imap_STORE, 1, L"COMMAND_"},
		{Mail::command_imap_COPY, 1, L"COMMAND_"},
		{Mail::command_imap_UID, 1, L"COMMAND_"},
		{Mail::command_imap_CAPABILITY, 1, L"COMMAND_"},
		{Mail::command_imap_NOOP, 1, L"COMMAND_"}
	};

	static void sock_connect(Socket* sock, Socket::SOCK* s)
	{
		Mail* mail(sock->get_user_data<Mail*>());
		if(sock->is_start_tls()) SetEvent(mail->hEvent);
	}

	static void sock_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf)
	{
		Mail* mail(sock->get_user_data<Mail*>());
		// ����������� ������� ����������
		mail->resp += String(buf);
		if(!log->is_email_blocked())  SSH_LOG(mail->resp);
		SetEvent(mail->hEvent);
	}

	Mail::Mail()
	{
		SSH_TRACE;
		recipients.setID(205);
		attach.setID(206);
		rx.set_pattern(0, LR"(^[a-z0-9\._-]+@[a-z0-9\._-]+\.[a-z]{2,4}$)");
		rx.set_pattern(1, LR"((?m)^\d{3,3}[-|=|\s|\r\n])");
		rx.set_pattern(2, LR"((?m)Date:[,\D]*(\d{1,2})\s+(\w\w\w\s)\s*([\d]+)\s+(\d{1,2}):(\d{1,2}):(\d{1,2}))");
		rx.set_pattern(3, LR"(From:\s+(?:(?:\s*)|(["�-��-�\w\d\.\s_-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))\s*<([\w\d\._-]+@[\w\d\._-]+\.[\w]{2,4})>\r\n)");
		rx.set_pattern(4, LR"((?m)X-Mailer: (?:([\w\d\.="\s-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))$)");
		rx.set_pattern(5, LR"((?m)Subject: (?:([\w\d\.="\s-]+)|(?:=\?([\w\d-]+)\?B\?([\w\d+=]+)\?=))$)");
		rx.set_pattern(6, LR"((?smi)Content-Type:.*multipart/mixed;.*boundary="(.*?))");
		rx.set_pattern(7, LR"((?mis)Content-Type:.*text/([\w\d-]+);.*charset=["]?([\w\d-]+)["]?;.*Content-Transfer-Encoding: ([\d\w-]+))");
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

	/*
	void Mail::send_cmd(ssh_u command, const Buffer<ssh_cs>& data, ssh_u flags)
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
		if(!log->is_email_blocked()) SSH_LOG(L"������ ��� ��������!");
	}
	*/

	void Mail::send_cmd(ssh_u command, ssh_wcs fmt, ssh_u flags, ...)
	{
		String data;
		// ��������� ���������
		va_list	arglist;
		va_start(arglist, flags);
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
		if(!log->is_email_blocked()) SSH_LOG(L"������ ��� ��������!");
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
				else if(protocol == _smtp) code = ((rx.match(resp, (ssh_u)1) > 0) ? resp.toNum<ssh_l>(rx.vec(0), String::_dec) : 0);
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
			// ***** ������� � �������� *****
			connect_pop3();
			// ***** ��������� ������ ����� *****
			// ���������� ���������� ����� � �� �������� ������ � ������
			send_cmd(command_pop_STAT, L"STAT\r\n");
			ssh_u count(resp.toNum<ssh_u>(4, String::_dec));
			// �������� ������ �� ��� �������
			for(ssh_u i = 1; i < count; i++)
			{
				MAIL* m(nullptr);
//				File f(cmd.fmt(L"%i.eml", i), File::open_read);
//				resp = f.read(L"windows-1251", 0);
				if(!check_keyword(L"TOP"))
				{
					send_cmd(command_pop_TOP, L"TOP %i 0\r\n", Mail::cont_resp, i);
					if(!(m = parse_mail(resp, x, nullptr, false))) continue;
				}
				send_cmd(command_pop_RETR, L"RETR %i\r\n", Mail::cont_resp, i);
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
			e.add(L"�� ������� ������� ������ �� ��������� POP3!");
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
		recv_resp(command_pop_INIT);
		ssh_l pos(resp.find(L" <"));
		if(pos >= 0) timestamp = resp.substr(pos + 1, resp.find_rev(L'>') - (pos + 1));
		send_cmd(command_pop_CAPA, L"CAPA\r\n");
		caps = resp;
		if(check_keyword(L"STLS") && (sock_flags & Socket::OPENTLS)) start_tls();
		// ��������� �� ��� ����������� (USER APOP)
		if(check_keyword(L"APOP"))
		{
			String str(ssh_md5(timestamp + pass));
			send_cmd(command_pop_APOP, L"APOP %s %s\r\n", 0, login, str);
		}
		else if(check_keyword(L"USER"))
		{
			send_cmd(command_pop_USER, L"USER %s\r\n", 0, login);
			send_cmd(command_pop_PASSWORD, L"PASS %s\r\n", 0, pass);
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
			e.add(L"�� ������� ������� ������ �� ��������� IMAP!");
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
		recv_resp(command_imap_INIT);
		ssh_l pos(resp.find(L" <"));
		send_cmd(command_imap_CAPABILITY, L"CAPABILITY\r\n");
		caps = resp;
//		if(check_keyword(L"STLS") && (sock_flags & Socket::OPENTLS)) start_tls();
	}

	void Mail::smtp(const String& subject, const String& body, bool is_html, bool is_notify)
	{
		SSH_TRACE;
		try
		{
			// ***** ������� � �������� *****
			connect_smtp();
			// ***** ������ ������� �������� *****
			ssh_u totalSize(0);
			auto n(attach.root());
			while(n)
			{
				try
				{
					File f(n->value, File::open_read);
					totalSize += f.length();
					f.close();
				}
				catch(const Exception&) {}
				if(totalSize > 5242880) SSH_THROW(L"������� ������� ������ �������� <%i> ��� �������� �� �����!", totalSize);
				n = n->next;
			}
			// ***** �������� ����� *****
			if(recipients.is_empty()) SSH_THROW(L"UNDEF_RECIPIENTS");
			if(sender.mail.is_empty()) SSH_THROW(L"UNDEF_MAIL_FROM");
			send_cmd(command_smtp_MAILFROM, L"MAIL FROM: %s\r\n", 0, sender.mail);
			auto nn(recipients.root());
			while(nn) { send_cmd(command_smtp_RCPTTO, L"RCPT TO: %s\r\n", 0, nn->value.mail); nn = nn->next; }
			// DATA <CRLF>
			send_cmd(command_smtp_DATA, L"DATA\r\n");
			send_cmd(command_smtp_DATABLOCK, headers(subject, is_html, is_notify).buffer(), Mail::no_resp);
			send_cmd(command_smtp_DATABLOCK, String(ssh_cnv(charset, body, true)), Mail::add_crlf | Mail::no_resp);
			n = attach.root();
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
					send_cmd(command_smtp_DATABLOCK, ssh_base64(f.read<ssh_cs>()), Mail::add_crlf | Mail::no_resp);
					f.close();
				}
				catch(const Exception& e) { e.add(L"�� ������� ������� ���� �������� ��� �������� ����������� �����!"); }
				n = n->next;
			}
			if(!attach.is_empty()) send_cmd(command_smtp_DATABLOCK, L"\r\n--%s--\r\n", Mail::no_resp, msg_id);
			send_cmd(command_smtp_DATAEND, L"\r\n.\r\n");
			say_quit();
		}
		catch(const Exception& e)
		{
			if(!log->is_email_blocked()) e.add(L"�� ������� ��������� ������ �� ��������� SMTP!");
		}
		sock.close();
	}

	void Mail::connect_smtp()
	{
		SSH_TRACE;
		protocol = Protocol::_smtp;
		sock.init(host, 0, sock_flags, L"", L"");
		recv_resp(command_smtp_INIT, 0); // ������ ������� �����
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
				send_cmd(command_smtp_AUTHPLAIN, L"AUTH PLAIN %s\r\n", 0, ssh_base64(cp_ansi, cmd));
			}
			else if(check_keyword(L"LOGIN"))
			{
				send_cmd(command_smtp_AUTHLOGIN, L"AUTH LOGIN");
				send_cmd(command_smtp_USER, L"%s\r\n", 0, ssh_base64(cp_ansi, login));
				send_cmd(command_smtp_PASSWORD, L"%s\r\n", 0, ssh_base64(cp_ansi, pass));
			}
			else SSH_THROW(L"LOGIN_NOT_SUPPORTED");
		}
	}

	void Mail::say_hello()
	{
		SSH_TRACE;
		send_cmd(command_smtp_EHLO, L"EHLO %s\r\n", 0, hlp->get_system_info(Helpers::siCompName));
		caps = resp;
	}

	void Mail::say_quit()
	{
		SSH_TRACE;
		send_cmd((protocol == _smtp ? command_smtp_QUIT : (protocol == _pop3 ? command_pop_QUIT : command_pop_QUIT)), L"QUIT\r\n");
	}

	void Mail::start_tls()
	{
		SSH_TRACE;
		if(protocol == _smtp)
		{
			if(!check_keyword(L"STARTTLS")) SSH_THROW(L"STARTTLS_NOT_SUPPORTED");
			send_cmd(command_smtp_STARTTLS, L"STARTTLS\r\n");
		}
		else if(protocol == _pop3)
		{
			send_cmd(command_pop_STLS, L"STLS\r\n");
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
		if(is_recipient) recipients.reset();
		if(is_attach) attach.reset();

		login = L"ostrov_skal";
		pass = L"IfnfkjdCthutq";
		x_ostrov.name = L"";
		x_ostrov.mail = L"";
		charset = L"windows-1251";
		sender = makeNameMail(L"������� ������", L"ostrov_skal@mail.ru");
		reply_to = makeNameMail(sender.name, sender.mail);
		x_msg = L"������� ��������� �.�. � ������� stdSSH";
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
		return cmd.fmt(L"=?%s?B?%s?=", charset, ssh_base64(charset, str));
	}

	Mail::MAIL_NAME Mail::makeNameMail(const String& name, const String& mail)
	{
		SSH_TRACE;
		return{cnv_rfc(name), ((rx.match(mail, (ssh_u)0) > 0) ? L'<' + mail + L'>' : L"")};
	}

	String Mail::decode_string(String charset, const String& subj, bool is_base64)
	{
		return ssh_cnv(charset, (is_base64 ? ssh_base64(subj, false) : ssh_cnv(cp_ansi, subj, false)), 0);
	}

	Mail::MAIL* Mail::parse_mail(const String& mail, const String& x, MAIL* m, bool is_body)
	{
		SSH_TRACE;
		// ������ - xcmd, subject, charset, date, from, x-mailer, body, attached(file_name, body)
		regx rx;
		MAIL* stk(nullptr);
		if(!m)
		{
			// ��������� - ��� ������ - �� ��� ��� �����?
			// xcmd
			if(rx.match(mail, (ssh_wcs)cmd.fmt(LR"((?im)%s:(.+)$)", x)) <= 0) return nullptr;
			m = stk = new MAIL;
			stk->xcmd = rx.substr(1);
			// date
			if(rx.match(mail, (ssh_u)2) > 0)
			{
				static String months(L"Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec ");
				int month((int)months.find(rx.substr(2)));
				if(month >= 0) month = (month / 4) + 1; else month = 0;
				stk->gmt_date = Time(mail.toNum<int>(rx.vec(3)), (int)month, mail.toNum<int>(rx.vec(1)),
									 mail.toNum<int>(rx.vec(4)), mail.toNum<int>(rx.vec(5)), mail.toNum<int>(rx.vec(6)));
			}
			if(rx.match(mail, (ssh_u)3) == 5)
			{
				// ��� ������, ���, ���������, ���, �����
				if(rx.vec(2) != -1) stk->sender.name = decode_string(rx.substr(2), rx.substr(3));
				else if(rx.vec(1) != -1) stk->sender.name = rx.substr(1).trim(L"\"");
				stk->sender.mail = rx.substr(4);
			}
			// x_mailer
			if(rx.match(mail, (ssh_u)4) > 0)
				stk->xmailer = rx.vec(1) != -1 ? rx.substr(1) : decode_string(rx.substr(2), rx.substr(3));
			// subject
			if(rx.match(mail, (ssh_u)5) > 0)
				stk->subject = rx.vec(1) != -1 ? rx.substr(1) : decode_string(rx.substr(2), rx.substr(3));
		}
		if(is_body)
		{
			String msg_id;
			if(rx.match(mail, (ssh_u)6) > 0)
				msg_id = rx.substr(1);
			/*
			// Content - type: text / plain; charset = "cp1251"
			// Content - Transfer - Encoding: 8bit
			//
			// ����� ������
			*/
			ssh_l idx(0);
			if(rx.match(mail, (ssh_u)7) > 0)
			{
				String charset(rx.substr(2).lower());
				m->body_type = rx.substr(1).lower();
				String coding_body(rx.substr(3).lower());
				idx = rx.vec(3, 1);
				if(rx.match(mail, (ssh_wcs)cmd.fmt(LR"((?mis)(?:\r\n\r\n(.*)|(?:(--%s\r\n)|(\r\n\.\r\n)))", msg_id), idx) > 0)
				{
					m->body = decode_string(charset, rx.substr(1), coding_body == L"base64");
					idx = rx.vec(0, 1) + 1;
				}
			}
			/*
			// --__MESSAGE__ID__10001FCB31950E475
			// Content - Type: application / x - msdownload; name = "=?UTF-8?B?MS5qcGc=?="
			// Content - Transfer - Encoding: base64
			// Content - Disposition : attachment; filename = "=?UTF-8?B?MS5qcGc=?="
			// ��������
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
											L"(.*?)\r\n")", msg_id));
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
					attach->obj = ssh_base64(rx.substr(6), false);
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
