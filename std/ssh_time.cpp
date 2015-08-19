
#include "stdafx.h"
#include "ssh_time.h"

namespace ssh
{
	static ssh_wcs m_day_of_week_big[] = {L"�����������", L"�������", L"�����", L"�������", L"�������", L"�������", L"�����������"};
	static ssh_wcs m_day_of_week_small[] = {L"��", L"��", L"��", L"��", L"��", L"��", L"��"};
	static ssh_wcs m_month_big[] = {L"������", L"�������", L"����", L"������", L"���", L"����", L"����", L"������", L"��������", L"�������", L"������", L"�������"};
	static ssh_wcs m_month_big_[] = {L"������", L"�������", L"�����", L"������", L"���", L"����", L"����", L"�������", L"��������", L"�������", L"������", L"�������"};
	static ssh_wcs m_month_big_smail[] = {L"��������", L"��������", L"�������", L"�������", L"�����", L"�������", L"������", L"��������", L"���������", L"��������", L"�������", L"�������"};
	static ssh_wcs m_month_big_smail_[] = {L"��������", L"��������", L"��������", L"�������", L"�����", L"�������", L"������", L"��������", L"���������", L"��������", L"�������", L"�������"};
	static ssh_wcs m_month_small[] = {L"���", L"���", L"���", L"���", L"���", L"���", L"���", L"���", L"���", L"���", L"���", L"���"};

	String Time::fmt(ssh_wcs str) const
	{
		String result(str), tmp;
		static ssh_wcs to[] = {L"$MN)+", L"$MN)", L"$MN+", L"$MN", L"$Mn", L"$m", L"$Y", L"$y", L"$dw", L"$dW", L"$dy", L"$d", L"$h", L"$H", L"$s", L"$nw", L"$nm", nullptr};
		int _month(month() - 1), _year(year()), _dw(dayOfWeek()), _hour(hour());
		tmp.fmt(L"%s\1%s\1%s\1%s\1%s\1%02i\1%02i\1%02i\1%s\1%s\1%02i\1%02i\1%02i\1%02i%s\1%02i\1%02i\1%02i\1\1",
				m_month_big_smail_[_month], m_month_big_smail[_month], m_month_big_[_month], m_month_big[_month], m_month_small[_month],
				_month + 1, _year, _year % 100, m_day_of_week_small[_dw], m_day_of_week_big[_dw], dayOfYear(), day(), _hour,
				(_hour < 12 ? _hour : _hour - 12), (_hour < 12 ? L"pm" : L"am"), second(), weekOfYear(), minute());
		tmp.replace(L'\1', L'\0');
		return result.replace(to, tmp);
	}

	int	Time::dayOfWeek() const
	{
		static int arr_week_days[] = {6, 0, 1, 2, 3, 4, 5};
		return arr_week_days[local()->tm_wday];
	}

	Time::Time(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST)
	{
		struct tm atm;

		atm.tm_sec = nSec;
		atm.tm_min = nMin;
		atm.tm_hour = nHour;
		atm.tm_mday = nDay;
		atm.tm_mon = nMonth - 1;
		atm.tm_year = nYear - 1900;
		atm.tm_isdst = nDST;
		time = _mktime64(&atm);
	}

