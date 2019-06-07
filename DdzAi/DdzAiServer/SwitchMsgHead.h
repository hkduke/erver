/* ----------------------------------------------------------------------------------------------------------


 in 2009-3-17
----------------------------------------------------------------------------------------------------------*/
#ifndef __SwitchMsgHead_h__
#define __SwitchMsgHead_h__

#ifndef MAX_SERVER_NUMBER
#define MAX_SERVER_NUMBER			256
#endif

#define SWITCH_RESERVE				((BYTE)0x01)
#define SWITCH_COMMAND				((BYTE)0x02)
#define SWITCH_IGNOREFAIL			((BYTE)0x04)
#define BASE_SWITCH_HEAD_LENGTH		14


enum eConnectServerTypes
{
	Cst_MainServer = 1,
	Cst_DBServer,
	Cst_UtilServer,
	Cst_DIRServer,
	Cst_SwitchServer,
	Cst_MissionServer,
	Cst_ServerMax,
};

enum eSwitchCommand
{
	COMMAND_NULL		= 0,
	COMMAND_REGSERVER	= 1,               // server register
	COMMAND_KEEPALIVE	= 2,               // heartbeat
};

#define MAX_SWITCH_OPTION_LENGTH		256

enum SwitchServerType
{
	SST_DBCACHE					= 0,
	SST_FILESERVER				= 1,
};

enum eTransferTypes
{
	TT_ServerToServer = 0,				// server to server
	TT_ServerToGroup,					// server to group
	TT_Broadcast,						// broadcast
	TT_ByUID,							// transfer by uid 
};

class SwitchMsgHead
{
public:
	SwitchMsgHead();
	~SwitchMsgHead();

	int EncodeMsg( unsigned char * pMsg, short& nMsgLength );
	int DecodeMsg( unsigned char * pMsg, short nMsgLength );


	short			m_nTotalLength;
	short			m_nSrcFE;
	short			m_nSrcID;
	short			m_nDstFE;
	unsigned char	m_bTransferType;

	union _U
	{
		unsigned int unUid;
		short		 nDestID;
		struct	_tagServers
		{
			short	nFENumber;
			short	anFEs[MAX_SERVER_NUMBER];
		} stServers;
	}				m_stTransferInfo;

	unsigned char	m_bOptionFlag;
	short			m_nOptionLength;
	unsigned char	m_vOption[MAX_SWITCH_OPTION_LENGTH];
	short			m_nBodyLength;
};

#endif
