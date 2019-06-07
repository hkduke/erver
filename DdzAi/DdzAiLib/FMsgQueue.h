/* ----------------------------------------------------------------------------------------------------------
 
	Popcap Shanghai studio

	Created by Zhang Hui in 2009-3-16
 ----------------------------------------------------------------------------------------------------------*/

#ifndef __FMsgQueue_h__
#define __FMsgQueue_h__

#ifndef QUEUERESERVELENGTH
#define QUEUERESERVELENGTH        8				/* reserve length */
#endif

class FMsgQueue
{
public:
	enum MsgQueueLock
	{
		MQL_GateToLogic = 0,
		MQL_LogicToGate,
		MQL_Max,
	};

	FMsgQueue();
	FMsgQueue( int nTotalSize );
	~FMsgQueue();

	int				Initialize( int nTotalSize );
	int				Resume( int nTotalSize );
	
	int				PushTailMsg( const unsigned char * pMsg, short nLength );
	int				GetHeadMsg( unsigned char * pOutMsg, short* pOutLength );

	void			CleanQueue();
	
	int				SaveToFile( const char* szFileName );
	int				LoadFromFile( const char* szFileName );

	int				IsQueueEmpty();

private:
	
	int				GetCriticalData( int* pBegin, int* pEnd );
	int				SetCriticalData( int nBegin, int nEnd );
	bool			IsQueueFull();

	int				m_nSize;
	int				m_nMsgBufOffset;
	int				m_nBegin;
	int				m_nEnd;
	short			m_nFullFlag;

	unsigned char*	m_pMsgBuffer;

};





#endif
