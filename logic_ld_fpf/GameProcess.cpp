
#include "stdafx.h"
#include <stdio.h>
#include "GameProcess.h"
#include "ITableFrame.h"
#include "CRand.h"
#include <time.h>
#include <vector>
#include <algorithm>
//#include <json/json.h>
#include <math.h>
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"

//打印日志
#define		PRINT_LOG_INFO				0
#define		PRINT_LOG_WARNING			1

////////////////////////////////////////////////////////////////////////

//动作状态
#define  SEND_CARD_STATUS						1					//发牌状态
#define  OUT_CARD_STATUS						2					//出牌状态

//定时器
#define ID_TIME_READY							(101)				//准备定时器
#define ID_TIME_OUT_CARD						(102)				//出牌定时器
#define ID_TIME_ACTION							(103)				//动作定时器
#define ID_TIME_SEND_CARD						(104)				//摸牌定时器

#define TIME_READY								(1000)				//准备


#define END_HUXI	100
////////////////////////////////////////////////////////////////////////


CGameProcess::CGameProcess(void) 
	: /*m_dwGameType(0)
	//, m_dwServiceCost(0)
	, */m_pGameDeskPtr(NULL)
{
	//庄家玩家
	m_dwBankerUser = INVALID_CHAIR;
	//清理游戏数据
	ClearGameData();
	//初始化数据
	InitGameData();
}

CGameProcess::~CGameProcess(void)
{
}

//获得游戏配置信息
void CGameProcess::GetGameConfigInfo()
{
	m_cbOutCardTime = 15;								//出牌时间
	m_cbOperateTime = 15;								//操作时间
}

//初始化游戏数据
void CGameProcess::InitGameData()
{
	//系统时间
	m_dwSysTime = 0;
	//游戏状态
	m_cbGameStatus = GAME_STATUS_FREE;
	//剩余数目
	m_cbLeftCardCount = MAX_CARD_TOTAL;
	//库存扑克
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//骰子点数
	memset(m_cbSice, 0, sizeof(CT_BYTE) * 2);
	//庄家玩家
	m_dwBankerUser = INVALID_CHAIR;
	//当前玩家
	m_dwCurrentUser = INVALID_CHAIR;
	//供应玩家
	m_dwProvideUser = INVALID_CHAIR;
	//还原玩家
	m_dwResumeUser = INVALID_CHAIR;
	//吃胡玩家
	m_dwHuCardUser = INVALID_CHAIR;
	//供应牌
	m_cbProvideCard = 0x00;
	//胡的扑克
	m_cbHuCard = 0x00;
	//胡牌类型(0平胡,1放炮,2自模)
	m_cbHuCardType = 0;
	//最大胡息
	m_wMaxHuXi = 200;

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		memset(m_cbCardIndex[i], 0, MAX_INDEX);				//玩家手牌数据
		memset(m_chUserNickName[i], 0, NICKNAME_LEN);			//玩家昵称
		memset(&m_WeaveArray[i], 0, sizeof(tagWeaveItem)*MAX_WEAVE);	//组合扑克
	}
	//组合数目
	memset(m_cbWeaveCount, 0, sizeof(m_cbWeaveCount));
	//丢弃数目
	memset(m_cbDiscardCount, 0, sizeof(m_cbDiscardCount));
	memset(&m_cbDiscardCard, 0, sizeof(m_cbDiscardCard));	//丢弃记录

	memset(m_stUserAction, 0, sizeof(m_stUserAction));

	//过牌
	memset(m_bGuoPengCard, 0, sizeof(m_bGuoPengCard));
	memset(m_bGuoChiCard, 0, sizeof(m_bGuoChiCard));
	//偎牌后不能胡牌
	memset(m_bWeiCannotHu, 0, sizeof(m_bWeiCannotHu));
	//出了别人有偎跑的牌
	memset(m_bOutOtherWeiPao, 0, sizeof(m_bOutOtherWeiPao));
	//是否摸过牌了(判断地胡）
	m_bSendCard = false;

	//特殊番判断
	//庄闲出牌次数
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));
	//天胡标志
	m_bTianHuFlag = CT_FALSE;
	//地胡标志
	m_bDiHuFlag = CT_FALSE;

	//响应标志
	memset(m_bResponse, 0, sizeof(m_bResponse));
	//玩家操作
	memset(m_stUserOperate, 0, sizeof(m_stUserOperate));
	//玩家弃胡
	memset(m_cbUserQiHu, 0, sizeof(m_cbUserQiHu));

	//动作状态
	m_cbActionStatus = 0;
	//是否有提跑动作
	m_cbTiPaoAction = 0;
	//偎动作(1有偎)
	m_cbWeiAction = 0;

	memset(&m_HuCardInfo, 0, sizeof(m_HuCardInfo));

	memset(&m_bTiPaoCannotOutCard, 0, sizeof(m_bTiPaoCannotOutCard));

	//动作流水号
	m_dwActionNumber = 0;						

	if (NULL!=m_pGameDeskPtr)
	{//获取私人房信息
		m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
	}
	memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));
}

//解释私人场数据
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{
	if (strlen(pBuf) == 0)
	{
		return;
	}
	//获得私人扬数据
	acl::json jsonCond(pBuf);

	const acl::json_node* pMaxHuxi = jsonCond.getFirstElementByTagName("max_huxi");
	if (pMaxHuxi == NULL)
	{
		LOG(WARNING) << "parse max hu xi fail, room num:" << m_PrivateTableInfo.dwRoomNum;
		m_wMaxHuXi = 200;
		return;
	}
	if (pMaxHuxi->is_number())
	{
		m_wMaxHuXi = (CT_WORD)*pMaxHuxi->get_int64();
		return;
	}
	m_wMaxHuXi = 200;
	
}

//更新庄家
CT_VOID CGameProcess::UpdateBankerUser()
{
	if (m_dwBankerUser == INVALID_CHAIR)
	{//房间房主椅子id
		m_dwBankerUser = m_pGameDeskPtr->GetPRoomOwnerChairID();
		if (m_dwBankerUser == INVALID_CHAIR)
		{
			m_dwBankerUser = rand() % GAME_PLAYER;
		}
	}
}

//洗牌
void CGameProcess::XiPai(CT_BYTE cbCardData[GAME_PLAYER][MAX_COUNT], CT_BYTE cbCardCount[GAME_PLAYER])
{
	//是否重新洗牌
	CT_BOOL bXiPai = false;
	CT_BYTE cbCount = 0;
	do
	{
		bXiPai = false;
		//洗牌
		m_cbLeftCardCount = MAX_CARD_TOTAL;
		m_GameLogic.RandCardData(m_cbRepertoryCard, MAX_CARD_TOTAL);

		//////////////////////////////////////////
		/*m_cbRepertoryCard[79] = 0x11;
		m_cbRepertoryCard[78] = 0x11;
		m_cbRepertoryCard[77] = 0x11;
		m_cbRepertoryCard[76] = 0x02;
		m_cbRepertoryCard[75] = 0x02;
		m_cbRepertoryCard[74] = 0x02;
		m_cbRepertoryCard[73] = 0x04;
		m_cbRepertoryCard[72] = 0x05;
		m_cbRepertoryCard[71] = 0x06;
		m_cbRepertoryCard[70] = 0x14;
		m_cbRepertoryCard[69] = 0x15;
		m_cbRepertoryCard[68] = 0x07;
		m_cbRepertoryCard[67] = 0x08;
		m_cbRepertoryCard[66] = 0x09;
		m_cbRepertoryCard[65] = 0x17;
		m_cbRepertoryCard[64] = 0x18;
		m_cbRepertoryCard[63] = 0x19;
		m_cbRepertoryCard[62] = 0x0A;
		m_cbRepertoryCard[61] = 0x0A;
		m_cbRepertoryCard[60] = 0x1A;

		m_cbRepertoryCard[59] = 0x12;
		m_cbRepertoryCard[58] = 0x12;
		m_cbRepertoryCard[57] = 0x01;
		m_cbRepertoryCard[56] = 0x04;
		m_cbRepertoryCard[55] = 0x04;
		m_cbRepertoryCard[54] = 0x14;
		m_cbRepertoryCard[53] = 0x05;
		m_cbRepertoryCard[52] = 0x05;
		m_cbRepertoryCard[51] = 0x15;
		m_cbRepertoryCard[50] = 0x15;
		m_cbRepertoryCard[49] = 0x06;
		m_cbRepertoryCard[48] = 0x06;
		m_cbRepertoryCard[47] = 0x16;
		m_cbRepertoryCard[46] = 0x07;
		m_cbRepertoryCard[45] = 0x08;
		m_cbRepertoryCard[44] = 0x19;
		m_cbRepertoryCard[43] = 0x19;
		m_cbRepertoryCard[42] = 0x0A;
		m_cbRepertoryCard[41] = 0x1A;
		m_cbRepertoryCard[40] = 0x01;

		m_cbRepertoryCard[39] = 0x11;
		m_cbRepertoryCard[38] = 0x12;
		m_cbRepertoryCard[37] = 0x03;
		m_cbRepertoryCard[36] = 0x13;
		m_cbRepertoryCard[35] = 0x13;
		m_cbRepertoryCard[34] = 0x13;
		m_cbRepertoryCard[33] = 0x04;
		m_cbRepertoryCard[32] = 0x16;
		m_cbRepertoryCard[31] = 0x06;
		m_cbRepertoryCard[30] = 0x07;
		m_cbRepertoryCard[29] = 0x07;
		m_cbRepertoryCard[28] = 0x18;
		m_cbRepertoryCard[27] = 0x18;
		m_cbRepertoryCard[26] = 0x09;
		m_cbRepertoryCard[25] = 0x09;
		m_cbRepertoryCard[24] = 0x09;
		m_cbRepertoryCard[23] = 0x19;
		m_cbRepertoryCard[22] = 0x1A;
		m_cbRepertoryCard[21] = 0x1A;
		m_cbRepertoryCard[20] = 0x0A;

		m_cbRepertoryCard[19] = 0x01;
		m_cbRepertoryCard[18] = 0x03;
		m_cbRepertoryCard[17] = 0x16;
		m_cbRepertoryCard[16] = 0x03;
		m_cbRepertoryCard[15] = 0x08;
		m_cbRepertoryCard[14] = 0x02;
		m_cbRepertoryCard[13] = 0x16;
		m_cbRepertoryCard[12] = 0x15;
		m_cbRepertoryCard[11] = 0x17;
		m_cbRepertoryCard[10] = 0x08;
		m_cbRepertoryCard[9] = 0x11;
		m_cbRepertoryCard[8] = 0x12;
		m_cbRepertoryCard[7] = 0x12;
		m_cbRepertoryCard[6] = 0x13;
		m_cbRepertoryCard[5] = 0x14;
		m_cbRepertoryCard[4] = 0x15;
		m_cbRepertoryCard[3] = 0x16;
		m_cbRepertoryCard[2] = 0x17;
		m_cbRepertoryCard[1] = 0x17;
		m_cbRepertoryCard[0] = 0x1A;
		m_dwBankerUser = 0;*/
		


	/*	m_cbRepertoryCard[79] = 0x09;
		m_cbRepertoryCard[78] = 0x09;
		m_cbRepertoryCard[77] = 0x08;
		m_cbRepertoryCard[76] = 0x19;
		m_cbRepertoryCard[75] = 0x19;
		m_cbRepertoryCard[74] = 0x19;
		m_cbRepertoryCard[73] = 0x19;
		m_cbRepertoryCard[72] = 0x01;
		m_cbRepertoryCard[71] = 0x11;
		m_cbRepertoryCard[70] = 0x02;
		m_cbRepertoryCard[69] = 0x12;
		m_cbRepertoryCard[68] = 0x12;
		m_cbRepertoryCard[67] = 0x03;
		m_cbRepertoryCard[66] = 0x13;
		m_cbRepertoryCard[65] = 0x13;
		m_cbRepertoryCard[64] = 0x14;
		m_cbRepertoryCard[63] = 0x14;
		m_cbRepertoryCard[62] = 0x04;
		m_cbRepertoryCard[61] = 0x16;
		m_cbRepertoryCard[60] = 0x16;

		m_cbRepertoryCard[59] = 0x06;
		m_cbRepertoryCard[58] = 0x06;
		m_cbRepertoryCard[57] = 0x08;
		m_cbRepertoryCard[56] = 0x08;
		m_cbRepertoryCard[55] = 0x07;
		m_cbRepertoryCard[54] = 0x07;
		m_cbRepertoryCard[53] = 0x0A;
		m_cbRepertoryCard[52] = 0x0A;
		m_cbRepertoryCard[51] = 0x01;
		m_cbRepertoryCard[50] = 0x01;
		m_cbRepertoryCard[49] = 0x03;
		m_cbRepertoryCard[48] = 0x03;
		m_cbRepertoryCard[47] = 0x04;
		m_cbRepertoryCard[46] = 0x04;
		m_cbRepertoryCard[45] = 0x13;
		m_cbRepertoryCard[44] = 0x13;
		m_cbRepertoryCard[43] = 0x14;
		m_cbRepertoryCard[42] = 0x14;
		m_cbRepertoryCard[41] = 0x15;
		m_cbRepertoryCard[40] = 0x15;

		m_cbRepertoryCard[39] = 0x16;
		m_cbRepertoryCard[38] = 0x16;
		m_cbRepertoryCard[37] = 0x17;
		m_cbRepertoryCard[36] = 0x17;
		m_cbRepertoryCard[35] = 0x18;
		m_cbRepertoryCard[34] = 0x18;
		m_cbRepertoryCard[33] = 0x1A;
		m_cbRepertoryCard[32] = 0x1A;
		m_cbRepertoryCard[31] = 0x01;
		m_cbRepertoryCard[30] = 0x01;
		m_cbRepertoryCard[29] = 0x02;
		m_cbRepertoryCard[28] = 0x02;
		m_cbRepertoryCard[27] = 0x03;
		m_cbRepertoryCard[26] = 0x03;
		m_cbRepertoryCard[25] = 0x04;
		m_cbRepertoryCard[24] = 0x04;
		m_cbRepertoryCard[23] = 0x05;
		m_cbRepertoryCard[22] = 0x09;
		m_cbRepertoryCard[21] = 0x11;
		m_cbRepertoryCard[20] = 0x12;

		m_cbRepertoryCard[19] = 0x1A;

		m_cbRepertoryCard[18] = 0x11;
		m_cbRepertoryCard[17] = 0x18;
		m_cbRepertoryCard[16] = 0x13;
		m_cbRepertoryCard[15] = 0x14;
		m_cbRepertoryCard[14] = 0x15;
		m_cbRepertoryCard[13] = 0x02;
		m_cbRepertoryCard[12] = 0x11;

		m_cbRepertoryCard[11] = 0x08;
		m_cbRepertoryCard[10] = 0x05;
		m_cbRepertoryCard[9] = 0x15;
		m_cbRepertoryCard[8] = 0x08;
		m_cbRepertoryCard[7] = 0x14;
		m_cbRepertoryCard[6] = 0x07;
		m_cbRepertoryCard[5] = 0x12;
		m_cbRepertoryCard[4] = 0x0A;
		m_cbRepertoryCard[3] = 0x05;
		m_cbRepertoryCard[2] = 0x11;
		m_cbRepertoryCard[1] = 0x08;
		m_cbRepertoryCard[0] = 0x12;
		m_dwBankerUser = 0;
		*/
		//////////////////////////////////////////
		//分发扑克
		CT_INT32 iHandCardNums = MAX_COUNT - 1;
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			cbCardCount[i] = iHandCardNums;
			m_cbLeftCardCount -= iHandCardNums;
			m_GameLogic.SwitchToCardIndex(&m_cbRepertoryCard[m_cbLeftCardCount], iHandCardNums, m_cbCardIndex[i]);
			memcpy(cbCardData[i], &m_cbRepertoryCard[m_cbLeftCardCount], sizeof(CT_BYTE)*iHandCardNums);
		}

		//发送扑克(庄家21张)
		m_cbProvideCard = m_cbRepertoryCard[--m_cbLeftCardCount];
		m_cbCardIndex[m_dwBankerUser][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)]++;
		cbCardData[m_dwBankerUser][MAX_COUNT - 1] = m_cbProvideCard;
		++cbCardCount[m_dwBankerUser];
		//是否重新洗牌
		bXiPai = IsRandCard();

		cbCount++;
	} while (bXiPai && cbCount<100);
}

//是否重新洗牌
CT_BOOL CGameProcess::IsRandCard()
{
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		CT_BYTE cbTiCardCount = 0;
		CT_BYTE cbTiCardIndex[5] = { 0 };
		memset(cbTiCardIndex, 0, sizeof(cbTiCardIndex));
		CT_BYTE cbWeiCardCount = 0;
		CT_BYTE cbWeiCardIndex[7] = { 0 };
		memset(cbWeiCardIndex, 0, sizeof(cbWeiCardIndex));

		//类型统计
		cbTiCardCount = m_GameLogic.GetAcitonTiCard(m_cbCardIndex[i], cbTiCardIndex);
		cbWeiCardCount = m_GameLogic.GetActionWeiCard(m_cbCardIndex[i], cbWeiCardIndex);
		CT_BYTE cbDuiZiCount = GetDuiZiCount(m_cbCardIndex[i]);

		//设置标志
		if (cbTiCardCount >= 2 || cbWeiCardCount >= 4 || cbDuiZiCount>=7)
		{//重新洗牌
			return true;
		}
	}
	return false;
}

//对子个数
CT_BYTE CGameProcess::GetDuiZiCount(CT_BYTE cbCardIndex[MAX_INDEX])
{
	CT_BYTE cbTiCardCount = 0;
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (cbCardIndex[i] >= 2)
		{
			++cbTiCardCount;
		}
	}
	return cbTiCardCount;
}

//获得下一个玩家
CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
{
	return (dwChairID + 1) % GAME_PLAYER;
	//return (dwChairID + GAME_PLAYER - 1) % GAME_PLAYER;
}

//清除所有定时器
void CGameProcess::ClearAllTimer()
{
	//出牌定时器
	m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);
	//动作定时器
	m_pGameDeskPtr->KillGameTimer(ID_TIME_ACTION);
	//摸牌定时器
	m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_CARD);
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	assert(0 != m_pGameDeskPtr);
	//初始化数据
	InitGameData();
	//解释私人场数据
	ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//获得游戏配置信息
	GetGameConfigInfo();
	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态
	m_cbGameStatus = GAME_STATUS_PLAY;
	//更新庄家
	UpdateBankerUser();

    for (int j = 0; j < GAME_PLAYER; ++j)
    {
        if(!m_pGameDeskPtr->IsExistUser(j)) continue;
        m_RecordScoreInfo[j].dwUserID = m_pGameDeskPtr->GetUserID(j);
        m_RecordScoreInfo[j].llSourceScore = m_pGameDeskPtr->GetUserScore(j);
    }
	//洗牌
	CT_BYTE cbCardCount[GAME_PLAYER] = { 0 };
	CT_BYTE cbCardData[GAME_PLAYER][MAX_COUNT];
	XiPai(cbCardData, cbCardCount);
	//配牌测试
	//TestCard(cbCardData);
	//当前玩家
	m_dwCurrentUser = m_dwBankerUser;
	m_dwProvideUser = m_dwBankerUser;

	//获得当前局数
	CT_WORD wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

	//庄家提牌
	CT_BYTE cbTiCardCount = 0;
	CT_BYTE cbTiCardList[MAX_TI_CARD] = { 0 };
	cbTiCardCount = BankerTiCard(cbTiCardList);
	for (CT_BYTE i = 0; i < cbTiCardCount; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "BankerTiCard = " << cbTiCardList[i];
	}

	//庄家胡牌判断
	CT_BOOL bHuCard = false;
	tagHuCardInfo	 HuCardInfo; //胡牌信息 
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	if (GetHuCardInfo(m_dwBankerUser, 0, HuCardInfo))
	{//天胡
		memcpy(&m_HuCardInfo[m_dwBankerUser], &HuCardInfo, sizeof(HuCardInfo));

		m_dwHuCardUser = m_dwBankerUser;
		m_cbHuCard = m_cbProvideCard;
		//天胡标志
		m_bTianHuFlag = true;
		//胡牌类型(0平胡,1放炮,2自模)
		m_cbHuCardType = 2;

		LOG_IF(INFO, PRINT_LOG_INFO) << "Banker Tian Hu!";
		bHuCard = true;
	}

	//庄家胡息
	CT_WORD wBankerHuXi = GetHuXi(m_dwBankerUser);
	//发送开始消息
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		FPF_CMD_S_GameStart GameStart;
		memset(&GameStart, 0, sizeof(FPF_CMD_S_GameStart));
		GameStart.dwBankerUser = m_dwBankerUser;
		GameStart.dwCurrentUser = i;	//发牌玩家
		GameStart.cbCardCount = cbCardCount[i];
		GameStart.cbLastCard = m_cbProvideCard;
		GameStart.wCurrPlayCount = wCurrPlayCount;
		memcpy(GameStart.cbCardData, &cbCardData[i], MAX_COUNT);
		GameStart.cbTiCardCount = cbTiCardCount;
		memcpy(GameStart.cbTiCardList, cbTiCardList, sizeof(CT_BYTE)*cbTiCardCount);

		GameStart.cbOutCardTime = m_cbOutCardTime;			//出牌时间
		GameStart.dwCurOutCardUser = INVALID_CHAIR;				//出牌玩家
		if (/*i==m_dwBankerUser &&*/ !bHuCard)
		{//庄家没有胡
			GameStart.dwCurOutCardUser = m_dwBankerUser;
		}
		GameStart.wCurHuXi[m_dwBankerUser] = wBankerHuXi;
		LOG_IF(INFO,PRINT_LOG_INFO) << "GameStart: user=" << (CT_WORD)i << ",name=" << m_pGameDeskPtr->GetNickName(i) << ",PlayCount=" << wCurrPlayCount;

		//发送数据
		SendTableData(i, FPF_SUB_S_GAME_START, &GameStart, sizeof(FPF_CMD_S_GameStart));
		//判断听牌
		EstimateTingCard(i);
	}

	if (bHuCard)
	{//庄家有天胡
		OnEventGameEnd(m_dwBankerUser, GER_NORMAL);
		return;
	}
}

