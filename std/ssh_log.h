
#pragma once

#include "ssh_str.h"
#include "ssh_sock.h"
#include "ssh_file.h"
#include "ssh_singl.h"
#include "ssh_mail.h"

#define SIGNAL_INTERRUPT		SIGINT
#define SIGNAL_INSTRUCTION		SIGILL
#define SIGNAL_FLOATING			SIGFPE
#define SIGNAL_FAULT			SIGSEGV
#define SIGNAL_TERMINATE		SIGTERM
#define SIGNAL_ABORT			SIGABRT
#define	UNHANDLED_EXCEPTION		0x8000
#define	TERMINATE_CALL			0x4000
#define	UNEXPECTED_CALL			0x2000
#define	PURE_CALL				0x1000
#define	SECURITY_ERROR			0x0800
#define	NEW_OPERATOR_ERROR		0x0400
#define	INVALID_PARAMETER_ERROR 0x0200

namespace ssh
{
	class SSH Exception
	{
	public:
		Exception() {}
		Exception(ssh_wcs func, ssh_wcs file, int line, ssh_wcs msg, ...);
		virtual ~Exception() {}
		virtual void add(ssh_wcs msg, ...) const;
	protected:
		String func, file, message;
		int line;
	};

	class SSH StackTrace
	{
		friend class Tracer;
	public:
		struct NodeTrace
		{
			SSH_NEW_DECL(NodeTrace, 256);
			NodeTrace() {}
			NodeTrace(const String& str, NodeTrace* n) : next(n), value(str) {}
			~NodeTrace() { SSH_DEL(next); }
			// значение
			String value;
			// следующий
			NodeTrace* next;
		};
		StackTrace() : cdepth(0), depth(512), indent(0), root(nullptr), last(nullptr) { }
		~StackTrace() { clear(); }
		// инициализировать трассировщик
		void init(ssh_u _depth) { depth = _depth; cdepth = 0; }
		// добавить новый элемент
		void add(bool is, ssh_wcs func, ssh_wcs file, int line);
		// вывести в лог
		void output();
		// установка признака отключени€
		void start() { StackTrace::is_disabled = false; }
		void stop() { StackTrace::is_disabled = true; }
		// признак отключение
		static bool is_disabled;
	protected:
		void clear()
		{
			auto m(NodeTrace::get_MemArrayNodeTrace());
			if(m->Valid())
			{
				SSH_DEL(root);
				m->Reset();
			}
		}
		void remove_node()
		{
			// удал€ем узел из списка
			auto n(root->next);
			root->next = nullptr;
			delete root;
			root->next = n;
		}
		void add_node(const String& str)
		{
			auto n(new NodeTrace(str, nullptr));
			if(root) last->next = n; else root = n;
			last = n;
		}
		// макс глубина и текуща€ глубина
		ssh_u depth, cdepth;
		// отступ
		ssh_l indent;
		// корень списка
		NodeTrace* root;
		// последний узел списка
		NodeTrace* last;
	};

	class SSH Log
	{
		friend class Singlton < Log >;
		friend void socket_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
	public:
		enum TypeMessage : int
		{
			mInfo		= 0,// информаци€
			mAssert		= 1,// проверка
			mException	= 2,// исключение
			mTracer		= 3 // трасировщмк
		};
		enum class TypeOutput : int
		{
			Null	= 0,
			Screen	= 1,// на экран
			File	= 2,// в файл
			Debug	= 3,// в поток отладчика
			Mail	= 4,// отправл€ть на почту
			Net		= 5 // отправить по сети на сервер
		};
		struct SSH LOG
		{
			// формат шаблонов:
			// $fn - файл
			// $ln - строка
			// $fl - файл
			// $ms - сообщение
			// $tm - врем€
			// $dt - дата длинна€ день мес€ц год день недели
			// $us - им€ пользовател€
			// $cm - им€ компьютера
			// $nm - им€ программы
			// $tp - тип сообщени€
			// $DT - дата коротка€ день.мес€ц.год
			// $

			LOG();
			TypeOutput _out;		// тип вывода
			String string_begin;	// заголовок
			String string_finish;	// завершение
			String string_continue;	// продолжение(дл€ почты)
			String screen_template; // шаблон дл€ вывода на экран
			String debug_template;	// шаблон дл€ вывода в поток отладчика
			String email_address;	// адрес почты
			String email_host;		// хост почты и порт
			String email_template;	// шаблон письма почты
			String email_subject;	// тема письма
			String email_login;		//
			String email_pass;		//
			bool email_blocked;		// статус почты
			int email_flags;		// флаги создани€ почты
			int email_max_msgs;		// максимальное количество сообщений на письмо
			int email_count_msgs;	// текущее количество сообщений дл€ письма
			String file_path;		// путь к файлу
			String file_template;	// шаблон файла
			int file_flags;			// флаги файла
			String host_name;		// host:port дл€ отправки по сети
			String host_cert;		// сертификат
			String host_pwd_cert;	// пароль к сертификату
			String host_template;	// шаблон дл€ хоста
			int host_flags;			// флаги дл€ создани€ сокета
			String trace_template;	// шаблон дл€ трассировщика
		};
		// добавить сообщение
		virtual void add(TypeMessage type, ssh_wcs func, ssh_wcs file, int line, ssh_wcs msg, ...);
		// инициализаци€
		virtual void init(LOG* lg);
		// добавить трассировку стека
		void add(const String& msg);
		// проверить на блокировку сообщений почты
		bool is_email_blocked() const { return  _log.email_blocked; }
		// вернуть трейсер
		StackTrace* get_tracer() { return &tracer; }
	protected:
		// конструктор
		Log() {}
		// деструктор
		virtual ~Log() { close(); }
		// отправка сообщени€ серверу
		void sendSocket(const String& msg);
		// применение шаблона к сообщению
		String apply_template(ssh_wcs fn, ssh_wcs fl, int ln, int tp, ssh_wcs msg, String templ) const;
		// закрыть
		virtual void close();
		// отправить почту
		void send_email(const String& ln);
		// структру описани€
		LOG _log;
		// сокет
		Socket sock;
		// файл
		File file;
		// почта
		Mail mail;
		// хэндл событи€
		HANDLE hEventSocket;
		// трейсер
		StackTrace tracer;
		// индекс синглтона
		static const ssh_u singl_idx = SSH_SINGL_LOG;
	};

	class SSH Section
	{
	public:
		Section() { EnterCriticalSection(get_section()); }
		~Section() { LeaveCriticalSection(get_section()); }
	protected:
		static CRITICAL_SECTION* get_section()
		{
			static CRITICAL_SECTION section;
			static bool init(false);
			if(!init)
			{
				InitializeCriticalSection(&section);
				init = true;
			}
			return &section;
		}
	};

	#define log		Singlton<Log>::Instance()

	class SSH Tracer
	{
	public:
		Tracer(ssh_wcs _fn, ssh_wcs _fl, int _ln) : ln(_ln), fl(_fl), fn(_fn)
		{
			if(!StackTrace::is_disabled) log->get_tracer()->add(true, _fn, _fl, _ln);
		}
		~Tracer()
		{
			if(!StackTrace::is_disabled) log->get_tracer()->add(false, fn, fl, ln);
		}
	protected:
		//Section sect;
		int ln;
		ssh_wcs fl;
		ssh_wcs fn;
	};
}
