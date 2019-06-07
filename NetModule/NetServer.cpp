#include "NetServer.h"
#include "NetModule.h"
#include "Utility.h"

//消息块池
CCObjPool<CMesBlock>	g_MsgBlockPool;

bool CNetWork::accept_callback(acl::aio_socket_stream* client)
{
	// 创建异步客户端流的回调对象并与该异步流进行绑定
	CNetWork_Io_callback* callback = new CNetWork_Io_callback(client, m_pWorkThread, m_wWorkThreadNum, m_bDataEncrypt);

	// 注册异步流的读回调过程
	client->add_read_callback(callback);

	// 注册异步流的写回调过程
	client->add_write_callback(callback);

	// 注册异步流的关闭回调过程
	client->add_close_callback(callback);

	// 注册异步流的超时回调过程
	client->add_timeout_callback(callback);

	// 限制数据包最大长度
	client->set_buf_max(SYS_NET_SENDBUF_SIZE);

	//插入客户端
	CNetModule::getSingleton().InsertClientSocket(client, callback);
	//通知网络连接打开
	int nLen = sizeof(stNetWorkPtr);
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock == NULL)
	{
		LOG(ERROR) << "get a null message block.";
		return false;
	}
	CT_CHAR* pBuf = pBlock->Allocate(nLen);
	if (pBuf == NULL)
	{
		LOG(ERROR) << "Allocate message block buf fail.";
		return false;
	}
	pBlock->m_byMsgTag = MSC_S_NETWORK_BIND;
	pBlock->m_pSocketStream = client;
	stNetWorkPtr ptr;
	ptr.pSocket = client;
	memcpy(pBlock->m_pBuf, &ptr, nLen);
	pBlock->m_dwLen = nLen;
	m_pWorkThread[0]->InsertMseBlock(pBlock);

	// 从异步流读取一行数据
	client->read(sizeof(CMD_Command), 0);

	LOG(INFO) << "accept io..." << client->get_peer(true);
	return true;
}

void CNetWork_Io_callback::close_callback()
{
	//删除客户端
	CNetModule::getSingleton().DeleteClientSocket(m_client);

	ClearData();

	//通知网络关闭
	int nLen = sizeof(stNetWorkPtr);
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock != NULL)
	{
		CT_CHAR* pBuf = pBlock->Allocate(nLen);
		if (pBuf != NULL)
		{
			pBlock->m_byMsgTag = MSC_S_NETWORK_SHUT;
			pBlock->m_pSocketStream = m_client;
			stNetWorkPtr ptr;
			ptr.pSocket = m_client;
			memcpy(pBlock->m_pBuf, &ptr, nLen);
			pBlock->m_dwLen = nLen;
			m_pWorkThread[0]->InsertMseBlock(pBlock);
		}
	}
	else
	{
		LOG(ERROR) << "get a null message block.";
	}

	LOG(INFO) << "delete io..." << m_client->get_peer(true);
	delete this;
}

