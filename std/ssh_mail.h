
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
			command_pop_DELE,// номер сообщения
			command_pop_LIST,// [номер сообщения]
			command_pop_NOOP,//
			command_pop_RETR,// сообщение
			command_pop_RSET,//
			command_pop_STAT,//
			command_pop_TOP,  // сообщение, количество строк
			command_pop_STLS,
			command_pop_QUIT,
			command_pop_CAPA,
			command_pop_UIDL,
			// imap
			command_imap_INIT,
			command_imap_LOGIN,// аутентификация - user, pass
			command_imap_AUTHENTICATE, // безопасная аутентификация
			command_imap_CLOSE,// закрыть ящик - имя
			command_imap_LOGOUT,// завершение соединения
			command_imap_CREATE,// создать ящик - имя
			command_imap_DELETE,// удалить ящик - имя
			command_imap_RENAME,// переименовать ящик - старое имя, новое имя
			command_imap_SUBSCRIBE,// добавить ящик в список активных - имя
			command_imap_UNSUBSCRIBE,// убрать ящик из списка активных - имя
			command_imap_LIST,// получить список всех ящиков - -
			command_imap_LSUB,// получить список активных ящиков - -
			command_imap_STATUS,// вернуть состояние ящик - имя ящика, - список критериев [MESSAGES,RECENT,UIDNEXT,UIDVALIDITY,UNSEEN] 
			command_imap_APPEND,
			command_imap_CHECK,
			command_imap_EXPUNGE,// удалить все сообщение, помеченные флагом DELETED
			command_imap_SEARCH,// поиск сообщение по каким-либо критериям
			command_imap_FETCH,// получить сообщение
			command_imap_STORE,// 
			command_imap_COPY,// скопировать сообщение на другой ящик
			command_imap_UID,// уникальный ID сообщения
			command_imap_CAPABILITY,// возможности сервера
			command_imap_NOOP// простой
		};
		struct Command_Entry
		{
			// комманда серверу
			SMTP_COMMAND command;
			// валидный код ответа от сервера
			ssh_u valid_code;
			//  сообщение об ошибке
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
		// конструктор
		Mail();
		// инициализирующий конструктор
		Mail(const String& host, const String& login, const String& pass, int type) : Mail() { set_host(host, type); set_login(login, pass); }
		// деструктор
		virtual ~Mail();
		// установить логин пользователя
		void set_login(const String& _login, const String& _pass) { login = _login; pass = _pass; }
		// установить пароль
		void set_pass(const String& str) { pass = str; }
		// от кого - имя:почта
		void set_sender(const String& name, const String& mail) { sender = makeNameMail(name, mail); }
		// установить обратный путь
		void set_reply_to(const String& name, const String& mail) { reply_to = makeNameMail(name, mail); }
		// установить хост
		void set_host(const String& host, int type);
		// установить свое сообщение
		void set_message(const String& str) { x_msg = str; }
		// установить мой маркер
		void set_xserg(const String& cmd, const String& val) { x_ostrov.name = cmd; x_ostrov.mail = val; }
		// установить кодировку
		void set_charset(const String& str) { charset = str; }
		// добавить адресатов - имя:почта
		void add_recipient(const String& name, const String& mail) { recipients += makeNameMail(name, mail); }
		// добавить вложения - путь к файлу
		void add_attach(const String& file) { attach += file; }
		// установить значения по умолчанию
		void default(bool is_recipients, bool is_attach);
		// отправить письмо
		void smtp(const String& subject, const String& body, bool is_html = false, bool is_notify = false);
		// принять список писем в соответствии с определенной коммандой(тегом)
		bool pop3(const String& cmd, List<Mail::MAIL*>* lst, bool is_del);
		bool imap(const String& cmd, List<Mail::MAIL*>* lst, bool is_del);
	protected:
		// декодировать строку из base64 и соответсвующей кодировке
		String decode_string(String charset, const String& base64, bool is_base64 = true);
		// парсер письма
		MAIL* parse_mail(const String& mail, const String& x, MAIL* m, bool is_body);
		String cnv_rfc(const String& str);
		// проверка на ключевое слово
		bool check_keyword(ssh_wcs keyword);
		// проверка на корректный адрес почты
		bool is_correct_mail(const String& mail);
		// форматируем заголовок для блока данных
		String headers(const String& subject, bool is_html, bool is_notify);
		// приветствие серверу
		void say_hello();
		// выход
		void say_quit();
		// запуск TLS
		void start_tls();
		// коннект с удаленным сервером
		void connect_pop3();
		void connect_imap();
		void connect_smtp();
		// отправка данных
		void send_cmd(ssh_u command, ssh_wcs fmt, ssh_u flags = 0, ...);
		//void send_cmd(ssh_u command, const Buffer<ssh_cs>& base64, ssh_u flags = 0);
		// получить ответ
		void recv_resp(ssh_u command, ssh_u flags = 0);
		// формирование связки имя <адрес>
		MAIL_NAME makeNameMail(const String& name, const String& mail);
		// сокет
		Socket sock;
		// для формирования комманды
		String cmd;
		// событие ожидание
		HANDLE hEvent;
		// ответ
		String resp;
		// идентификатор вложений
		String msg_id;
		// хост:порт
		String host;
		// флаги создания сокета
		int sock_flags;
		// отправитель - имя:адрес
		MAIL_NAME sender;
		// логин
		String login;
		// пароль
		String pass;
		// свое сообщение
		String x_msg;
		// мой маркер
		MAIL_NAME x_ostrov;
		// обратный адрес - почта
		MAIL_NAME reply_to;
		// кодировка
		String charset;
		// список прикрепленных файлов
		List<String, SSH_TYPE> attach;
		// список адресатов
		List<MAIL_NAME, SSH_TYPE> recipients;
		// тип протокола
		int protocol;
		// возможности
		String caps;
		// для операций с регулярными выражениями
		regx rx;
	};
}