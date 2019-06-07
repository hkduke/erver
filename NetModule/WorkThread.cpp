#include "WorkThread.h"
#include "MesBlock.h"
#include "ObjPool.h"
#include "Utility.h"
#include "sysdata.h"
#include "glog_wrapper.h"
#include "NetConnector.h"
#include "NetModule.h"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

//mes block pool
extern CCObjPool<CMesBlock>	g_MsgBlockPool;


CWorkThread::CWorkThread()
{
}

CWorkThread::~CWorkThread()
{
}

//打开
CT_VOID CWorkThread::Open()
{
}

//运行
CT_VOID CWorkThread::Run()
{
	while(m_running)
	{
		try
		{
			if (m_MesBlockQue.GetSize() == 0)
			{
				Utility::Sleep(SYS_NET_MAX_SLEEP_TIME);
			}

		
			while (CMesBlock* pBlock = m_MesBlockQue.GetHead())
			{
				//unsigned int startTick = Utility::GetTick();
				_EXAM_ASSERT(NULL != pBlock);
				acl::aio_socket_stream* pSocket = pBlock->m_pSocketStream;
				switch (pBlock->m_byMsgTag)
				{
				case MSC_S_SOCKET_LINK:
				{
					//连接打开
					OnTCPSocketLink(((stConnectorPtr*)(pBlock->m_pBuf))->pConnector);
				}
				break;
				case MSC_S_SOCKET_SHUT:
				{
					//连接关闭
					OnTCPSocketShut(((stConnectorPtr*)(pBlock->m_pBuf))->pConnector);
				}
				break;
				case MSC_S_NETWORK_BIND:
				{
					//网络连接
					OnTCPNetworkBind(pSocket);
				}
				break;
				case MSC_S_NETWORK_SHUT:
				{
					//网络关闭
					OnTCPNetworkShut(pSocket);
				}
				break;
				case MSC_S_HANDLE_INFO:
				{
					//数据到来
					CMD_Command* cmdHead = (CMD_Command*)pBlock->m_pBuf;
					CT_WORD wHeadLen = (CT_WORD)sizeof(CMD_Command);
					if (pBlock->m_dwLen >= wHeadLen)
					{
						OnNetEventRead(pSocket, cmdHead, pBlock->m_pBuf + wHeadLen, pBlock->m_dwLen - wHeadLen);
					}
					else
					{
						CT_CHAR buf[128] = { 0 };
						std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
						if (!strIp.empty())
						{
							_snprintf_info(buf, 128, "IP=%s, ERROR Message Len: mainID=%u, subID=%u", strIp.c_str(), cmdHead->dwMainID, cmdHead->dwSubID);
							LOG(ERROR) << buf;
						}
					}
				}
				break;
				case MSC_S_HANDLE_TIMER:
				{
					//定时器
					CT_DWORD iTimerID = *(CT_DWORD*)(pBlock->m_pBuf);
					OnTimer(iTimerID);
				}
				break;;
				default:
					break;
				}
				//释放
				pBlock->Deallocate();
				g_MsgBlockPool.DeleteObj(pBlock);
				/*unsigned int endTick = Utility::GetTick();
				if (endTick - startTick > 200 && (CNetModule::getSingleton().GetServerType() == GAME_SERVER || CNetModule::getSingleton().GetServerType() == PROXY_SERVER))
				{
					LOG(ERROR) << "CWorkThread run use too much time, tick: " << endTick - startTick;
				}*/
			}
			
		}
		catch (const std::exception& e)
		{
			LOG(ERROR) << "CWorkThread run catch a std exception: " << e.what();
		}
		catch (...)
		{
			LOG(ERROR) << "CWorkThread run catch a unkown exception!";
		}
	}
}

//网络定时器
CT_VOID CWorkThread::OnTimer(CT_DWORD dwTimerID)
{
	
}


//插入消息块
CT_VOID CWorkThread::InsertMseBlock(CMesBlock* pBlock)
{
	m_MesBlockQue.PushBack(pBlock);
}



#ifdef SOCKETS_NAMESPACE
}
#endif




























