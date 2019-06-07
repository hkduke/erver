#include "NetConnector.h"
#include "MesBlock.h"
#include "ObjPool.h"
#include "sysdata.h"
#include "NetModule.h"

//消息块池
extern CCObjPool<CMesBlock>	g_MsgBlockPool;

CT_DWORD CConnector_Io_Callback::m_dwOpenCount = 0;
bool CConnector_Io_Callback::read_callback(char* data, int len)
{
	// 当前状态是处理数据头时
	if (m_readStatus == STATUS_T_HDR)
	{
		// 检验头部长度是否符合要求
		if (len != sizeof(CMD_Command))
		{
			printf("invalid len(%d) != CMD_Command(%d)\r\n", len, (int)sizeof(CMD_Command));
			return false;
		}

		//写入流
		m_inputStream.WriteBytes(data, len);

		// 取出数据体长度，并读指定长度的数据体
		CMD_Command* pMsgHead = (CMD_Command*)data;

		//如果收到心跳则回复心跳
		if (pMsgHead->dwMainID == MSG_HEART_BEAT_MAIN && pMsgHead->dwSubID == SUB_HEART_BEAT)
		{
			CMD_Command heartBeat;
			heartBeat.dwDataSize = 0;
			heartBeat.dwMainID = MSG_HEART_BEAT_MAIN;
			heartBeat.dwSubID = SUB_HEART_BEAT;
			m_client->write(&heartBeat, sizeof(heartBeat));
		}

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
				m_Connector->m_pWorkThread->InsertMseBlock(pBlock);
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
		memset(szBuffer, 0, sizeof(szBuffer));
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
	m_Connector->m_pWorkThread->InsertMseBlock(pBlock);

	//清空
	m_inputStream.SetEmpty();

	// 设置状态为读取下一个数据包
	m_readStatus = STATUS_T_HDR;

	// 从异步流读数据包头
	m_client->read(sizeof(CMD_Command), 0);

	return true;
}

void CConnector_Io_Callback::close_callback()
{
	// 如果连接没有成功，启动定时器重连
	/*if (m_client->is_opened() == false)
	{
		LOG(WARNING) << "open connect：" << m_client->get_peer(true) << " fail...";
	}
	else
	{
		CNetModule::getSingleton().DeleteServerSocket(m_client);
	}*/

	CNetModule::getSingleton().DeleteServerSocket(m_client);

	ClearData();

	//连接打开
	int nLen = sizeof(stConnectorPtr);
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock != NULL)
	{
		CT_CHAR* pBuf = pBlock->Allocate(nLen);
		if (pBuf != NULL)
		{
			pBlock->m_byMsgTag = MSC_S_SOCKET_SHUT;
			pBlock->m_pSocketStream = m_client;
			stConnectorPtr ptr;
			ptr.pConnector = m_Connector;
			memcpy(pBlock->m_pBuf, &ptr, nLen);
			pBlock->m_dwLen = nLen;
			m_Connector->m_pWorkThread->InsertMseBlock(pBlock);
		}
	}
	else
	{
		LOG(ERROR) << "get a null message block.";
	}

	//m_Connector->m_bRunning = false;
	//设置状态
	m_Connector->m_enConnectStatus = ENULL;
	//m_Connector->m_pSocket = NULL;

	delete this;
}

bool CConnector_Io_Callback::open_callback()
{
	m_client->add_read_callback(this);
	m_client->add_write_callback(this);

	m_Connector->m_pSocket = m_client;
	//m_Connector->m_bRunning = true;
	m_Connector->m_enConnectStatus = EConnected;
	CNetModule::getSingleton().InsertServerSocket(m_client, this);

	//连接打开
	int nLen = sizeof(stConnectorPtr);
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
	pBlock->m_byMsgTag = MSC_S_SOCKET_LINK;
	pBlock->m_pSocketStream = m_client;
	stConnectorPtr ptr;
	ptr.pConnector = m_Connector;
	memcpy(pBlock->m_pBuf, &ptr, nLen);
	pBlock->m_dwLen = nLen;
	m_Connector->m_pWorkThread->InsertMseBlock(pBlock);

	++m_dwOpenCount;
	if (m_dwOpenCount < 10000)
	{
		LOG(INFO) << "connect to: " << m_client->get_peer(true) << " success! ";
	}
	//从异步服务器读取数据
	m_client->read(sizeof(CMD_Command), 0);

	return true;
}


void CConnector_Io_Callback::SendData(CMesBlock* pBlock)
{
	m_SendDataList.PushBack(pBlock);
}

void CConnector_Io_Callback::DataToOutstream(void)
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

void CConnector_Io_Callback::ClearData(void)
{
	m_inputStream.SetEmpty();
	m_outputStream.SetEmpty();

	while (CMesBlock* pBlock = m_SendDataList.GetHead())
	{
		pBlock->Deallocate();
		g_MsgBlockPool.DeleteObj(pBlock);
	}
}

bool CConnector_Io_Callback::OnEventSend(void)
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

bool CNetConnector::ConnectServer()
{
	aio_socket_stream* stream = aio_socket_stream::open(m_pHandle, m_strAddr.c_str(), 10);
	if (stream == NULL)
	{
		//设置状态
		m_enConnectStatus = ENULL;

		LOG(WARNING) << "open connect: " << m_strAddr.c_str() << " fail.";
		int nLen = sizeof(stConnectorPtr);
		CMesBlock* pBlock = g_MsgBlockPool.NewObj();
		if (pBlock != NULL)
		{
			CT_CHAR* pBuf = pBlock->Allocate(nLen);
			if (pBuf != NULL)
			{
				pBlock->m_byMsgTag = MSC_S_SOCKET_SHUT;
				pBlock->m_pSocketStream = m_pSocket;
				stConnectorPtr ptr;
				ptr.pConnector = this;
				memcpy(pBlock->m_pBuf, &ptr, nLen);
				pBlock->m_dwLen = nLen;
				m_pWorkThread->InsertMseBlock(pBlock);
			}
		}
		return false;
	}

	// 创建连接后的回调函数类
	CConnector_Io_Callback* callback = new CConnector_Io_Callback(stream, this, m_bDataEncrypt);

	// 添加连接成功的回调函数类
	stream->add_open_callback(callback);

	// 添加连接失败后回调函数类
	stream->add_close_callback(callback);

	// 添加连接超时后回调函数类
	stream->add_timeout_callback(callback);

	//设置状态
	m_enConnectStatus = EConnecting;

	return true;
}