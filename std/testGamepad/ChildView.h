
// ChildView.h : ��������� ������ CChildView
//


#pragma once


// ���� CChildView

class CChildView : public CWnd
{
// ��������
public:
	CChildView();

// ��������
public:

// ��������
public:

// ���������������
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
// ����������
public:
	virtual ~CChildView();
	long x, y;
	long xx, yy;
	long sizex, sizey;
	long sizexx, sizeyy;
	long styles0, styles;
	COLORREF color0, color;
	bool is_cross1, is_cross2, is_update;
	// ��������� ������� ����� ���������
protected:
	void draw_figure(CPaintDC* dc, bool is_cross1, bool is_cross2, long sizex, long sizey, long x, long y);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	CPen white_pen, black_pen;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};