//通知游戏结束，带有类型信息（网管解散、强退、协商退出、正常结束）
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: dwChairID=" << dwChairID << "GETag=" << (CT_WORD)GETag;

	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态-空闲状态
	m_cbGameStatus = GAME_STATUS_FREE;

	switch (GETag)
	{
	case GER_NORMAL://积分规则见本文档最后面。
	{//正常退出
		NormalGameEnd();
		break;
	}
	case GER_FLOW_BUREAU:
	{//流局游戏结束
		FlowBureauEnd();
		break;
	}
	case GER_USER_LEFT:
	{//玩家强制退出,则把该玩家托管不出。
		break;
	}
	case GER_DISMISS:
	{//游戏解散
		//DismissEnd();
		break;
	}
	default:
		break;
	}

	//当前时间
	std::string strTime = Utility::GetTimeNowString();

	//游戏结束
	m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, strTime.c_str());

	//是否游戏结算算
	/*LOG_IF(INFO, PRINT_LOG_INFO) << " m_pGameDeskPtr->IsGameClearing()";

	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		//发送总结算
		SendTotalClearing(strTime);
		//清理桌子上的用户
		m_pGameDeskPtr->ClearTableUser();

		//庄家玩家
		m_dwBankerUser = INVALID_CHAIR;

		LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: SendTotalClearing";
	}*/
}

//发送场景
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return;
	}

	switch (m_cbGameStatus)
	{
	case GAME_STATUS_FREE:	//空闲状态
	{
		FPF_MSG_GS_FREE GameFree;
		memset(&GameFree, 0, sizeof(FPF_MSG_GS_FREE));
		GameFree.dwCellScore = 1;
		GameFree.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_FREE, &GameFree, sizeof(FPF_MSG_GS_FREE),false);

		//设置玩家自动准备
		m_pGameDeskPtr->SetUserReady(dwChairID);

		break;
	}
	case GAME_STATUS_PLAY:	//游戏中状态
	{
		//场景数据
		FPF_MSG_GS_PLAY GamePlay;
		memset(&GamePlay, 0, sizeof(FPF_MSG_GS_PLAY));
		GamePlay.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

		//剩余时间
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		CT_BYTE cbGameTime = (m_dwCurrentUser != INVALID_CHAIR) ? m_cbOutCardTime : m_cbOperateTime;
		GamePlay.cbTimeLeave = (CT_BYTE)(cbGameTime - std::min<CT_BYTE>(dwPassTime, cbGameTime));

		GamePlay.dwCellScore = 1;																			//基础积分
		GamePlay.dwBankerUser = m_dwBankerUser;											//庄家用户
		GamePlay.dwCurrentUser = m_dwCurrentUser;											//当前用户(出牌玩家)

		GamePlay.cbActionStatus = m_cbActionStatus;											//动作状态(1摸牌,2出牌)
		GamePlay.dwProvideUser = m_dwProvideUser;											//供应玩家(cbActionStatuso为1: 摸牌玩家; 为2:出牌玩家)
		GamePlay.cbProvideCard = m_cbProvideCard;											//供应牌(cbActionStatuso为1: 摸的牌; 为2:出的牌)


		for (CT_BYTE i=0; i<ACTION_COUNT; ++i)
		{//动作
			GamePlay.cbActionCode[i] = m_stUserAction[dwChairID].cbActionCode[i];		//动作代码（吃，碰，跑，喂，提，胡）
		}
		GamePlay.cbMustIndex = m_stUserAction[dwChairID].cbMustIndex;
		GamePlay.dwActionNumber = m_dwActionNumber;
		GamePlay.cbChiWeaveCount = m_stUserAction[dwChairID].cbChiWeaveCount;							//吃牌组合个数(吃用)
		//吃牌组合牌数据(吃用)
		memcpy(GamePlay.cbChiWeaveCard, &m_stUserAction[dwChairID].cbChiWeaveCard, sizeof(GamePlay.cbChiWeaveCard));
		GamePlay.cblastCardCount = m_cbLeftCardCount;																		//牌堆剩余牌数

		memcpy(GamePlay.cbDiscardCardCount, m_cbDiscardCount, sizeof(m_cbDiscardCount));			//出牌数
		memcpy(GamePlay.cbDiscardCard, m_cbDiscardCard, sizeof(m_cbDiscardCard));						//出牌记录 

		GamePlay.cbUserCardCount = m_GameLogic.SwitchToCardData(m_cbCardIndex[dwChairID], GamePlay.cbUserCard);		//牌张数
		memcpy(GamePlay.cbWeaveItemCount, m_cbWeaveCount, sizeof(m_cbWeaveCount));				//出牌数
		memcpy(GamePlay.WeaveItem, m_WeaveArray, sizeof(m_WeaveArray));										//组合数据
		memcpy(GamePlay.cbUserQiHu, m_cbUserQiHu, sizeof(m_cbUserQiHu));									//玩家弃胡

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{//当前胡息
			GamePlay.wCurHuXi[i] = GetHuXi(i);
		}

		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_PLAY, &GamePlay, sizeof(FPF_MSG_GS_PLAY), false);
		//判断听牌
		EstimateTingCard(dwChairID);
		break;
	}
	case GAME_STATUS_END:					//游戏结束
	{
		//场景数据
		FPF_MSG_GS_END GameEnd;
		memset(&GameEnd, 0, sizeof(FPF_MSG_GS_END));
		GameEnd.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

		//剩余时间
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		CT_BYTE cbGameTime = (m_dwCurrentUser != INVALID_CHAIR) ? m_cbOutCardTime : m_cbOperateTime;
		GameEnd.cbTimeLeave = (CT_BYTE)(cbGameTime - std::min<CT_BYTE>(dwPassTime, cbGameTime));

		GameEnd.dwCellScore = 1;																		//基础积分
		GameEnd.dwBankerUser = m_dwBankerUser;											//庄家用户
		GameEnd.cblastCardCount = m_cbLeftCardCount;																		//牌堆剩余牌数
		memcpy(GameEnd.cbDiscardCardCount, m_cbDiscardCount, sizeof(m_cbDiscardCount));			//出牌数
		memcpy(GameEnd.cbDiscardCard, m_cbDiscardCard, sizeof(m_cbDiscardCard));						//出牌记录 
		GameEnd.cbUserCardCount = m_GameLogic.SwitchToCardData(m_cbCardIndex[dwChairID], GameEnd.cbUserCard);		//牌张数
		memcpy(GameEnd.cbWeaveItemCount, m_cbWeaveCount, sizeof(m_cbWeaveCount));				//出牌数
		memcpy(GameEnd.WeaveItem, m_WeaveArray, sizeof(m_WeaveArray));										//组合数据
		memcpy(GameEnd.cbUserQiHu, m_cbUserQiHu, sizeof(m_cbUserQiHu));											//玩家弃胡
		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_END, &GameEnd, sizeof(FPF_MSG_GS_END),false);
		break;
	}
	default:
		break;
	}
}

//事件接口
//定时器事件
void  CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnTimerMessage: dwTimerID=" << dwTimerID;

	switch (dwTimerID)
	{
	case ID_TIME_OUT_CARD:			//出牌定时器
	{
		//游戏状态判断
		if (m_cbGameStatus != GAME_STATUS_PLAY) break;
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);

		//用户出牌
		CT_BOOL bRtn = OnAutoOutCard(m_dwCurrentUser);

		LOG_IF(INFO, PRINT_LOG_INFO) << "ID_TIME_OUT_CARD bRtn=" << (CT_WORD)bRtn;
		break;
	}
	case ID_TIME_ACTION:				//动作定时器 
	{
		//游戏状态判断
		if (m_cbGameStatus != GAME_STATUS_PLAY) break;
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_ACTION);

		//用户操作 
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{//是否有动作
			if (!IsHaveAction(m_stUserAction[i])) continue;
			//用户操作
			OnAutoOperateCard(i);
		}
		LOG_IF(INFO, PRINT_LOG_INFO) << "ID_TIME_ACTION";
		break;
	}
	case ID_TIME_SEND_CARD:		//发牌定时器
	{
		m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_CARD);

		CT_BOOL bRtn = false;
		if (m_dwResumeUser != INVALID_CHAIR)
		{
			bRtn = SendUserCard(m_dwResumeUser);
		}

		LOG_IF(INFO, PRINT_LOG_INFO) << "ID_TIME_SEND_CARD bRtn=" << (CT_WORD)bRtn;
		break;
	}
	default:
		break;
	}
}

//游戏消息
CT_BOOL  CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID* pDataBuffer, CT_DWORD dwDataSize)
{
	CT_INT32 iRtn = false;
	switch (dwSubCmdID)
	{
	case FPF_SUB_C_OUT_CARD:					//出牌消息
	{
		//效验消息
		if (sizeof(FPF_CMD_C_OutCard) != dwDataSize)
		{
			return false;
		}
		//消息处理
		FPF_CMD_C_OutCard* pOutCard = (FPF_CMD_C_OutCard*)pDataBuffer;
		assert(NULL != pOutCard);
		if (NULL == pOutCard)
		{
			return false;
		}
		iRtn = OnUserOutCard(wChairID, pOutCard->cbOutCard);

		std::string strName("empty nick");
		if (m_pGameDeskPtr->IsExistUser(wChairID) && m_pGameDeskPtr->GetNickName(wChairID) != NULL)
		{
			strName = m_pGameDeskPtr->GetNickName(wChairID);
		}
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnGameMessage: OnUserOutCard iRtn=" << iRtn << ",name=" << strName.c_str();
		return iRtn > 0;
	}
	case FPF_SUB_C_OPERATE:			//操作消息(吃、碰、胡、过)
	{
		//效验消息
		if (sizeof(FPF_CMD_C_OperateCard) != dwDataSize)
		{
			return CT_FALSE;
		}
		//消息处理
		FPF_CMD_C_OperateCard* pOperate = (FPF_CMD_C_OperateCard*)pDataBuffer;
		assert(NULL != pOperate);
		if (NULL == pOperate)
		{
			return false;
		}
		iRtn = OnUserOperate(wChairID, pOperate);
		
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnGameMessage: OnUserOperate iRtn=" << iRtn;

		return iRtn > 0;
	}
	case FPF_SUB_C_QI_HU:			//玩家弃胡
	{
		if (m_cbGameStatus != GAME_STATUS_PLAY)
		{
			return false;
		}
		if (1 == m_cbUserQiHu[wChairID])
		{
			return false;
		}
		//记录弃胡标志
		m_cbUserQiHu[wChairID] = 1;
		//发送数据弃胡
		FPF_CMD_S_UserQiHu QiHu;
		memset(&QiHu, 0, sizeof(QiHu));
		QiHu.dwQiHuUser = wChairID;
		//检查闲家提牌
		CT_BYTE cbTiCardList[GAME_PLAYER][MAX_TI_CARD];		//提牌扑克列表
		memset(cbTiCardList, 0, sizeof(cbTiCardList));
		CT_BYTE cbTiCardCount[GAME_PLAYER] = { 0 };							//提牌扑克个数
		if (wChairID == m_dwBankerUser && 0 == m_cbOutCardCount[m_dwBankerUser])
		{
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{	//过滤庄家
				if (i == m_dwBankerUser) continue;
				//闲家提牌处理
				HandTiCard(i, cbTiCardList[i], cbTiCardCount[i]);
				//计算胡息
				QiHu.wCurHuXi[i] = GetHuXi(i);
			}
		}
		//提牌扑克列表
		memcpy(QiHu.cbTiCardList, cbTiCardList, sizeof(cbTiCardList));
		//提牌扑克个数
		memcpy(QiHu.cbTiCardCount, cbTiCardCount, sizeof(cbTiCardCount));

		

		SendTableData(INVALID_CHAIR, FPF_SUB_S_USER_QI_HU, &QiHu, sizeof(FPF_CMD_S_UserQiHu));

		if (wChairID==m_dwCurrentUser)
		{
			//玩家弃胡 下一个玩家摸牌
			CT_DWORD dwNextUser = GetNextUser(m_dwCurrentUser);
			LOG_IF(INFO, PRINT_LOG_INFO) << "FPF_SUB_C_QI_HU:  QiUser=" << m_pGameDeskPtr->GetNickName(wChairID) << ",NextUser=" << m_pGameDeskPtr->GetNickName(m_dwCurrentUser);
			//下一个玩家摸牌
			SendUserCard(dwNextUser);
		}

		if (IsHaveAction(m_stUserAction[wChairID]))
		{
			FPF_CMD_C_OperateCard Operate;
			Operate.dwOperateCode = 0;
			Operate.dwActionNumber = m_dwActionNumber;
			iRtn = OnUserOperate(wChairID, &Operate);
		}
			
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnGameMessage: OnUserQiHu name=" << m_pGameDeskPtr->GetNickName(wChairID);
	}
	default:
		break;
	}
	return CT_TRUE;
}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	//初始化游戏数据
	InitGameData();
	//更新游戏配置
	//UpdateGameConfig();

	//CT_BYTE cbTempCard0[MAX_COUNT] = { 0x07, 0x17, 0x17, 0x18, 0x18, 0x19,0x12, 0x03,0x1A, 0x04 };
	//m_GameLogic.SwitchToCardIndex(cbTempCard0, MAX_COUNT - 1, m_cbCardIndex[0]);

	//CT_BYTE cbChiWeaveCount = 0;							//吃牌组合个数(吃用)
	//CT_BYTE cbChiWeaveCard[MAX_ITEM_COUNT][3] = {0};	//吃牌组合牌数据(吃用)
	//CT_BYTE	cbMustIndex=250;									//必选索引
	//GetChiCard(0, 0x17, cbChiWeaveCard, cbChiWeaveCount, cbMustIndex);

}

//清理游戏数据
void CGameProcess::ClearGameData()
{
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		m_wHuCount[i] = 0;			//各玩家胡牌次数
		m_wFPCount[i] = 0;				//各玩家放炮次数
		m_wHZCount[i] = 0;			//各玩家荒庄次数
		m_iTotalHuXi[i] = 0;				//各玩家累计胡息
		m_iTotalLWScore[i] = 0;		//各玩家总输赢分
	}
}

//私人房游戏大局是否结束
bool CGameProcess::IsGameRoundClearing(CT_BOOL bDismissGame)
{
	if (bDismissGame)
	{//计算总输赢分
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: bDismissGame";
		CalcTotalLWScore();
		return true;
	}

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_iTotalHuXi[i]>= END_HUXI)
		{
			LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: true";
			//计算总输赢分
			CalcTotalLWScore();
			return true;
		}
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: false";
	return false;
}

//获取大局结算的总分
CT_INT32 CGameProcess::GetGameRoundTotalScore(CT_DWORD dwChairID)
{
	if (dwChairID >= GAME_PLAYER)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Err dwChairID=" << dwChairID;
		return 0;
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Score=" << m_iTotalLWScore[dwChairID];
	return m_iTotalLWScore[dwChairID];
}

//发送数据
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
		return false;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}

