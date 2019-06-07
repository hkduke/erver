
#include "../include/MsgBodyLogic.h"

#include <new>
#include <stdio.h>
/*---------------------------------------------------------------------------------------------------------*/

#define MAP_MSGID_TO_MSGBODY(id, cls)	\
	case id: { NetMsgBody* msgbody = new (pBuf) cls(); if ( msgbody->GetType() != id )  { return 0; } return msgbody; }

NetMsgBody* NetMsg::CreateMsgBody( void* pBuf, short sMsgID )
{
	assert(pBuf != NULL);

	switch( sMsgID ) {
	//----------------------------Start of Mapping --------------------------------
	// Login <-> Clients
	MAP_MSGID_TO_MSGBODY(MC_C2S_Grab_Landlord,								    C2S_Grab_Landlord);
	MAP_MSGID_TO_MSGBODY(MC_S2C_Grab_Landlord_Result,							S2C_Grab_Landlord_Result);
	MAP_MSGID_TO_MSGBODY(MC_C2S_Play_Card,										C2S_Play_Card);
	MAP_MSGID_TO_MSGBODY(MC_S2C_Play_Card_Result,								S2C_Play_Card_Result);
	MAP_MSGID_TO_MSGBODY(MC_C2S_Passive_Play_Card,								C2S_Passive_Play_Card);
	MAP_MSGID_TO_MSGBODY(MC_S2C_Passive_Play_Card_Result,						S2C_Passive_Play_Card_Result);

	//---------------------------- End of Mapping --------------------------------
	default: printf("MsgDefine.cpp: msg %d not defined.\n", sMsgID);
	}
	return NULL;
}

#undef MAP_MSGID_TO_MSGBODY


//---------------------------------------------------------------------------------------------------------
//class NetMsgHead
NetMsgHead::NetMsgHead()
{
}

NetMsgHead::~NetMsgHead()
{
}

int NetMsgHead::EncodeMsg( unsigned char* pOutBuffer, short& nBufLength )
{
	if ( !pOutBuffer )
	{
		return -1;
	}

	unsigned char* pMsg = pOutBuffer;
	short nMsgLength = 0;
	nBufLength = 0;
	nMsgLength = short(EncodeShort( &pMsg, ( unsigned short )m_sMsgID ));
	//add for keyindex
	nBufLength +=nMsgLength;
	return 0;
}

int	NetMsgHead::DecodeMsg( const unsigned char* pInBuffer, short nBufLength )
{
	if ( !pInBuffer || nBufLength <= 0 )
	{
		return -1;
	}

	short nMsgLength = nBufLength;
	unsigned char* pMsg = const_cast<unsigned char*>(pInBuffer);

	nMsgLength -= short(DecodeShort( &pMsg, (unsigned short *)&m_sMsgID ));
	if( nMsgLength < 0 )
	{
		return -1;
	}

	return 0;
}

void NetMsgHead::print()
{
}

//---------------------------------------------------------------------------------------------------------
//class NetMsgBody
NetMsgBody::NetMsgBody()
{
}

NetMsgBody::~NetMsgBody()
{
}

//---------------------------------------------------------------------------------------------------------
//class NetMsg
NetMsg::NetMsg( void* pMsg )
{
	m_pMsgBody = ( NetMsgBody* )pMsg;
}

NetMsg::~NetMsg()
{
	m_pMsgBody = 0;
}

int NetMsg::EncodeMsg( unsigned char* pOutBuffer, short& nBufLength )
{
	if ( !pOutBuffer )
	{
		return NetMsgEncodeMsgError - 1;
	}

	unsigned char* pMsgBuffer = pOutBuffer;
	short nMsgLength;
	nBufLength = 0;

	if ( m_oHead.EncodeMsg( pMsgBuffer, nMsgLength ) )
	{
		return NetMsgEncodeMsgError - 2;
	}

	nBufLength += nMsgLength;
	pMsgBuffer += nMsgLength;

	if( !m_pMsgBody )
	{
		return 0;
	}

	if( m_pMsgBody->EncodeMsg( pMsgBuffer, nMsgLength ) )
	{
		return NetMsgEncodeMsgError - 3;
	}

	nBufLength += nMsgLength;
	pMsgBuffer += nMsgLength;

	return 0;
}

int NetMsg::DecodeMsg( const unsigned char* pInBuffer, short nBufLength )
{
	if ( !pInBuffer )
	{
		return NetMsgDecodeMsgError - 1;
	}

	if ( !m_pMsgBody )
	{
		return NetMsgDecodeMsgError - 2;
	}

	const unsigned char* pMsgBuffer = pInBuffer;
	if ( m_oHead.DecodeMsg( pMsgBuffer, nBufLength ) )
	{
		return NetMsgDecodeMsgError - 3;
	}
	
	if ( nBufLength == sizeof(m_oHead.m_sMsgID) )
    {
           return 0;
    }
	pMsgBuffer += sizeof(m_oHead.m_sMsgID);
	short nMsgLength = nBufLength - sizeof(m_oHead.m_sMsgID);

	NetMsgBody* pMsgBody = CreateMsgBody( ( void* )m_pMsgBody, m_oHead.m_sMsgID );
	if ( !pMsgBody )
	{
		return NetMsgDecodeMsgError - 4;
	}

	if ( pMsgBody->DecodeMsg( (const unsigned char*)pMsgBuffer, nMsgLength ) )
	{
		return NetMsgDecodeMsgError - 5;
	}

	return 0;
}

void NetMsg::print()
{
	m_oHead.print();

	if( m_pMsgBody )
	{
		m_pMsgBody->print();
	}
}

