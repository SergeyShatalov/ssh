
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
			// ��������
			String value;
			// ���������
			NodeTrace* next;
		};
		StackTrace() : cdepth(0), depth(512), indent(0), root(nullptr), last(nullptr) { }
		~StackTrace() { clear(); }
		// ���������������� ������������
		void init(ssh_u _depth) { depth = _depth; cdepth = 0; }
		// �������� ����� �������
		void add(bool is, ssh_wcs func, ssh_wcs file, int line);
		// ������� � ���
		void output();
		// ��������� �������� ����������
		void start() { StackTrace::is_disabled = false; }
		void stop() { StackTrace::is_disabled = true; }
		// ������� ����������
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
			// ������� ���� �� ������
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
		// ���� ������� � ������� �������
		ssh_u depth, cdepth;
		// ������
		ssh_l indent;
		// ������ ������
		NodeTrace* root;
		// ��������� ���� ������
		NodeTrace* last;
	};

	class SSH Log
	{
		friend class Singlton < Log >;
		friend void socket_receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
	public:
		enum TypeMessage : int
		{
			mInfo		= 0,// ����������
			mAssert		= 1,// ��������
			mException	= 2,// ����������
			mTracer		= 3 // �����������
		};
		enum class TypeOutput : int
		{
			Null	= 0,
			Screen	= 1,// �� �����
			File	= 2,// � ����
			Debug	= 3,// � ����� ���������
			Mail	= 4,// ���������� �� �����
			Net		= 5 // ��������� �� ���� �� ������
		};
		struct SSH LOG
		{
			// ������ ��������:
			// $fn - ����
			// $ln - ������
			// $fl - ����
			// $ms - ���������
			// $tm - �����
			// $dt - ���� ������� ���� ����� ��� ���� ������
			// $us - ��� ������������
			// $cm - ��� ����������
			// $nm - ��� ���������
			// $tp - ��� ���������
			// $DT - ���� �������� ����.�����.���
			// $

			LOG();
			TypeOutput _out;		// ��� ������
			String string_begin;	// ���������
			String string_finish;	// ����������
			String string_continue;	// �����������(��� �����)
			String screen_template; // ������ ��� ������ �� �����
			String debug_template;	// ������ ��� ������ � ����� ���������
			String email_address;	// ����� �����
			String email_host;		// ���� ����� � ����
			String email_template;	// ������ ������ �����
			String email_subject;	// ���� ������
			String email_login;		//
			String email_pass;		//
			bool email_blocked;		// ������ �����
			int email_flags;		// ����� �������� �����
			int email_max_msgs;		// ������������ ���������� ��������� �� ������
			int email_count_msgs;	// ������� ���������� ��������� ��� ������
			String file_path;		// ���� � �����
			String file_template;	// ������ �����
			int file_flags;			// ����� �����
			String host_name;		// host:port ��� �������� �� ����
			String host_cert;		// ����������
			String host_pwd_cert;	// ������ � �����������
			String host_template;	// ������ ��� �����
			int host_flags;			// ����� ��� �������� ������
			String trace_template;	// ������ ��� �������������
		};
		// �������� ���������
		virtual void add(TypeMessage type, ssh_wcs func, ssh_wcs file, int line, ssh_wcs msg, ...);
		// �������������
		virtual void init(LOG* lg);
		// �������� ����������� �����
		void add(const String& msg);
		// ��������� �� ���������� ��������� �����
		bool is_email_blocked() const { return  _log.email_blocked; }
		// ������� �������
		StackTrace* get_tracer() { return &tracer; }
	protected:
		// �����������
		Log() {}
		// ����������
		virtual ~Log() { close(); }
		// �������� ��������� �������
		void sendSocket(const String& msg);
		// ���������� ������� � ���������
		String apply_template(ssh_wcs fn, ssh_wcs fl, int ln, int tp, ssh_wcs msg, String templ) const;
		// �������
		virtual void close();
		// ��������� �����
		void send_email(const String& ln);
		// �������� ��������
		LOG _log;
		// �����
		Socket sock;
		// ����
		File file;
		// �����
		Mail mail;
		// ����� �������
		HANDLE hEventSocket;
		// �������
		StackTrace tracer;
		// ������ ���������
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
