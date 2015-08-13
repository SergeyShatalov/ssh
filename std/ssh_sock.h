
#pragma once

#include "ssh_str.h"
#include "ssh_buf.h"

namespace ssh
{
	class SSH Socket
	{
		friend DWORD WINAPI proc_socket(void* arg);
		Socket(const Socket& sock);
	public:
		enum Status : int
		{
			// ������
			METHOD_SSLv23	= 0,
			METHOD_SSLv2	= 1,
			METHOD_SSLv3	= 2,
			METHOD_TLSv1	= 3,
			METHOD_TLSv11	= 4,
			METHOD_TLSv12	= 5,
			METHOD_DTLS		= 6,
			METHOD_DTLSv1	= 7,
			METHOD_DTLSv12	= 8,
			// ����� �������
			STARTTLS		= 16, // ������� ������� TLS
			SERVER			= 32, // ������
			THREAD			= 64, // ����� �������
			OPENSSL			= 128,// SSL ����������
			OPENTLS			= 256,// TLS ����������
			BLOCKED			= 512,// �������������
			CERTIFICATED	= 1024,// ����������
			CONNECT			= 2048
		};
		enum AccessFD
		{
			fd_read		= 1,
			fd_write	= 2,
			fd_except	= 4,
			fd_all		= 7
		};
		struct SSH VERIFY_CERT
		{
			int verbose_mode;
			int verify_depth;
			int always_continue;
		};
		struct SSH SOCK
		{
			SOCK() : ssl(nullptr), h(0) { }
			~SOCK() { close(); }
			void init(SSL_CTX* ctx)
			{
				ssl = SSL_new(ctx);
				SSL_set_fd(ssl, (int)h);
			}
			bool is_wouldblock(int err) const
			{
				err = SSL_get_error(ssl, err);
				bool is(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
				if(is) Sleep(50);
				return is;
			}
			bool connect()
			{
				int err;
				while((err = SSL_connect(ssl)) != 1)
				{
					if(!is_wouldblock(err)) return false;
				}
				return true;
			}
			bool accept()
			{
				int err;
				while((err = SSL_accept(ssl)) != 1)
				{
					if(!is_wouldblock(err)) return false;
				}
				return true;
			}
			void close()
			{
				if(h)
				{
					if(ssl)
					{
						SSL_shutdown(ssl);
						SSL_free(ssl);
						ssl = nullptr;
					}
					shutdown(h, SD_BOTH);
					closesocket(h);
					h = 0;
				}
			}
			SOCKET h;
			sockaddr_in addr;
			SSL* ssl;
		};
		// ����������� ��� ������� ��������� ������
		typedef void(*_receive)(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
		typedef void(*_connect)(Socket* sock, Socket::SOCK* s);
		typedef void(*_close)(Socket* sock, Socket::SOCK* s);
		typedef void(*_accept)(Socket* sock, Socket::SOCK* s);
		// �����������
		Socket() : hThread(0), status(0), sslCtx(nullptr), flags(0), m_close(nullptr), m_receive(nullptr), m_accept(nullptr), m_connect(nullptr), user_data(nullptr) { }
		// ���������������� �����������
		Socket(_receive rec, _connect con, _close cls, _accept acc, void* user) : hThread(0), status(0), sslCtx(nullptr), flags(0), m_close(cls), m_receive(rec), m_accept(acc), m_connect(con), user_data(user) {}
		// ����������
		virtual ~Socket() { close(); }
		// ������
		virtual void init(const String& host, int max_clients, int flags, ssh_wcs cert = nullptr, ssh_wcs pwd = nullptr);
		// ��������
		virtual void close(SOCK* s = nullptr);
		// �������� ������
		virtual bool send(const SOCK* s, const String& str) { return send(s, Buffer<ssh_cs>(str)); }
		virtual bool send(const SOCK* s, const Buffer<ssh_cs>& buf) { return send(s, buf, buf.count()); }
		virtual bool send(const SOCK* s, ssh_cs* buf, ssh_u sz);
		// ����� ������� ��������� ������
		void resetCallbacks();
		// ��������� ������� ��������� ������
		void setCallbacks(_receive rec, _connect con, _close cls, _accept acc, void* user_data = nullptr);
		// ������� �������������
		int is_closed() const { return !(status & THREAD); }
		// ������� �������
		int is_server() const { return (status & SERVER); }
		// ������� ������� TLS
		int is_start_tls() const { return (status & STARTTLS); }
		// ������� ���������������� ������
		template<typename T> T get_user_data() { return (T)user_data; }
		// ������ TLS
		void startTLS();
		// ������� ��������� ������ �� �������
		const SOCK* get_sock(ssh_u idx) { return &socks[idx]; }
	protected:
		// ��� ������ ������
		_receive m_receive;
		// ��� �������� � ��������
		_connect m_connect;
		// ��� �������� ������
		_close m_close;
		// ��� �������� �������
		_accept m_accept;
		// ������� ����� ����������
		const SSL_METHOD* initMethod();
		// ������������� ��������� SSL
		bool initCTX(const SSL_METHOD* meth);
		// �������� �� �������� ��� ��������� � �������
		bool is_wouldblock(SSL* ssl, int err);
		// �������� �����������
		virtual bool checkCert(SOCK* s);
		// ������� �������
		virtual bool accept();
		// ������� � ��������
		virtual bool connect(SOCK* s);
		// ����� ������
		virtual bool receive(SOCK* s);
		// �������� SSL
		SSL_CTX* sslCtx = nullptr;
		// feed� select
		fd_set fds;
		// ������� ��������
		SOCK socks[32];
		// ������ ������
		int status = 0;
		// ����� ������ ������
		HANDLE hThread = 0;
		// ��������� �������� �����������
		VERIFY_CERT verify;
		// ��������� ������������� ���������� �������
		WSADATA wsa;
		// ���� � SSL �����������
		String ssl_cert;
		// ������ � SSL �����������
		String ssl_pwd_cert;
		// ����� ������������� ������
		ssh_u flags = 0;
		// ���������������� ������
		void* user_data = nullptr;
		// ������� �������� ������ � ������
		int fd_flags = 0;
	};
}
