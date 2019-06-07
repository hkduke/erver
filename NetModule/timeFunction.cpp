
/*----------------- timeFunction.cpp
*
* Copyright (C): 2017  GaoXin Network
* Author       : luwei
* Version      : V1.0
* Date         : 2017-09-22 17:23:16
*--------------------------------------------------------------
*
*------------------------------------------------------------*/

#include "timeFunction.h"
#include <time.h>
#include <string.h>
#include <memory>

#ifdef _WIN32
#include <windows.h>
#else //WIN32
#include <sys/time.h>
#endif // WIN32

#ifdef _WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

struct timezone
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};

int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag = 0;

  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);

    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;

    tmpres /= 10;  /*convert into microseconds*/
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS;
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }

  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }

  return 0;
}
#endif

//-------------------------------------------------------------
//------------------------------ 获得本地时间
CT_INT32	getLocalTime(struct tm* _Tm, const CT_UINT64* _Time)
{
	if (!_Tm || !_Time)
		return -1;

	time_t _time = *_Time;
#ifdef _WIN32
	return ::localtime_s(_Tm, &_time);
#else//WIN32
	if (_Tm)
		memset(_Tm, 0xff, sizeof(struct tm));
	struct tm* tmp = ::localtime(&_time);
	if (tmp && _Tm)
		memcpy(_Tm, tmp, sizeof(struct tm));
	return 0;
#endif // WIN32
}
//-------------------------------------------------------------
//------------------------------ 获得本地时间
CT_INT32	getLocalTime(struct tm* _Tm, CT_UINT64 _Time)
{
	if (!_Tm || !_Time)
		return -1;

	return getLocalTime(_Tm, &_Time);
}

//-------------------------------------------------------------
//------------------------------ 获得时间是否跨天
bool isAcrossTheDay(time_t _early, time_t _late)
{
	/*struct timezone tvTimeZone;
	gettimeofday(NULL, &tvTimeZone);
	int iTimeZoneSeconds = (-tvTimeZone.tz_minuteswest*60);
	int iDayOld = static_cast<int>((_early+iTimeZoneSeconds)/86400);
	int iDayNew = static_cast<int>((_late+iTimeZoneSeconds)/86400);
	if(iDayNew > iDayOld)
	{
		return true;
	}
	else
	{
		return false;
	}*/

	//UTC时间+8小时，等于东八区时间
	/*CT_INT64 time1 = _early - 28800;
	CT_INT64 time2 = _late - 28800;

	CT_INT64 today = time1 / 86400;
	CT_INT64 yesterday = time2 / 86400;

	if (today == yesterday)
	{
		return false;
	}
	else
	{
		return true;
	}*/

	struct tm oldDate = *localtime(&_early);
	struct tm curDate = *localtime(&_late);

	if (oldDate.tm_year == curDate.tm_year &&
		oldDate.tm_mon == curDate.tm_mon &&
		oldDate.tm_mday == curDate.tm_mday)
	{
		return false;
	}

	return true;
}

//-------------------------------------------------------------
//------------------------------ 获得时间相差天数
CT_INT32	diffTimeDay(CT_UINT64 _early, CT_UINT64 _late)
{
	if (_early == 0 || _late == 0)
		return 0;

	static tm	tm_early;
	static tm	tm_late;
	memset(&tm_early, 0, sizeof(tm_early));
	memset(&tm_late, 0, sizeof(tm_late));

	getLocalTime(&tm_early, _early);
	getLocalTime(&tm_late, _late);

	if (tm_early.tm_year > tm_late.tm_year)
		return 0;

	//同年同日
	if (tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
		return 0;

	//同年判断
	if (tm_early.tm_year == tm_late.tm_year)
	{
		if (tm_early.tm_yday >= tm_late.tm_yday)
			return 0;

		return (tm_late.tm_yday - tm_early.tm_yday);
	}

	CT_INT32 iDay = 0;
	//不同年时
	if (tm_early.tm_year != tm_late.tm_year)
	{
		tm tm_temp = tm_early;

		//获取12月31日时间
		tm_temp.tm_mon = 11;
		tm_temp.tm_mday = 31;
		tm_temp.tm_yday = 0;
		CT_UINT64 _temp = mktime(&tm_temp);

		getLocalTime(&tm_temp, _temp);
		iDay = tm_temp.tm_yday - tm_early.tm_yday;

		iDay += 1;//跨年+1

				  //获得相差年天数
		for (CT_INT32 i = tm_early.tm_year + 1; i < tm_late.tm_year; i++)
		{
			tm_temp.tm_year++;
			tm_temp.tm_yday = 0;

			_temp = mktime(&tm_temp);
			getLocalTime(&tm_temp, _temp);

			iDay += tm_temp.tm_yday;
			iDay += 1;//跨年+1
		}
	}

	return (iDay + tm_late.tm_yday);
}

//-------------------------------------------------------------
//------------------------------ 获得时间相差周数
CT_INT32	diffTimeWeek(CT_UINT64 _early, CT_UINT64 _late)
{
	if (_early == 0 || _late == 0)
		return 0;

	static tm	tm_early;
	static tm	tm_late;
	memset(&tm_early, 0, sizeof(tm_early));
	memset(&tm_late, 0, sizeof(tm_late));

	getLocalTime(&tm_early, _early);
	getLocalTime(&tm_late, _late);

	if (tm_early.tm_year > tm_late.tm_year)
		return 0;

	//同年同日
	if (tm_early.tm_year == tm_late.tm_year && tm_early.tm_yday == tm_late.tm_yday)
		return 0;

	//计算两个日期的每一个周六相差多少天
	if (tm_early.tm_wday != 6)
		tm_early.tm_mday += (6 - tm_early.tm_wday);
	if (tm_late.tm_wday != 6)
		tm_late.tm_mday += (6 - tm_late.tm_wday);

	CT_INT32 iDay = diffTimeDay(mktime(&tm_early), mktime(&tm_late));

	CT_INT32 iWeek = 0;
	if (iDay > 0)
		iWeek = iDay / 7;//肯定相差都是7的倍数因为都是周六

	return iWeek;
}

//-------------------------------------------------------------
//------------------------------ 获得本月月初时间戳
unsigned long long getMonthTime()
{
	time_t t = time(0);   // get time now
	struct tm * zeroTm = localtime(&t);
	zeroTm->tm_hour = 0;
	zeroTm->tm_min = 0;
	zeroTm->tm_sec = 0;
	zeroTm->tm_mday = 0;
	unsigned long long zeroTime = mktime(zeroTm);
	return zeroTime*1000;
}

//-------------------------------------------------------------
//------------------------------ 根据时间戳获得零点时间戳
unsigned long long getMorningTime(time_t t)
{
	//time_t t = time(0);   // get time now
	struct tm *zeroTm = localtime(&t);
	zeroTm->tm_hour = 0;
	zeroTm->tm_min = 0;
	zeroTm->tm_sec = 0;
	unsigned long long zeroTime = mktime(zeroTm);
	return zeroTime * 1000;
}