//玩家操作
CT_INT32 CGameProcess::OnUserOperate(CT_DWORD dwChairID, FPF_CMD_C_OperateCard* pOperateData, CT_BOOL bTimeout)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: dwChairID=" << dwChairID << "Code=" << pOperateData->dwOperateCode << "Card=" << (CT_WORD)pOperateData->cbOperateCard;
	for (CT_WORD i = 0; i < pOperateData->cbBaiCardCount && i < MAX_ITEM_COUNT; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: BaiCard[" << (CT_WORD)pOperateData->cbBaiCard[i][0] << "," << (CT_WORD)pOperateData->cbBaiCard[i][1] << ","\
			<< (CT_WORD)pOperateData->cbBaiCard[i][2] << "]";
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";

	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: Action=[" << (CT_WORD)m_stUserAction[dwChairID].cbActionCode[0] << "," << (CT_WORD)m_stUserAction[dwChairID].cbActionCode[1] << "," << (CT_WORD)m_stUserAction[dwChairID].cbActionCode[2] << ","\
		<< (CT_WORD)m_stUserAction[dwChairID].cbActionCode[3] << "],ActionCard=" << (CT_WORD)m_stUserAction[dwChairID].cbActionCard << ",MustIndex" << (CT_WORD)m_stUserAction[dwChairID].cbMustIndex;

	for (CT_WORD i = 0; i < m_stUserAction[dwChairID].cbChiWeaveCount && i < MAX_ITEM_COUNT; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: WeaveCard[" << (CT_WORD)m_stUserAction[dwChairID].cbChiWeaveCard[i][0] << "," << (CT_WORD)m_stUserAction[dwChairID].cbChiWeaveCard[i][1] << ","\
			<< (CT_WORD)m_stUserAction[dwChairID].cbChiWeaveCard[i][2]<< "]";
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";

	if (pOperateData->dwActionNumber != m_dwActionNumber)
	{
		LOG_IF(INFO,PRINT_LOG_INFO)<<"ActionNumber  Err! OpNumber ="<< pOperateData->dwActionNumber<<", m_dwActionNumber ="<< m_dwActionNumber;
		return 1;
	}
	if (!IsHaveAction(m_stUserAction[dwChairID]))
	{//没有动作
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate User Action Err! name=%s" << m_pGameDeskPtr->GetNickName(dwChairID);
		return 2;
	}
	if (m_cbGameStatus != GAME_STATUS_PLAY)
	{//状态判断
		return 3;
	}
	if (m_bResponse[dwChairID])
	{//已经操作过了
		return 4;
	}
	if (0 != pOperateData->dwOperateCode && m_cbProvideCard != pOperateData->cbOperateCard)
	{//校验操作牌
		return 5;
	}
	if (!CheckUserOperate(dwChairID, pOperateData))
	{//校验操作
		return 6;
	}

	//记录操作
	//memcpy(&m_stUserOperate[dwChairID], pOperateData, sizeof(FPF_CMD_C_OperateCard));
	m_stUserOperate[dwChairID].dwUserID = pOperateData->dwUserID;
	m_stUserOperate[dwChairID].dwOperateCode = pOperateData->dwOperateCode;
	m_stUserOperate[dwChairID].cbOperateCard = pOperateData->cbOperateCard;
	m_stUserOperate[dwChairID].cbBaiCardCount = pOperateData->cbBaiCardCount;
	for (CT_WORD i = 0; i < m_stUserOperate[dwChairID].cbBaiCardCount && i < MAX_ITEM_COUNT; ++i)
	{
		m_stUserOperate[dwChairID].cbBaiCard[i][0] = pOperateData->cbBaiCard[i][0];
		m_stUserOperate[dwChairID].cbBaiCard[i][1] = pOperateData->cbBaiCard[i][1];
		m_stUserOperate[dwChairID].cbBaiCard[i][2] = pOperateData->cbBaiCard[i][2];
	}

	//设置操作标志
	m_bResponse[dwChairID] = true;

	//变量定义
	CT_DWORD dwTargetAction = ACK_NULL;
	CT_DWORD dwTargetUser = INVALID_CHAIR;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		CT_DWORD dwResumeUser = i;
		if (m_dwProvideUser < GAME_PLAYER)
		{
			dwResumeUser = (m_dwProvideUser + i) % GAME_PLAYER;
		}

		//获取动作 0过, 1吃，2碰，3胡，
		CT_DWORD dwUserAction = m_bResponse[dwResumeUser] ? m_stUserOperate[dwResumeUser].dwOperateCode : GetMaxAction(dwResumeUser);
		//动作判断
		if (dwUserAction > dwTargetAction)
		{
			dwTargetUser = dwResumeUser;
			dwTargetAction = dwUserAction;
		}
	}

	//最大动作
	CT_DWORD dwMaxAction = ACK_NULL;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		CT_DWORD dwTempAction = GetMaxAction(i);
		if (dwTempAction > dwMaxAction)
		{
			dwMaxAction = dwTempAction;
		}
	}

	if (m_cbActionStatus == SEND_CARD_STATUS && 0==m_cbWeiAction && dwMaxAction == ACK_CHIHU && dwTargetAction < ACK_CHIHU)
	{//发牌后动作 (翻牌后 没有偎 放弃胡牌)
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate:  guo hu";
		CT_DWORD dwPaoUser = INVALID_CHAIR;
		if (EstimateSendCardPao(m_dwProvideUser, m_cbProvideCard, dwPaoUser, false))
		{
			//发牌玩家
			CT_DWORD dwSendCardUser = INVALID_CHAIR;
			m_dwCurrentUser = INVALID_CHAIR;
			if (IsChongPao(dwPaoUser) || m_bWeiCannotHu[dwPaoUser] || (m_cbUserQiHu[dwPaoUser] == 1))
			{//发送牌
				LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate:  Pao Card  SendCard ";
				//发牌玩家
				dwSendCardUser = GetNextUser(dwPaoUser);
			}
			else
			{//出牌提示
				LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate:  Pao Card  OutCard";

				m_dwCurrentUser = dwPaoUser;
			}

			FPF_CMD_S_OperateResult OperateResult;
			memset(&OperateResult, 0, sizeof(OperateResult));
			OperateResult.dwCurOutCardUser = INVALID_CHAIR;

			OperateResult.dwOperateUser = dwPaoUser;
			OperateResult.dwOperateCode = ACK_PAO;	//跑牌
			OperateResult.cbOperateCard = m_cbProvideCard;
			OperateResult.cbOutCardTime = m_cbOutCardTime;
			OperateResult.dwCurOutCardUser = m_dwCurrentUser;
			
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{//当前胡息
				OperateResult.wCurHuXi[i] = GetHuXi(i);
			}

			//发送数据
			SendTableData(INVALID_CHAIR, FPF_SUB_S_OPERATE_RESULT, &OperateResult, sizeof(FPF_CMD_S_OperateResult));

			LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: Pao Card Result1111!  dwOperateUser=" << OperateResult.dwOperateUser << ",cbOperateCard=" << (CT_WORD)OperateResult.cbOperateCard << ",OutCardUser=" << OperateResult.dwCurOutCardUser;
	
			//玩家发牌
			if (dwSendCardUser!=INVALID_CHAIR)
			{
				SendUserCard(dwSendCardUser);
				return 10;
			}
			memset(m_stUserAction, 0, sizeof(m_stUserAction));
			return 11;
		}
	}

	if ((dwTargetUser != INVALID_CHAIR) && (m_bResponse[dwTargetUser] == false))
	{//目标玩家还没操作
		return 12;
	}

	if (dwTargetUser == INVALID_CHAIR)
	{//没有人操作

		//处理过牌
		//如果出牌,没有人操作出牌玩家的下一个玩家过张
		if (m_cbActionStatus == OUT_CARD_STATUS)
		{
			CT_DWORD dwNextUser = GetNextUser(m_dwProvideUser);
			m_bGuoChiCard[dwNextUser][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
		}
		else if (m_cbActionStatus == SEND_CARD_STATUS) //如果是发牌，则本家和下一家都过张
		{
			CT_DWORD dwNextUser = GetNextUser(m_dwProvideUser);
			m_bGuoChiCard[m_dwProvideUser][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
			m_bGuoChiCard[dwNextUser][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
		}

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (1 == m_stUserAction[i].cbActionCode[ACK_CHI])
			{//吃
				m_bGuoChiCard[i][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
			}
			if (1 == m_stUserAction[i].cbActionCode[ACK_PENG])
			{//碰
				m_bGuoPengCard[i][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
			}
		}
		//丢弃记录
		m_cbDiscardCard[m_dwProvideUser][m_cbDiscardCount[m_dwProvideUser]++] = m_cbProvideCard;

		CT_DWORD dwSendCardUser = INVALID_CHAIR;
		m_dwCurrentUser = INVALID_CHAIR;
		if ((m_cbTiPaoAction>0 && !IsChongPao(m_dwResumeUser)) || m_cbWeiAction==1)
		{//通知玩家出牌
			m_dwCurrentUser = m_dwResumeUser;
		}
		else
		{
			//发牌玩家
			dwSendCardUser = GetNextUser(m_dwProvideUser);
		}

		FPF_CMD_S_OperateResult OperateResult;
		memset(&OperateResult, 0, sizeof(OperateResult));
		OperateResult.dwCurOutCardUser = INVALID_CHAIR;

		OperateResult.dwOperateUser = INVALID_CHAIR;
		OperateResult.cbOperateCard = m_cbProvideCard;
		OperateResult.dwOperateCode = ACK_NULL;
		OperateResult.cbOutCardTime = m_cbOutCardTime;
		OperateResult.dwCurOutCardUser = m_dwCurrentUser;
	
		for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
		{//当前胡息
			OperateResult.wCurHuXi[i] = GetHuXi(i);
		}

		//发送数据
		SendTableData(INVALID_CHAIR, FPF_SUB_S_OPERATE_RESULT, &OperateResult, sizeof(FPF_CMD_S_OperateResult));

		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: Pao Card Result2222!  dwOperateUser=" << OperateResult.dwOperateUser << ",cbOperateCard=" << (CT_WORD)OperateResult.cbOperateCard << ",OutCardUser=" << OperateResult.dwCurOutCardUser;

		//玩家发牌
		if (dwSendCardUser != INVALID_CHAIR)
		{
			SendUserCard(dwSendCardUser);
			return 13;
		}
		memset(m_stUserAction, 0, sizeof(m_stUserAction));
		return 14;
	}
	else
	{//有人操作
		//处理过牌
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			CT_DWORD dwUser = i;
			if (m_dwProvideUser < GAME_PLAYER)
			{
				dwUser = (m_dwProvideUser + i) % GAME_PLAYER;
			}
			if (dwUser == dwTargetUser)
			{
				break;
			}
			if (1 == m_stUserAction[dwUser].cbActionCode[ACK_CHI] && ACK_CHI >= dwTargetAction)
			{//吃
				m_bGuoChiCard[dwUser][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
			}
		}

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (1 == m_stUserAction[i].cbActionCode[ACK_PENG] && ACK_PENG >= dwTargetAction)
			{//碰
				m_bGuoPengCard[i][m_GameLogic.SwitchToCardIndex(m_cbProvideCard)] = true;
				break;
			}
		}

		//胡牌
		if (ACK_CHIHU == dwTargetAction)
		{ //结束游戏
			m_dwHuCardUser = dwTargetUser;
			m_cbHuCard = m_cbProvideCard;
			//胡牌类型(0平胡,1放炮,2自模)
			m_cbHuCardType = (m_dwHuCardUser == m_dwProvideUser) ? 2 : 0;

			LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate 222: dwTargetUser=" << dwTargetUser;
			OnEventGameEnd(dwTargetUser, GER_NORMAL);
			return 15;
		}

		CT_BOOL bRtn = false;
		//记录组合
		if (ACK_CHI == dwTargetAction)
		{//吃组合
			bRtn = SetChiCard(dwTargetUser, m_stUserOperate[dwTargetUser].cbOperateCard, m_stUserOperate[dwTargetUser].cbBaiCard, m_stUserOperate[dwTargetUser].cbBaiCardCount);
		}
		else if (ACK_PENG == dwTargetAction)
		{//碰组合
			bRtn = SetPengCard(dwTargetUser, m_stUserOperate[dwTargetUser].cbOperateCard);
		}

		if (bRtn)
		{//发送出牌提示
			m_dwCurrentUser = dwTargetUser;

			FPF_CMD_S_OperateResult OperateResult;
			memset(&OperateResult, 0, sizeof(OperateResult));
			OperateResult.dwCurOutCardUser = INVALID_CHAIR;

			OperateResult.dwOperateUser = dwTargetUser;
			OperateResult.cbOperateCard = m_cbProvideCard;
			if (ACK_CHI == dwTargetAction)
			{//吃动作
				OperateResult.dwOperateCode = ACK_CHI;		//操作代码(1吃,2碰,3跑)
				OperateResult.cbBaiCardCount = m_stUserOperate[dwTargetUser].cbBaiCardCount;
				memcpy(OperateResult.cbBaiCard, &m_stUserOperate[dwTargetUser].cbBaiCard, sizeof(OperateResult.cbBaiCard));
			}
			else if (ACK_PENG == dwTargetAction)
			{//碰动作
				OperateResult.dwOperateCode = ACK_PENG;		//操作代码(1吃,2碰,3跑)
			}
			OperateResult.cbOutCardTime = m_cbOutCardTime;
			OperateResult.dwCurOutCardUser = m_dwCurrentUser;
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{//当前胡息
				OperateResult.wCurHuXi[i] = GetHuXi(i);
			}

			//发送数据
			SendTableData(INVALID_CHAIR, FPF_SUB_S_OPERATE_RESULT, &OperateResult, sizeof(FPF_CMD_S_OperateResult));

			memset(m_stUserAction, 0, sizeof(m_stUserAction));
			memset(m_stUserOperate, 0, sizeof(m_stUserOperate));
			LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOperate: 66666!  dwOperateUser=";
			return 16;
		}
		return 17;
	}
}

//玩家出牌
CT_INT32 CGameProcess::OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard, CT_BOOL bTimeout)
{
	if (m_cbGameStatus != GAME_STATUS_PLAY)
	{//状态不对
		return 1;
	}
	if (!m_GameLogic.IsValidCard(cbOutCard))
	{//不是有效牌
		return 2;
	}
	if (dwChairID != m_dwCurrentUser)
	{//不是出牌玩家
		return 3;
	}
	CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbOutCard);
	if (3 == m_cbCardIndex[dwChairID][cbCardIndex])
	{//偎牌不能拆
		return 4;
	}

	if (0 == m_cbCardIndex[dwChairID][cbCardIndex])
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "OnUserOutCard data not found:" << "room num: " << m_PrivateTableInfo.dwRoomNum << ", current play count:" 
			<< m_pGameDeskPtr->GetCurrPlayCount() <<", dwChairID=" << dwChairID << ",Card=" << (CT_WORD)cbOutCard;
		return 5;
	}

	if (!m_GameLogic.RemoveCard(m_cbCardIndex[dwChairID], cbOutCard))
	{//删除扑克错误
		return 6;
	}

	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: dwChairID=" << dwChairID << ",Card=" << (CT_WORD)cbOutCard;

	//玩家出牌次数
	++m_cbOutCardCount[dwChairID];

	//过牌记录
	m_bGuoChiCard[dwChairID][cbCardIndex] = true;
	m_bGuoPengCard[dwChairID][cbCardIndex] = true;

	//初始化数据
	memset(m_stUserAction, 0, sizeof(m_stUserAction));
	memset(m_stUserOperate, 0, sizeof(m_stUserOperate));
	memset(m_bResponse, 0, sizeof(m_bResponse));
	memset(&m_HuCardInfo, 0, sizeof(m_HuCardInfo));
	m_cbTiPaoAction = 0;
	m_cbWeiAction = 0;
	m_dwSysTime = (CT_DWORD)time(NULL);

	//用户切换
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwResumeUser = INVALID_CHAIR;
	m_dwProvideUser = dwChairID;
	m_cbProvideCard = cbOutCard;
	m_cbActionStatus = OUT_CARD_STATUS;	//动作状态

	//检查闲家提牌
	CT_BYTE cbTiCardList[GAME_PLAYER][MAX_TI_CARD];		//提牌扑克列表
	memset(cbTiCardList, 0, sizeof(cbTiCardList));
	CT_BYTE cbTiCardCount[GAME_PLAYER] = { 0 };							//提牌扑克个数
	if (dwChairID == m_dwBankerUser && 1 == m_cbOutCardCount[m_dwBankerUser])
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{	//过滤庄家
			if (i == m_dwBankerUser) continue;
			//闲家提牌处理
			HandTiCard(i, cbTiCardList[i], cbTiCardCount[i]);
		}
	}

	//判断胡牌
	CT_BOOL bHuCard = false;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		CT_DWORD dwTempUser = (dwChairID + i) % GAME_PLAYER;
		if (dwTempUser == dwChairID)
		{//出牌玩家不能胡
			continue;
		}
		//判断出牌胡
		if (EstimateOutCardHu(dwTempUser, dwChairID, cbOutCard))
		{
			LOG_IF(INFO, PRINT_LOG_INFO) << "hu card 1111111!";
			bHuCard = true;
			break;
		}
	}

	//发牌玩家
	CT_DWORD dwSendCardUser = INVALID_CHAIR;
	CT_DWORD dwPaoCardUser = INVALID_CHAIR;
	//动作标志
	CT_BOOL bAction = false;
	if (!bHuCard)
	{//没有胡牌 判断跑
		CT_BOOL bPaoCard = EstimateOutCardPao(dwChairID, cbOutCard, dwPaoCardUser);

		LOG_IF(INFO, PRINT_LOG_INFO) << "UserOutCard: bPaoCard=" << (CT_WORD)bPaoCard << ",PaoUser=" << (CT_WORD)dwPaoCardUser;

		if (bPaoCard && dwPaoCardUser != INVALID_CHAIR)
		{
			bAction = true;

			if (EstimateOutCardHu(dwPaoCardUser, dwChairID, cbOutCard, true))
			{//跑后有胡牌
				LOG_IF(INFO, PRINT_LOG_INFO) << " pao hu 2222222!";
				bHuCard = true;
			}
			else
			{
				//跑动作 
				m_stUserAction[dwPaoCardUser].cbActionCard = cbOutCard;
				m_stUserAction[dwPaoCardUser].cbActionCode[ACK_PAO] = 1;  //动作下标（吃，碰，跑，喂，提，胡）

				//判断重跑
				if (IsChongPao(dwPaoCardUser) || (m_cbUserQiHu[dwPaoCardUser] == 1) || m_bTiPaoCannotOutCard[dwPaoCardUser])
				{//给下一个玩家发牌
					EstimateTingCard(dwPaoCardUser);
					dwSendCardUser = GetNextUser(dwPaoCardUser);
				}
				else if (!IsTiPaoCanOutCard(dwPaoCardUser))
				{
					EstimateTingCard(dwPaoCardUser);
					m_bTiPaoCannotOutCard[dwPaoCardUser] = true;
					dwSendCardUser = GetNextUser(dwPaoCardUser);
				}
				else
				{//跑牌玩家出牌
					m_dwCurrentUser = dwPaoCardUser;
				}
			}
		}
		else
		{//无跑 看吃，碰
			//判断下一个玩家吃
			CT_DWORD dwNextUser = GetNextUser(dwChairID);
			CT_BYTE cbMustIndex = 250;
			if (GetChiCard(dwNextUser, cbOutCard, m_stUserAction[dwNextUser].cbChiWeaveCard, m_stUserAction[dwNextUser].cbChiWeaveCount, cbMustIndex))
			{
				m_stUserAction[dwNextUser].cbActionCard = cbOutCard;
				m_stUserAction[dwNextUser].cbActionCode[ACK_CHI] = 1; //吃
				m_stUserAction[dwNextUser].cbMustIndex = cbMustIndex;
				bAction = true;
			}

			//判断碰
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (i == dwChairID)
				{//出牌玩家不能碰
					continue;
				}
				if (IsPengCard(i, cbOutCard))
				{//碰牌 最多一个碰
					m_stUserAction[i].cbActionCard = cbOutCard;
					m_stUserAction[i].cbActionCode[ACK_PENG] = 1; //碰
					bAction = true;
					break;
				}
			}

			if (!bAction)
			{//没动作
				//丢弃的牌(没有被吃碰的牌)
				m_cbDiscardCard[dwChairID][m_cbDiscardCount[dwChairID]++] = cbOutCard;

				//给下一个玩家发牌
				dwSendCardUser = GetNextUser(dwChairID);

				//上家打的牌，下家没吃动作，就永远不能吃了
				m_bGuoChiCard[dwSendCardUser][cbCardIndex] = true;
			}
		}
	}

	CT_WORD wCurHuXi[GAME_PLAYER] = { 0 };				//当前胡息
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		wCurHuXi[i] = GetHuXi(i);
	}

	//动作流水号
	++m_dwActionNumber;
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		//出牌结果
		FPF_CMD_S_OutCard OutCard;
		memset(&OutCard, 0, sizeof(OutCard));
		OutCard.dwCurOutCardUser = INVALID_CHAIR;
		OutCard.dwPaoCardUser = INVALID_CHAIR;

		OutCard.dwOutCardUser = dwChairID;
		OutCard.cbOutCard = cbOutCard;
		OutCard.cbOutCardTime = m_cbOutCardTime;				//出牌时间
		OutCard.dwCurOutCardUser = m_dwCurrentUser;			//当前出牌玩家

		OutCard.dwActionNumber = m_dwActionNumber;
		OutCard.dwPaoCardUser = dwPaoCardUser;
		OutCard.cbActionTime = m_cbOperateTime;
		memcpy(OutCard.wCurHuXi, wCurHuXi, sizeof(wCurHuXi));

		if (bAction && m_GameLogic.IsValidCard(m_stUserAction[i].cbActionCard))
		{//有动作
			memcpy(OutCard.cbActionCode, &m_stUserAction[i].cbActionCode, sizeof(m_stUserAction[i].cbActionCode));
			OutCard.cbActionCard = m_stUserAction[i].cbActionCard;
			OutCard.cbMustIndex = m_stUserAction[i].cbMustIndex;
			OutCard.cbChiWeaveCount = m_stUserAction[i].cbChiWeaveCount;
			memcpy(OutCard.cbChiWeaveCard, &m_stUserAction[i].cbChiWeaveCard, sizeof(m_stUserAction[i].cbChiWeaveCard));

			LOG_IF(INFO, PRINT_LOG_INFO) << "OutCardAction: User=" << (CT_WORD)i << ",Action=[" << (CT_WORD)OutCard.cbActionCode[ACK_CHI] << "," << (CT_WORD)OutCard.cbActionCode[ACK_PENG] << ","\
				<< (CT_WORD)OutCard.cbActionCode[ACK_PAO] << "," << (CT_WORD)OutCard.cbActionCode[ACK_CHIHU] << "]" \
				<< ",ACard=" << (CT_WORD)OutCard.cbActionCard << "MustIndex" << (CT_WORD)OutCard.cbMustIndex;

			for (CT_WORD i = 0; i < OutCard.cbChiWeaveCount; ++i)
			{
				LOG_IF(INFO, PRINT_LOG_INFO) << "Weave_" << (CT_WORD)i << "=[" << (CT_WORD)OutCard.cbChiWeaveCard[i][0] << "," << (CT_WORD)OutCard.cbChiWeaveCard[i][1] << "," << (CT_WORD)OutCard.cbChiWeaveCard[i][2] << "]";
			}
		}
		//else if (dwPaoCardUser != INVALID_CHAIR)
		if (dwPaoCardUser != INVALID_CHAIR)
		{//有玩家跑
			OutCard.cbActionCode[ACK_PAO] = 1;
		}

		//提牌扑克列表
		memcpy(OutCard.cbTiCardList, cbTiCardList,sizeof(cbTiCardList));
		//提牌扑克个数
		memcpy(OutCard.cbTiCardCount, cbTiCardCount, sizeof(cbTiCardCount));

		//发送数据
		SendTableData(i, FPF_SUB_S_OUT_CARD, &OutCard, sizeof(FPF_CMD_S_OutCard), (i == 0 ? true : false));
	}
	//判断听牌
	EstimateTingCard(dwChairID);

	if (bHuCard)
	{//有胡牌
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: HuCard!";
		OnEventGameEnd(m_dwHuCardUser, GER_NORMAL);
		return 10;
	}

	if (dwSendCardUser!=INVALID_CHAIR)
	{//发牌
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: SendCard User=" << dwSendCardUser;
		SendUserCard(dwSendCardUser);
	}
	return 11;
}

