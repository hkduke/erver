#ifndef		__DEF_H_GAME_LOGIC_EXPORT_H__
#define		__DEF_H_GAME_LOGIC_EXPORT_H__

#ifdef _WIN32
#if	defined (__GAME_LOGIC__)
#define		_GAME_LOGIC_EXPORT_	__declspec( dllexport )  
#else
#define		_GAME_LOGIC_EXPORT_	__declspec( dllimport )
#endif
#else
#define  _GAME_LOGIC_EXPORT_
#endif

#include "CTType.h"



class ITableFrameSink;

//得到桌子实例
extern "C" _GAME_LOGIC_EXPORT_ ITableFrameSink* GetTableInstance();
//删除桌子实例
extern "C" _GAME_LOGIC_EXPORT_ void DelTableInstance(ITableFrameSink* pITableFrameSink);
//确认配置数据
extern "C" _GAME_LOGIC_EXPORT_ void ValidateRoomConfig(CT_DWORD& dwLittlePlayNums,CT_DWORD& dwManyPlayNums);


#endif	//__DEF_H_GAME_LOGIC_EXPORT_H__








