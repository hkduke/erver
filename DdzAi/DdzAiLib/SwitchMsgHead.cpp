/* ----------------------------------------------------------------------------------------------------------

Popcap Shanghai studio

Created by Zhang Hui in 2009-3-17
-----------------------------------------------------------------------------------------------------------*/
#include "PopWorldDefine.h"
#include "SwitchMsgHead.h"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <string.h>
#endif

/*---------------------------------------------------------------------------------------------------------*/
SwitchMsgHead::SwitchMsgHead()
{
	m_nTotalLength	= 0;
	m_nSrcFE	= -1;
	m_nDstFE	= -1;
	m_nSrcID	= -1;

	memset((void *)&m_stTransferInfo, 0, sizeof(m_stTransferInfo));

	m_bOptionFlag	= 0;
	m_nOptionLength	= 0;
	m_nBodyLength	= 0;
}

/*---------------------------------------------------------------------------------------------------------*/
SwitchMsgHead::~SwitchMsgHead()
{
}

/*---------------------------------------------------------------------------------------------------------*/
inline int EncodeShort( unsigned char ** pstrEncode, unsigned short usSrc )
{
	unsigned short usTemp;

	if( pstrEncode == 0 || *pstrEncode == 0 )
	{
		return 0;
	}

	usTemp = htons( usSrc );

	memcpy( ( void* )( *pstrEncode ), (const void *)&usTemp, sizeof( unsigned short ) );
	*pstrEncode += sizeof( unsigned short );

	return (int)sizeof( unsigned short );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int EncodeBYTE( unsigned char ** pstrEncode, unsigned char ucSrc )
{
	if( pstrEncode == 0 || *pstrEncode == 0 )
	{
		return 0;
	}

	**pstrEncode = ( unsigned char )ucSrc;
	*pstrEncode += sizeof( unsigned char );

	return ( int ) sizeof( unsigned char );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int EncodeInt( unsigned char **pstrEncode, unsigned int uiSrc )
{
	unsigned int uiTemp;

	if( pstrEncode == 0 || *pstrEncode == 0 )
	{
		return 0;
	}

	uiTemp = htonl( uiSrc );

	memcpy( ( void* )( *pstrEncode ), (const void *)&uiTemp, sizeof( unsigned int ) );
	*pstrEncode += sizeof( unsigned int );

	return ( int ) sizeof( unsigned int );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int EncodeMemory( unsigned char **pstrEncode, unsigned char *pcSrc, short sMemSize)
{
	if( pstrEncode == 0 || *pstrEncode == 0 || pcSrc == 0 || sMemSize <= 0 )
	{
		return 0;
	}

	memcpy(( void* )( *pstrEncode ), ( const void* )pcSrc, sMemSize );
	*pstrEncode += sMemSize;
	return sMemSize;
}

/*---------------------------------------------------------------------------------------------------------*/
int SwitchMsgHead::EncodeMsg( unsigned char * pMsg, short & nMsgLength )
{
	if( !pMsg )
	{
		return -1;
	}

	unsigned char * pTemp = pMsg;
	nMsgLength = 0;
	short nTempMsgLength = 0;

	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )nTempMsgLength );
	nMsgLength += nTempMsgLength;

	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_nSrcFE );
	nMsgLength += nTempMsgLength;

	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_nSrcID );
	nMsgLength += nTempMsgLength;

	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_nDstFE );
	nMsgLength += nTempMsgLength;

	nTempMsgLength = (short)EncodeBYTE( &pTemp, ( unsigned char )m_bTransferType );
	nMsgLength += nTempMsgLength;

	switch( m_bTransferType )
	{
	case TT_ServerToServer:
		{
			nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_stTransferInfo.nDestID );
			nMsgLength += nTempMsgLength;
		}
		break;
	
	case TT_ServerToGroup:
		{
			nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_stTransferInfo.stServers.nFENumber );
			nMsgLength += nTempMsgLength;

			int i;
			for( i = 0; i < m_stTransferInfo.stServers.nFENumber; i++ )
			{
				nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_stTransferInfo.stServers.anFEs[i] );
				nMsgLength += nTempMsgLength;
			}
		}
		break;

	case TT_Broadcast:
		{
		}
		break;
	
	case TT_ByUID:
		{
			nTempMsgLength = (short)EncodeInt( &pTemp, m_stTransferInfo.unUid );
			nMsgLength += nTempMsgLength;
			break;
		}
		break;

	default:
		{
			return -1;
		}
	}

	nTempMsgLength = (short)EncodeBYTE( &pTemp, ( unsigned char )m_bOptionFlag );
	nMsgLength += nTempMsgLength;
	
	if( m_nOptionLength > sizeof( m_vOption ) )
	{
		m_nOptionLength = sizeof( m_vOption );
	}
	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_nOptionLength );
	nMsgLength += nTempMsgLength;

	if( m_nOptionLength > 0 )
	{
		nTempMsgLength = (short)EncodeMemory( &pTemp, m_vOption, m_nOptionLength );
		nMsgLength += nTempMsgLength;
	}

	nTempMsgLength = (short)EncodeShort( &pTemp, ( unsigned short )m_nBodyLength );
	nMsgLength += nTempMsgLength;

	nTempMsgLength = nMsgLength + m_nBodyLength;
	pTemp = pMsg;
	EncodeShort( &pTemp, ( unsigned short )nTempMsgLength );

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
inline int DecodeShort( unsigned char ** pstrDecode, unsigned short * pusDest )
{
	unsigned short usTemp;

	if( pstrDecode == 0 || *pstrDecode == 0 || pusDest == 0 )
	{
		return 0;
	}

	memcpy( ( void* )&usTemp, ( const void* )( *pstrDecode ), sizeof( unsigned short ) );
	*pstrDecode += sizeof( unsigned short );

	*pusDest = ntohs( usTemp );

	return ( int ) sizeof( unsigned short );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int DecodeBYTE( unsigned char ** pstrDecode, unsigned char * pucDest )
{
	if( pstrDecode == 0 || *pstrDecode == 0 || pucDest == 0 )
	{
		return 0;
	}

	*pucDest = **pstrDecode;
	*pstrDecode += sizeof( unsigned char );

	return ( int ) sizeof( unsigned char );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int DecodeInt( unsigned char ** pstrDecode, unsigned int * puiDest )
{
	unsigned int uiTemp;

	if( pstrDecode == 0 || *pstrDecode == 0|| puiDest == 0 )
	{
		return 0;
	}

	memcpy( ( void* )&uiTemp, ( const void* )( *pstrDecode ), sizeof( unsigned int ) );
	*pstrDecode += sizeof( unsigned int );

	*puiDest = ntohl(uiTemp);

	return ( int ) sizeof( unsigned int );
}

/*---------------------------------------------------------------------------------------------------------*/
inline int DecodeMemory( unsigned char ** pstrDecode, unsigned char * pcDest, short sMemSize, int buffer_length )
{
	if( pstrDecode == 0 || *pstrDecode == 0 || pcDest == 0 || sMemSize <= 0 )
	{
		return 0;
	}

	if (sMemSize <= buffer_length)
	{
		memcpy( ( void* )pcDest, ( const void* )( *pstrDecode ), sMemSize );
		*pstrDecode += sMemSize;

		return sMemSize;
	}
	else
	{
		return 0;
	}
}


/*---------------------------------------------------------------------------------------------------------*/
int SwitchMsgHead::DecodeMsg( unsigned char * pMsg, short nMsgLength )
{
	if( !pMsg || nMsgLength <= 0 )
	{
		return -1;
	}

	unsigned char * pTemp = pMsg;
	short sLength = nMsgLength;
	short sTempLength;

	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short * )&m_nTotalLength );
	sLength -= sTempLength;

	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short* )&m_nSrcFE );
	sLength -= sTempLength;

	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short* )&m_nSrcID );
	sLength -= sTempLength;

	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short* )&m_nDstFE );
	sLength -= sTempLength;

	sTempLength = (short)DecodeBYTE( &pTemp, ( unsigned char * )&m_bTransferType );
	sLength -= sTempLength;

	switch( m_bTransferType )
	{
	case TT_ServerToServer:
		{
			sTempLength = (short)DecodeShort( &pTemp, ( unsigned short * )&m_stTransferInfo.nDestID );
			sLength -= sTempLength;
		}
		break;
	
	case TT_ServerToGroup:
		{
			sTempLength = (short)DecodeShort( &pTemp, ( unsigned short * )&m_stTransferInfo.stServers.nFENumber );
			sLength -= sTempLength;

			int i;
			for( i = 0; i < m_stTransferInfo.stServers.nFENumber; i++ )
			{
				sTempLength = (short)DecodeShort( &pTemp, (unsigned short *)&m_stTransferInfo.stServers.anFEs[i] );
				sLength -= sTempLength;
			}
			break;
		}
	
	case TT_Broadcast:
		{
		}
		break;

	case TT_ByUID:
		{
			sTempLength = (short)DecodeInt( &pTemp, &m_stTransferInfo.unUid );
			sLength -= sTempLength;
		}
		break;

	default:
		{
			return -1;
		}
	}

	sTempLength = (short)DecodeBYTE( &pTemp, ( unsigned char * )&m_bOptionFlag );
	sLength -= sTempLength;

	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short * )&m_nOptionLength );
	sLength -= sTempLength;

	if( m_nOptionLength > sizeof( m_vOption ) )
	{
		m_nOptionLength = sizeof( m_vOption );
	}
	
	if( m_nOptionLength > 0 )
	{
		sTempLength = (short)DecodeMemory( &pTemp, m_vOption, m_nOptionLength , sLength);
		sLength -= sTempLength;
	}
	
	sTempLength = (short)DecodeShort( &pTemp, ( unsigned short * )&m_nBodyLength );
	sLength -= sTempLength;
	
	if( sLength < 0 )
	{
		return -1;
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------
End of file */

