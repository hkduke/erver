/* ----------------------------------------------------------------------------------------------------------


-----------------------------------------------------------------------------------------------------------*/
#ifndef __MsgDefineMacro_h__
#define __MsgDefineMacro_h__

#include "MsgDefine.h"

#define Declare_Msg_Begin( _MsgId, _MsgClassName )										\
class _MsgClassName : public NetMsgBody											\
		{																				\
		public:																			\
		virtual int GetType()														\
			{																			\
			return _MsgId;															\
			}																			\
			virtual void print() {}														\
			_MsgClassName() {}														\
			~_MsgClassName() {}

//complex class
#define Declare_Complex_Msg_Begin( _MsgId, _MsgClassName, _MsgParentClassName )										\
class _MsgClassName : public _MsgParentClassName											\
		{																				\
		public:																			\
		virtual int GetType()														\
			{																			\
			return _MsgId;															\
			}																			\
			virtual void print() {}														\
			/*_MsgClassName() {}*/														\
			~_MsgClassName() {}

#define	Declare_DecodeMsg_Function_Begin												\
	virtual int DecodeMsg( const unsigned char* pInBuffer, short nBufLen )		\
			{																			\
			if ( !pInBuffer || nBufLen <= 0 )										\
				{																		\
				return -1;															\
				}																		\
				short nMsgLength = nBufLen;												\
				unsigned char* pMsg = ( unsigned char* )pInBuffer;						\
				short sTempLength;														

#define	Declare_DecodeMsg_Empty_Function_Begin												\
	virtual int DecodeMsg( const unsigned char* pInBuffer, short nBufLen )		\
			{

#define Declare_DecodeMsg_Memory( _pMem, _nMemLen )										\
	sTempLength = ( short )DecodeMemory( &pMsg, ( char* )&_pMem, _nMemLen, nMsgLength);	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_Short( _sShort )												\
	sTempLength = ( short )DecodeShort( &pMsg, ( unsigned short* )&_sShort );\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_UnsignedShort( _usShort )										\
	sTempLength = ( short )DecodeShort( &pMsg, ( unsigned short* )&_usShort );	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_String( _szString, _nStrLen )									\
	sTempLength = ( short )DecodeString( &pMsg, _szString, _nStrLen, nMsgLength );	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_Int( nVal )													\
	sTempLength = ( short )DecodeInt( &pMsg, ( unsigned int* )&nVal );		\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_UnsignedInt( unVal )											\
	sTempLength = ( short )DecodeInt( &pMsg, ( unsigned int* )&unVal );		\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_Char( cVal )													\
	sTempLength = ( short )DecodeChar( &pMsg, ( unsigned char* )&cVal );	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_UnsignedChar( ucVal )											\
	sTempLength = ( short )DecodeChar( &pMsg, ( unsigned char* )&ucVal );	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_DateTime( dtVal )												\
	sTempLength = ( short )DecodeDateTime( &pMsg, dtVal, nMsgLength );		\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_CK( ckVal )													\
	sTempLength = ( short )DecodeCK( &pMsg, ckVal, nMsgLength );			\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_FileChunk( fcVal )													\
	sTempLength = ( short )DecodeFileChunk( &pMsg, fcVal, nMsgLength );			\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_Long( nlVal )													\
	sTempLength = ( short )DecodeLong( &pMsg, ( unsigned long* )&nlVal );	\
	nMsgLength -= sTempLength;

#define Declare_DecodeMsg_UnsignedLongLong( unVal )												\
	sTempLength = ( short )DecodeLongLong( &pMsg, ( unsigned long long* )&unVal );	\
	nMsgLength -= sTempLength;


#define Declare_DecodeMsg_Function_End													\
	if ( nMsgLength < 0 )													\
				{																		\
				return -1;															\
				}																		\
				return 0;																\
			}

#define Declare_DecodeMsg_Empty_Function_End													\
	return 0;}

#define Declare_EncodeMsg_Function_Begin													\
	virtual int EncodeMsg( unsigned char* pOutBuffer, short& nBufLength )		\
			{																			\
			if ( !pOutBuffer )														\
				{																		\
				return -1;															\
				}																		\
				unsigned char* pMsg = pOutBuffer;										\
				nBufLength = 0;															\
				short nMsgLength;

#define Declare_EncodeMsg_Empty_Function_Begin													\
	virtual int EncodeMsg( unsigned char* pOutBuffer, short& nBufLength )		\
			{

#define Declare_EncodeMsg_Memory( _pMem, _nMemLen )										\
	nMsgLength = ( short )EncodeMemory( &pMsg, ( char* )&_pMem, _nMemLen );	\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_Short( _sShort )												\
	nMsgLength = ( short )EncodeShort( &pMsg, ( unsigned short )_sShort );	\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_UnsignedShort( _usShort )										\
	nMsgLength = ( short )EncodeShort( &pMsg, ( unsigned short )_usShort );	\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_String( _szString, _nStrLen )									\
	nMsgLength = ( short )EncodeString( &pMsg, _szString, _nStrLen - 1 );	\
	nBufLength += nMsgLength;

#define Declare_EncodeMsg_Int( nVal )													\
	nMsgLength = ( short )EncodeInt( &pMsg, ( unsigned int )nVal );			\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_UnsignedInt( unVal )											\
	nMsgLength = ( short )EncodeInt( &pMsg, ( unsigned int )unVal );		\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_Char( cVal )													\
	nMsgLength = ( short )EncodeChar( &pMsg, ( unsigned char )cVal );		\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_UnsignedChar( ucVal )											\
	nMsgLength = ( short )EncodeChar( &pMsg, ( unsigned char )ucVal );		\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_DateTime( dtVal )												\
	nMsgLength = ( short )EncodeDateTime( &pMsg, dtVal );					\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_CK( ckVal )													\
	nMsgLength = ( short )EncodeCK( &pMsg, ckVal );							\
	nBufLength += nMsgLength;

#define Declare_EncodeMsg_FileChunk( fcVal )													\
	nMsgLength = ( short )EncodeFileChunk( &pMsg, fcVal );							\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_Long( nlVal )													\
	nMsgLength = ( short )EncodeLong( &pMsg, ( unsigned long )nlVal );		\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_UnsignedLongLong( unVal )											\
	nMsgLength = ( short )EncodeLongLong( &pMsg, ( unsigned long long )unVal );	\
	nBufLength += nMsgLength; 

#define Declare_EncodeMsg_Function_End													\
	return 0; }

#define Declare_EncodeMsg_Empty_Function_End													\
	return 0; }

#define Declare_Msg_Member( _sType, _sValue )											\
	_sType		_sValue;

#define Declare_Msg_Member_Array( _sType, _sValue, _sCount )							\
	_sType		_sValue[ _sCount ];

#define Declare_Msg_Member_TwoDimensional_Array( _sType, _sValue, _sCount1, _sCount2 )							\
	_sType		_sValue[ _sCount1 ][ _sCount2 ];

#define Declare_Msg_End };

#endif
