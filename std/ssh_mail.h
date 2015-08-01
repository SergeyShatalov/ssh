
#pragma once

#include "ssh_list.h"
#include "ssh_map.h"

namespace ssh
{
	class SSH Mail
	{
		friend void sock_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
		friend void sock_connect(Socket* sock, Socket::SOCK* s);
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
			command_smtp_QUIT,
			// pop3
			command_pop_INIT,
			command_pop_USER,
			command_pop_PASSWORD,
			command_pop_APOP,//
			command_pop_DELE,// ����� ���������
			command_pop_LIST,// [����� ���������]
			command_pop_NOOP,//
			command_pop_RETR,// ���������
			command_pop_RSET,//
			command_pop_STAT,//
			command_pop_TOP,  // ���������, ���������� �����
			command_pop_STLS,
			command_pop_QUIT,
			command_pop_CAPA,
			command_pop_UIDL,
			// imap
			command_imap_INIT,
			command_imap_LOGIN,// �������������� - user, pass
			command_imap_AUTHENTICATE, // ���������� ��������������
			command_imap_CLOSE,// ������� ���� - ���
			command_imap_LOGOUT,// ���������� ����������
			command_imap_CREATE,// ������� ���� - ���
			command_imap_DELETE,// ������� ���� - ���
			command_imap_RENAME,// ������������� ���� - ������ ���, ����� ���
			command_imap_SUBSCRIBE,// �������� ���� � ������ �������� - ���
			command_imap_UNSUBSCRIBE,// ������ ���� �� ������ �������� - ���
			command_imap_LIST,// �������� ������ ���� ������ - -
			command_imap_LSUB,// �������� ������ �������� ������ - -
			command_imap_STATUS,// ������� ��������� ���� - ��� �����, - ������ ��������� [MESSAGES,RECENT,UIDNEXT,UIDVALIDITY,UNSEEN] 
			command_imap_APPEND,
			command_imap_CHECK,
			command_imap_EXPUNGE,// ������� ��� ���������, ���������� ������ DELETED
			command_imap_SEARCH,// ����� ��������� �� �����-���� ���������
			command_imap_FETCH,// �������� ���������
			command_imap_STORE,// 
			command_imap_COPY,// ����������� ��������� �� ������ ����
			command_imap_UID,// ���������� ID ���������
			command_imap_CAPABILITY,// ����������� �������
			command_imap_NOOP// �������
		};
		struct Command_Entry
		{
			// �������� �������
			SMTP_COMMAND command;
			// �������� ��� ������ �� �������
			ssh_u valid_code;
			//  ��������� �� ������
			ssh_wcs	error;
		};
		enum CommandFlags
		{
			no_send		= 0x01,
			cont_resp	= 0x02,
			add_crlf	= 0x04,
			no_resp		= 0x08
		};
		enum SecurityType
		{
			stSimple,
			stTLS,
			stSSL
		};
		enum Protocol
		{
			_pop3,
			_imap,
			_smtp
		};
		struct MAIL_NAME
		{
			String name;
			String mail;
		};
		struct MAIL
		{
			MAIL() { attached.setID(310); }
			struct ATTACH
			{
				String name;
				String filename;
				String bits;
				Buffer<ssh_cs> obj;
			};
			MAIL_NAME sender;
			String subject;
			Time gmt_date;
			String xmailer;
			String body;
			String body_type;
			String xcmd;
			List<ATTACH*> attached;
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
		// ������� ������ ����� � ������������ � ������������ ���������(�����)
		bool pop3(const String& cmd, List<Mail::MAIL*>* lst, bool is_del);
		bool imap(const String& cmd, List<Mail::MAIL*>* lst, bool is_del);
	protected:
		// ������������ ������ �� base64 � �������������� ���������
		String decode_string(String charset, const String& base64, bool is_base64 = true);
		// ������ ������
		MAIL* parse_mail(const String& mail, const String& x, MAIL* m, bool is_body);
		String cnv_rfc(const String& str);
		// �������� �� �������� �����
		bool check_keyword(ssh_wcs keyword);
		// �������� �� ���������� ����� �����
		bool is_correct_mail(const String& mail);
		// ����������� ��������� ��� ����� ������
		String headers(const String& subject, bool is_html, bool is_notify);
		// ����������� �������
		void say_hello();
		// �����
		void say_quit();
		// ������ TLS
		void start_tls();
		// ������� � ��������� ��������
		void connect_pop3();
		void connect_imap();
		void connect_smtp();
		// �������� ������
		void send_cmd(ssh_u command, ssh_wcs fmt, ssh_u flags = 0, ...);
		//void send_cmd(ssh_u command, const Buffer<ssh_cs>& base64, ssh_u flags = 0);
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
		List<String, SSH_TYPE> attach;
		// ������ ���������
		List<MAIL_NAME, SSH_TYPE> recipients;
		// ��� ���������
		int protocol;
		// �����������
		String caps;
		// ��� �������� � ����������� �����������
		regx rx;
	};
}