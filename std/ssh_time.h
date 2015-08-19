
#pragma once

#include "ssh_str.h"

namespace ssh
{
	class SSH Time
	{
		__time64_t time;
	public:
		Time() : time(0) {}
		Time(const time_t& t) { *this = t; }
		Time(const Time& t) { *this = t; }
		Time(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec, int nDST = -1);
		Time(const SYSTEMTIME& sysTime, int nDST = -1);
		Time(const FILETIME& fileTime, int nDST = -1);
		// Операции
		bool operator == (const Time& t) const { return (time == t.time); }
		bool operator != (const Time& t) const { return (time != t.time); }
		bool operator < (const Time& t) const { return (time < t.time); }
		bool operator > (const Time& t) const { return (time > t.time); }
		bool operator <= (const Time& t) const { return (time <= t.time); }
		bool operator >= (const Time& t) const { return (time >= t.time); }
		const Time& operator = (const Time& t) { time = t.time; return *this; }
		const Time& operator = (const time_t& t) { time = t; return *this; }
		// Атрибуты
		struct tm* local() const;
		struct tm* gmt() const;
		SYSTEMTIME getAsSystemTime() const;
		time_t	getTime() const { return time; }
		int	year() const { return local()->tm_year + 1900; }
		int	month() const { return local()->tm_mon + 1; }
		int	day() const { return local()->tm_mday; }
		int	hour() const { return local()->tm_hour; }
		int	minute() const { return local()->tm_min; }
		int	second() const { return local()->tm_sec; }
		int	dayOfWeek() const;
		int	dayOfYear() const { return local()->tm_yday + 1; }
		int	weekOfYear() const;
		String fmt(ssh_wcs str) const;
		static Time current() { return Time(::time(nullptr)); }
	};

	class SSH Timer
	{
	public:
		// конструктор
		Timer();
		// сброс
		void reset();
		// запуск
		void start();
		// завершение (или пауза)
		void stop();
		// advance the timer by 0.1 seconds
		void advance() { m_llStopTime += m_llQPFTicksPerSec / 10; }
		// вернуть абсолютное системнок время
		double get_absolute_time();
		// вернуть текущее время
		double get_time();
		// get the time that elapsed between Get*ElapsedTime() calls
		float get_elapsed_time();
		// вернуть единовременные значения времени
		void get_time_values(double* pfTime, double* pfAbsoluteTime, float* pfElapsedTime);
		// признак паузы (завершения)
		bool is_stopped() const { return m_bTimerStopped; }
		// Limit the current thread to one processor (the current one). This ensures that timing code runs
		// on only one processor, and will not suffer any ill effects from power management.
		void LimitThreadAffinityToCurrentProc();
		static Timer* get_timer() { static Timer timer; return &timer; }
	protected:
		LARGE_INTEGER GetAdjustedCurrentTime();
		bool m_bUsingQPF;
		bool m_bTimerStopped;
		ssh_l m_llQPFTicksPerSec;
		ssh_l m_llStopTime;
		ssh_l m_llLastElapsedTime;
		ssh_l m_llBaseTime;
	};
}
