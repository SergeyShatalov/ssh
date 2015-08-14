
#pragma once

#include "ssh_list.h"
#include "ssh_map.h"

namespace ssh
{
	class SSH Mail
	{
		friend void sock_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
		friend void sock_connect(Socket* sock, Socket::SOCK* s);
		Mail(const Mail& m) {}
	public:
		enum SMTP_COMMAND
		{
			// smtp
			command_smtp_INIT,
			command_smtp_EHLO,
			command_smtp_AUTHPLAIN,
			command_smtp_AUTHLOGIN,
			command_smtp_AUTHCRAMMD5,
			command_smtp_AUTHDIGESTMD5,
			command_smtp_DIGESTMD5,
			command_smtp_USER,
			command_smtp_PASSWORD,
			command_smtp_MAILFROM,
			command_smtp_RCPTTO,
			command_smtp_DATA,
			command_smtp_DATABLOCK,
			command_smtp_DATAEND,
			command_smtp_STARTTLS,
			command_smtp_QUIT
		};
		struct Command_Entry
		{
			// �������� ��� ������ �� �������
			ssh_u valid_code;
			//  ��������� �� ������
			ssh_wcs	error;
		};
		enum CommandFlags
		{
			no_send = 0x01,
			cont_resp = 0x02,
			add_crlf = 0x04,
			no_resp = 0x08
		};
		enum SecurityType
		{
			stSimple,
			stTLS,
			stSSL
		};
		struct MAIL_NAME
		{
			String name;
			String mail;
		};
		// �����������
		Mail();
		// ���������������� �����������
		Mail(const String& host, const String& login, const String& pass, int type) : Mail() { set_host(host, type); set_login(login, pass); }
		// ����������
		virtual ~Mail();
		// ���������� ����� ������������
		void set_login(const String& _login, const String& _pass) { login = _login; pass = _pass; }
		// ���������� ������
		void set_pass(const String& str) { pass = str; }
		// �� ���� - ���:�����
		void set_sender(const String& name, const String& mail) { sender = makeNameMail(name, mail); }
		// ���������� �������� ����
		void set_reply_to(const String& name, const String& mail) { reply_to = makeNameMail(name, mail); }
		// ���������� ����
		void set_host(const String& host, int type);
		// ���������� ���� ���������
		void set_message(const String& str) { x_msg = str; }
		// ���������� ��� ������
		void set_xserg(const String& cmd, const String& val) { x_ostrov.name = cmd; x_ostrov.mail = val; }
		// ���������� ���������
		void set_charset(const String& str) { charset = str; }
		// �������� ��������� - ���:�����
		void add_recipient(const String& name, const String& mail) { recipients += makeNameMail(name, mail); }
		// �������� �������� - ���� � �����
		void add_attach(const String& file) { attach += file; }
		// ���������� �������� �� ���������
		void default(bool is_recipients, bool is_attach);
		// ��������� ������
		void smtp(const String& subject, const String& body, bool is_html = false, bool is_notify = false);
	protected:
		// ������ ������
		String cnv_rfc(const String& str);
		// �������� �� �������� �����
		bool check_keyword(ssh_wcs keyword);
		// �������� �� ���������� ����� �����
		bool is_correct_mail(const String& mail);
		// ����������� ��������� ��� ����� ������
		String headers(const String& subject, bool is_html, bool is_notify);
		// ����������� �������
		void say_hello();
		// ������ TLS
		void start_tls();
		// �������� ������
		void _send_cmd(ssh_u command, ssh_wcs data, ssh_u flags = 0);
		void send_cmd(ssh_u command, ssh_wcs fmt, ssh_u flags = 0, ...);
		// �������� �����
		void recv_resp(ssh_u command, ssh_u flags = 0);
		// ������������ ������ ��� <�����>
		MAIL_NAME makeNameMail(const String& name, const String& mail);
		// �����
		Socket sock;
		// ��� ������������ ��������
		String cmd;
		// ������� ��������
		HANDLE hEvent;
		// �����
		String resp;
		// ������������� ��������
		String msg_id;
		// ����:����
		String host;
		// ����� �������� ������
		int sock_flags;
		// ����������� - ���:�����
		MAIL_NAME sender;
		// �����
		String login;
		// ������
		String pass;
		// ���� ���������
		String x_msg;
		// ��� ������
		MAIL_NAME x_ostrov;
		// �������� ����� - �����
		MAIL_NAME reply_to;
		// ���������
		String charset;
		// ������ ������������� ������
		List<String, SSH_TYPE> attach{ID_ATTACH_MAIL};
		// ������ ���������
		List<MAIL_NAME, SSH_TYPE> recipients{ID_RECIPIENTS_MAIL};
		// �����������
		String caps;
		// ��� �������� � ����������� �����������
		regx rx;
	};
}