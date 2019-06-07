#include "QueryRecordThread.h"
#include "CTType.h"
#include "Utility.h"
#include "glog_wrapper.h"
#include "NetModule.h"
#include "ServerCfg.h"
#include "CMD_Customer.h"
#include "CMD_Inner.h"


CQueryRecordThread::CQueryRecordThread(bool bAutoDestroy)
	: m_bAutoDestroy(bAutoDestroy)
{

}

CQueryRecordThread::~CQueryRecordThread()
{

}

std::shared_ptr<tagChatUser> CQueryRecordThread::GetFreeChatUserQue()
{
	if (m_ChatUserFreeQue.GetSize() == 0)
	{
		return std::make_shared<tagChatUser>();
	}

	return m_ChatUserFreeQue.GetHead();
}


void CQueryRecordThread::InsertQueryChatRecord(std::shared_ptr<tagChatUser>& chatUserPtr)
{
	m_ChatUserQue.PushBack(chatUserPtr);
}

void* CQueryRecordThread::run()
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
	dbaddr.format("%s:%d", CServerCfg::m_chatdbIP, CServerCfg::m_chatdbPort);

	acl::mysql_conf dbconf(dbaddr, CServerCfg::m_chatdbName);
	dbconf.set_dbuser(CServerCfg::m_chatdbUser).set_dbpass(CServerCfg::m_chatdbPwd).\
		set_dblimit(1).set_auto_commit(true).set_rw_timeout(60).set_charset(charset.c_str());

	std::unique_ptr<acl::mysql_pool> dbRecordPool(new acl::mysql_pool(dbconf));

	while (true)
	{
		if (m_ChatUserQue.GetSize() == 0)
		{
			Utility::Sleep(10);
			continue;
		}

		std::shared_ptr<tagChatUser> chatUser = m_ChatUserQue.GetHead();

		acl::db_handle* db = dbRecordPool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "query chat record db fail, get db handle null.";
            continue;
		}

		std::vector<MSG_S2C_ChatRecord_To_User> vecChatRecord;
		do
		{
			acl::query query;
			query.create_sql("select * from (select * from record.record_im_chat where userid = :userid and msgType = :msgType union all \n"
							 "select a.* from record.record_im_chat a inner join record.record_im_chat b on a.replyid = b.id \n"
							 "where b.userid = :userid and b.msgType = :msgType) x order by id desc LIMIT 20")
				.set_format("userid", "%u", chatUser->dwUserID)
				.set_format("msgType", "%d", chatUser->cbMsgType);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "Select record_im_chat fail, errno: " << db->get_errno() << ", error: " << db->get_error();
                break;
			}

			MSG_S2C_ChatRecord_To_User chatRecord;
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				chatRecord.dwMsgId = atoi((*row)["id"]);
				chatRecord.cbType = (CT_BYTE)atoi((*row)["type"]);
				chatRecord.cbContentType = (CT_BYTE)atoi((*row)["contentType"]);
				chatRecord.cbIsRead = (CT_BYTE)atoi((*row)["isRead"]);
				std::string strMsg = (*row)["body"];
				_snprintf_info(chatRecord.szContent, sizeof(chatRecord.szContent), "%s", strMsg.c_str());
				CT_DWORD dwTime = (CT_DWORD)atoi((*row)["fixdate"]);
				_snprintf_info(chatRecord.szTime, sizeof(chatRecord.szTime), "%s", Utility::ChangeTimeToString(dwTime).c_str());
				vecChatRecord.push_back(chatRecord);
			}

            //插入提醒消息
            if (vecChatRecord.size() < 20)
            {
                if (chatUser->cbMsgType == 4)
                {
                    _snprintf_info(chatRecord.szContent, sizeof(chatRecord.szContent), "%s", "尊敬的玩家，您好！欢迎咨询充值问题。");
                }
                else if (chatUser->cbMsgType == 5)
                {
                    _snprintf_info(chatRecord.szContent, sizeof(chatRecord.szContent), "%s", "尊敬的玩家，您好！欢迎咨询兑换问题。");
                }
                else
                {
                    _snprintf_info(chatRecord.szContent, sizeof(chatRecord.szContent), "%s", "尊敬的玩家，您好，在线客服很高兴为您服务！");
                }

                chatRecord.cbType = 2;
                chatRecord.cbContentType = 1;
                chatRecord.cbIsRead = 1;
                if (vecChatRecord.size() != 0)
                {
                    auto itRecord = vecChatRecord.begin();
                    chatRecord.dwMsgId = itRecord->dwMsgId - 1;
                    _snprintf_info(chatRecord.szTime, sizeof(chatRecord.szTime), "%s", itRecord->szTime);
                }
                else
                {
                    chatRecord.dwMsgId = 999999999;
                    _snprintf_info(chatRecord.szTime, sizeof(chatRecord.szTime), "%s", Utility::ChangeTimeToString(time(NULL)).c_str());
                }
                vecChatRecord.push_back(chatRecord);
            }
        } while (0);

		db->free_result();
		dbRecordPool->put(db);

		if (!vecChatRecord.empty())
		{
			static CT_BYTE cbBuff[8192];
			CT_DWORD dwSendSize = 0;

			//最多10条的游戏记录
			for (auto it = vecChatRecord.rbegin(); it != vecChatRecord.rend(); ++it)
			{
				memcpy(cbBuff+dwSendSize, &(*it), sizeof(MSG_S2C_ChatRecord_To_User));
				dwSendSize += sizeof(MSG_S2C_ChatRecord_To_User);
			}

			SendMsgToUser(chatUser->pSocket, chatUser->dwUserID, SUB_SERVER2CLIENT_CHAT_RECORD, cbBuff, dwSendSize, chatUser->cbMsgType);
		}
		/*else
		{
			//如果没有聊天记录，则下发一条空消息
			SendMsgToUser(chatUser->pSocket, chatUser->dwUserID, SUB_SERVER2CLIENT_CHAT_RECORD, NULL, 0, chatUser->cbType);
		}*/

		//插入空闲队列
		m_ChatUserFreeQue.PushBack(chatUser);
	}

	if (m_bAutoDestroy)
		delete this;

	return NULL;
}

//发送消息给客户端
CT_VOID CQueryRecordThread::SendMsgToUser(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BYTE cbType)
{
	CT_DWORD dwDownSize = sizeof(MSG_GameMsgDownHead);
	static CT_CHAR chBuf[8192] = { 0 };
	MSG_GameMsgDownHead DownData;
	memset(&DownData, 0, dwDownSize);
	DownData.dwMainID = MSG_CUSTOMER_MAIN;
	DownData.dwSubID = wSubCmdID;
	DownData.dwValue2 = dwUserID;
	memcpy(chBuf, &DownData, dwDownSize);
	memcpy(chBuf+dwDownSize, &cbType, sizeof(CT_BYTE));
	if (dwLen != 0)
		memcpy(&chBuf[dwDownSize+sizeof(CT_BYTE)], pBuf, dwLen);

	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, chBuf, dwDownSize + sizeof(CT_BYTE) + dwLen);
}