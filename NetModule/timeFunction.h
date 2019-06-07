/*----------------- timeFunction.h
*
* Copyright (C): 2017  GaoXin Network
* Author       : luwei
* Version      : V1.0
* Date         : 2017-09-22 17:23:16
*--------------------------------------------------------------
*
*------------------------------------------------------------*/


#pragma once
#include "CTType.h"

//--- 获得本地时间
extern CT_INT32	getLocalTime(struct tm* _Tm, const CT_UINT64* _Time);
extern CT_INT32	getLocalTime(struct tm* _Tm, CT_UINT64 _Time);

//判断是否垮天
#ifdef _WIN32
extern int gettimeofday(struct timeval *tv, struct timezone *tz)
#endif

extern bool isAcrossTheDay(time_t _early, time_t _late);

//--- 获得时间相差天数
extern CT_INT32	diffTimeDay(CT_UINT64 _early, CT_UINT64 _late);
//--- 获得时间相差周数
extern CT_INT32	diffTimeWeek(CT_UINT64 _early, CT_UINT64 _late);

//---获得本月月初时间戳
extern unsigned long long getMonthTime();
//--- 根据时间戳获得本地时间零点的时间戳
extern unsigned long long getMorningTime(time_t t);