//玩家托管
CT_BOOL CGameProcess::OnUserTrustee(CT_DWORD dwChairID)
{
	return CT_TRUE;
}

//校验操作
CT_BOOL CGameProcess::CheckUserOperate(CT_DWORD dwChairID, FPF_CMD_C_OperateCard* pOperateData)
{
	if (NULL == pOperateData)
	{//数据不对
		return false;
	}
	if (dwChairID >= GAME_PLAYER)
	{//玩家不对
		return false;
	}
	if (ACK_NULL == pOperateData->dwOperateCode)
	{//过牌
		return true;
	}
	if (!m_GameLogic.IsValidCard(pOperateData->cbOperateCard))
	{//操作牌无效
		return false;
	}
	if (0 == m_stUserAction[dwChairID].cbActionCard || m_stUserAction[dwChairID].cbActionCard != pOperateData->cbOperateCard)
	{//没有动作
		return false;
	}
	CT_BOOL bRtn = false;
	switch (pOperateData->dwOperateCode)
	{
	case ACK_CHI:			//吃牌
	{
		//检查吃牌
		bRtn = CheckChiCard(dwChairID, pOperateData->cbBaiCard, pOperateData->cbBaiCardCount, pOperateData->cbOperateCard);
		if (!bRtn)
		{
			LOG_IF(INFO,PRINT_LOG_INFO) << "CheckUserOperate 111: bRtn=" << (CT_WORD)bRtn;
		}
		return bRtn;
	}
	case ACK_PENG:		//碰
	{
		//检查碰牌
		bRtn = CheckPengCard(dwChairID, pOperateData->cbOperateCard);
		if (!bRtn)
		{
			LOG_IF(INFO,PRINT_LOG_INFO) << "CheckUserOperate 222: bRtn=" << (CT_WORD)bRtn;
		}
		return bRtn;
	}
	case ACK_CHIHU:	//胡
	{
		bRtn = (m_stUserAction[dwChairID].cbActionCode[ACK_CHIHU] == 1) ? true : false;
		if (!bRtn)
		{
			LOG_IF(INFO, PRINT_LOG_INFO) << "CheckUserOperate 333: bRtn=" << (CT_WORD)bRtn;
		}
		return bRtn;
	}
	}

	LOG_IF(INFO,PRINT_LOG_INFO) << "CheckUserOperate Err";
	return false;
}

//检查吃牌
CT_BOOL CGameProcess::CheckChiCard(CT_DWORD dwChairID, CT_BYTE cbBaiCard[MAX_ITEM_COUNT][3], CT_BYTE cbBaiCardCount, CT_BYTE cbCurCard)
{
	CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3];
	memset(cbChiWeave, 0, sizeof(cbChiWeave));
	CT_BYTE cbChiWeaveCount = m_stUserAction[dwChairID].cbChiWeaveCount;
	memcpy(cbChiWeave, &m_stUserAction[dwChairID].cbChiWeaveCard, sizeof(cbChiWeave));

	if (0 == cbBaiCardCount || 0 == cbChiWeaveCount)
	{//组合个数不对
		return false;
	}

	//校验组合个数
	if (cbBaiCardCount > cbChiWeaveCount)
	{//摆的个数不对
		return false;
	}

	//统计摆牌个数
	CT_BYTE cbBaiCount = m_GameLogic.FindCardCount(cbCurCard, cbBaiCard, cbBaiCardCount);
	//手上总个数
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurCard);
	CT_BYTE cbHandTotalCount = (m_cbCardIndex[dwChairID][cbCurrentIndex] + 1);

	if (cbBaiCount != cbHandTotalCount)
	{//不能摆
		LOG_IF(INFO,PRINT_LOG_INFO) << "CheckChiCard 111: cbBaiCount=" << (CT_WORD)cbBaiCount << ",cbHandTotalCount" << (CT_WORD)cbHandTotalCount;
		return false;
	}

	//校验组合
	for (CT_WORD i = 0; i < cbBaiCardCount; ++i)
	{
		bool bFalg = false;
		for (CT_WORD j = 0; j < cbChiWeaveCount; ++j)
		{
			//是否相同组合
			if (m_GameLogic.IsSameWeave(cbBaiCard[i], cbChiWeave[j]))
			{
				bFalg = true;
				break;
			}
		}
		if (!bFalg)
		{//没有这个组合
			LOG_IF(INFO,PRINT_LOG_INFO) << "CheckChiCard 222";
			return false;
		}
	}

	//尝试删摆牌
	CT_BOOL bSucc = TryRemoveBaiCard(dwChairID, cbBaiCard, cbBaiCardCount, cbCurCard);
	return bSucc;
}

//尝试删摆牌
CT_BOOL CGameProcess::TryRemoveBaiCard(CT_DWORD dwChairID, CT_BYTE cbBaiCard[MAX_ITEM_COUNT][3], CT_BYTE cbBaiCardCount, CT_BYTE cbCurCard)
{
	//先尝试删除
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };		//玩家手牌数据
	memset(cbTempCardIndex, 0, sizeof(cbTempCardIndex));
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));

	//删除吃牌
	CT_BYTE cbChiIndex = m_GameLogic.SwitchToCardIndex(cbCurCard);
	cbTempCardIndex[cbChiIndex] = 0;

	for (CT_WORD i = 0; i < cbBaiCardCount; ++i)
	{//删除扑克
		for (CT_WORD n = 0; n < 3; ++n)
		{
			CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbBaiCard[i][n]);
			if (cbChiIndex == cbCardIndex)
			{//吃的牌不处理
				continue;
			}
			if (0 == cbTempCardIndex[cbCardIndex])
			{//牌不对
				LOG_IF(INFO,PRINT_LOG_INFO) << "TryRemoveBaiCard";
				return false;
			}
			--cbTempCardIndex[cbCardIndex];
		}
	}
	return true;
}

//检查碰牌
CT_BOOL CGameProcess::CheckPengCard(CT_DWORD dwChairID, CT_BYTE cbCurCard)
{
	if (!m_GameLogic.IsValidCard(cbCurCard))
	{//无效牌
		return false;
	}
	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurCard);

	//碰判断
	return (m_cbCardIndex[dwChairID][cbCurrentIndex] == 2) ? true : false;
}

//获得最大动作(1吃，2碰，3跑，4胡)
CT_BYTE CGameProcess::GetMaxAction(CT_DWORD dwChairID)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return 0;
	}

	if (m_stUserAction[dwChairID].cbActionCode[ACK_CHIHU] > 0)
	{//胡
		return ACK_CHIHU;
	}
	else if (m_stUserAction[dwChairID].cbActionCode[ACK_PENG] > 0)
	{//碰
		return ACK_PENG;
	}
	else if (m_stUserAction[dwChairID].cbActionCode[ACK_CHI] > 0)
	{//吃
		return ACK_CHI;
	}
	return 0;
}

//派发扑克
CT_BOOL CGameProcess::SendUserCard(CT_DWORD dwChairID)
{
	//用户状态
	memset(m_stUserAction, 0, sizeof(m_stUserAction));
	memset(m_stUserOperate, 0, sizeof(m_stUserOperate));
	memset(m_bResponse, 0, sizeof(m_bResponse));
	memset(&m_HuCardInfo, 0, sizeof(m_HuCardInfo));
	m_cbTiPaoAction = 0;
	m_cbWeiAction = 0;
	m_dwSysTime = (CT_DWORD)time(NULL);

	//荒庄结束
	if (0 == m_cbLeftCardCount)
	{
		m_cbProvideCard = 0x00;
		m_dwProvideUser = INVALID_CHAIR;

		LOG_IF(INFO, PRINT_LOG_INFO) << "SendUserCard 111111";

		OnEventGameEnd(INVALID_CHAIR, GER_FLOW_BUREAU);
		return CT_TRUE;
	}

	//摸的牌
	CT_BYTE cbSendCard = m_cbRepertoryCard[--m_cbLeftCardCount];

	//设置变量
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwResumeUser = GetNextUser(dwChairID);
	m_dwProvideUser = dwChairID;			//供应玩家
	m_cbProvideCard = cbSendCard;			//供应牌
	m_cbActionStatus = SEND_CARD_STATUS;	//动作状态
	m_bSendCard = true;

	CT_BOOL bHaveHu = false;
	
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{//判断玩家胡跑牌
		if (EstimateSendCardHu(i, cbSendCard))
		{
			bHaveHu = true;
		}
	}

	CT_BOOL bTiCard = false;
	CT_BOOL bWeiCard = false;
	CT_BOOL bPaoCard = false;
	CT_DWORD dwPaoUser = INVALID_CHAIR;
	//判断提牌
	bTiCard = EstimateTiCard(dwChairID, cbSendCard);
	if (!bTiCard)
	{//没有提 判断偎牌
		bWeiCard = EstimateWeiCard(dwChairID, cbSendCard);
		m_cbWeiAction = bWeiCard ? 1 : 0;
	}
	if (!bHaveHu && !bTiCard && !bWeiCard)
	{//没有胡,提,偎 判断跑牌
		bPaoCard = EstimateSendCardPao(dwChairID, cbSendCard, dwPaoUser);
	}
	
	LOG_IF(INFO, PRINT_LOG_INFO) << "SendUserCard: bTiCard=" << (CT_WORD)bTiCard << ",bPaoCard=" << (CT_WORD)bPaoCard << " ,bWeiCard=" << (CT_WORD)bWeiCard;
	
	//发牌玩家
	CT_DWORD dwSendCardUser = INVALID_CHAIR;
	//判断玩家
	CT_DWORD dwEstimateUser = INVALID_CHAIR;
	CT_BOOL bAction = false;
	if (bTiCard || bWeiCard || bPaoCard)
	{//有提偎跑 就需判断胡牌
		bAction = true;
		//判断玩家
		if (bTiCard || bWeiCard)
		{
			memset(m_stUserAction, 0, sizeof(m_stUserAction));
		}
		dwEstimateUser = bPaoCard ? dwPaoUser : dwChairID;
		m_stUserAction[dwEstimateUser].cbActionCard = cbSendCard;
		m_stUserAction[dwEstimateUser].cbActionCode[ACK_TI] = bTiCard ? 1 : 0;				//动作下标（吃，碰，跑，喂，提，胡）
		m_stUserAction[dwEstimateUser].cbActionCode[ACK_WEI] = bWeiCard ? 1 : 0;			//动作下标（吃，碰，跑，喂，提，胡）
		m_stUserAction[dwEstimateUser].cbActionCode[ACK_PAO] = bPaoCard ? 1 : 0;			//动作下标（吃，碰，跑，喂，提，胡）

		if (bTiCard || bPaoCard)
		{
			m_cbTiPaoAction = bTiCard ? 1 : 2;
		}

		//胡牌判断
		memset(&m_HuCardInfo, 0, sizeof(m_HuCardInfo));
		if (EstimateSendCardHu(dwEstimateUser, cbSendCard, true))
		{
			m_dwResumeUser = dwEstimateUser;
			//m_dwResumeUser = GetNextUser(dwEstimateUser);
		}
		else
		{//没有胡牌
			if (bWeiCard && !IsWeiCanOutCard(dwChairID))
			{//偎了且不能出牌
				m_bWeiCannotHu[dwChairID] = true;
				LOG_IF(INFO, PRINT_LOG_INFO) << "SendUserCard: WeiCannotOutCard Name=" << m_pGameDeskPtr->GetNickName(dwChairID);
			}

			if (((bTiCard || bPaoCard) && IsChongPao(dwEstimateUser)) || m_bWeiCannotHu[dwEstimateUser] || (m_cbUserQiHu[dwEstimateUser] == 1))
			{//发送牌
				dwSendCardUser = GetNextUser(dwEstimateUser);
			}
			else
			{//出牌
				m_dwCurrentUser = dwEstimateUser;
			}
		}
	}
	else
	{//判断 吃碰胡
		if (bHaveHu)
		{//有胡
			bAction = true;
		}
		//判断发牌玩家吃
		CT_BYTE cbMustIndex = 250;
		CT_BOOL bChiCard = EstimatePaoCard(dwChairID, cbSendCard) ? false : true;
		if (bChiCard && GetChiCard(dwChairID, cbSendCard, m_stUserAction[dwChairID].cbChiWeaveCard, m_stUserAction[dwChairID].cbChiWeaveCount, cbMustIndex))
		{
			m_stUserAction[dwChairID].cbActionCard = cbSendCard;
			m_stUserAction[dwChairID].cbActionCode[ACK_CHI] = 1; //吃
			m_stUserAction[dwChairID].cbMustIndex = cbMustIndex;
			bAction = true;
		}

		//判断下一个玩家吃
		CT_DWORD dwNextUser = GetNextUser(dwChairID);
		cbMustIndex = 250;
		CT_BOOL bChiCard2 = EstimatePaoCard(dwNextUser, cbSendCard) ? false : true;
		if (bChiCard2 && GetChiCard(dwNextUser, cbSendCard, m_stUserAction[dwNextUser].cbChiWeaveCard, m_stUserAction[dwNextUser].cbChiWeaveCount, cbMustIndex))
		{
			m_stUserAction[dwNextUser].cbActionCard = cbSendCard;
			m_stUserAction[dwNextUser].cbActionCode[ACK_CHI] = 1; //吃
			m_stUserAction[dwNextUser].cbMustIndex = cbMustIndex;
			bAction = true;
		}

		//判断碰
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (i == dwChairID)
			{//模牌玩家不能碰
				continue;
			}
			if (IsPengCard(i, cbSendCard))
			{//碰牌 最多一个碰
				m_stUserAction[i].cbActionCard = cbSendCard;
				m_stUserAction[i].cbActionCode[ACK_PENG] = 1; //碰
				bAction = true;
				break;
			}
		}

		if (!bAction)
		{//没有动作
			m_cbDiscardCard[dwChairID][m_cbDiscardCount[dwChairID]++] = cbSendCard;
			//发送牌
			dwSendCardUser = GetNextUser(dwChairID);

			//本家和下家都不能吃了
			m_bGuoChiCard[dwSendCardUser][m_GameLogic.SwitchToCardIndex(cbSendCard)] = true;
			m_bGuoChiCard[dwChairID][m_GameLogic.SwitchToCardIndex(cbSendCard)] = true;
		}
	}

	CT_WORD wCurHuXi[GAME_PLAYER] = { 0 };				//当前胡息
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		wCurHuXi[i] = GetHuXi(i);
	}

	//动作流水号
	++m_dwActionNumber;
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		//出牌结果
		FPF_CMD_S_SendCard SendCard;
		memset(&SendCard, 0, sizeof(SendCard));
		SendCard.dwWeiTiPaoUser = INVALID_CHAIR;
		SendCard.dwCurOutCardUser = INVALID_CHAIR;

		SendCard.dwSendCardUser = dwChairID;
		SendCard.cbSendCard = cbSendCard;
		SendCard.cbShow = 1;
		SendCard.cbOutCardTime = m_cbOutCardTime;					//出牌时间
		SendCard.dwCurOutCardUser = m_dwCurrentUser;				//当前出牌玩家

		SendCard.dwActionNumber = m_dwActionNumber;
		SendCard.dwWeiTiPaoUser = dwEstimateUser;
		SendCard.cbActionTime = m_cbOperateTime;
		memcpy(SendCard.wCurHuXi, wCurHuXi, sizeof(wCurHuXi));

		if (bAction && m_GameLogic.IsValidCard(m_stUserAction[i].cbActionCard))
		{//有动作
			memcpy(SendCard.cbActionCode, &m_stUserAction[i].cbActionCode, sizeof(m_stUserAction[i].cbActionCode));
			SendCard.cbActionCard = m_stUserAction[i].cbActionCard;
			SendCard.cbMustIndex = m_stUserAction[i].cbMustIndex;
			SendCard.cbChiWeaveCount = m_stUserAction[i].cbChiWeaveCount;
			memcpy(SendCard.cbChiWeaveCard, &m_stUserAction[i].cbChiWeaveCard, sizeof(m_stUserAction[i].cbChiWeaveCard));

			LOG_IF(INFO, PRINT_LOG_INFO) << "SendCardAction: User=" << i << ",Action=[" << (CT_WORD)SendCard.cbActionCode[ACK_CHI] << "," << (CT_WORD)SendCard.cbActionCode[ACK_PENG] << ","\
				<< (CT_WORD)SendCard.cbActionCode[ACK_PAO] << "," << (CT_WORD)SendCard.cbActionCode[ACK_WEI] << "," << (CT_WORD)SendCard.cbActionCode[ACK_TI] \
				<< "," << (CT_WORD)SendCard.cbActionCode[ACK_CHIHU] << "]" << ",ACard=" << (CT_WORD)SendCard.cbActionCard << "MustIndex" << (CT_WORD)SendCard.cbMustIndex;

			for (CT_WORD i = 0; i < SendCard.cbChiWeaveCount; ++i)
			{
				LOG_IF(INFO, PRINT_LOG_INFO) << "Weave_" << (CT_WORD)i << "=[" << (CT_WORD)SendCard.cbChiWeaveCard[i][0] << "," << (CT_WORD)SendCard.cbChiWeaveCard[i][1] << "," << (CT_WORD)SendCard.cbChiWeaveCard[i][2] << "]";
			}
		}
		//else if (dwEstimateUser != INVALID_CHAIR)
		if (dwEstimateUser != INVALID_CHAIR)
		{
			CT_BYTE cbTempAction = ACK_TI;
			if (bWeiCard)
			{
				cbTempAction = ACK_WEI;
			}
			else if (bPaoCard)
			{
				cbTempAction = ACK_PAO;
			}
			SendCard.cbActionCode[cbTempAction] = 1;

			LOG_IF(INFO, PRINT_LOG_INFO) << "SendCard  User =" << (CT_WORD)i << ",dwWeiTiPaoUser=" << SendCard.dwWeiTiPaoUser;
		}

		//发送数据
		SendTableData(i, FPF_SUB_S_SEND_CARD, &SendCard, sizeof(FPF_CMD_S_SendCard), (i == 0 ? true : false));

		LOG_IF(INFO, PRINT_LOG_INFO) << "SendUserCard: User=" << SendCard.dwSendCardUser << " Card=" << (CT_WORD)SendCard.cbSendCard << " bShow=" << (CT_WORD)SendCard.cbShow;
	}
	//判断听牌
	if (bTiCard || bWeiCard || bPaoCard)
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			EstimateTingCard(i);
		}
	}
	
	if (dwSendCardUser != INVALID_CHAIR)
	{//发牌
		LOG_IF(INFO, PRINT_LOG_INFO) << "SendUserCard: Continue Send Card!";
		SendUserCard(dwSendCardUser);
	}
	return CT_TRUE;
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd()
{
	if (m_dwHuCardUser < GAME_PLAYER)
	{//设胡牌玩家为庄家
		m_dwBankerUser = m_dwHuCardUser;
	}

	LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd HuCardInfo: cbCardEye=" << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].cbCardEye << "cbHuXiCount=" << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].cbHuXiCount;
	for (CT_WORD i = 0; i < m_HuCardInfo[m_dwHuCardUser].cbWeaveCount; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd HuCardInfo Weave: Kind=" << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbWeaveKind << "HuXi=" << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbWeaveHuXi << \
			"Card[" << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbCardList[0] << "," << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbCardList[1] << ","\
			<< (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbCardList[2] << "," << (CT_WORD)m_HuCardInfo[m_dwHuCardUser].WeaveItemArray[i].cbCardList[3] << "]";
	}

	FPF_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));

	//获得名堂
	GetMT(GameEnd.cbMTType, GameEnd.wMTAward);

	for (CT_WORD i = 0; i < MAX_MT_COUNT; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd MT:" << "Type=" << (CT_WORD)GameEnd.cbMTType[i] << "Award[" << GameEnd.wMTAward[i][0] << "][" << GameEnd.wMTAward[i][1] << "]";
	}

	//计算基础胡息和倍数
	CT_WORD wBaseHuXi = 0;						//基础胡息
	CT_WORD wTotalMultiple = 1;				//总倍数
	CT_WORD wTotalHuXi = 0;
	for (CT_WORD i = 0; i < MAX_MT_COUNT; ++i)
	{
		wBaseHuXi += GameEnd.wMTAward[i][0];
		if (GameEnd.wMTAward[i][1] > 0)
		{
			wTotalMultiple *= GameEnd.wMTAward[i][1];
		}
	}
	wBaseHuXi = (wBaseHuXi >= 100) ? wBaseHuXi : m_HuCardInfo[m_dwHuCardUser].cbHuXiCount;
	wTotalHuXi = wBaseHuXi*wTotalMultiple;

	//胡息上限
	CT_WORD wWinLimit = m_cbHuCardType == 1 ? 100 : m_wMaxHuXi; //放炮最大胡息100，by hyz 2017.08.14
	if (wTotalHuXi> wWinLimit)
	{//限制判断
		wTotalHuXi = wWinLimit;
		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd Limit:  wWinLimit=" << wWinLimit << ",wTotalHuXi" << wTotalHuXi;
	}

	GameEnd.dCellScore = m_dwlCellScore*TO_DOUBLE;
	GameEnd.cbReason = 1;														//结束原因(0荒庄,1正常结束)
	GameEnd.cbHuCard = m_cbHuCard;												//胡牌扑克
	GameEnd.cbHuCardType = m_cbHuCardType;										//胡牌类型(0平胡,1放炮,2自模)
	GameEnd.dwHuCardUser = m_dwHuCardUser;										//胡牌玩家
	GameEnd.dwProvideUser = m_dwProvideUser;									//放跑玩家
	GameEnd.wTotalMultiple = wTotalMultiple;									//总倍数
	GameEnd.wBaseHuXi = wBaseHuXi;												//基础胡息
	GameEnd.wTotalHuXi = wTotalHuXi;											//总胡息
	GameEnd.cblastCardCount = m_cbLeftCardCount;								//剩余牌数
	
	
	memcpy(GameEnd.cbUserWeaveCount,m_cbWeaveCount,sizeof(m_cbWeaveCount));		//组合数目
	memcpy(GameEnd.UserWeaveArray, m_WeaveArray, sizeof(m_WeaveArray));			//组合扑克
	for (CT_BYTE i=0; i<GAME_PLAYER; ++i)
	{//剩余牌
		GameEnd.cbRemainCardCount[i] = m_GameLogic.SwitchToCardData(m_cbCardIndex[i], GameEnd.cbRemainCard[i]);
	}

	//剩余牌
	memcpy(GameEnd.cbLastCard, m_cbRepertoryCard, sizeof(CT_BYTE)*m_cbLeftCardCount);
	//胡牌玩家组合数据(胡牌为七个组合)
	memcpy(GameEnd.HuWeaveItem, &m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, sizeof(GameEnd.HuWeaveItem));

	//玩家输赢
	if (1 == m_cbHuCardType)
	{//放炮
		GameEnd.iLWHuXi[m_dwProvideUser] -= wTotalHuXi;
		GameEnd.iLWHuXi[m_dwHuCardUser] = wTotalHuXi;
	}
	else
	{
		GameEnd.iLWHuXi[m_dwHuCardUser] = wTotalHuXi;
	}

	if (m_pGameDeskPtr->IsPrivateRoom())
	{//私人场 
		//记录总数据
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (1 == m_cbHuCardType && i == m_dwProvideUser)
			{//各玩家放炮次数
				++m_wFPCount[i];
			}
			if (i==m_dwHuCardUser)
			{//各玩家胡牌次数
				++m_wHuCount[i];
			}
			//各玩家累计胡息
			m_iTotalHuXi[i] += GameEnd.iLWHuXi[i];

			LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: WriteData FP=" << m_wFPCount[i] << ", HC=" << m_wHuCount[i] << ",TotalHX=" << m_iTotalHuXi[i];
		}
	}

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
        m_RecordScoreInfo[i].iScore = (GameEnd.iLWHuXi[i]*m_dwlCellScore);
        m_RecordScoreInfo[i].dwRevenue = m_pGameDeskPtr->CalculateRevenue(i, m_RecordScoreInfo[i].iScore);
        m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
        m_RecordScoreInfo[i].cbStatus = 1;
		ScoreInfo ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
        ScoreData.bBroadcast = true;
		ScoreData.llScore = (m_RecordScoreInfo[i].iScore-m_RecordScoreInfo[i].dwRevenue);
        ScoreData.llRealScore= ScoreData.llScore;
        GameEnd.dGameScore[i] = (ScoreData.llScore*TO_DOUBLE);
        m_pGameDeskPtr->WriteUserScore(i, ScoreData);

		/*if (0 == ScoreData.llScore)
		{
			ScoreData.scoreKind = enScoreKind_Draw;
		}
		else
		{
			ScoreData.scoreKind = (ScoreData.llScore > 0) ? enScoreKind_Win : enScoreKind_Lost;
		}*/
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
	}

	/*for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{//累计输赢胡息
		GameEnd.iTotalLWHuXi[i] = m_iTotalHuXi[i];
	}*/

	//发送数据
	SendTableData(INVALID_CHAIR, FPF_SUB_S_GAME_END, &GameEnd, sizeof(FPF_CMD_S_GameEnd));

    //记录基本信息
    m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, m_pGameDeskPtr->GetUserID(m_dwBankerUser), 0, 0, 0, 0, 0);

	//基础数据
	LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: HC=" << (CT_WORD)GameEnd.cbHuCard << "HT=" << (CT_WORD)GameEnd.cbHuCardType << "HUser=" << GameEnd.dwHuCardUser << "PUser=" << GameEnd.dwProvideUser;

	LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: TMultiple=" << GameEnd.wTotalMultiple << "BaseHX" << GameEnd.wBaseHuXi << "THX" << GameEnd.wTotalHuXi;

	/*LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: LWHX[" << GameEnd.iLWHuXi[0] << "," << GameEnd.iLWHuXi[1] << "," << GameEnd.iLWHuXi[2] << "]" << \
		"TLWHX[" << GameEnd.iTotalLWHuXi[0] << "," << GameEnd.iTotalLWHuXi[1] << "," << GameEnd.iTotalLWHuXi[2] << "]";*/

	for (CT_WORD i = 0; i < MAX_WEAVE; ++i)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd Weave: kind=" << (CT_WORD)GameEnd.HuWeaveItem[i].cbWeaveKind << "HuXi=" << (CT_WORD)GameEnd.HuWeaveItem[i].cbWeaveHuXi << "Card[" << (CT_WORD)GameEnd.HuWeaveItem[i].cbCardList[0] << ","\
			<< (CT_WORD)GameEnd.HuWeaveItem[i].cbCardList[1] << "," << (CT_WORD)GameEnd.HuWeaveItem[i].cbCardList[2] << "," << (CT_WORD)GameEnd.HuWeaveItem[i].cbCardList[3] << "]";
	}
}

