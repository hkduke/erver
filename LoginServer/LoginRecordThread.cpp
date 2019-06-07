#include "LoginRecordThread.h"
#include "CTType.h"
#include "Utility.h"
#include "glog_wrapper.h"
#include "ServerCfg.h"


CLoginRecordThread::CLoginRecordThread(bool bAutoDestroy)
	: m_bAutoDestroy(bAutoDestroy)
{

}

CLoginRecordThread::~CLoginRecordThread()
{

}

std::shared_ptr<tagUserLoginInfo> CLoginRecordThread::GetFreeLoginInfoQue()
{
	if (m_LoginInfoFreeQue.GetSize() == 0)
	{
		return std::make_shared<tagUserLoginInfo>();
	}

	return m_LoginInfoFreeQue.GetHead();
}


void CLoginRecordThread::InsertLoginInfo(std::shared_ptr<tagUserLoginInfo>& loginInfoPtr)
{
	m_LoginInfoQue.PushBack(loginInfoPtr);
}

void* CLoginRecordThread::run()
{
	acl::string charset;
#ifdef _OS_WIN32_CODE
	acl::db_handle::set_loadpath("libmysql.dll");
	charset.format("%s", "gbk");
#elif defined(_OS_LINUX_CODE)
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.so");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#else
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.dylib");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#endif
	Utility::Sleep(2000);

	//创建写记录的连接
	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);

	acl::mysql_conf dbconf(dbaddr, CServerCfg::m_RecorddbName);
	dbconf.set_dbuser(CServerCfg::m_RecorddbUser).set_dbpass(CServerCfg::m_RecorddbPwd).\
		set_dblimit(1).set_auto_commit(true).set_rw_timeout(60).set_charset(charset.c_str());

	std::unique_ptr<acl::mysql_pool> dbRecordPool(new acl::mysql_pool(dbconf));

	while (true)
	{
		if (m_LoginInfoQue.GetSize() == 0)
		{
			Utility::Sleep(10);
			continue;
		}

		std::shared_ptr<tagUserLoginInfo> LoginInfo = m_LoginInfoQue.GetHead();

		acl::db_handle* db = dbRecordPool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "new user id from db fail, get db handle null.";
			return 0;
		}

		do
		{
			std::string date(Utility::GetTimeNowString());
			std::string year = date.substr(0, 4);
			std::string month = date.substr(5, 2);
			std::string table("INSERT INTO record_login_");
			table.append(year);
			table.append(month);

			acl::query query;
			query.create_sql(" (userid, fixdate, loginSerial,loginMachine, ip) VALUES (:userid, UNIX_TIMESTAMP(NOW()), :serial, :machine, :ip)")
				.set_format("userid", "%d", LoginInfo->dwUserID)
				.set_format("serial", "%s", LoginInfo->szLoginSerial)
				.set_format("machine", "%s", LoginInfo->szloginMachineType)
				.set_format("ip", "%s", LoginInfo->szLoginIp);

			table.append(query.to_string());

			if (db->sql_update(table.c_str()) == false)
			{
				LOG(WARNING) << "insert user login record fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
				break;
			}
		} while (0);

		dbRecordPool->put(db);

		//插入空闲队列
		m_LoginInfoFreeQue.PushBack(LoginInfo);
	}

	if (m_bAutoDestroy)
		delete this;

	return NULL;
}