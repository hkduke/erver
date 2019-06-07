#include "DefGameLogicExport.h"
#include "GameProcess.h"


//得到桌子实例
extern "C" ITableFrameSink* GetTableInstance()
{
	return new CGameProcess;
}

//删除桌子实例
extern "C" void DelTableInstance(ITableFrameSink* pITableFrameSink)
{
	if (NULL != pITableFrameSink)
	{
		delete pITableFrameSink;
	}
	pITableFrameSink = NULL;
}

//确认配置数据
extern "C" void ValidateRoomConfig(CT_DWORD& dwLittlePlayNums,CT_DWORD& dwManyPlayNums)
{
}