//流局游戏结束
CT_VOID CGameProcess::FlowBureauEnd()
{
	FPF_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));
	GameEnd.cbReason = 0;			//结束原因(0荒庄,1正常结束)

	if (m_pGameDeskPtr->IsPrivateRoom() && m_dwBankerUser < GAME_PLAYER)
	{//私人场 
		//荒庄 庄家扣10分
		m_iTotalHuXi[m_dwBankerUser] -= 10;				//各玩家累计胡息
		++m_wHZCount[m_dwBankerUser];					//各玩家荒庄次数	

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			//写入总积分
			ScoreInfo ScoreData;
			memset(&ScoreData, 0, sizeof(ScoreInfo));
			ScoreData.dwUserID = i;
			if (i == m_dwBankerUser)
			{
				ScoreData.llScore -= 10;
			}
			//ScoreData.scoreKind = enScoreKind_Draw;
			m_pGameDeskPtr->WriteUserScore(i, ScoreData);
		}

		/*for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{//总荒庄分
			GameEnd.iTotalHZScore[i] -= m_wHZCount[i] * 10;
		}*/
	}

	//LOG_IF(INFO, PRINT_LOG_INFO) << "FlowBureauEnd: THZS[" << GameEnd.iTotalHZScore[0] << "," << GameEnd.iTotalHZScore[1] << "," << GameEnd.iTotalHZScore[2] << "]";

	//发送数据
	SendTableData(INVALID_CHAIR, FPF_SUB_S_GAME_END, &GameEnd, sizeof(FPF_CMD_S_GameEnd));
}

//解散结束游戏
CT_VOID CGameProcess::DismissEnd()
{

}

//手上提牌
CT_BYTE CGameProcess::HandTiCard(CT_DWORD dwChairID, CT_BYTE cbTiCardList[MAX_TI_CARD], CT_BYTE& cbTiCardCount)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return 0;
	}

	cbTiCardCount = 0;

	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (4 != m_cbCardIndex[dwChairID][i]) continue;

		//变量定义
		CT_BYTE cbTiCardData = m_GameLogic.SwitchToCardData(i);
		//删除提的牌
		m_cbCardIndex[dwChairID][i] = 0;
		//设置组合
		CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
		m_WeaveArray[dwChairID][cbIndex].cbCardCount = 4;
		m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_TI;
		m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbTiCardData;
		m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbTiCardData;
		m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbTiCardData;
		m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbTiCardData;
		m_WeaveArray[dwChairID][cbIndex].cbCardList[3] = cbTiCardData;
		CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
		m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;

		//记录提的牌
		cbTiCardList[cbTiCardCount++] = cbTiCardData;
	}
	return cbTiCardCount;
}

//判断庄家提牌
CT_BYTE CGameProcess::BankerTiCard(CT_BYTE cbTiCardList[MAX_TI_CARD])
{
	if (m_dwBankerUser>=GAME_PLAYER)
	{//校验庄家
		return 0;
	}
	//提牌扑克个数
	CT_BYTE cbTiCardCount = 0;									

	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (4 != m_cbCardIndex[m_dwBankerUser][i]) continue;

		//变量定义
		CT_BYTE cbTiCardData = m_GameLogic.SwitchToCardData(i);
		//删除提的牌
		m_cbCardIndex[m_dwBankerUser][i] = 0;
		//设置组合
		CT_BYTE cbIndex = m_cbWeaveCount[m_dwBankerUser]++;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCardCount = 4;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbWeaveKind = ACK_TI;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCenterCard = cbTiCardData;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCardList[0] = cbTiCardData;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCardList[1] = cbTiCardData;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCardList[2] = cbTiCardData;
		m_WeaveArray[m_dwBankerUser][cbIndex].cbCardList[3] = cbTiCardData;
		CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[m_dwBankerUser][cbIndex]);
		m_WeaveArray[m_dwBankerUser][cbIndex].cbWeaveHuXi = cbHuXi;

		//记录提的牌
		cbTiCardList[cbTiCardCount++] = cbTiCardData;
	}
	return cbTiCardCount;
}

//获得组合胡息
CT_BYTE CGameProcess::GetHuXi(CT_DWORD dwChairID)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return 0;
	}
	CT_BYTE cbHuXi = 0;
	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		cbHuXi += m_WeaveArray[dwChairID][i].cbWeaveHuXi;
	}
	return cbHuXi;
}

//判断出牌跑牌
CT_BOOL CGameProcess::EstimateOutCardPao(CT_DWORD dwOutCardUser, CT_BYTE cbOutCard, CT_DWORD& dwPaoUser)
{
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (i == dwOutCardUser)
		{//出牌 不可能跑牌
			continue;
		}
		CT_BOOL bHand = IsHandWeiPao(i, cbOutCard);
		if (bHand)
		{//设置手牌偎跑
			SetHandWeiPao(i, cbOutCard);
			dwPaoUser = i;
			return true;
		}

		CT_BOOL bWei = IsWeiPao(i, cbOutCard);
		if (bWei)
		{//设置偎跑
			SetWeiPao(i, cbOutCard);
			dwPaoUser = i;

			//出了别人有偎跑的牌
			m_bOutOtherWeiPao[dwOutCardUser] = true;
			return true;
		}
	}
	return false;
}

//判断发牌跑牌
CT_BOOL CGameProcess::EstimateSendCardPao(CT_DWORD dwSendCardUser, CT_BYTE cbSendCard, CT_DWORD& dwPaoUser, CT_BOOL bDelay)
{
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (IsPengPao(i, cbSendCard))
		{//设置碰跑
			SetPengPao(i, cbSendCard, bDelay);
			dwPaoUser = i;
			return true;
		}
		//判断手里3张 和 已偎的牌
		if (IsHandWeiPao(i, cbSendCard))
		{//设置手牌偎跑
			SetHandWeiPao(i, cbSendCard, bDelay);
			dwPaoUser = i;
			return true;
		}
		if (IsWeiPao(i, cbSendCard))
		{//设置偎跑
			SetWeiPao(i, cbSendCard, bDelay);
			dwPaoUser = i;
			return true;
		}
	}
	return false;
}

//判断提牌
CT_BOOL CGameProcess::EstimateTiCard(CT_DWORD dwCenterUser, CT_BYTE cbCard)
{
	//效验玩家
	if (dwCenterUser >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCard)) return false;

	//手牌提牌
	if (IsHandTiCard(dwCenterUser, cbCard))
	{
		return SetHandTiCard(dwCenterUser, cbCard);
	}
	//偎牌提牌
	if (IsWeiTiCard(dwCenterUser, cbCard))
	{
		return SetWeiTiCard(dwCenterUser, cbCard);
	}
	return false;
}

//判断偎牌
CT_BOOL CGameProcess::EstimateWeiCard(CT_DWORD dwCenterUser, CT_BYTE cbCard)
{
	//效验玩家
	if (dwCenterUser >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCard)) return false;

	if (IsWeiCard(dwCenterUser, cbCard))
	{//有偎牌
		return SetWeiCard(dwCenterUser, cbCard);
	}
	return false;
}

//判断出牌胡
CT_BOOL CGameProcess::EstimateOutCardHu(CT_DWORD dwChairID, CT_DWORD dwOutCardUser, CT_BYTE cbOutCard, CT_BOOL bPaoCard)
{
	tagHuCardInfo	 HuCardInfo; //胡牌信息 
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	CT_BYTE cbTempCard = bPaoCard ? 0 : cbOutCard;
	if (GetHuCardInfo(dwChairID, cbTempCard, HuCardInfo))
	{//跑牌玩家胡
		memcpy(&m_HuCardInfo[dwChairID], &HuCardInfo, sizeof(HuCardInfo));

		m_dwHuCardUser = dwChairID;
		m_cbHuCard = cbOutCard;
		//判断地胡
		CT_BOOL bDiHu = EstimateDiHu(dwOutCardUser);
		if (bDiHu)
		{//地胡
			m_bDiHuFlag = true;
		}
		//胡牌类型(0平胡,1放炮,2自模)
		m_cbHuCardType = 1;

		/*LOG_IF(INFO, PRINT_LOG_INFO) << "EstimateOutCardHu: User=" << dwChairID << ",OutCard=" << (CT_WORD)cbOutCard << ",bPaoCard" << (CT_WORD)bPaoCard<<",bDiHu="<< (CT_WORD)bDiHu;
		OnEventGameEnd(dwChairID, GER_NORMAL);*/
		return true;
	}
	return false;
}

//判断发牌胡跑
CT_BOOL CGameProcess::EstimateSendCardHu(CT_DWORD dwChairID, CT_BYTE cbSendCard, CT_BOOL bTiPaoWei)
{
	tagHuCardInfo	 HuCardInfo; //胡牌信息 
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	CT_BYTE cbTempCard = bTiPaoWei ? 0 : cbSendCard;
	if (GetHuCardInfo(dwChairID, cbTempCard, HuCardInfo))
	{//有胡
		memcpy(&m_HuCardInfo[dwChairID], &HuCardInfo, sizeof(HuCardInfo));
		m_stUserAction[dwChairID].cbActionCard = cbSendCard;
		m_stUserAction[dwChairID].cbActionCode[ACK_CHIHU] = 1;		//胡

		LOG_IF(INFO, PRINT_LOG_INFO) << "EstimateSendCardHu: User=" << dwChairID << ",cbSendCard=" << (CT_WORD)cbSendCard << ",bTiPaoWei" << (CT_WORD)bTiPaoWei;
		return true;
	}
	return false;
}

//判断发牌是否有跑
CT_BOOL CGameProcess::EstimatePaoCard(CT_DWORD dwChairID, CT_BYTE cbSendCard)
{
	//是否碰跑
	if (IsPengPao(dwChairID, cbSendCard))
	{
		return true;
	}
	//是否偎跑
	if (IsWeiPao(dwChairID, cbSendCard))
	{
		return true;
	}
	//是否手牌偎跑
	if (IsHandWeiPao(dwChairID, cbSendCard))
	{
		return true;
	}
	return false;
}

//判断地胡
CT_BOOL CGameProcess::EstimateDiHu(CT_DWORD dwOutCardUser)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "EstimateDiHu: dwOutCardUser=" << dwOutCardUser << ",BankerUser=" << m_dwBankerUser << ",SendCard=" << (CT_WORD)m_bSendCard;
	LOG_IF(INFO, PRINT_LOG_INFO) << "EstimateDiHu: OutCardCount=" << (CT_WORD)m_cbOutCardCount[0] << "," << (CT_WORD)m_cbOutCardCount[1] << "," << (CT_WORD)m_cbOutCardCount[2];

	if (dwOutCardUser != m_dwBankerUser)
	{//出牌玩家不是庄家
		return false;
	}
	if (m_bSendCard)
	{//已经摸过牌
		return false;
	}
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (i == m_dwBankerUser)
		{
			if (1 != m_cbOutCardCount[i])
			{//庄家出过多张牌
				return false;
			}
		}
		else
		{
			if (0 != m_cbOutCardCount[i])
			{//闲家出过牌
				return false;
			}
		}
	}
	return true;
}

//获得名堂
CT_BYTE CGameProcess::GetMT(CT_BYTE cbMTType[MAX_MT_COUNT], CT_WORD wMTAward[MAX_MT_COUNT][2])
{
	if (m_dwHuCardUser >= GAME_PLAYER)
	{
		return false;
	}
	//判断名堂
	CT_BOOL bMT[MAX_MT_COUNT] = { 0 };
	bMT[FPF_MT_ZI_MO] = (m_cbHuCardType == 2) ? true : false;
	bMT[FPF_MT_TIAN_HU] = m_bTianHuFlag;
	bMT[FPF_MT_DI_HU] = m_bDiHuFlag;
	bMT[FPF_MT_WU_HU] = m_GameLogic.IsWuHu(m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, m_HuCardInfo[m_dwHuCardUser].cbWeaveCount);
	bMT[FPF_MT_SHI_SAN_HONG] = m_GameLogic.IsShiSanHong(m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, m_HuCardInfo[m_dwHuCardUser].cbWeaveCount);
	bMT[FPF_MT_SHI_HONG] = m_GameLogic.IsShiHong(m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, m_HuCardInfo[m_dwHuCardUser].cbWeaveCount);
	bMT[FPF_MT_YI_KUAI_BIAN] = m_GameLogic.IsYiKuaiBian(m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, m_HuCardInfo[m_dwHuCardUser].cbWeaveCount);
	bMT[FPF_MT_YI_DIAN_HONG] = m_GameLogic.IsYiDianHong(m_HuCardInfo[m_dwHuCardUser].WeaveItemArray, m_HuCardInfo[m_dwHuCardUser].cbWeaveCount);
	bMT[FPF_MT_HAI_DI_HU] = ((2 == m_cbHuCardType || 0 == m_cbHuCardType) && m_cbLeftCardCount == 0) ? true : false;
	bMT[FPF_MT_30_KA_HU] = (m_HuCardInfo[m_dwHuCardUser].cbHuXiCount == 30) ? true : false;
	bMT[FPF_MT_20_KA_HU] = (m_HuCardInfo[m_dwHuCardUser].cbHuXiCount == 20) ? true : false;

	//过滤
	if (bMT[FPF_MT_TIAN_HU])
	{//有天胡 就没自摸
		bMT[FPF_MT_ZI_MO] = false;
	}
	if (bMT[FPF_MT_SHI_SAN_HONG])
	{//有十三红就没十红
		bMT[FPF_MT_SHI_HONG] = false;
	}
	//设置奖励
	for (CT_WORD i = 0; i < MAX_MT_COUNT; ++i)
	{
		if (!bMT[i]) continue;

		cbMTType[i] = 1;
		switch (i)
		{
		case FPF_MT_TIAN_HU:						//天胡+100
		case FPF_MT_DI_HU:							//地胡+100
		case FPF_MT_WU_HU:							//乌胡  +100
		case FPF_MT_SHI_SAN_HONG:					//十三红+100
		case FPF_MT_30_KA_HU:						//30卡胡+100
		{
			wMTAward[i][0] = 100;
			break;
		}
		case FPF_MT_ZI_MO:							//自摸x2
		case FPF_MT_SHI_HONG:						//十红x2
		case FPF_MT_YI_KUAI_BIAN:					//一块扁x2
		case FPF_MT_YI_DIAN_HONG:					//一点红x2
		case FPF_MT_HAI_DI_HU:						//海底胡x2
		case FPF_MT_20_KA_HU:						//20卡胡x2
		{
			wMTAward[i][1] = 2;
			break;
		}
		}
	}
	return true;
}