	Time::Time(const SYSTEMTIME& sysTime, int nDST)
	{
		Time timeT(sysTime.wYear < 1900 ? 0L : ((int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay, (int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond, nDST));
		*this = timeT;
	}

	Time::Time(const FILETIME& fileTime, int nDST)
	{
		FILETIME localTime;
		SYSTEMTIME sysTime;
		if(FileTimeToLocalFileTime(&fileTime, &localTime) && FileTimeToSystemTime(&localTime, &sysTime))
			*this = Time(sysTime, nDST);
	}

	struct tm* Time::local() const
	{
		static struct tm ptm;
		localtime_s(&ptm, &time);
		return &ptm;
	}

	struct tm* Time::gmt() const
	{
		static struct tm ptm;
		gmtime_s(&ptm, &time);
		return &ptm;
	}
	
	SYSTEMTIME Time::getAsSystemTime() const
	{
		struct tm* ptm(local());
		SYSTEMTIME timeDest;
		timeDest.wYear = (WORD)(1900 + ptm->tm_year);
		timeDest.wMonth = (WORD)(1 + ptm->tm_mon);
		timeDest.wDayOfWeek = (WORD)ptm->tm_wday;
		timeDest.wDay = (WORD)ptm->tm_mday;
		timeDest.wHour = (WORD)ptm->tm_hour;
		timeDest.wMinute = (WORD)ptm->tm_min;
		timeDest.wSecond = (WORD)ptm->tm_sec;
		timeDest.wMilliseconds = 0;
		return timeDest;
	}

	int Time::weekOfYear() const
	{
		// �������� 1 ������ �������� ����
		Time bt(year(), 1, 1, 0, 0, 0);
		// ��������� ������� ����� ������� ����� � ������� ����
		time_t wd(bt.getTime() - (bt.dayOfWeek() * 24 * 60 * 60));
		// ��������� ����� ������
		return (int)(((time - wd) / (7 * 24 * 60 * 60)) + 1);
	}

	Timer::Timer()
	{
		m_bTimerStopped = true;
		m_llQPFTicksPerSec = 0;

		m_llStopTime = 0;
		m_llLastElapsedTime = 0;
		m_llBaseTime = 0;
		// Use QueryPerformanceFrequency to get the frequency of the counter
		LARGE_INTEGER qwTicksPerSec = {0};
		QueryPerformanceFrequency(&qwTicksPerSec);
		m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
	}

	void Timer::reset()
	{
		LARGE_INTEGER qwTime(GetAdjustedCurrentTime());
		m_llBaseTime = qwTime.QuadPart;
		m_llLastElapsedTime = qwTime.QuadPart;
		m_llStopTime = 0;
		m_bTimerStopped = FALSE;
	}

	void Timer::start()
	{
		LARGE_INTEGER qwTime = { 0 };
		QueryPerformanceCounter( &qwTime );
		if( m_bTimerStopped ) m_llBaseTime += qwTime.QuadPart - m_llStopTime;
		m_llStopTime = 0;
		m_llLastElapsedTime = qwTime.QuadPart;
		m_bTimerStopped = FALSE;
	}

	void Timer::stop()
	{
		if( !m_bTimerStopped )
		{
			LARGE_INTEGER qwTime = { 0 };
			QueryPerformanceCounter( &qwTime );
			m_llStopTime = qwTime.QuadPart;
			m_llLastElapsedTime = qwTime.QuadPart;
			m_bTimerStopped = TRUE;
		}
	}

	double Timer::get_absolute_time()
	{
		LARGE_INTEGER qwTime = { 0 };
		QueryPerformanceCounter( &qwTime );
		return qwTime.QuadPart / ( double )m_llQPFTicksPerSec;
	}

	double Timer::get_time()
	{
		LARGE_INTEGER qwTime = GetAdjustedCurrentTime();
		return ( double )( qwTime.QuadPart - m_llBaseTime ) / ( double )m_llQPFTicksPerSec;
	}

	void Timer::get_time_values( double* pfTime, double* pfAbsoluteTime, float* pfElapsedTime )
	{
		LARGE_INTEGER qwTime = GetAdjustedCurrentTime();
		float fElapsedTime = ( float )( ( double )( qwTime.QuadPart - m_llLastElapsedTime ) / ( double ) m_llQPFTicksPerSec );
		m_llLastElapsedTime = qwTime.QuadPart;
		if( fElapsedTime < 0.0f ) fElapsedTime = 0.0f;
		*pfAbsoluteTime = qwTime.QuadPart / ( double )m_llQPFTicksPerSec;
		*pfTime = ( qwTime.QuadPart - m_llBaseTime ) / ( double )m_llQPFTicksPerSec;
		*pfElapsedTime = fElapsedTime;
	}

	float Timer::get_elapsed_time()
	{
		LARGE_INTEGER qwTime = GetAdjustedCurrentTime();
		double fElapsedTime = ( float )( ( double )( qwTime.QuadPart - m_llLastElapsedTime ) / ( double ) m_llQPFTicksPerSec );
		m_llLastElapsedTime = qwTime.QuadPart;
		if( fElapsedTime < 0.0f ) fElapsedTime = 0.0f;
		return ( float )fElapsedTime;
	}

	LARGE_INTEGER Timer::GetAdjustedCurrentTime()
	{
		LARGE_INTEGER qwTime;
		if( m_llStopTime != 0 ) qwTime.QuadPart = m_llStopTime; else QueryPerformanceCounter( &qwTime );
		return qwTime;
	}

	void Timer::LimitThreadAffinityToCurrentProc()
	{
		HANDLE hCurrentProcess = GetCurrentProcess();
		DWORD_PTR dwProcessAffinityMask = 0;
		DWORD_PTR dwSystemAffinityMask = 0;
		if( GetProcessAffinityMask( hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask ) != 0 && dwProcessAffinityMask )
		{
			DWORD_PTR dwAffinityMask = ( dwProcessAffinityMask & ( ( ~dwProcessAffinityMask ) + 1 ) );
			HANDLE hCurrentThread = GetCurrentThread();
			if( INVALID_HANDLE_VALUE != hCurrentThread )
			{
				SetThreadAffinityMask( hCurrentThread, dwAffinityMask );
				CloseHandle( hCurrentThread );
			}
		}
		CloseHandle( hCurrentProcess );
	}
}

