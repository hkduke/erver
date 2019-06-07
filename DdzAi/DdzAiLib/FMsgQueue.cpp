/* ------------------------------------------------------------------------------------------------------------
 
	Popcap Shanghai studio
 
	Created by Zhang Hui in 2009-3-16
 ------------------------------------------------------------------------------------------------------------*/
#include "FMsgQueue.h"
#include "Log.h"
#include <cstring>
#include <cstdio>

/* ----------------------------------------------------------------------------------------------------------*/
FMsgQueue::FMsgQueue()
{
	m_nSize			= 0;
	m_nMsgBufOffset	= -1;
	m_nBegin		= 0;
	m_nEnd			= 0;
	m_nFullFlag		= 0;

	m_pMsgBuffer	= 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
FMsgQueue::~FMsgQueue()
{
	if ( m_pMsgBuffer )
	{
		delete [] m_pMsgBuffer;
		m_pMsgBuffer = 0;
	}
}

/* ----------------------------------------------------------------------------------------------------------*/
FMsgQueue::FMsgQueue( int nTotalSize )
{
	Initialize( nTotalSize );
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::Initialize( int nTotalSize )
{
	m_nSize		= nTotalSize;
	m_nBegin	= 0;
	m_nEnd		= 0;
	m_nFullFlag	= 0;

	unsigned char * pMsgBuf = new unsigned char [ nTotalSize ];
	if ( !pMsgBuf )
	{
		m_nMsgBufOffset = -1;
		return -1;
	}

	m_nMsgBufOffset = 0;
	m_pMsgBuffer = pMsgBuf;

	return 0;
}


/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::Resume( int nTotalSize )
{
	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
bool FMsgQueue::IsQueueFull()
{
	int nMaxLength	= 0;
	int nBegin		= -1;
	int nEnd		= -1;

	GetCriticalData( &nBegin, &nEnd );

	if ( nBegin == nEnd )
	{
		nMaxLength = m_nSize;
	}
	else if ( nBegin > nEnd )
	{
		nMaxLength = nBegin - nEnd;
	}
	else
	{
		nMaxLength = ( m_nSize - nEnd ) + nBegin;
	}

	nMaxLength -= QUEUERESERVELENGTH;
	
	return ( nMaxLength > 0 ) ? false : true;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::PushTailMsg( const unsigned char * pInMsg, short nInMsgLength )
{
	if ( !pInMsg || nInMsgLength < 0 )
	{
		return -1;
	}

	if ( m_nMsgBufOffset < 0 || m_nSize <= 0 )
	{
		return -1;
	}

	if ( IsQueueFull() )
	{
		return -2;
	}

	int nBegin		= -1;
	int nEnd		= -1;

	if( GetCriticalData(&nBegin, &nEnd) )
	{
		return -3;
	}

	if( nBegin < 0 || nBegin >= m_nSize	|| nEnd < 0 || nEnd >= m_nSize )
	{
		Log_Msg( Log_Failed, "FMsgQueue: In MsgQueue::PushTailMsg, data crashed: begin = %d, end = %d", nBegin, nEnd );

		nBegin = 0;
		nEnd = 0;
		SetCriticalData( nBegin, nEnd );

		return -3;
	}

	int nMaxLength	= 0;

	if ( nBegin == nEnd )
	{
		nMaxLength = m_nSize;
	}
	else if ( nBegin > nEnd )
	{
		nMaxLength = nBegin - nEnd;
	}
	else
	{
		nMaxLength = ( m_nSize - nEnd ) + nBegin;
	}

	nMaxLength -= QUEUERESERVELENGTH;

	int nRet		= nEnd;

	if( (int)(nInMsgLength + sizeof(short)) > nMaxLength )
	{
		return -2;
	}
	
	unsigned char *	pMsgBuf = m_pMsgBuffer + m_nMsgBufOffset;

	unsigned char * pDest = &pMsgBuf[0];
	unsigned char * pSrc = (unsigned char  *)&nInMsgLength;

	int	i = 0;
	for( ; i < sizeof( nInMsgLength ); ++i )
	{
		pDest[nEnd] = pSrc[i];
		nEnd = ( nEnd + 1 ) % m_nSize;
	}

	if ( nBegin > nEnd )
	{
		memcpy((void *)&pMsgBuf[nEnd], (const void *)pInMsg, (size_t)nInMsgLength );
	}
	else
	{
		if( (int)nInMsgLength > ( m_nSize - nEnd ) )
		{
			memcpy( ( void* )&pMsgBuf[nEnd], ( const void* )&pInMsg[0], ( size_t )( m_nSize - nEnd ) );
			memcpy( ( void* )&pMsgBuf[0],(const void *)&pInMsg[ ( m_nSize - nEnd ) ], 
					(size_t)( nInMsgLength + nEnd - m_nSize ) );				/* pOutLength - ( m_nSize - nBegin ) */
		}
		else
		{
			memcpy((void *)&pMsgBuf[ nEnd ], ( const void* )&pInMsg[0], ( size_t )nInMsgLength);
		}
	}

	nEnd = ( nEnd + nInMsgLength ) % m_nSize;

	SetCriticalData( -1, nEnd );

	return nRet;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::GetHeadMsg( unsigned char * pOutMsg, short* pOutLength )
{
	if( !pOutMsg || !pOutLength )
	{
		return -1;
	}

	if ( m_nMsgBufOffset < 0 || m_nSize <= 0 )
	{
		return -2;
	}

	int nBegin	= -1;
	int nEnd	= -1;
	if( GetCriticalData( &nBegin, &nEnd ) )
	{
		return -3;
	}

	int nRet	= nBegin;
	int nLength	= *pOutLength;

	if ( nBegin == nEnd )
	{
		*pOutLength = 0;
		return nRet;
	}

	int nMaxLength = 0;
	if ( nBegin < nEnd )
	{
		nMaxLength = nEnd - nBegin;
	}
	else
	{
		nMaxLength = m_nSize - nBegin + nEnd;
	}

	if( nMaxLength < sizeof(short) )
	{
		*pOutLength = 0;
		nBegin = nEnd;
		SetCriticalData( nBegin, -1 );
		return -4;
	}

	unsigned char * pMsgBuf = m_pMsgBuffer + m_nMsgBufOffset;
	unsigned char * pDest	= (unsigned char  *)pOutLength;
	unsigned char * pSrc	= (unsigned char  *)&pMsgBuf[0];

	int i = 0;
	for( ; i < sizeof(short); ++i )
	{
		pDest[i] = pSrc[ nBegin ];
		nBegin = ( nBegin + 1 ) % m_nSize; 
	}

	if( nLength <= *pOutLength )
	{
		return -5;
	}

	if( ( *pOutLength ) > (int)( nMaxLength - sizeof(short) ) || *pOutLength < 0 )
	{
		*pOutLength = 0;

		nBegin = nEnd;
		SetCriticalData( nBegin, -1 );

		return -6;
	}

	pDest = (unsigned char  *)&pOutMsg[0];
	if ( nBegin < nEnd )
	{
		memcpy( ( void* )pDest, ( const void* )&pSrc[nBegin], ( size_t )( *pOutLength ) );
	}
	else
	{
		if ( *pOutLength > ( m_nSize - nBegin ) )
		{
			memcpy( ( void* )pDest, ( const void* )&pSrc[nBegin], ( size_t )( m_nSize - nBegin ) );
			pDest += ( m_nSize - nBegin );
			memcpy( ( void* )pDest, ( const void* )&pSrc[0], ( size_t )( *pOutLength + nBegin - m_nSize ) );		/* pOutLength - ( m_nSize - nBegin ) */

		}
		else
		{
			memcpy( ( void* )pDest, ( const void* )&pSrc[nBegin], ( size_t )( *pOutLength ) );
		}
	}

	nBegin = ( nBegin + ( *pOutLength ) ) % m_nSize;
	SetCriticalData( nBegin, -1 );

	return nRet;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::GetCriticalData( int* pBegin, int* pEnd )
{
	if( pBegin )
	{
		*pBegin = m_nBegin;
	}
	if( pEnd )
	{
		*pEnd = m_nEnd;
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::SetCriticalData( int nBegin, int nEnd )
{
	if ( nBegin >= 0 && nBegin < m_nSize )
	{ 
		m_nBegin = nBegin;
	}

	if ( nEnd >= 0 && nEnd < m_nSize )
	{
		m_nEnd = nEnd;
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
void FMsgQueue::CleanQueue()
{
	m_nBegin	= 0;
	m_nEnd		= 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::LoadFromFile( const char* szFileName )
{
	if( !szFileName || !m_pMsgBuffer )
	{
		return -1;
	}

	FILE* fpLoadFile = fopen( szFileName, "r" );
	int iPageSize = 4096;
	int iPageCount = 0;
	int i;
	unsigned char  *pPage = m_pMsgBuffer;

	if( !fpLoadFile )
	{
		return -1;
	}

	fread( ( void* )&m_nSize, sizeof( int ), 1, fpLoadFile );
	fread( ( void* )&m_nMsgBufOffset, sizeof( int ), 1, fpLoadFile );
	fread( ( void* )&m_nBegin, sizeof( int ), 1, fpLoadFile );
	fread( ( void* )&m_nEnd, sizeof( int ), 1, fpLoadFile );

	iPageCount = m_nSize / iPageSize;
	for( i = 0; i < iPageCount; i++ )
	{
		fread( ( void* )pPage, iPageSize, 1, fpLoadFile );
		pPage += iPageSize;
	}

	fread( ( void* )pPage, m_nSize - iPageSize*iPageCount, 1, fpLoadFile );

	fclose( fpLoadFile );

	return 0;	
	
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::SaveToFile( const char* szFileName )
{
	if( !szFileName || !m_pMsgBuffer )
	{
		return -1;
	}

	FILE* fpSaveFile = fopen( szFileName, "w" );
	int iPageSize = 4096;
	int iPageCount = 0;
	int i;
	unsigned char  *pPage = m_pMsgBuffer;

	if( !fpSaveFile )
	{
		return -1;
	}

	fwrite( ( const void* )&m_nSize, sizeof(int), 1, fpSaveFile );
	fwrite( ( const void* )&m_nMsgBufOffset, sizeof(int), 1, fpSaveFile );
	fwrite( ( const void* )&m_nBegin, sizeof(int), 1, fpSaveFile );
	fwrite( ( const void* )&m_nEnd, sizeof(int), 1, fpSaveFile );

	iPageCount = m_nSize / iPageSize;
	for( i = 0; i < iPageCount; i++ )
	{
		fwrite( ( const void* )pPage, iPageSize, 1, fpSaveFile );
		pPage += iPageSize;
	}
	fwrite( ( const void* )pPage, m_nSize - iPageSize*iPageCount, 1, fpSaveFile );

	fclose( fpSaveFile );

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int FMsgQueue::IsQueueEmpty()
{
	if ( m_nBegin == m_nEnd )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


/* ----------------------------------------------------------------------------------------------------------
   End of file */
