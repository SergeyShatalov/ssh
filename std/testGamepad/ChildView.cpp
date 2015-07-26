
// ChildView.cpp : реализация класса CChildView
//

#include "stdafx.h"
#include "testGamepad.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	xx = yy = x = y = 100;
	sizex = sizexx = 20;
	sizeyy = sizey = 20;
	styles0 = styles = 0;
	color0 = color = 0;
	is_cross1 = is_cross2 = false;
	is_update = true;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// обработчики сообщений CChildView

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	white_pen.CreatePen(PS_SOLID, 1, (COLORREF)RGB(255, 255, 255));
	black_pen.CreatePen(PS_SOLID, 1, color);
	return TRUE;
}

void CChildView::draw_figure(CPaintDC* dc, bool is_cross1, bool is_cross2, long sizex, long sizey, long x, long y)
{
	dc->MoveTo(x - sizex, y - sizey);
	dc->LineTo(x - sizex, y + sizey);
	dc->LineTo(x + sizex, y + sizey);
	dc->LineTo(x + sizex, y - sizey);
	dc->LineTo(x - sizex, y - sizey);
	if(is_cross1) dc->LineTo(x + sizex, y + sizey);
	if(is_cross2)
	{
		dc->MoveTo(x + sizex, y - sizey);
		dc->LineTo(x - sizex, y + sizey);
	}
}

void CChildView::OnPaint() 
{
	static bool is = false;
	if(!is)
	{
		SetTimer(1, 10, nullptr);
		is = true;
	}
	CPaintDC dc(this); // контекст устройства для рисования
	if(is_update)
	{
		// стереть
		is_update = false;
		dc.SelectObject(white_pen);
		draw_figure(&dc, true, true, sizexx, sizeyy, xx, yy);
		xx = x; yy = y;
		sizexx = sizex;
		sizeyy = sizey;
		if(styles0 != styles || color0 != color)
		{
			black_pen.DeleteObject();
			black_pen.CreatePen(styles & 3, 1, color);
			styles0 = styles;
			color0 = color;
		}
	}
	dc.SelectObject(black_pen);
	draw_figure(&dc, is_cross1, is_cross2, sizex, sizey, x, y);
}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: добавьте свой код обработчика сообщений или вызов стандартного
	gamepad->update();
	Timer* tm(Timer::get_timer());
	double ftime, abstime;
	float eltime;
	tm->get_time_values(&ftime, &abstime, &eltime);
	eltime *= 200.0f;
	if(gamepad->is_pressed(0, Gamepad::cross_down))  y += eltime;
	else if(gamepad->is_pressed(0, Gamepad::cross_up)) y -= eltime;
	if(gamepad->is_pressed(0, Gamepad::cross_left)) x -= eltime;
	else if(gamepad->is_pressed(0, Gamepad::cross_right)) x += eltime;
	if(x == xx && y == yy)
	{
		Range<float> fl(gamepad->get_stick(0, Gamepad::right));
		if(!fl.is_null())
		{
			x += fl.w * eltime;
			y += fl.h * eltime;
		}
	}
	Range<float> fr(gamepad->get_stick(0, Gamepad::left));
	if(!fr.is_null())
	{
		sizex += fr.w * eltime;
		sizex = SSH_CLAMP(sizex, 10, 200);
		sizey += fr.h * eltime;
		sizey = SSH_CLAMP(sizey, 10, 200);
	}
	bool is1(is_cross1), is2(is_cross2);
	if(gamepad->is_pressed(0, Gamepad::a))  styles = 0;
	else if(gamepad->is_pressed(0, Gamepad::b))  styles = 1;
	else if(gamepad->is_pressed(0, Gamepad::x))  styles = 2;
	else if(gamepad->is_pressed(0, Gamepad::y))  styles = 3;
	if(gamepad->is_trigger(0, Gamepad::left))  color = RGB(0, 0, 255);
	else if(gamepad->is_trigger(0, Gamepad::right))  color = RGB(255, 0, 0);
	else if(gamepad->is_pressed(0, Gamepad::escape))  color = 0;
	if(gamepad->is_once_pressed(0, Gamepad::left_turn))  is_cross1 = !is_cross1;
	if(gamepad->is_once_pressed(0, Gamepad::right_turn))  is_cross2 = !is_cross2;

	if(is1 != is_cross1 || is2 != is_cross2 || color0 != color || styles != styles0 || sizex != sizexx || sizey != sizeyy || xx != x || yy != y)
	{
		is_update = true;
		Invalidate();
	}
	CWnd::OnTimer(nIDEvent);
}
