/* ----------------------------------------------------------------------------------------------------------



 in 2009-3-19
-----------------------------------------------------------------------------------------------------------*/
#ifndef __DBCacheServer_h__
#define __DBCacheServer_h__

#include "TcpConnection.h"
#include "Communication2.h"
#include "AiThread.h"

class NetMsg;

typedef enum _DBCacheServerRunFlag
{
	Dbcsrf_Running				= 0 ,
	Dbcsrf_ReConfig				= 1 , 
	Dbcsrf_DBCacheServerExit	= 2 ,
}DBCacheServerRunFlag;

const int Max_Ai_Thread = 16;

typedef struct
{
	int                   m_iServerID;
	char                  m_szServerIP[30];
} DdzAiServerConfig;

class DdzAiServer : public ICommunicationCommand
{
public:
	DdzAiServer();
	~DdzAiServer();
	
	int				Initialize( const char* szCfg, bool bRunTestCase = false );
	void			Run();
	void			SetRunFlag( DBCacheServerRunFlag eFlag );

public:
	virtual int Execute( SwitchMsgHead * header, void* pMsg, short nMsgLength, int iConnection );
	virtual void Notify(){};


private:
	void			PrintMsgToLog( BYTE* pMsg, int nMsgLength );

	int				ReadConfigFile( const char* szCfg );

	void			InternalCheck();
	int				SendInternalMsgToDBThread( int iDBThreadId, NetMsg* pMsg );
	void			PrintServerStatistic();

	int				ReceiveAndDispatchMsg();

	static void		RecvMsgCallback(SwitchMsgHead * header, short msgLen, char * pMsg, short bufLen, char * pBuf, void * cbData);

	int									m_eRunFlag;

	Communication2                      m_oCommunication2;

	time_t								m_tLastPrintStatTime;
	time_t								m_tLastDisconnectCheckTime;

	AiThread*							m_vAiThreadList[Max_Ai_Thread];

	const char*							m_szCfgFile;
	DdzAiServerConfig					m_oServerConfig;

	bool m_bCommunicationInit;
};





#endif
