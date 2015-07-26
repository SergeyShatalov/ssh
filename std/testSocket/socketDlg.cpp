
// socketDlg.cpp : файл реализации
//

#include "stdafx.h"
#include "socket.h"
#include "socketDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static CsocketDlg* dlg = nullptr;

void receive(Socket* sock, Socket::SOCK* s, const Buffer<ssh_cs>& buf)
{
	dlg->m_msgs.AddString(CString(String(buf.to<ssh_ws>(), buf.count() / 2)));
	if(sock->is_server()) dlg->sock.response(s, String(L"OK"));
}

void close(Socket* sock, Socket::SOCK* s)
{
	if(dlg->m_server.GetCheck())
	{
		int sel = dlg->m_clients.FindString(-1, CString((ssh_wcs)String((ssh_u)s->h, String::_dec)));
		dlg->m_clients.DeleteString(sel);
		dlg->m_clients.SetCurSel(sel - 1);
		if(dlg->m_clients.GetCount() == 1)
			dlg->stop(false);
	}
	else
	{
		dlg->stop(true);
	}
}

void accept(Socket* sock, Socket::SOCK* s)
{
	dlg->m_clients.SetCurSel(dlg->m_clients.AddString(CString((ssh_wcs)String(s->h, String::_dec))));
	dlg->start(false);
}

void connect(Socket* sock, Socket::SOCK* s)
{
	dlg->start(true);
}

CsocketDlg::CsocketDlg(CWnd* pParent /*=NULL*/) : CDialogEx(CsocketDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	hostname = L"127.0.0.1";
	hostport = L"11111";
	dlg = this;
}

void CsocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVER, m_server);
	DDX_Control(pDX, IDC_HOSTNAME, m_hostname);
	DDX_Control(pDX, IDC_HOSTPORT, m_hostport);
	DDX_Control(pDX, IDC_SENDMSG, m_sendmsg);
	DDX_Control(pDX, IDC_RCPT, m_clients);
	DDX_Control(pDX, IDC_MESSAGES, m_msgs);
	DDX_Control(pDX, IDC_START, m_start);
	DDX_Control(pDX, IDC_STOP, m_stop);
	DDX_Control(pDX, IDC_SENDER, m_send);
}

BEGIN_MESSAGE_MAP(CsocketDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SENDER, &CsocketDlg::OnBnClickedSender)
	ON_BN_CLICKED(IDC_START, &CsocketDlg::OnBnClickedStart)
	ON_BN_CLICKED(IDC_STOP, &CsocketDlg::OnBnClickedStop)
	ON_BN_CLICKED(IDC_SERVER, &CsocketDlg::OnBnClickedServer)
	ON_EN_CHANGE(IDC_HOSTPORT, &CsocketDlg::OnEnChangeHostport)
	ON_EN_CHANGE(IDC_HOSTNAME, &CsocketDlg::OnEnChangeHostname)
	ON_EN_CHANGE(IDC_SENDMSG, &CsocketDlg::OnEnChangeSendmsg)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// обработчики сообщений CsocketDlg

BOOL CsocketDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	sock.setCallbacks(receive, connect, close, accept);
	SetIcon(m_hIcon, TRUE);			// Крупный значок
	SetIcon(m_hIcon, FALSE);		// Мелкий значок
	m_hostname.SetWindowTextW(hostname);
	m_hostport.SetWindowTextW(hostport);

	return TRUE;  // возврат значения TRUE, если фокус не передан элементу управления
}

void CsocketDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CsocketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CsocketDlg::OnBnClickedSender()
{
	sock.send(m_server.GetCheck() ? m_clients.GetCurSel() : 0, String(sendmsg));
	sendmsg.Empty();
	m_sendmsg.SetWindowTextW(sendmsg);
}

void CsocketDlg::OnBnClickedStart()
{
	sock.init(String(hostname + L':' + hostport), 5, (m_server.GetCheck() * Socket::SERVER), m_server.GetCheck() ? L"c:\\ca.pem" : L"c:\\client.pem", L"");
}

void CsocketDlg::OnBnClickedStop()
{
	stop(true);
	sock.close();
}

void CsocketDlg::OnBnClickedServer()
{
	m_hostname.EnableWindow(!m_server.GetCheck());
}

void CsocketDlg::OnEnChangeHostport()
{
	m_hostport.GetWindowTextW(hostport);
	m_start.EnableWindow(!hostname.IsEmpty() && !hostport.IsEmpty());
}

void CsocketDlg::OnEnChangeHostname()
{
	m_hostname.GetWindowTextW(hostname);
	m_start.EnableWindow(!hostname.IsEmpty() && !hostport.IsEmpty());
}

void CsocketDlg::OnEnChangeSendmsg()
{
	m_sendmsg.GetWindowTextW(sendmsg);
	m_send.EnableWindow(!sendmsg.IsEmpty());
}

void CsocketDlg::stop(bool is_start)
{
	if(is_start)
	{
		m_stop.EnableWindow(FALSE);
		m_start.EnableWindow(TRUE);
	}
	m_clients.EnableWindow(m_server.GetCheck());
	m_clients.ResetContent();
	m_clients.AddString(L"Всем");
	m_clients.SetCurSel(0);
	m_sendmsg.EnableWindow(FALSE);
	m_send.EnableWindow(FALSE);
	m_server.EnableWindow(TRUE);
	m_hostname.EnableWindow(!m_server.GetCheck());
	m_hostport.EnableWindow(TRUE);
	//m_msgs.ResetContent();
	sendmsg.Empty();
	m_sendmsg.SetWindowTextW(sendmsg);
}

void CsocketDlg::OnClose()
{
	sock.resetCallbacks();
	CDialogEx::OnClose();
}

void CsocketDlg::start(bool is)
{
	m_start.EnableWindow(FALSE);
	m_stop.EnableWindow(TRUE);
	m_sendmsg.EnableWindow(TRUE);
	m_server.EnableWindow(FALSE);
	m_hostname.EnableWindow(FALSE);
	m_hostport.EnableWindow(FALSE);
	m_clients.EnableWindow(m_server.GetCheck());
	if(is)
	{
		m_clients.ResetContent();
		m_clients.AddString(L"Всем");
		m_clients.SetCurSel(0);
	}
}
