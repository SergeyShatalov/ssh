
// socketDlg.h : ���� ���������
//

#pragma once
#include "afxwin.h"

// ���������� ���� CsocketDlg
class CsocketDlg : public CDialogEx
{
// ��������
public:
	CsocketDlg(CWnd* pParent = NULL);	// ����������� �����������

// ������ ����������� ����
	enum { IDD = IDD_SOCKET_DIALOG };

	Socket sock;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// ��������� DDX/DDV


	// ����������
protected:
	HICON m_hIcon;
	// ��������� ������� ����� ���������
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	void stop(bool is_start);
	void start(bool is_start);
	CString hostname, hostport, sendmsg;
	afx_msg void OnLbnSelchangeMessages();
	CButton m_server;
	CEdit m_hostname;
	CEdit m_hostport;
	CEdit m_sendmsg;
	CComboBox m_clients;
	CListBox m_msgs;
	CButton m_start;
	CButton m_stop;
	CButton m_send;
	afx_msg void OnBnClickedSender();
	afx_msg void OnCbnSelchangeRcpt();
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedServer();
	afx_msg void OnEnChangeHostport();
	afx_msg void OnEnChangeHostname();
	afx_msg void OnEnChangeSendmsg();
	afx_msg void OnClose();
};