//用户出牌
CT_BOOL CGameProcess::OnAutoOutCard(CT_DWORD dwChairID)
{
	//寻找单牌
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (m_cbCardIndex[dwChairID][i] == 1)
		{
			OnUserOutCard(dwChairID, m_GameLogic.SwitchToCardData(i), true);
			return true;
		}
	}

	//寻找对牌
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (m_cbCardIndex[dwChairID][i] == 2)
		{
			OnUserOutCard(dwChairID, m_GameLogic.SwitchToCardData(i), true);
			return true;
		}
	}
	return false;
}

//用户操作
CT_BOOL CGameProcess::OnAutoOperateCard(CT_DWORD dwChairID)
{
	FPF_CMD_C_OperateCard OperateCard;
	memset(&OperateCard, 0, sizeof(OperateCard));
	OperateCard.dwOperateCode = ACK_NULL;

	OnUserOperate(dwChairID, &OperateCard, true);
	return true;
}

//是否有动作
CT_BOOL CGameProcess::IsHaveAction(const UserAction& stUserAction)
{
	for (CT_WORD i = 0; i < ACTION_COUNT; ++i)
	{
		if (1 == stUserAction.cbActionCode[i])
		{
			return true;
		}
	}
	return false;
}

//胡牌结果
CT_BOOL CGameProcess::GetHuCardInfo(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo, CT_BOOL bIsCheckTingCard /*= false*/)
{
	if (m_bWeiCannotHu[dwChairID])
	{//偎牌后不能胡牌
		LOG_IF(INFO, PRINT_LOG_INFO) << "GetHuCardInfo: m_bWeiCannotHu Err! name=" << m_pGameDeskPtr->GetNickName(dwChairID);
		return false;
	}
	if (1 == m_cbUserQiHu[dwChairID])
	{//弃胡
		LOG_IF(INFO, PRINT_LOG_INFO) << "GetHuCardInfo: m_cbUserQiHu Err! name=" << m_pGameDeskPtr->GetNickName(dwChairID);
		return false;
	}

	//分析组合
	std::vector<tagAnalyseItem> AnalyseItemArray;
	AnalyseItemArray.clear();
	//设置结果
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	//构造扑克
	CT_BYTE cbCardIndexTemp[MAX_INDEX] = { 0 };
	memset(cbCardIndexTemp, 0, sizeof(cbCardIndexTemp));
	memcpy(cbCardIndexTemp, &m_cbCardIndex[dwChairID], sizeof(cbCardIndexTemp));

	tagHuCardInfo ChiHuInfo;
	memset(&ChiHuInfo, 0, sizeof(ChiHuInfo));
	tagHuCardInfo PengHuInfo;
	memset(&PengHuInfo, 0, sizeof(PengHuInfo));
	tagHuCardInfo PaoHuInfo;
	memset(&PaoHuInfo, 0, sizeof(PaoHuInfo));

	//吃胡分析
	CT_BOOL bChiHu = ChiHuAnalyse(dwChairID, cbCardIndexTemp, cbCurrentCard, ChiHuInfo);
	//碰胡分析
	CT_BOOL bPengHu = false;
	if (m_GameLogic.IsValidCard(cbCurrentCard) && m_GameLogic.IsWeiPengCard(cbCardIndexTemp, cbCurrentCard))
	{
		bPengHu = PengHuAnalyse(dwChairID, cbCardIndexTemp, cbCurrentCard, PengHuInfo);
	}
	//跑胡分析
	CT_BOOL bPaoHu = false;
	if (m_GameLogic.IsValidCard(cbCurrentCard))
	{
		bPaoHu = PaoHuAnalyse(dwChairID, cbCardIndexTemp, cbCurrentCard, PaoHuInfo, bIsCheckTingCard);
	}
	if (!bChiHu && !bPengHu && !bPaoHu)
	{
		return false;
	}

	//吃,碰,跑
	CT_INT32 iHuXi[3] = { 0 };
	CT_BOOL b30KaHu[3] = { false };
	CT_BOOL b20KaHu[3] = { false };
	CT_BOOL bYiKuaiBianHu[3] = { false };
	if (bChiHu)
	{//吃
		iHuXi[0] = ChiHuInfo.cbHuXiCount;
		b30KaHu[0] = (ChiHuInfo.cbHuXiCount == 30) ? true : false;
		b20KaHu[0] = (ChiHuInfo.cbHuXiCount == 20) ? true : false;
		bYiKuaiBianHu[0] = m_GameLogic.IsYiKuaiBian(ChiHuInfo.WeaveItemArray, ChiHuInfo.cbWeaveCount);
	}
	if (bPengHu)
	{//碰
		iHuXi[1] = PengHuInfo.cbHuXiCount;
		b30KaHu[1] = (PengHuInfo.cbHuXiCount == 30) ? true : false;
		b20KaHu[1] = (PengHuInfo.cbHuXiCount == 20) ? true : false;
		bYiKuaiBianHu[1] = m_GameLogic.IsYiKuaiBian(PengHuInfo.WeaveItemArray, PengHuInfo.cbWeaveCount);
	}
	if (bPaoHu)
	{//跑
		iHuXi[2] = PaoHuInfo.cbHuXiCount;
		b30KaHu[2] = (PaoHuInfo.cbHuXiCount == 30) ? true : false;
		b20KaHu[2] = (PaoHuInfo.cbHuXiCount == 20) ? true : false;
		bYiKuaiBianHu[2] = m_GameLogic.IsYiKuaiBian(PaoHuInfo.WeaveItemArray, PaoHuInfo.cbWeaveCount);
	}

	CT_INT32 iMaxHuXi = 0;
	CT_INT32 iMaxHuXiIndex = -1;
	CT_INT32 i30KaHuIndex = -1;
	CT_INT32 i20KaHuIndex = -1;
	CT_INT32 iYiKuaiBianHuIndex = -1;
	for (CT_WORD i = 0; i < 3; ++i)
	{
		if (iHuXi[i] > iMaxHuXi)
		{
			iMaxHuXi = iHuXi[i];
			iMaxHuXiIndex = i;
		}
		if (b30KaHu[i])
		{
			i30KaHuIndex = i;
		}
		if (b20KaHu[i])
		{
			i20KaHuIndex = i;
		}
		if (bYiKuaiBianHu[i])
		{
			iYiKuaiBianHuIndex = i;
		}
	}

	CT_INT32 iChairIndex = -1;
	if (i30KaHuIndex >= 0)
	{//有30卡胡
		iChairIndex = i30KaHuIndex;
	}
	else if (i20KaHuIndex >= 0)
	{//有20卡胡
		iChairIndex = i20KaHuIndex;
		//有20卡胡，又有一块扁胡时，取最大
		if (iYiKuaiBianHuIndex >= 0)
		{
			if (iHuXi[i20KaHuIndex] < iHuXi[iYiKuaiBianHuIndex])
			{
				iChairIndex = iYiKuaiBianHuIndex;
			}
		}
	}
	else if (iMaxHuXiIndex >= 0)
	{//最大胡息
		iChairIndex = iMaxHuXiIndex;
	}

	if (-1 == iChairIndex)
	{
		return false;
	}

	if (0 == iChairIndex)
	{
		memcpy(&HuCardInfo, &ChiHuInfo, sizeof(ChiHuInfo));
	}
	else if (1 == iChairIndex)
	{
		memcpy(&HuCardInfo, &PengHuInfo, sizeof(PengHuInfo));
	}
	else
	{
		memcpy(&HuCardInfo, &PaoHuInfo, sizeof(PaoHuInfo));
	}
	return true;
}

//吃胡分析
CT_BOOL CGameProcess::ChiHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo)
{
	//分析组合
	std::vector<tagAnalyseItem> AnalyseItemArray;
	AnalyseItemArray.clear();
	//设置结果
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	//构造扑克
	CT_BYTE cbTempIndex[MAX_INDEX] = { 0 };
	memset(cbTempIndex, 0, sizeof(cbTempIndex));
	memcpy(cbTempIndex, cbCardIndex, sizeof(cbTempIndex));

	//加上已有组合
	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID] && i < MAX_WEAVE; ++i)
	{
		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		HuCardInfo.WeaveItemArray[cbIndex] = m_WeaveArray[dwChairID][i];
	}

	//提取偎牌
	TakeWeiCard(cbTempIndex, HuCardInfo);

	//加入手牌
	if (m_GameLogic.IsValidCard(cbCurrentCard))
	{
		CT_BYTE cbCurIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
		++cbTempIndex[cbCurIndex];
	}

	//获得胡息
	CT_BYTE cbHuXiNum = 0;
	for (CT_WORD i = 0; i < HuCardInfo.cbWeaveCount && i < MAX_WEAVE; ++i)
	{
		cbHuXiNum += HuCardInfo.WeaveItemArray[i].cbWeaveHuXi;
	}

	//数目统计
	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(cbTempIndex);
	if (0 == cbCardCount)
	{
		if (cbHuXiNum >= START_HU_XI && HuCardInfo.cbWeaveCount == 7)
		{
			HuCardInfo.cbHuXiCount = cbHuXiNum;
			return true;
		}
		return false;
	}

	//分析 加入操作牌后 胡牌
	m_GameLogic.AnalyseCard(cbTempIndex, AnalyseItemArray);

	//胡牌组合个数
	CT_BYTE cbHuCardCount = (CT_BYTE)AnalyseItemArray.size();
	if (0 == cbHuCardCount)
	{//没有可胡的牌
		return false;
	}

	//查找最大组合
	if (FindMaxHuCard(cbHuXiNum, AnalyseItemArray, HuCardInfo))
	{
		return ((7 == HuCardInfo.cbWeaveCount) && (HuCardInfo.cbHuXiCount >= START_HU_XI));
	}
	return false;
}

//碰胡分析
CT_BOOL CGameProcess::PengHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo)
{
	//分析组合
	std::vector<tagAnalyseItem> AnalyseItemArray;
	AnalyseItemArray.clear();
	//设置结果
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));
	//构造扑克
	CT_BYTE cbTempIndex[MAX_INDEX] = { 0 };
	memset(cbTempIndex, 0, sizeof(cbTempIndex));
	memcpy(cbTempIndex, cbCardIndex, sizeof(cbTempIndex));

	if (!m_GameLogic.IsWeiPengCard(cbTempIndex, cbCurrentCard))
	{//没有碰
		return false;
	}

	//加上已有组合
	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID] && i < MAX_WEAVE; ++i)
	{
		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		HuCardInfo.WeaveItemArray[cbIndex] = m_WeaveArray[dwChairID][i];
	}

	//提取碰牌
	CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
	HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 3;
	HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_PENG;
	HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = cbCurrentCard;
	HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = cbCurrentCard;
	HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = cbCurrentCard;
	HuCardInfo.WeaveItemArray[cbIndex].cbCardList[2] = cbCurrentCard;
	//设置胡息
	HuCardInfo.WeaveItemArray[cbIndex].cbWeaveHuXi = m_GameLogic.GetWeaveHuXi(HuCardInfo.WeaveItemArray[cbIndex]);
	//删除扑克
	cbTempIndex[m_GameLogic.SwitchToCardIndex(cbCurrentCard)] = 0;

	//提取偎牌
	TakeWeiCard(cbTempIndex, HuCardInfo);
	//获得胡息
	CT_BYTE cbHuXiNum = 0;
	for (CT_WORD i = 0; i < HuCardInfo.cbWeaveCount && i < MAX_WEAVE; ++i)
	{
		cbHuXiNum += HuCardInfo.WeaveItemArray[i].cbWeaveHuXi;
	}

	//数目统计
	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(cbTempIndex);
	if (0 == cbCardCount)
	{
		if (cbHuXiNum >= START_HU_XI && HuCardInfo.cbWeaveCount==7)
		{
			HuCardInfo.cbHuXiCount = cbHuXiNum;
			return true;
		}
		return false;
	}

	//分析 加入操作牌后 胡牌
	m_GameLogic.AnalyseCard(cbTempIndex, AnalyseItemArray);

	//胡牌组合个数
	CT_BYTE cbHuCardCount = (CT_BYTE)AnalyseItemArray.size();
	if (0 == cbHuCardCount)
	{//没有可胡的牌
		return false;
	}
	//查找最大组合
	if (FindMaxHuCard(cbHuXiNum, AnalyseItemArray, HuCardInfo))
	{
		return ((7 == HuCardInfo.cbWeaveCount) && (HuCardInfo.cbHuXiCount >= START_HU_XI));
	}
	return false;
}

//跑胡分析
CT_BOOL CGameProcess::PaoHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo, CT_BOOL bIsCheckTingCard /*= false*/)
{
	//分析组合
	std::vector<tagAnalyseItem> AnalyseItemArray;
	AnalyseItemArray.clear();
	//构造扑克
	CT_BYTE cbTempIndex[MAX_INDEX] = { 0 };
	memcpy(cbTempIndex, cbCardIndex, sizeof(cbTempIndex));

	//是否碰跑
	CT_BOOL bPengPao = false;
	if (m_cbActionStatus == SEND_CARD_STATUS || bIsCheckTingCard)
	{
		bPengPao = IsPengPao(dwChairID, cbCurrentCard);
	}
	//是否偎跑
	CT_BOOL bWeiPao = IsWeiPao(dwChairID, cbCurrentCard);
	//是否手牌偎跑
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	CT_BOOL bHandWeiPao = (cbTempIndex[cbCurrentIndex] == 3) ? true : false;

	if (!bPengPao && !bWeiPao && !bHandWeiPao)
	{//没有跑
		return false;
	}

	//加上已有组合
	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID] && i < MAX_WEAVE; ++i)
	{
		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		HuCardInfo.WeaveItemArray[cbIndex] = m_WeaveArray[dwChairID][i];
	}

	if (bPengPao || bWeiPao)
	{//设置碰/偎跑
		CT_WORD wACKType = bPengPao ? ACK_PENG : ACK_WEI;
		for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
		{
			if (wACKType == HuCardInfo.WeaveItemArray[i].cbWeaveKind && HuCardInfo.WeaveItemArray[i].cbCenterCard == cbCurrentCard)
			{
				HuCardInfo.WeaveItemArray[i].cbWeaveKind = ACK_PAO;
				HuCardInfo.WeaveItemArray[i].cbCardCount = 4;
				HuCardInfo.WeaveItemArray[i].cbCardList[3] = cbCurrentCard;

				CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(HuCardInfo.WeaveItemArray[i]);
				HuCardInfo.WeaveItemArray[i].cbWeaveHuXi = cbHuXi;
				break;
			}
		}
	}
	else if (bHandWeiPao)
	{//设置手上偎跑
		//删除牌
		cbTempIndex[cbCurrentIndex] = 0;

		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 4;
		HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_PAO;
		HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = cbCurrentCard;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = cbCurrentCard;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = cbCurrentCard;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[2] = cbCurrentCard;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[3] = cbCurrentCard;
		CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(HuCardInfo.WeaveItemArray[cbIndex]);
		HuCardInfo.WeaveItemArray[cbIndex].cbWeaveHuXi = cbHuXi;
	}

	//提取偎牌
	TakeWeiCard(cbTempIndex, HuCardInfo);
	//获得胡息
	CT_BYTE cbHuXiNum = 0;
	for (CT_WORD i = 0; i < HuCardInfo.cbWeaveCount && i < MAX_WEAVE; ++i)
	{
		cbHuXiNum += HuCardInfo.WeaveItemArray[i].cbWeaveHuXi;
	}

	//数目统计
	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(cbTempIndex);
	if (0 == cbCardCount)
	{
		if (cbHuXiNum >= START_HU_XI && HuCardInfo.cbWeaveCount == 7)
		{
			HuCardInfo.cbHuXiCount = cbHuXiNum;
			return true;
		}
		return false;
	}

	//分析 加入操作牌后 胡牌
	m_GameLogic.AnalyseCard(cbTempIndex, AnalyseItemArray);

	//胡牌组合个数
	CT_BYTE cbHuCardCount = (CT_BYTE)AnalyseItemArray.size();
	if (0 == cbHuCardCount)
	{//没有可胡的牌
		return false;
	}
	//查找最大组合
	if (FindMaxHuCard(cbHuXiNum, AnalyseItemArray, HuCardInfo))
	{
		return ((7 == HuCardInfo.cbWeaveCount) && (HuCardInfo.cbHuXiCount >= START_HU_XI));
	}
	return false;
}

//查找最大组合
CT_BOOL CGameProcess::FindMaxHuCard(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray, tagHuCardInfo & HuCardInfo)
{
	//查找最大组合索引
	CT_INT32 iMaxIndex = FindMaxHuXiIndex(AnalyseItemArray);
	//查找30卡胡索引
	CT_INT32 i30KaHuIndex = Find30KaHuIndex(cbHuXi, AnalyseItemArray);
	//查找20卡胡索引
	CT_INT32 i20KaHuIndex = Find20KaHuIndex(cbHuXi, AnalyseItemArray);
	//查找一块匾胡索引
	CT_INT32 iYiKuaiBianHuIndex = FindYiKuaiBianHuIndex(cbHuXi, AnalyseItemArray, HuCardInfo, i20KaHuIndex);
	
	//取最优组合
	CT_INT32 iHuIndex = iMaxIndex;
	if (i20KaHuIndex > -1) iHuIndex = i20KaHuIndex;
	if (iYiKuaiBianHuIndex > -1) iHuIndex = iYiKuaiBianHuIndex;
	if (i30KaHuIndex > -1) iHuIndex = i30KaHuIndex;

	LOG_IF(INFO, PRINT_LOG_INFO) << "FindMaxHuCard: iMaxIndex = " << iMaxIndex;
	LOG_IF(INFO, PRINT_LOG_INFO) << "FindMaxHuCard: i30KaHuIndex = " << i30KaHuIndex;
	LOG_IF(INFO, PRINT_LOG_INFO) << "FindMaxHuCard: i20KaHuIndex = " << i20KaHuIndex;
	LOG_IF(INFO, PRINT_LOG_INFO) << "FindMaxHuCard: iYiKuaiBianHuIndex = " << iYiKuaiBianHuIndex;
	LOG_IF(INFO, PRINT_LOG_INFO) << "FindMaxHuCard: iHuIndex = " << iHuIndex;

	if (iHuIndex < 0)
	{//胡牌错误
		return false;
	}

	//获取子项
	tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[iHuIndex];
	//牌眼
	HuCardInfo.cbCardEye = pAnalyseItem->cbCardEye;
	//设置总基础胡息
	HuCardInfo.cbHuXiCount += cbHuXi;
	//设置组合
	for (CT_WORD i = 0; i < pAnalyseItem->cbWeaveCount; i++)
	{
		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		if (cbIndex >= 7)
		{
			return false;
		}
		HuCardInfo.WeaveItemArray[cbIndex] = pAnalyseItem->WeaveItemArray[i];
		HuCardInfo.cbHuXiCount += pAnalyseItem->WeaveItemArray[i].cbWeaveHuXi;
	}

	if (HuCardInfo.cbWeaveCount == 6 && m_GameLogic.IsValidCard(HuCardInfo.cbCardEye))
	{//设置牌眼
		CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
		HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_JIANG;
		HuCardInfo.WeaveItemArray[cbIndex].cbWeaveHuXi = 0;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 2;
		HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = HuCardInfo.cbCardEye;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = HuCardInfo.cbCardEye;
		HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = HuCardInfo.cbCardEye;
	}
	return true;
}

//提取偎牌
CT_VOID CGameProcess::TakeWeiCard(CT_BYTE cbCardIndex[MAX_INDEX], tagHuCardInfo & HuCardInfo)
{
	//提取三牌
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (cbCardIndex[i] == 3)
		{
			//设置扑克
			cbCardIndex[i] = 0;

			//设置组合
			CT_BYTE cbCardData = m_GameLogic.SwitchToCardData(i);
			CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 3;
			HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_WEI;
			HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[2] = cbCardData;
			//设置胡息
			HuCardInfo.WeaveItemArray[cbIndex].cbWeaveHuXi = m_GameLogic.GetWeaveHuXi(HuCardInfo.WeaveItemArray[cbIndex]);
		}
	}
}

//查找最大组合索引
CT_INT32 CGameProcess::FindMaxHuXiIndex(std::vector<tagAnalyseItem> AnalyseItemArray)
{
	CT_BYTE cbMaxHuXi = 0;
	CT_INT32 iMaxIndex = -1;
	for (CT_WORD i = 0; i < AnalyseItemArray.size(); ++i)
	{
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[i];

		CT_BYTE cbTempHuXi = 0;
		for (CT_WORD j = 0; j < pAnalyseItem->cbWeaveCount; ++j)
		{
			cbTempHuXi += pAnalyseItem->WeaveItemArray[j].cbWeaveHuXi;
		}

		//胡息分析
		if (cbTempHuXi >= cbMaxHuXi)
		{//最大胡息
			iMaxIndex = i;
			cbMaxHuXi = cbTempHuXi;
		}
	}
	return iMaxIndex;
}