bool CNetWork_Io_callback::read_callback(char* data, int len)
{
	m_dwRecvTickCount = Utility::GetTick();
	// 当前状态是处理数据头时
	if (m_readStatus == STATUS_T_HDR)
	{
		// 检验头部长度是否符合要求
		if (len != sizeof(CMD_Command))
		{
			printf("invalid len(%d) != CMD_Command(%d)\r\n", len, (int)sizeof(CMD_Command));
			return false;
		}

		m_inputStream.WriteBytes(data, len);

		// 取出数据体长度，并读指定长度的数据体
		CMD_Command* pMsgHead = (CMD_Command*)data;

		//如果只有消息头
		if (pMsgHead->dwDataSize == 0)
		{
			//处理消息一个消息块
			if (pMsgHead->dwMainID != MSG_HEART_BEAT_MAIN)
			{
				int nLen = (int)m_inputStream.GetReadSize();
				CMesBlock* pBlock = g_MsgBlockPool.NewObj();
				if (pBlock == NULL)
				{
					LOG(ERROR) << "get a null message block.";
					return false;
				}
				CT_CHAR* pBuf = pBlock->Allocate(nLen);
				if (pBuf == NULL)
				{
					LOG(ERROR) << "Allocate message block buf fail.";
					return false;
				}
				pBlock->m_byMsgTag = MSC_S_HANDLE_INFO;
				pBlock->m_pSocketStream = m_client;
				memcpy(pBlock->m_pBuf, m_inputStream.GetReadBuffer(), nLen);
				pBlock->m_dwLen = nLen;
				m_pWorkThread[0]->InsertMseBlock(pBlock);
			}

			//清空
			m_inputStream.SetEmpty();

			// 设置状态为读取下一个数据包
			m_readStatus = STATUS_T_HDR;

			// 从异步流读数据包头
			m_client->read(sizeof(CMD_Command), 0);
			return true;
		}

		// 修改状态位，表明下一步需要读取数据体
		m_readStatus = STATUS_T_DAT;

		// 异步读指定长度的数据
		m_client->read(pMsgHead->dwDataSize, 0);
		return true;
	}

	if (m_readStatus != STATUS_T_DAT)
	{
		printf("invalid status: %d\r\n", (int)m_readStatus);
		return false;
	}

	static CT_CHAR szBuffer[SYS_NET_RECVBUF_SIZE*2] = { 0 };
	if (m_bDataEncrypt)
	{
		//memset(szBuffer, 0, sizeof(szBuffer));
		m_encrypt.decrypt((CT_UCHAR*)data, len, (CT_UCHAR*)szBuffer);
		m_inputStream.WriteBytes(szBuffer, len);
	}
	else
	{
		m_inputStream.WriteBytes(data, len);
	}

	//处理消息一个消息块
	int nLen = (int)m_inputStream.GetReadSize();
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock == NULL)
	{
		LOG(ERROR) << "get a null message block.";
		return false;
	}
	CT_CHAR* pBuf = pBlock->Allocate(nLen);
	if (pBuf == NULL)
	{
		LOG(ERROR) << "Allocate message block buf fail.";
		return false;
	}
	pBlock->m_byMsgTag = MSC_S_HANDLE_INFO;
	pBlock->m_pSocketStream = m_client;
	memcpy(pBlock->m_pBuf, m_inputStream.GetReadBuffer(), nLen);
	pBlock->m_dwLen = nLen;
	if (m_wWorkThreadNum == 1)
	{
		m_pWorkThread[0]->InsertMseBlock(pBlock);
	}
	else
	{
		m_pWorkThread[m_wCurrentWorkThread++]->InsertMseBlock(pBlock);
		if (m_wCurrentWorkThread >= m_wWorkThreadNum)
		{
			m_wCurrentWorkThread = 0;
		}
	}

	//清空
	m_inputStream.SetEmpty();

	// 设置状态为读取下一个数据包
	m_readStatus = STATUS_T_HDR;

	// 从异步流读数据包头
	m_client->read(sizeof(CMD_Command), 0);

	return true;
}

void CNetWork_Io_callback::SendData(CMesBlock* pBlock)
{
	m_SendDataList.PushBack(pBlock);
}

void CNetWork_Io_callback::DataToOutstream(void)
{
	CT_DWORD dwMsgNums = m_SendDataList.GetSize();
	if (0 == dwMsgNums)
	{
		return;
	}
	while (CMesBlock* pBlock = m_SendDataList.GetHead())
	{
		m_outputStream.WriteBytes(pBlock->m_pBuf, pBlock->m_dwLen);
		pBlock->Deallocate();
		g_MsgBlockPool.DeleteObj(pBlock);
	}
	m_outputStream.Compact();
}

void CNetWork_Io_callback::ClearData(void)
{
	m_inputStream.SetEmpty();
	m_outputStream.SetEmpty();

	while (CMesBlock* pBlock = m_SendDataList.GetHead())
	{
		pBlock->Deallocate();
		g_MsgBlockPool.DeleteObj(pBlock);
	}
}

bool CNetWork_Io_callback::OnEventSend(void)
{
	DataToOutstream();

	const CT_CHAR* pBuf = m_outputStream.GetReadBuffer();
	CT_INT32 dwLen = (CT_INT32)m_outputStream.GetReadSize();
	if (dwLen <= 0)
	{
		return true;
	}

	m_client->write(pBuf, dwLen);

	//提交读大小
	m_outputStream.CommitReadBytes(dwLen);
	//重置缓冲
	m_outputStream.Compact();

	return true;
}