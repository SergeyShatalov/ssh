
#include "stdafx.h"
#include "ssh_sock.h"
#include "ssh_array.h"

namespace ssh
{
	static int err = 0;
	static int idx_verify;

	static int pem_passwd_cb(ssh_cs* buf, int size, int rwflag, void* userdata)
	{
		strncpy(buf, (ssh_cs*)(userdata), size);
		buf[size - 1] = '\0';
		return (int)strlen(buf);
	}

	static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
	{
		SSH_TRACE;
		ssh_cs buf_ansi[MAX_PATH];
		X509* err_cert(X509_STORE_CTX_get_current_cert(ctx));
		int err(X509_STORE_CTX_get_error(ctx));
		int depth(X509_STORE_CTX_get_error_depth(ctx));
		SSL* ssl((SSL*)X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx()));
		Socket::VERIFY_CERT* mydata((Socket::VERIFY_CERT*)SSL_get_ex_data(ssl, idx_verify));
		X509_NAME_oneline(X509_get_subject_name(err_cert), buf_ansi, 256);
		if(depth > mydata->verify_depth)
		{
			preverify_ok = 0;
			err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
			X509_STORE_CTX_set_error(ctx, err);
		}
		if(!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT))
			X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert), buf_ansi, 256);
		return (mydata->always_continue ? 1 : preverify_ok);
	}

	bool Socket::is_wouldblock(SSL* ssl, int err)
	{
		bool is;
		if(ssl)
		{
			err = SSL_get_error(ssl, err);
			is = (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE);
		}
		else is = (WSAGetLastError() == WSAEWOULDBLOCK);
		if(is) Sleep(50);
		return is;
	}

	DWORD WINAPI proc_socket(void* arg)
	{
		SSH_TRACE;
		int result;
		Socket* sock((Socket*)arg);
		struct timeval tv;
		tv.tv_sec = 60;
		tv.tv_usec = 0;
		while((sock->status & Socket::THREAD))
		{
			fd_set fd_read, fd_write, fd_exc;
			if(sock->fd_flags & Socket::fd_read) memcpy(&fd_read, &sock->fds, sizeof(fd_set));
			if(sock->fd_flags & Socket::fd_write) memcpy(&fd_write, &sock->fds, sizeof(fd_set));
			if(sock->fd_flags & Socket::fd_except) memcpy(&fd_exc, &sock->fds, sizeof(fd_set));
			if((result = select(0, &fd_read, &fd_write, &fd_exc, &tv)) == SOCKET_ERROR)
			{
				sock->close();
				break;
			}
			else if(result > 0)
			{
				for(ssh_u i = 0; i < sock->fds.fd_count; i++)
				{
					Socket::SOCK* cl(&sock->socks[i]);
					if(FD_ISSET(cl->h, &fd_read))
					{
						// если это сервер
						if((sock->status & Socket::SERVER))
						{
							// коннект клиента
							if(sock->accept()) continue;
						}
						// возможно прием данных?
						if(!sock->receive(cl))
						{
							// ошибка - значит сокет отвалилс€
							sock->close(cl);
						}
					}
					if((sock->status & Socket::CONNECT))
					{
						if(FD_ISSET(cl->h, &fd_write))
						{
							// коннект сработал
							if(!sock->connect(cl)) sock->close(cl);
							sock->status &= ~Socket::CONNECT;
							sock->fd_flags = (Socket::fd_read | Socket::fd_except);
						}
					}
					if(FD_ISSET(cl->h, &fd_exc))
					{
						// ошибка коннекта
						sock->close(cl);
						sock->status &= ~Socket::CONNECT;
						sock->fd_flags = Socket::fd_read;
					}
				}
			}
		}
		return 0;
	}

	void Socket::init(const String& host, int max_clients, int flgs, ssh_wcs cert, ssh_wcs pwd)
	{
		SSH_TRACE;
		ssh_l pos_dpt;
		if((pos_dpt = host.find_rev(L':')) < 0)
			SSH_THROW(L"Ќедопустимо задано им€ хоста (%s)!", host);
		if((status & THREAD))
			SSH_THROW(L"ќшибка. —окет уже запущен!");
		memset(socks, 0, sizeof(SOCK) * 32);
		FD_ZERO(&fds);
		// 1. инициализируем библиотеку
		if(WSAStartup(0x0202, &wsa) || wsa.wVersion != 0x0202)
			SSH_THROW(L"Ќе удалось инициализировать библиотеку winsock2.dll!");
		// 2. создаем неблокирующий сокет
		if((socks[0].h = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERROR)
			SSH_THROW(L"Ќе удалось создать сокет!");
		FD_SET(socks[0].h, &fds);
		DWORD dw = true;
		if(!(flgs & BLOCKED)) ioctlsocket(socks[0].h, FIONBIO, &dw);
		socks[0].addr.sin_family = AF_INET;
		socks[0].addr.sin_port = htons(host.toNum<ssh_w>(pos_dpt + 1, String::_dec));
		// 3. если сервер, то:
		if((flgs & SERVER))
		{
			socks[0].addr.sin_addr.s_addr = INADDR_ANY;
			if(bind(socks[0].h, (sockaddr*)&socks[0].addr, sizeof(sockaddr)) == SOCKET_ERROR)
				SSH_THROW(L"Ќе удалось забиндить сокет сервера!");
			if(listen(socks[0].h, max_clients) == SOCKET_ERROR)
				SSH_THROW(L"ќшибка прослушивани€ сокета сервера!");
			status |= SERVER;
			fd_flags = fd_read;
			if(m_connect) m_connect(this, nullptr);
		}
		else
		{
			ssh_d addr;
			// пробуем преобразовать в числовой IP-адрес из строкового
			Buffer<ssh_cs> buf(ssh_cnv(cp_ansi, host.left(pos_dpt), true));
			if((addr = inet_addr(buf)) == INADDR_NONE)
			{
				hostent* HE;
				// не IP-адрес, может это им€ хоста?
				if(!(HE = gethostbyname(buf)))
					SSH_THROW(L"Ќе удалось определить адрес хоста сервера %S!", buf);
 				addr = *((u_long*)HE->h_addr_list[0]);
			}
			socks[0].addr.sin_addr.s_addr = addr;
			fd_flags = fd_all;
			if(::connect(socks[0].h, (sockaddr*)&socks[0].addr, sizeof(sockaddr)) == SOCKET_ERROR)
			{
				if(WSAGetLastError() != WSAEWOULDBLOCK)
					SSH_THROW(L"Ќе удалось законектитьс€ с сервером!");
				status |= CONNECT;
			}
			else
			{
				if(m_connect) m_connect(this, &socks[0]);
			}
		}
		// 4. запустить тред дл€ select
		status |= THREAD;
		if(!(hThread = CreateThread(nullptr, 0, proc_socket, (void*)this, 0, nullptr)))
			SSH_THROW(L"ќшибка при запуске потока сокета!");
		flags = flgs;
		ssl_cert = cert;
		ssl_pwd_cert = pwd;
	}

	void Socket::close(SOCK* s)
	{
		Section sect;
		if((status & THREAD))
		{
			SSH_TRACE;
			if(!s) s = &socks[0];
			if(s)
			{
				FD_CLR(s->h, &fds);
				if(m_close) m_close(this, s);
				s->close();
				// завершить поток и деинициализировать библиотеку, если это базовый сокет
				if(s == &socks[0])
				{
					if(sslCtx)
					{
						SSL_CTX_free(sslCtx);
						sslCtx = nullptr;
						EVP_cleanup();
						CRYPTO_cleanup_all_ex_data();
					}
					if(hThread) { CloseHandle(hThread); hThread = 0; }
					status = 0;
					WSACleanup();
				}
				memcpy(s, s + 1, (ssh_u)(&socks[31]) - (ssh_u)s);
			}
		}
	}

	void Socket::startTLS()
	{
		SSH_TRACE;
		if((status & (OPENTLS | THREAD)) && !(status & SERVER))
		{
			fd_flags = Socket::fd_write | Socket::fd_except;
			status |= (CONNECT | STARTTLS | OPENSSL);
		}
	}

	bool Socket::connect(SOCK* s)
	{
		SSH_TRACE;
		if(!initCTX(initMethod())) return false;
		if((status & OPENSSL))
		{
			s->init(sslCtx);
			if(!(s->connect()) || !(checkCert(s))) return false;
		}
		if(m_connect) m_connect(this, s);
		return true;
	}

	bool Socket::accept()
	{
		SSH_TRACE;
		for(ssh_u n = 1; n < 32; n++)
		{
			if(!socks[n].h)
			{
				int namelen(sizeof(sockaddr));
				SOCKET h;
				if((h = ::accept(socks[0].h, (sockaddr*)&socks[n].addr, &namelen)) == SOCKET_ERROR) return false;
				SOCK* s(&socks[n]);
				FD_SET(h, &fds);
				socks[n].h = h;
				if(!initCTX(initMethod())) return false;
				if((status & OPENSSL))
				{
					s->init(sslCtx);
					if(!s->accept()) return false;
					// проверка сертификата (если есть)
					if((status & CERTIFICATED))
					{
						SSL_set_ex_data(s->ssl, idx_verify, &verify);
						if(SSL_get_verify_result(s->ssl) != X509_V_OK) return false;
						if(!checkCert(s)) return false;
						SSL_set_verify_result(s->ssl, X509_V_OK);
					}
				}
				if(m_accept) m_accept(this, s);
				return true;
			}
		}
		return false;
	}

	bool Socket::receive(SOCK* s)
	{
		Buffer<ssh_cs> _recv;
		static ssh_cs _tmp[1024];
		ssh_u size(0);
		int _size(0);
		while(true)
		{
			err = (s->ssl ? SSL_read(s->ssl, _tmp + _size, 1024 - _size) : recv(s->h, _tmp + _size, 1024 - _size, 0));
			// 1. или ошибка, или ожидаем
			if(err == SOCKET_ERROR)
			{
				if(!_size && is_wouldblock(s->ssl, err)) continue;
				err = 0;
			}
			// 2. прин€ли данные
			_size += err;
			if(_size >= 1024 || (!err && _size))
			{
				Buffer<ssh_cs> buf(size + _size);
				memcpy(buf, _recv, size);
				memcpy(buf + size, _tmp, _size);
				size += _size;
				_size = 0;
				_recv = Buffer<ssh_cs>(buf, size);
			}
			if(!err) break;
		}
		// 3. если данные есть - сообщить клиенту
		if(size)
		{
			if(m_receive) m_receive(this, s, _recv);
			return true;
		}
		return false;
	}

	bool Socket::send(const SOCK* s, ssh_cs* buf, ssh_u size)
	{
		ssh_u pos(0);
		while(pos < size)
		{
			if((err = (s->ssl) ? SSL_write(s->ssl, buf + pos, (int)(size - pos)) : ::send(s->h, buf + pos, (int)(size - pos), 0)) <= 0)
			{
				if(is_wouldblock(s->ssl, err)) continue;
				return false;
			}
			pos += err;
		}
		return true;
	}

	const SSL_METHOD* Socket::initMethod()
	{
		SSH_TRACE;
		switch(flags & 15)
		{
			case METHOD_SSLv2:
				return (is_server() ? SSLv2_server_method() : SSLv2_client_method());
			case METHOD_SSLv3:
				return (is_server() ? SSLv3_server_method() : SSLv3_client_method());
			case METHOD_TLSv1:
				return (is_server() ? TLSv1_server_method() : TLSv1_client_method());
			case METHOD_TLSv11:
				return (is_server() ? TLSv1_1_server_method() : TLSv1_1_client_method());
			case METHOD_TLSv12:
				return (is_server() ? TLSv1_2_server_method() : TLSv1_2_client_method());
			case METHOD_DTLS:
				return (is_server() ? DTLS_server_method() : DTLS_client_method());
			case METHOD_DTLSv1:
				return (is_server() ? DTLSv1_server_method() : DTLSv1_client_method());
			case METHOD_DTLSv12:
				return (is_server() ? DTLSv1_2_server_method() : DTLSv1_2_client_method());
		}
		return (is_server() ? SSLv23_server_method() : SSLv23_client_method());
	}

	bool Socket::initCTX(const SSL_METHOD* meth)
	{
		SSH_TRACE;
		if(!(status & (OPENSSL | OPENTLS)) && (flags & (OPENSSL | OPENTLS)))
		{
			SSL_library_init();
			SSL_load_error_strings();

			if(!(sslCtx = SSL_CTX_new(meth))) return false;
			if(!ssl_pwd_cert.is_empty())
			{
				// если зашифрованный ключь
				SSL_CTX_set_default_passwd_cb_userdata(sslCtx, (void*)ssh_cnv(cp_ansi, ssl_pwd_cert, true));
				SSL_CTX_set_default_passwd_cb(sslCtx, pem_passwd_cb);
			}
			if(!ssl_cert.is_empty())
			{
				Buffer<ssh_cs> buf(ssh_cnv(cp_ansi, ssl_cert, true));
				if(SSL_CTX_load_verify_locations(sslCtx, buf, buf) != 1) return false;
				if(SSL_CTX_set_default_verify_paths(sslCtx) != 1) return false;
				if(SSL_CTX_use_certificate_file(sslCtx, buf, SSL_FILETYPE_PEM) != 1) return false;
				if(SSL_CTX_use_PrivateKey_file(sslCtx, buf, SSL_FILETYPE_PEM) != 1) return false;
				if(!SSL_CTX_check_private_key(sslCtx)) return false;
				if((status & SERVER))
				{
					idx_verify = SSL_get_ex_new_index(0, "mydata index", nullptr, nullptr, nullptr);
					SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, verify_callback);
					SSL_CTX_set_verify_depth(sslCtx, 11);
					verify.verify_depth = 10;
				}
				status |= CERTIFICATED;
			}
			status |= (flags & (OPENSSL | OPENTLS));
		}
		return true;
	}

	bool Socket::checkCert(SOCK* s)
	{
		SSH_TRACE;
		if((status & CERTIFICATED))
		{
			X509* cert;
			if(!(cert = SSL_get_peer_certificate(s->ssl))) return false;
			X509_free(cert);
		}
		return true;
	}

	void Socket::setCallbacks(_receive rec, _connect con, _close cls, _accept acc, void* _user)
	{
		SSH_TRACE;
		m_connect = con;
		m_receive = rec;
		m_close = cls;
		m_accept = acc;
		user_data = _user;
	}

	void Socket::resetCallbacks()
	{
		SSH_TRACE;
		m_connect = nullptr;
		m_close = nullptr;
		m_receive = nullptr;
		m_accept = nullptr;
	}
}
