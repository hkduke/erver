#ifndef _GAME_LOGGER_H_
#define _GAME_LOGGER_H_
//#include <Windows.h>
#include "../../NetModule/Mutex.h"
#include <stdio.h>
//#pragma warning(disable:4996)
//#pragma warning(disable:4482)

#pragma once
/*
	* ������Logger
	* ���ã��ṩд��־���ܣ�֧�ֶ��̣߳�֧�ֿɱ��β���������֧��д��־���������
	* �ӿڣ�SetLogLevel������д��־����
			TraceKeyInfo��������־����д�ؼ���Ϣ
			TraceError��д������Ϣ
			TraceWarning��д������Ϣ
			TraceInfo��дһ����Ϣ

	*ʹ�ã�
	#include "Common/Logger/Logger.h"
	Logger log;

	log.SetLogLevel(LogLevelAll);
	log.TraceInfo("Ready Go");
*/
//��־�������ʾ��Ϣ
static const char * KEYINFOPREFIX   = " Key: \n";
static const char * ERRORPREFIX		= " Error: \n";
static const char * WARNINGPREFIX   = " Warning: \n";
static const char * INFOPREFIX      = " Info: \n";

static const int MAX_STR_LEN = 1024;
//��־����ö��
typedef enum EnumLogLevel
{
	LogLevelAll = 0,    //������Ϣ��д��־
	LogLevelMid,        //д���󡢾�����Ϣ
	LogLevelNormal,     //ֻд������Ϣ
	LogLevelStop        //��д��־
}_EnumLogLevel;

class Logger
{
public:
	//Ĭ�Ϲ��캯��
	Logger();
	//���캯��
	Logger(const char * strLogPath,  int64_t lPlayerID, EnumLogLevel nLogLevel = EnumLogLevel::LogLevelNormal);
	//��������
	virtual ~Logger();
public:
	//д�ؼ���Ϣ
	void TraceKeyInfo(const char * strInfo, ...);
	//д������Ϣ
	void TraceError(const char* strInfo, ...);
	//д������Ϣ
	void TraceWarning(const char * strInfo, ...);
	//дһ����Ϣ
	void TraceInfo(const char * strInfo, ...);
	//����д��־����
	void SetLogLevel(EnumLogLevel nLevel);
private:
	//д�ļ�����
	void Trace(const char * strInfo);
	//��ȡ��ǰϵͳʱ��
	char * GetCurrentTime();
	//������־�ļ�����
	void GenerateLogName(int64_t lPlayerID);
	//������־·��
	void CreateLogPath();
private:
	//д��־�ļ���
	FILE * m_pFileStream;
	//д��־����
	EnumLogLevel m_nLogLevel;
	//��־��·��
	char m_strLogPath[MAX_STR_LEN];
	//��־������
	char m_strCurLogName[MAX_STR_LEN];
	//�߳�ͬ�����ٽ�������
	//CRITICAL_SECTION m_cs;
	CTMutex m_mutex;
};

#endif