//查找30卡胡索引
CT_INT32 CGameProcess::Find30KaHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray)
{
	CT_INT32 i30Index = -1;
	for (CT_WORD i = 0; i < AnalyseItemArray.size(); ++i)
	{
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[i];

		CT_BYTE cbTempHuXi = 0;
		for (CT_WORD j = 0; j < pAnalyseItem->cbWeaveCount; ++j)
		{
			cbTempHuXi += pAnalyseItem->WeaveItemArray[j].cbWeaveHuXi;
		}

		//胡息分析
		if ((cbTempHuXi + cbHuXi) == 30)
		{//30卡胡
			i30Index = i;
			break;
		}
	}
	return i30Index;
}

//查找20卡胡索引
CT_INT32 CGameProcess::Find20KaHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray)
{
	CT_INT32 i20Index = -1;
	for (CT_WORD i = 0; i < AnalyseItemArray.size(); ++i)
	{
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[i];

		CT_BYTE cbTempHuXi = 0;
		for (CT_WORD j = 0; j < pAnalyseItem->cbWeaveCount; ++j)
		{
			cbTempHuXi += pAnalyseItem->WeaveItemArray[j].cbWeaveHuXi;
		}

		//胡息分析
		if ((cbTempHuXi + cbHuXi) == 20)
		{//20卡胡
			i20Index = i;
			break;
		}
	}
	return i20Index;
}

CT_INT32 CGameProcess::FindYiKuaiBianHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray, tagHuCardInfo & HuCardInfo, CT_INT32 i20KaHuIndex)
{
	CT_INT32 iYiKuaiBianIndex = -1;
	for (CT_WORD i = 0; i < AnalyseItemArray.size(); ++i)
	{
		tagHuCardInfo TempHuCardInfo;
		memset(&TempHuCardInfo, 0, sizeof(TempHuCardInfo));
		memcpy(&TempHuCardInfo, &HuCardInfo, sizeof(HuCardInfo));
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[i];
		//牌眼
		TempHuCardInfo.cbCardEye = pAnalyseItem->cbCardEye;
		//设置总基础胡息
		TempHuCardInfo.cbHuXiCount += cbHuXi;
		//设置组合
		for (CT_WORD i = 0; i < pAnalyseItem->cbWeaveCount; i++)
		{
			CT_BYTE cbIndex = TempHuCardInfo.cbWeaveCount++;
			if (cbIndex >= 7)
			{
				return false;
			}
			TempHuCardInfo.WeaveItemArray[cbIndex] = pAnalyseItem->WeaveItemArray[i];
			TempHuCardInfo.cbHuXiCount += pAnalyseItem->WeaveItemArray[i].cbWeaveHuXi;
		}
		//判断是否能胡
		if (TempHuCardInfo.cbHuXiCount >= START_HU_XI)
		{
			//有20卡胡，取最大胡息
			if ((i20KaHuIndex != -1 && TempHuCardInfo.cbHuXiCount >= 20) || (i20KaHuIndex == -1))
			{
				if (TempHuCardInfo.cbWeaveCount == 6 && m_GameLogic.IsValidCard(TempHuCardInfo.cbCardEye))
				{//设置牌眼
					CT_BYTE cbIndex = TempHuCardInfo.cbWeaveCount++;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_JIANG;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbWeaveHuXi = 0;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 2;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = TempHuCardInfo.cbCardEye;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = TempHuCardInfo.cbCardEye;
					TempHuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = TempHuCardInfo.cbCardEye;
				}

				if (m_GameLogic.IsYiKuaiBian(TempHuCardInfo.WeaveItemArray, TempHuCardInfo.cbWeaveCount))
				{
					iYiKuaiBianIndex = i;
					break;
				}
			}		
		}	
	}
	
	return iYiKuaiBianIndex;
}

//////////////////////////////////////////////////////////////////
//是否碰跑
CT_BOOL CGameProcess::IsPengPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_PENG == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			return true;
		}
	}
	return false;
}

//是否偎跑
CT_BOOL CGameProcess::IsWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_WEI == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			return true;
		}
	}
	return false;
}

//是否手牌偎跑
CT_BOOL CGameProcess::IsHandWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);

	//跑偎判断
	return (m_cbCardIndex[dwChairID][cbCurrentIndex] == 3) ? true : false;
}

//设置碰跑
CT_BOOL CGameProcess::SetPengPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_PENG == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			m_WeaveArray[dwChairID][i].cbCardCount = 4;
			m_WeaveArray[dwChairID][i].cbWeaveKind = ACK_PAO;
			m_WeaveArray[dwChairID][i].cbCenterCard = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[0] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[1] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[2] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[3] = cbCurrentCard;
			CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][i]);
			m_WeaveArray[dwChairID][i].cbWeaveHuXi = cbHuXi;

			return true;
		}
	}
	return false;
}

//设置偎跑
CT_BOOL CGameProcess::SetWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_WEI == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			m_WeaveArray[dwChairID][i].cbCardCount = 4;
			m_WeaveArray[dwChairID][i].cbWeaveKind = ACK_PAO;
			m_WeaveArray[dwChairID][i].cbCenterCard = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[0] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[1] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[2] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[3] = cbCurrentCard;
			CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][i]);
			m_WeaveArray[dwChairID][i].cbWeaveHuXi = cbHuXi;
			return true;
		}
	}
	return false;
}

//设置手牌偎跑
CT_BOOL CGameProcess::SetHandWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;
	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//牌张数
	if (3 != m_cbCardIndex[dwChairID][cbCurrentIndex]) return false;

	//删除牌
	m_cbCardIndex[dwChairID][cbCurrentIndex] = 0;

	CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
	m_WeaveArray[dwChairID][cbIndex].cbCardCount = 4;
	m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_PAO;
	m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[3] = cbCurrentCard;
	CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
	m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;

	return true;
}

//是否手牌提牌
CT_BOOL CGameProcess::IsHandTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;
	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//偎判断
	return (m_cbCardIndex[dwChairID][cbCurrentIndex] == 3) ? true : false;
}

//是否偎提牌
CT_BOOL CGameProcess::IsWeiTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_WEI == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			return true;
		}
	}
	return false;
}

//设置手牌提牌
CT_BOOL CGameProcess::SetHandTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;
	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//牌张数
	if (3 != m_cbCardIndex[dwChairID][cbCurrentIndex]) return false;

	//删除牌
	m_cbCardIndex[dwChairID][cbCurrentIndex] = 0;

	CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
	m_WeaveArray[dwChairID][cbIndex].cbCardCount = 4;
	m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_TI;
	m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[3] = cbCurrentCard;
	CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
	m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;

	return true;
}

//设置偎提牌
CT_BOOL CGameProcess::SetWeiTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (ACK_WEI == m_WeaveArray[dwChairID][i].cbWeaveKind && m_WeaveArray[dwChairID][i].cbCenterCard == cbCurrentCard)
		{
			m_WeaveArray[dwChairID][i].cbCardCount = 4;
			m_WeaveArray[dwChairID][i].cbWeaveKind = ACK_TI;
			m_WeaveArray[dwChairID][i].cbCenterCard = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[0] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[1] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[2] = cbCurrentCard;
			m_WeaveArray[dwChairID][i].cbCardList[3] = cbCurrentCard;
			CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][i]);
			m_WeaveArray[dwChairID][i].cbWeaveHuXi = cbHuXi;

			return true;
		}
	}
	return true;
}

//是否偎牌
CT_BOOL CGameProcess::IsWeiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//偎判断
	return (m_cbCardIndex[dwChairID][cbCurrentIndex] == 2) ? true : false;
}

//设置偎牌
CT_BOOL CGameProcess::SetWeiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//牌张数
	if (2 != m_cbCardIndex[dwChairID][cbCurrentIndex]) return false;

	//删除牌
	m_cbCardIndex[dwChairID][cbCurrentIndex] = 0;

	CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
	m_WeaveArray[dwChairID][cbIndex].cbCardCount = 3;
	m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_WEI;
	m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbCurrentCard;
	CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
	m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;

	return true;
}

//获得吃牌
CT_BOOL CGameProcess::GetChiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BYTE cbChiWeaveCard[MAX_ITEM_COUNT][3], CT_BYTE& cbChiWeaveCount, CT_BYTE& cbMustIndex)
{
	if (1 == m_cbUserQiHu[dwChairID])
	{//弃胡
		return false;
	}
	cbChiWeaveCount = 0;
	cbMustIndex = 250;
	if (dwChairID >= GAME_PLAYER)
	{//效验玩家
		return false;
	}
	if (!m_GameLogic.IsValidCard(cbCurrentCard))
	{//校验牌
		return false;
	}
	if (m_bGuoChiCard[dwChairID][m_GameLogic.SwitchToCardIndex(cbCurrentCard)])
	{//过牌
		LOG_IF(INFO,PRINT_LOG_INFO) << "GetChiCard: User Guo Chi! User=" << dwChairID;
		return false;
	}
	if (m_bOutOtherWeiPao[dwChairID])
	{//出了别人有偎跑的牌 就不能再吃碰了
		return false;
	}

	//拷贝牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memset(cbTempCardIndex, 0, sizeof(cbTempCardIndex));
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));

	//获得组合
	CT_BYTE cbTempChiCount = 0;
	CT_BYTE cbTempChiWeave[MAX_ITEM_COUNT][3];
	memset(cbTempChiWeave, 0, sizeof(cbTempChiWeave));
	cbTempChiCount = m_GameLogic.GetChiWeave(cbTempCardIndex, cbCurrentCard, cbTempChiWeave);
	if (0 == cbTempChiCount)
	{//没吃
		return false;
	}

	//获得能摆组合
	CT_BYTE WeaveCard[100][MAX_ITEM_COUNT][3];
	CT_BYTE cbCount = GetBaiCardWeave(cbTempCardIndex, cbTempChiWeave, cbTempChiCount, cbCurrentCard, WeaveCard);
	if (0 == cbCount)
	{//没有可摆组合
		return false;
	}

	for (CT_WORD i = 0; i < cbTempChiCount; ++i)
	{//吃的个数
		if (IsHaveChiSameSevae(WeaveCard, cbCount, cbTempChiWeave[i]))
		{
			cbChiWeaveCard[cbChiWeaveCount][0] = cbTempChiWeave[i][0];
			cbChiWeaveCard[cbChiWeaveCount][1] = cbTempChiWeave[i][1];
			cbChiWeaveCard[cbChiWeaveCount][2] = cbTempChiWeave[i][2];
			++cbChiWeaveCount;
		}
	}

	//判断摆牌后能不能出牌
	CT_BYTE cbSuccWeave[100][MAX_ITEM_COUNT][3];
	CT_BYTE cbSuccCount = 0;
	if (EstimateBaiCanOutCard(cbTempCardIndex, cbCurrentCard, WeaveCard, cbCount, cbSuccWeave, cbSuccCount))
	{//所有能摆的都可以出牌
		//return true;
	}

	if (cbSuccCount == 0)
	{//没有能摆的牌
		return false;
	}
	if (cbSuccCount==1)
	{//只有一种摆法
		return true;
	}

	LOG_IF(INFO,PRINT_LOG_INFO) << "Find MustIndex! ";

	//找必选的
	CT_BYTE cbFindCount = 0;
	CT_BYTE cbFindWeave[3][3];
	for (CT_WORD i = 0; i < 3; ++i)
	{
		if (0 == cbSuccWeave[0][i][0]) continue;

		CT_BOOL bSucc = true;
		for (CT_WORD j = 1; j < cbSuccCount; ++j)
		{//是否有相同组合
			if (!IsHaveSameWeave(cbSuccWeave[j], cbSuccWeave[0][i]))
			{
				bSucc = false;
			}
		}
		if (bSucc)
		{
			cbFindWeave[cbFindCount][0] = cbSuccWeave[0][i][0];
			cbFindWeave[cbFindCount][1] = cbSuccWeave[0][i][1];
			cbFindWeave[cbFindCount][2] = cbSuccWeave[0][i][2];
			++cbFindCount;
		}
	}

	LOG_IF(INFO,PRINT_LOG_INFO) << "=====================";
	for (CT_WORD i = 0; i < cbFindCount; ++i)
	{
		LOG_IF(INFO,PRINT_LOG_INFO) << "GetChiCard: SameWeave=[" << (CT_WORD)cbFindWeave[i][0] << "," << (CT_WORD)cbFindWeave[i][1] << "," << (CT_WORD)cbFindWeave[i][2] << "]";
	}
	for (CT_WORD i = 0; i < cbChiWeaveCount; ++i)
	{
		LOG_IF(INFO,PRINT_LOG_INFO) << "GetChiCard: ChiWeave=[" << (CT_WORD)cbChiWeaveCard[i][0] << "," << (CT_WORD)cbChiWeaveCard[i][1] << "," << (CT_WORD)cbChiWeaveCard[i][2] << "]";
	}

	//if (cbFindCount == 1)
	{
		for (CT_BYTE i = 0; i < cbChiWeaveCount; ++i)
		{
			if (m_GameLogic.IsSameWeave(cbChiWeaveCard[i], cbFindWeave[0]))
			{
				cbMustIndex = i;
				LOG_IF(INFO,PRINT_LOG_INFO) << "GetChiCard: cbMustIndex=" << (CT_WORD)cbMustIndex;
				LOG_IF(INFO,PRINT_LOG_INFO) << "=====================";
				return true;
			}
		}
	}
	return true;
}

//设置吃牌组合
CT_BOOL CGameProcess::SetChiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BYTE cbChiWeaveCard[MAX_ITEM_COUNT][3], CT_BYTE cbChiWeaveCount)
{
	if (dwChairID >= GAME_PLAYER)
	{//效验玩家
		return false;
	}
	if (0 == cbChiWeaveCount)
	{//没有组合
		return false;
	}
	if (!m_GameLogic.IsValidCard(cbCurrentCard))
	{//效验牌
		return false;
	}

	//先尝试删除
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };		//玩家手牌数据
	memset(cbTempCardIndex, 0, sizeof(cbTempCardIndex));
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));

	//删除吃牌
	CT_BYTE cbChiIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	cbTempCardIndex[cbChiIndex] = 0;

	for (CT_WORD i = 0; i < cbChiWeaveCount; ++i)
	{
		//删除扑克
		for (CT_WORD n = 0; n < 3; ++n)
		{
			CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbChiWeaveCard[i][n]);
			if (cbChiIndex == cbCardIndex)
			{//吃的牌不处理
				continue;
			}
			if (0 == cbTempCardIndex[cbCardIndex])
			{//牌不对
				return false;
			}
			--cbTempCardIndex[cbCardIndex];
		}
	}

	//////////////////////////////////////////////////////////////////////////////////

	//正式删除吃的牌
	m_cbCardIndex[dwChairID][cbChiIndex] = 0;

	for (CT_WORD i = 0; i < cbChiWeaveCount; ++i)
	{
		//删除扑克
		for (CT_WORD n = 0; n < 3; ++n)
		{
			CT_BYTE cbCardIndex = m_GameLogic.SwitchToCardIndex(cbChiWeaveCard[i][n]);
			if (cbChiIndex == cbCardIndex)
			{//吃的牌不处理
				continue;
			}
			if (m_cbCardIndex[dwChairID][cbCardIndex] > 0)
			{//删除
				--m_cbCardIndex[dwChairID][cbCardIndex];
			}
		}
	}

	//设置组合
	for (CT_WORD i = 0; i < cbChiWeaveCount; ++i)
	{
		CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
		m_WeaveArray[dwChairID][cbIndex].cbCardCount = 3;
		m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_CHI;
		m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbCurrentCard;
		m_WeaveArray[dwChairID][cbIndex].cbShowCenter = (i==0)?1:0;
		m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbChiWeaveCard[i][0];
		m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbChiWeaveCard[i][1];
		m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbChiWeaveCard[i][2];
		CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
		m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;
	}
	return true;
}

//是否碰牌
CT_BOOL CGameProcess::IsPengCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	if (1 == m_cbUserQiHu[dwChairID])
	{//弃胡
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsPengCard: QiHu! User="<<dwChairID;
		return false;
	}

	if (dwChairID >= GAME_PLAYER)
	{//玩家不对
		return false;
	}
	if (!m_GameLogic.IsValidCard(cbCurrentCard))
	{// 效验扑克
		return false;
	}
	if (m_bGuoPengCard[dwChairID][m_GameLogic.SwitchToCardIndex(cbCurrentCard)])
	{//过牌
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsPengCard: GuoPeng! User="<<dwChairID;
		return false;
	}
	if (m_bOutOtherWeiPao[dwChairID])
	{//出了别人有偎跑的牌 就不能再吃碰了
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsPengCard: WeiPao! User=" << dwChairID;
		return false;
	}
	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);

	//临时牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));

	if (cbTempCardIndex[cbCurrentIndex] != 2)
	{//牌数量不对
		return false;
	}

	//清碰的牌
	cbTempCardIndex[cbCurrentIndex] = 0;

	for (CT_WORD i = 0; i < MAX_INDEX; ++i)
	{//清坎
		if (cbTempCardIndex[i] == 3)
		{
			cbTempCardIndex[i] = 0;
		}
	}

	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(cbTempCardIndex);
	return (cbCardCount > 0) ? true : false;
}

//设置碰牌
CT_BOOL CGameProcess::SetPengCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	//效验扑克
	if (!m_GameLogic.IsValidCard(cbCurrentCard)) return false;

	//转换索引
	CT_BYTE cbCurrentIndex = m_GameLogic.SwitchToCardIndex(cbCurrentCard);
	//牌张数
	if (2 != m_cbCardIndex[dwChairID][cbCurrentIndex]) return false;

	//删除牌
	m_cbCardIndex[dwChairID][cbCurrentIndex] = 0;

	CT_BYTE cbIndex = m_cbWeaveCount[dwChairID]++;
	m_WeaveArray[dwChairID][cbIndex].cbCardCount = 3;
	m_WeaveArray[dwChairID][cbIndex].cbWeaveKind = ACK_PENG;
	m_WeaveArray[dwChairID][cbIndex].cbCenterCard = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[0] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[1] = cbCurrentCard;
	m_WeaveArray[dwChairID][cbIndex].cbCardList[2] = cbCurrentCard;
	CT_BYTE cbHuXi = m_GameLogic.GetWeaveHuXi(m_WeaveArray[dwChairID][cbIndex]);
	m_WeaveArray[dwChairID][cbIndex].cbWeaveHuXi = cbHuXi;
	return true;
}

//设置跑牌
CT_BOOL CGameProcess::SetPaoCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard)
{
	if (IsPengPao(dwChairID, cbCurrentCard))
	{//设置碰跑
		return SetPengPao(dwChairID, cbCurrentCard);
	}
	//判断手里3张 和 已偎的牌
	if (IsHandWeiPao(dwChairID, cbCurrentCard))
	{//设置手牌偎跑
		return SetHandWeiPao(dwChairID, cbCurrentCard);
	}
	if (IsWeiPao(dwChairID, cbCurrentCard))
	{//设置偎跑
		return SetWeiPao(dwChairID, cbCurrentCard);
	}
	return false;
}

//是否重跑
CT_BOOL CGameProcess::IsChongPao(CT_DWORD dwChairID)
{
	//效验玩家
	if (dwChairID >= GAME_PLAYER) return false;
	CT_BYTE cbCount = 0;
	for (CT_WORD i = 0; i < m_cbWeaveCount[dwChairID]; ++i)
	{
		if (m_WeaveArray[dwChairID][i].cbWeaveKind == ACK_PAO || m_WeaveArray[dwChairID][i].cbWeaveKind == ACK_TI)
		{
			++cbCount;
		}
	}
	return (cbCount >= 2) ? true : false;
}

//发送总结算
CT_VOID CGameProcess::SendTotalClearing(std::string strTime)
{
	FPF_CMD_S_CLEARING ClearingData;
	memset(&ClearingData, 0, sizeof(FPF_CMD_S_CLEARING));
	//当前时间
	_snprintf_info(ClearingData.szCurTime, sizeof(ClearingData.szCurTime), "%s", strTime.c_str());

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		ClearingData.wHuCount[i] = m_wHuCount[i];							//各玩家胡牌次数
		ClearingData.wFPCount[i] = m_wFPCount[i];								//各玩家放炮次数
		ClearingData.wHZCount[i] = m_wHZCount[i];							//各玩家荒庄次数
		ClearingData.iTotalHuXi[i] = m_iTotalHuXi[i];								//各玩家总胡息
		ClearingData.iTotalLWScore[i] = m_iTotalLWScore[i];				//各玩家总输赢分
	}

	//SendTableData(INVALID_CHAIR, FPF_SUB_S_CLEARING, &ClearingData, sizeof(FPF_CMD_S_CLEARING), false);
}

