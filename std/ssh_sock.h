
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
			// методы
			METHOD_SSLv23	= 0,
			METHOD_SSLv2	= 1,
			METHOD_SSLv3	= 2,
			METHOD_TLSv1	= 3,
			METHOD_TLSv11	= 4,
			METHOD_TLSv12	= 5,
			METHOD_DTLS		= 6,
			METHOD_DTLSv1	= 7,
			METHOD_DTLSv12	= 8,
			// флаги статуса
			STARTTLS		= 16, // признак запуска TLS
			SERVER			= 32, // сервер
			THREAD			= 64, // поток запущен
			OPENSSL			= 128,// SSL шифрование
			OPENTLS			= 256,// TLS шифрование
			BLOCKED			= 512,// блокированный
			CERTIFICATED	= 1024,// сертификат
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
		// определения для функций обратного вызова
		typedef void(*_receive)(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf);
		typedef void(*_connect)(Socket* sock, Socket::SOCK* s);
		typedef void(*_close)(Socket* sock, Socket::SOCK* s);
		typedef void(*_accept)(Socket* sock, Socket::SOCK* s);
		// конструктор
		Socket() : hThread(0), status(0), sslCtx(nullptr), flags(0), m_close(nullptr), m_receive(nullptr), m_accept(nullptr), m_connect(nullptr), user_data(nullptr) { }
		// инициализирующий конструктор
		Socket(_receive rec, _connect con, _close cls, _accept acc, void* user) : hThread(0), status(0), sslCtx(nullptr), flags(0), m_close(cls), m_receive(rec), m_accept(acc), m_connect(con), user_data(user) {}
		// деструктор
		virtual ~Socket() { close(); }
		// запуск
		virtual void init(const String& host, int max_clients, int flags, ssh_wcs cert = nullptr, ssh_wcs pwd = nullptr);
		// закрытие
		virtual void close(SOCK* s = nullptr);
		// отправка данных
		virtual bool send(const SOCK* s, const String& str) { return send(s, Buffer<ssh_cs>(str)); }
		virtual bool send(const SOCK* s, const Buffer<ssh_cs>& buf) { return send(s, buf, buf.count()); }
		virtual bool send(const SOCK* s, ssh_cs* buf, ssh_u sz);
		// сброс функций обратного вызова
		void resetCallbacks();
		// установка функций обратного вызова
		void setCallbacks(_receive rec, _connect con, _close cls, _accept acc, void* user_data = nullptr);
		// признак недоступности
		int is_closed() const { return !(status & THREAD); }
		// признак сервера
		int is_server() const { return (status & SERVER); }
		// признак запуска TLS
		int is_start_tls() const { return (status & STARTTLS); }
		// вернуть пользовательские данные
		template<typename T> T get_user_data() { return (T)user_data; }
		// запуск TLS
		void startTLS();
		// вернуть структуру сокета по индексу
		const SOCK* get_sock(ssh_u idx) { return &socks[idx]; }
	protected:
		// при приеме данных
		_receive m_receive;
		// при коннекте с сервером
		_connect m_connect;
		// при закрытии сокета
		_close m_close;
		// при коннекте клиента
		_accept m_accept;
		// вернуть метод шифрования
		const SSL_METHOD* initMethod();
		// инициализация контекста SSL
		bool initCTX(const SSL_METHOD* meth);
		// проверка на ожидание при операциях с сокетом
		bool is_wouldblock(SSL* ssl, int err);
		// проверка сертификата
		virtual bool checkCert(SOCK* s);
		// коннект клиента
		virtual bool accept();
		// коннект с сервером
		virtual bool connect(SOCK* s);
		// прием данных
		virtual bool receive(SOCK* s);
		// контекст SSL
		SSL_CTX* sslCtx = nullptr;
		// feedы select
		fd_set fds;
		// клиенты серверка
		SOCK socks[32];
		// статус сокета
		int status = 0;
		// хэндл потока сокета
		HANDLE hThread = 0;
		// структура проверки сертификата
		VERIFY_CERT verify;
		// структура инициализации библиотеки сокетов
		WSADATA wsa;
		// путь к SSL сертификату
		String ssl_cert;
		// пароль к SSL сертификату
		String ssl_pwd_cert;
		// флаги инициализации сокета
		ssh_u flags = 0;
		// пользовательские данные
		void* user_data = nullptr;
		// признак пропуска записи и ошибок
		int fd_flags = 0;
	};
}
