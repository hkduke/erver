/******************************************************************
Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
File name:		CCData.h
Version:		1.0
Description:    用户数据
Author:			pengwen
Create Date:	2011-12-3
History:

******************************************************************/
//文件名: Log.h
#ifndef LOGWXN_H_INCLUDED

#define LOGWXN_H_INCLUDED


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>


#include <stdarg.h>
#include <time.h>


#define LOG_FILE "mylog.log"
#define BUFF_MAX_LENGTH 512


int WrtLog(int nRoomKindId,  int LogLevel, const char * format, ...)
{
    char sbuff[BUFF_MAX_LENGTH];
    char sTmpStr[BUFF_MAX_LENGTH] = "";
    char sEnter[2] = "";


    struct tm t;
    time_t now;
    int year;


    va_list *ap;
    FILE *fh;
    sbuff[0] = '\0';

	std::stringstream strFileName;
	strFileName << "mylog_" << nRoomKindId << ".log";

    if ((fh = fopen(strFileName.str().c_str(), "a+")) == NULL)
    {
        perror("Create or Open LogFile Error!\n");
        return -1;
    }
    else
    {
        ap = (va_list *)
             malloc(sizeof(va_list));
        if (ap != NULL)
        {
            va_start(*ap, format);
#ifdef WIN32
            _vsnprintf(sbuff, BUFF_MAX_LENGTH, format, *ap);
#else
            vsnprintf(sbuff, BUFF_MAX_LENGTH, format, *ap);
#endif
            va_end(*ap);
            free(ap);
        }
		if (sbuff[strlen(sbuff) - 1] != '\n')
			strcpy(sEnter, " ");
            //strcpy(sEnter, "\n");


        time(&now);
        t =  *localtime(&now);
        year = t.tm_year;
        if (year < 50)
            year += 2000;
        else
            year += 1900;
        sprintf(sTmpStr,"%04d-%02d-%02d %02d:%02d:%02d ",year,t.tm_mon+1,t.tm_mday,t.tm_hour,t.tm_min,t.tm_sec);
       // fprintf(fh, "%s %s%s", sTmpStr, sbuff, sEnter);
		fprintf(fh, "%s%s", sbuff, sEnter);
        fclose(fh);
    }
    return 0;
}

#endif