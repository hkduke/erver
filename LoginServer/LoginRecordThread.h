#ifndef _LOGIN_RECORD_THREAH_H_
#define _LOGIN_RECORD_THREAH_H_

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "DataQueue.h"
#include "CGlobalData.h"
#include <memory>
#include "glog_wrapper.h"

struct tagUserLoginInfo
{
	CT_DWORD	dwUserID;
	CT_CHAR		szLoginSerial[MACHINESERIAL_LEN];
	CT_CHAR		szloginMachineType[MACHINETYPE_LEN];
	CT_CHAR		szLoginIp[STRING_IP_LEN];

	tagUserLoginInfo()
	{
		Reset();
		//LOG(WARNING) << "tagUserLoginInfo()";
	}

	~tagUserLoginInfo()
	{
		//LOG(WARNING) << "~tagUserLoginInfo()";
	}

	void Reset()
	{
		dwUserID = 0;
		memset(szLoginSerial, 0, sizeof(szLoginSerial));
		memset(szloginMachineType, 0, sizeof(szloginMachineType));
		memset(szLoginIp, 0, sizeof(szLoginIp));
	}
};

class CLoginRecordThread : public acl::thread
{
public:
	CLoginRecordThread(bool bAutoDestroy);
	~CLoginRecordThread();

	virtual void* run();

public:
	/************
	插入登录数据
	*************/
	std::shared_ptr<tagUserLoginInfo> GetFreeLoginInfoQue();
	void InsertLoginInfo(std::shared_ptr<tagUserLoginInfo>& loginInfoPtr);

private:
	bool		 m_bAutoDestroy;
	CDataQueue<std::shared_ptr<tagUserLoginInfo>>	m_LoginInfoQue;		//需要处理登录信息
	CDataQueue<std::shared_ptr<tagUserLoginInfo>>	m_LoginInfoFreeQue;	//空闲的登录信息
};

#endif