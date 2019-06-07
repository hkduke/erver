#include "stdafx.h"
#include "GameLog.h"
#include <time.h>
#include <string.h>
#include <stdarg.h>
//#include <Windows.h>
//#include <imagehlp.h>
//#pragma comment(lib, "DbgHelp.lib")


//Ĭ�Ϲ��캯��
Logger::Logger()
{
	//��ʼ��
	memset(m_strLogPath, 0, MAX_STR_LEN);
	memset(m_strCurLogName, 0, MAX_STR_LEN);
	m_pFileStream = NULL;
	//����Ĭ�ϵ�д��־����
	m_nLogLevel = EnumLogLevel::LogLevelNormal;
	//��ʼ���ٽ�������
	//InitializeCriticalSection(&m_cs);
	//������־�ļ���
	//GenerateLogName(0);
}

//���캯��
Logger::Logger(const char * strLogPath, int64_t lPlayerID,EnumLogLevel nLogLevel):m_nLogLevel(nLogLevel)
{
	//��ʼ��
	m_pFileStream = NULL;
	strcpy(m_strLogPath, strLogPath);
	//InitializeCriticalSection(&m_cs);
	CreateLogPath();
	GenerateLogName(lPlayerID);
}

//��������
Logger::~Logger()
{
	//�ͷ��ٽ���
	//DeleteCriticalSection(&m_cs);
	//�ر��ļ���
	if(m_pFileStream)
		fclose(m_pFileStream);
}

//д�ؼ���Ϣ�ӿ�
void Logger::TraceKeyInfo(const char * strInfo, ...)
{
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, KEYINFOPREFIX);
	//��ȡ�ɱ��β�
	/*va_list arg_ptr = NULL;*/
	va_list arg_ptr;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	//д��־�ļ�
	Trace(pTemp);
	/*arg_ptr = NULL;*/
}

//д������Ϣ
void Logger::TraceError(const char* strInfo, ...)
{
	//�жϵ�ǰ��д��־����������Ϊ��д��־��������
	if(m_nLogLevel >= EnumLogLevel::LogLevelStop)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, ERRORPREFIX);
	va_list arg_ptr;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
}

//д������Ϣ
void Logger::TraceWarning(const char * strInfo, ...)
{
	//�жϵ�ǰ��д��־����������Ϊֻд������Ϣ��������
	if(m_nLogLevel >= EnumLogLevel::LogLevelNormal)
		return;
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	strcpy(pTemp, GetCurrentTime());
	strcat(pTemp, WARNINGPREFIX);
	va_list arg_ptr;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
}


//дһ����Ϣ
void Logger::TraceInfo(const char * strInfo, ...)
{
	//�жϵ�ǰ��д��־����������ֻд����;�����Ϣ��������
	/*if(m_nLogLevel >= EnumLogLevel::LogLevelMid)
		return;*/
	if(!strInfo)
		return;
	char pTemp[MAX_STR_LEN] = {0};
	//strcpy(pTemp, GetCurrentTime());
	//strcat(pTemp,INFOPREFIX);
	va_list arg_ptr;
	va_start(arg_ptr, strInfo);
	vsprintf(pTemp + strlen(pTemp), strInfo, arg_ptr);
	va_end(arg_ptr);
	Trace(pTemp);
}

//��ȡϵͳ��ǰʱ��
char * Logger::GetCurrentTime()
{
	time_t curTime;
	struct tm * pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);
	char temp[MAX_STR_LEN] = {0};
	sprintf(temp, "%04d-%02d-%02d %02d:%02d:%02d", pTimeInfo->tm_year + 1990, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday,pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec);
	char * pTemp = temp;
	return pTemp;
}

//����д��־����
void Logger::SetLogLevel(EnumLogLevel nLevel)
{
	m_nLogLevel = nLevel;
}

//д�ļ�����
void Logger::Trace(const char * strInfo)
{
	if(!strInfo)
		return;
	try
	{
		//�����ٽ���
		//EnterCriticalSection(&m_cs);
		m_mutex.Lock();
		//���ļ���û�д򿪣������´�
		if(!m_pFileStream)
		{
			char temp[1024] = {0};
			strcat(temp, m_strLogPath);
			strcat(temp, m_strCurLogName);
			m_pFileStream = fopen(temp, "a+");
			if(!m_pFileStream)
			{
				return;
			}
		}

		time_t curTime;
		struct tm * pTimeInfo = NULL;
		time(&curTime);
		pTimeInfo = localtime(&curTime);

		//д��־��Ϣ���ļ���
		fprintf(m_pFileStream, "%04d-%02d-%02d %02d:%02d:%02d %s\n",pTimeInfo->tm_year + 1990, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday,pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec, strInfo);
		fflush(m_pFileStream);
		//�뿪�ٽ���
		//LeaveCriticalSection(&m_cs);
		m_mutex.Unlock();
	}
	//�������쳣�������뿪�ٽ�������ֹ����
	catch(...)
	{
		//LeaveCriticalSection(&m_cs);
		m_mutex.Unlock();
	}
}

//������־�ļ�������
void Logger::GenerateLogName(int64_t lPlayerID)
{
	time_t curTime;
	struct tm * pTimeInfo = NULL;
	time(&curTime);
	pTimeInfo = localtime(&curTime);
	char temp[1024] = {0};
	//��־�������磺2013-01-01.log
	//sprintf(temp, "%04d-%02d-%02d.log", pTimeInfo->tm_year + 1900, pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday);
	sprintf(temp, "%lld.log", lPlayerID);
	if(0 != strcmp(m_strCurLogName, temp))
	{
		strcpy(m_strCurLogName,temp);
		if(m_pFileStream)
		{
			fclose(m_pFileStream);
		}
		char temp[1024] = {0};
		strcat(temp, m_strLogPath);
		strcat(temp, m_strCurLogName);
		//��׷�ӵķ�ʽ���ļ���
		m_pFileStream = fopen(temp, "a+");
	}

}

//������־�ļ���·��
void Logger::CreateLogPath()
{
	if(0 != strlen(m_strLogPath))
	{
		strcat(m_strLogPath, "\\");
	}
	//无用屏蔽
	//MakeSureDirectoryPathExists(m_strLogPath);
}