//计算总输赢分
CT_VOID CGameProcess::CalcTotalLWScore()
{
	//计算输赢
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//个位数四舍五入
		CT_INT32 iUnitsDigit = m_iTotalHuXi[i];
		if (iUnitsDigit >= 0)
		{
			iUnitsDigit += 5;
			iUnitsDigit -= iUnitsDigit % 10;
		}
		else
		{
			iUnitsDigit = abs(iUnitsDigit);
			iUnitsDigit += 5;
			iUnitsDigit -= iUnitsDigit % 10;

			//取反
			iUnitsDigit = ~iUnitsDigit;
			iUnitsDigit += 1;
		}
		m_iTotalHuXi[i] = iUnitsDigit;

		////胡息上限
		//CT_WORD wWinLimit = m_PrivateTableInfo.wWinLimit;
		//if (abs(m_iTotalHuXi[i]) > wWinLimit)
		//{//限制判断
		//	m_iTotalHuXi[i] = (m_iTotalHuXi[i] > 0) ? wWinLimit : (-1)*wWinLimit;
		//}
	}

	//计算总输赢
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		for (CT_WORD j = 0; j < GAME_PLAYER; ++j)
		{
			if (j == i) continue;
			CT_INT32 iTempScore = m_iTotalHuXi[i] - m_iTotalHuXi[j];
			m_iTotalLWScore[i] += iTempScore;
		}
	}
}

//获得能摆组合
CT_BYTE CGameProcess::GetBaiCardWeave(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbChiCount, CT_BYTE cbCurCard, CT_BYTE WeaveCard[15][MAX_ITEM_COUNT][3])
{
	CT_BYTE cbWeaveCount = 0;
	if (0 == cbChiCount)
	{
		return cbWeaveCount;
	}

	//记录临时组合
	CT_BYTE cbTempWeaveCount = 0;
	CT_BYTE cbTempWeaveCard[130][MAX_ITEM_COUNT][3];
	memset(cbTempWeaveCard, 0, sizeof(cbTempWeaveCard));

	//拷贝牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memset(cbTempCardIndex, 0, sizeof(cbTempCardIndex));
	memcpy(cbTempCardIndex, cbCardIndex, sizeof(cbTempCardIndex));

	///////////////////////三个组合////////////////////////////
	for (CT_WORD i = 0; i < cbChiCount - 2; ++i)
	{
		for (CT_WORD j = i + 1; j < cbChiCount; ++j)
		{
			for (CT_WORD k = j + 1; k < cbChiCount; ++k)
			{
				CT_BYTE cbCurCardIndex[MAX_INDEX] = { 0 };
				memcpy(cbCurCardIndex, &cbTempCardIndex[0], sizeof(cbCurCardIndex));
				cbCurCardIndex[m_GameLogic.SwitchToCardIndex(cbCurCard)]++;

				if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[i]))
				{//不能删完
					continue;
				}
				if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[j]))
				{//不能删完
					continue;
				}
				if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[k]))
				{//不能删完
					continue;
				}

				cbTempWeaveCard[cbTempWeaveCount][0][0] = cbChiWeave[i][0];
				cbTempWeaveCard[cbTempWeaveCount][0][1] = cbChiWeave[i][1];
				cbTempWeaveCard[cbTempWeaveCount][0][2] = cbChiWeave[i][2];

				cbTempWeaveCard[cbTempWeaveCount][1][0] = cbChiWeave[j][0];
				cbTempWeaveCard[cbTempWeaveCount][1][1] = cbChiWeave[j][1];
				cbTempWeaveCard[cbTempWeaveCount][1][2] = cbChiWeave[j][2];

				cbTempWeaveCard[cbTempWeaveCount][2][0] = cbChiWeave[k][0];
				cbTempWeaveCard[cbTempWeaveCount][2][1] = cbChiWeave[k][1];
				cbTempWeaveCard[cbTempWeaveCount][2][2] = cbChiWeave[k][2];
				++cbTempWeaveCount;
			}
		}
	}

	///////////////////////二个组合////////////////////////////

	for (CT_WORD i = 0; i < cbChiCount - 1; ++i)
	{
		for (CT_WORD j = i + 1; j < cbChiCount; ++j)
		{
			CT_BYTE cbCurCardIndex[MAX_COUNT] = { 0 };
			memcpy(cbCurCardIndex, &cbTempCardIndex[0], sizeof(cbCurCardIndex));
			cbCurCardIndex[m_GameLogic.SwitchToCardIndex(cbCurCard)]++;

			if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[i]))
			{//不能删完
				continue;
			}
			if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[j]))
			{//不能删完
				continue;
			}

			cbTempWeaveCard[cbTempWeaveCount][0][0] = cbChiWeave[i][0];
			cbTempWeaveCard[cbTempWeaveCount][0][1] = cbChiWeave[i][1];
			cbTempWeaveCard[cbTempWeaveCount][0][2] = cbChiWeave[i][2];

			cbTempWeaveCard[cbTempWeaveCount][1][0] = cbChiWeave[j][0];
			cbTempWeaveCard[cbTempWeaveCount][1][1] = cbChiWeave[j][1];
			cbTempWeaveCard[cbTempWeaveCount][1][2] = cbChiWeave[j][2];
			++cbTempWeaveCount;
		}
	}

	///////////////////////一个组合////////////////////////////

	for (CT_WORD i = 0; i < cbChiCount; ++i)
	{
		CT_BYTE cbCurCardIndex[MAX_COUNT] = { 0 };
		memcpy(cbCurCardIndex, &cbTempCardIndex[0], sizeof(cbCurCardIndex));
		cbCurCardIndex[m_GameLogic.SwitchToCardIndex(cbCurCard)]++;

		if (!RemoveDesCard(cbCurCardIndex, cbChiWeave[i]))
		{//不能删完
			continue;
		}

		cbTempWeaveCard[cbTempWeaveCount][0][0] = cbChiWeave[i][0];
		cbTempWeaveCard[cbTempWeaveCount][0][1] = cbChiWeave[i][1];
		cbTempWeaveCard[cbTempWeaveCount][0][2] = cbChiWeave[i][2];
		++cbTempWeaveCount;
	}

	//吃牌总张数
	CT_BYTE cbChiNum = cbCardIndex[m_GameLogic.SwitchToCardIndex(cbCurCard)] + 1;
	for (CT_WORD m = 0; m < cbTempWeaveCount; ++m)
	{
		//查出指定牌数量
		CT_BYTE cbNum = m_GameLogic.FindCardCount(cbCurCard, cbTempWeaveCard[m], 3);
		if (cbChiNum != cbNum)
		{
			continue;
		}

		WeaveCard[cbWeaveCount][0][0] = cbTempWeaveCard[m][0][0];
		WeaveCard[cbWeaveCount][0][1] = cbTempWeaveCard[m][0][1];
		WeaveCard[cbWeaveCount][0][2] = cbTempWeaveCard[m][0][2];

		WeaveCard[cbWeaveCount][1][0] = cbTempWeaveCard[m][1][0];
		WeaveCard[cbWeaveCount][1][1] = cbTempWeaveCard[m][1][1];
		WeaveCard[cbWeaveCount][1][2] = cbTempWeaveCard[m][1][2];

		WeaveCard[cbWeaveCount][2][0] = cbTempWeaveCard[m][2][0];
		WeaveCard[cbWeaveCount][2][1] = cbTempWeaveCard[m][2][1];
		WeaveCard[cbWeaveCount][2][2] = cbTempWeaveCard[m][2][2];
		++cbWeaveCount;
	}

	return cbWeaveCount;
}

//判断摆牌后能不能出牌
CT_BOOL CGameProcess::EstimateBaiCanOutCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurCard, CT_BYTE cbWeaveCard[100][MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount, \
	CT_BYTE cbSuccWeave[100][MAX_ITEM_COUNT][3], CT_BYTE& cbSuccCount)
{
	//拷贝牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memset(cbTempCardIndex, 0, sizeof(cbTempCardIndex));
	memcpy(cbTempCardIndex, cbCardIndex, sizeof(cbTempCardIndex));

	for (CT_WORD i = 0; i < MAX_INDEX; ++i)
	{//过滤坎
		if (cbTempCardIndex[i] == 3)
		{
			cbTempCardIndex[i] = 0;
		}
	}

	CT_BOOL bAllOut = true;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		CT_BYTE cbCurIndex[MAX_INDEX] = { 0 };
		memset(cbCurIndex, 0, sizeof(cbCurIndex));
		memcpy(cbCurIndex, cbTempCardIndex, sizeof(cbCurIndex));
		cbCurIndex[m_GameLogic.SwitchToCardIndex(cbCurCard)]++;

		//删除指定组合牌
		CT_BOOL bRtn = RemoveDesWeaveCard(cbCurIndex, cbWeaveCard[i]);
		CT_BYTE cbCount = m_GameLogic.GetCardCount(cbCurIndex);
		if (bRtn && cbCount > 0)
		{
			cbSuccWeave[cbSuccCount][0][0] = cbWeaveCard[i][0][0];
			cbSuccWeave[cbSuccCount][0][1] = cbWeaveCard[i][0][1];
			cbSuccWeave[cbSuccCount][0][2] = cbWeaveCard[i][0][2];

			cbSuccWeave[cbSuccCount][1][0] = cbWeaveCard[i][1][0];
			cbSuccWeave[cbSuccCount][1][1] = cbWeaveCard[i][1][1];
			cbSuccWeave[cbSuccCount][1][2] = cbWeaveCard[i][1][2];

			cbSuccWeave[cbSuccCount][2][0] = cbWeaveCard[i][2][0];
			cbSuccWeave[cbSuccCount][2][1] = cbWeaveCard[i][2][1];
			cbSuccWeave[cbSuccCount][2][2] = cbWeaveCard[i][2][2];
			++cbSuccCount;
		}
		else
		{
			bAllOut = false;
		}
	}

	return (bAllOut && (cbSuccCount > 0)) ? true : false;
}

//删除指定牌
CT_BOOL CGameProcess::RemoveDesCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[3])
{
	for (CT_WORD n = 0; n < 3; ++n)
	{
		if (!m_GameLogic.IsValidCard(cbChiWeave[n]))
		{
			continue;
		}
		CT_BYTE cbCurIndex = m_GameLogic.SwitchToCardIndex(cbChiWeave[n]);
		if (cbCardIndex[cbCurIndex] > 0)
		{
			cbCardIndex[cbCurIndex]--;
		}
		else
		{
			return false;
		}
	}
	return true;
}

//删除指定组合牌
CT_BOOL CGameProcess::RemoveDesWeaveCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3])
{
	bool bRtn = false;
	for (CT_WORD i = 0; i < 3; ++i)
	{//最多3个组合
		bRtn = RemoveDesCard(cbCardIndex, cbChiWeave[i]);
		if (!bRtn) break;
	}
	return bRtn;
}

//是否有相同组合
CT_BOOL CGameProcess::IsHaveSameWeave(CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCard[3])
{
	for (CT_WORD i = 0; i < 3; ++i)
	{
		if (!m_GameLogic.IsValidCard(cbChiWeave[i][0]) || !m_GameLogic.IsValidCard(cbWeaveCard[0]))
		{//无效牌
			continue;
		}
		if (m_GameLogic.IsSameWeave(cbChiWeave[i], cbWeaveCard))
		{
			return true;
		}
	}
	return false;
}

//是否有吃相同组合
CT_BOOL CGameProcess::IsHaveChiSameSevae(CT_BYTE cbChiWeaveCard[100][MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount, CT_BYTE cbWeaveCard[3])
{
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		if (IsHaveSameWeave(cbChiWeaveCard[i], cbWeaveCard))
		{
			return true;
		}
	}
	return false;
}

//偎后能否出牌
CT_BOOL CGameProcess::IsWeiCanOutCard(CT_DWORD dwChairID)
{
	//临时牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));
	for (CT_WORD i = 0; i < MAX_INDEX; ++i)
	{//清坎
		if (cbTempCardIndex[i] == 3)
		{
			cbTempCardIndex[i] = 0;
		}
	}

	CT_BYTE cbCardCount = m_GameLogic.GetCardCount(cbTempCardIndex);
	return (cbCardCount > 0) ? true : false;
}

CT_BOOL CGameProcess::IsTiPaoCanOutCard(CT_DWORD dwChairID)
{
	//临时牌
	CT_BYTE cbTempCardIndex[MAX_INDEX] = { 0 };
	memcpy(cbTempCardIndex, &m_cbCardIndex[dwChairID], sizeof(cbTempCardIndex));
	CT_BYTE cbCardCount = 0;
	for (CT_WORD i = 0; i < MAX_INDEX; ++i)
	{//清坎
		if (cbTempCardIndex[i] == 3)
		{
			cbTempCardIndex[i] = 0;
		}
		else if (cbTempCardIndex[i] != 0)
		{
			cbCardCount += cbTempCardIndex[i];
		}
	}
	return (cbCardCount > 0) ? true : false;
}

//判断听牌
CT_VOID CGameProcess::EstimateTingCard(CT_DWORD dwChairID)
{
	FPF_CMD_S_TingCard TingCard;
	memset(&TingCard, 0, sizeof(TingCard));
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		//庄家胡牌判断
		tagHuCardInfo HuCardInfo;
		memset(&HuCardInfo, 0, sizeof(HuCardInfo));

		CT_BYTE cbCurCard = m_GameLogic.SwitchToCardData(i);
		if (GetHuCardInfo(dwChairID, cbCurCard, HuCardInfo,true))
		{
			//记录当前牌
			TingCard.cbCardList[TingCard.cbCardCount++] = cbCurCard;
		}
	}

	/*if (0 == TingCard.cbCardCount)
	{//没有可听的牌
		return;
	}*/
	//下发听牌数据
	SendTableData(dwChairID, FPF_SUB_S_TING_CARD, &TingCard, sizeof(FPF_CMD_S_TingCard), false);
}

//测试牌
CT_VOID CGameProcess::TestCard(CT_BYTE cbCardData[GAME_PLAYER][MAX_COUNT])
{
	return;

	m_dwBankerUser = 1;

	CT_BYTE cbTempCard0[MAX_COUNT] = { 0x02, 0x02, 0x03, 0x04, 0x04, 0x05, 0x06, 0x06, 0x08, 0x08, 0x09, 0x0A, 0x12, 0x13, 0x13, 0x14, 0x15, 0x15, 0x018, 0x18 };
	//CT_BYTE cbTempCard1[MAX_COUNT] = { 0x01, 0x03, 0x07, 0x07, 0x09, 0x0A, 0x11, 0x11, 0x11, 0x12, 0x13, 0x14, 0x16,0x16, 0x17, 0x17, 0x19, 0x19, 0x1A, 0x01A, 0x1A };
	CT_BYTE cbTempCard1[MAX_COUNT] = { 0x01, 0x04, 0x07, 0x07, 0x09, 0x0A, 0x11, 0x11, 0x11, 0x12, 0x13, 0x14, 0x16,0x16, 0x17, 0x17, 0x19, 0x19, 0x1A, 0x01A, 0x1A };
	//CT_BYTE cbTempCard2[MAX_COUNT] = { 0x01, 0x03, 0x03, 0x04, 0x05, 0x05, 0x06, 0x06, 0x09, 0x12, 0x12, 0x14, 0x14, 0x15, 0x15, 0x16, 0x16, 0x17, 0x018, 0x19 };
	CT_BYTE cbTempCard2[MAX_COUNT] = { 0x03, 0x03};

	m_cbWeaveCount[2] = 6;		//组合数目
	m_cbWeaveCount[1] = 1;		//组合数目
	//组合0
	m_WeaveArray[1][0].cbWeaveKind = ACK_PENG;
	m_WeaveArray[1][0].cbWeaveHuXi = 1;
	m_WeaveArray[1][0].cbCardCount = 3;
	m_WeaveArray[1][0].cbCenterCard = 0x03;
	m_WeaveArray[1][0].cbCardList[0] = 0x03;
	m_WeaveArray[1][0].cbCardList[1] = 0x03;
	m_WeaveArray[1][0].cbCardList[2] = 0x03;


	//组合0
	m_WeaveArray[2][0].cbWeaveKind = ACK_CHI;
	m_WeaveArray[2][0].cbWeaveHuXi = 15;
	m_WeaveArray[2][0].cbCardCount = 3;
	m_WeaveArray[2][0].cbCenterCard = 0x02;
	m_WeaveArray[2][0].cbCardList[0] = 0x02;
	m_WeaveArray[2][0].cbCardList[1] = 0x07;
	m_WeaveArray[2][0].cbCardList[2] = 0x0A;
	//组合1
	m_WeaveArray[2][1].cbWeaveKind = ACK_PENG;
	m_WeaveArray[2][1].cbWeaveHuXi = 3;
	m_WeaveArray[2][1].cbCardCount = 3;
	m_WeaveArray[2][1].cbCenterCard = 0x03;
	m_WeaveArray[2][1].cbCardList[0] = 0x03;
	m_WeaveArray[2][1].cbCardList[1] = 0x03;
	m_WeaveArray[2][1].cbCardList[2] = 0x03;
	//组合2
	m_WeaveArray[2][2].cbWeaveKind = ACK_WEI;
	m_WeaveArray[2][2].cbWeaveHuXi = 6;
	m_WeaveArray[2][2].cbCardCount = 3;
	m_WeaveArray[2][2].cbCenterCard = 0x11;
	m_WeaveArray[2][2].cbCardList[0] = 0x11;
	m_WeaveArray[2][2].cbCardList[1] = 0x11;
	m_WeaveArray[2][2].cbCardList[2] = 0x11;
	//组合3
	m_WeaveArray[2][3].cbWeaveKind = ACK_CHI;
	m_WeaveArray[2][3].cbWeaveHuXi = 6;
	m_WeaveArray[2][3].cbCardCount = 3;
	m_WeaveArray[2][3].cbCenterCard = 0x12;
	m_WeaveArray[2][3].cbCardList[0] = 0x12;
	m_WeaveArray[2][3].cbCardList[1] = 0x17;
	m_WeaveArray[2][3].cbCardList[2] = 0x1A;
	//组合4
	m_WeaveArray[2][4].cbWeaveKind = ACK_CHI;
	m_WeaveArray[2][4].cbWeaveHuXi = 0;
	m_WeaveArray[2][4].cbCardCount = 3;
	m_WeaveArray[2][4].cbCenterCard = 0x17;
	m_WeaveArray[2][4].cbCardList[0] = 0x15;
	m_WeaveArray[2][4].cbCardList[1] = 0x17;
	m_WeaveArray[2][4].cbCardList[2] = 0x16;
	//组合5
	m_WeaveArray[2][5].cbWeaveKind = ACK_CHI;
	m_WeaveArray[2][5].cbWeaveHuXi = 0;
	m_WeaveArray[2][5].cbCardCount = 3;
	m_WeaveArray[2][5].cbCenterCard = 0x18;
	m_WeaveArray[2][5].cbCardList[0] = 0x08;
	m_WeaveArray[2][5].cbCardList[1] = 0x18;
	m_WeaveArray[2][5].cbCardList[2] = 0x18;

	////组合6
	//m_WeaveArray[2][0].cbWeaveKind = ACK_PENG;
	//m_WeaveArray[2][0].cbWeaveHuXi = 3;
	//m_WeaveArray[2][0].cbCardCount = 3;
	//m_WeaveArray[2][0].cbCenterCard = 0x15;
	//m_WeaveArray[2][0].cbCardList[0] = 0x15;
	//m_WeaveArray[2][0].cbCardList[1] = 0x15;
	//m_WeaveArray[2][0].cbCardList[2] = 0x15;


	memcpy(&cbCardData[0], cbTempCard0, sizeof(CT_BYTE)*MAX_COUNT);
	memcpy(&cbCardData[1], cbTempCard1, sizeof(CT_BYTE)*MAX_COUNT);
	memcpy(&cbCardData[2], cbTempCard2, sizeof(CT_BYTE)*MAX_COUNT);

	m_GameLogic.SwitchToCardIndex(cbTempCard0, MAX_COUNT - 1, m_cbCardIndex[0]);
	m_GameLogic.SwitchToCardIndex(cbTempCard1, MAX_COUNT, m_cbCardIndex[1]);
	m_GameLogic.SwitchToCardIndex(cbTempCard2, MAX_COUNT - 1, m_cbCardIndex[2]);
}





