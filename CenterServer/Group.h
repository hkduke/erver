#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <unordered_map>
#include <set>
#include <memory>
#include "CTType.h"
#include "GlobalStruct.h"
#include "CMD_Inner.h"

struct stPRoomClearingInfo
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_BYTE			cbCount;
	CT_DWORD		dwRoomNum;
	CT_INT32		iScore[MAX_PLAYER];
	CT_CHAR			szNickName[MAX_PLAYER][NICKNAME_LEN];
	CT_CHAR			szTime[TIME_LEN];
};

using MapGroupUser = std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>;
using MapGroupApplyUser = std::map<CT_DWORD, std::unique_ptr<tagApplyAddGroupInfo>>;
using MapGroupLinkUser = std::map<CT_DWORD, std::vector<tagGroupUserInfo*>>;
using MapGroupChat = std::map<CT_DWORD, std::vector<tagGroupChat>>;

//税收
using MapGroupUserRevenue = std::unordered_map<CT_DWORD, CT_DWORD>;                   //格式userid, day税收
using MapGroupUserDateRevenue = std::unordered_map<CT_DWORD, MapGroupUserRevenue>;    //格式20190426,MapGroupUserRevenue

//每天收入
using MapGroupUserIncome = std::map<CT_DWORD, std::unique_ptr<tagGroupUserDateIncome>>; //DirUserId, tagGroupUserDateIncome
using MapGroupUserDateIncome = std::map<CT_DWORD, MapGroupUserIncome>;                  //格式20190430, MapGroupUserIncome
using MapGroupAllUserDateIncome = std::map<CT_DWORD, MapGroupUserDateIncome>;           //格式parentuserId,MapGroupUserIncome

//总收入
using MapGroupUserTotalIncome = std::map<CT_DWORD, CT_LONGLONG>;                      //格式userid, taotalincome
using MapGroupParentUserTotalIncome = std::map<CT_DWORD, MapGroupUserTotalIncome>;    //格式ParentUserid, MapGroupUserTotalIncome

//私人房间
using VecGroupPRoom = std::vector<tagGroupPRoomInfo>;
using MapGroupPRoom = std::map<CT_DWORD, std::vector<tagGroupPRoomInfo>>;              //格式gameid*10000+kindid*100, VecGroupPRoom

//打开UI的玩家
using MapGroupOpenUIUser = std::map<CT_DWORD, CT_DWORD>;                               //格式userid, gameid*10000+kindid*100

class CGroup
{
public:
	CGroup();
	~CGroup();

	CT_VOID		InsertGroupInfo(const tagGroupInfo* pGroupInfo);
	CT_VOID		InsertGroupUser(const tagGroupUserInfo* pGroupUser);
	CT_VOID     InsertGroupChat(const tagGroupChat* pGroupChat);
	CT_VOID		InsertPRoomCfg(const tagGroupRoomConfig* pGroupCfg);
	CT_VOID		RemoveGroupUser(CT_DWORD dwUserID);
	CT_VOID		InsertApplyGroupInfo(const tagApplyAddGroupInfo* pApplyInfo);
	//CT_VOID		InsertPRoomClearingInfo(MSG_G2CS_GroupPRoom_Clearing* pClearing);
	CT_VOID 	AddUserRoomCount();

	CT_DWORD	GetGroupMasterID();
	CT_VOID		AddGroupUser(tagGroupUserInfo* pGroupUser);
	CT_VOID		OperateApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwApplyUserID, CT_DWORD dwMasterID, CT_BYTE cbOperateCode);
	CT_VOID		QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID		TickOutGroupUser(CT_DWORD dwGroupID, CT_DWORD dwMasterID, CT_DWORD dwUserID);
	CT_VOID     QueryGroupUserInfo(CT_DWORD dwUserID);

	CT_BOOL		IsGroupUser(CT_DWORD dwUserID);
	CT_BOOL		IsApplyAddGroup(CT_DWORD dwUserID);
	CT_VOID		RemoveGroupUserFromDB(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_DWORD	GetGroupUserCount();

	CT_VOID     ModifyMemberIncomeRate(CT_DWORD dwUserID, CT_BYTE cbIncomeRate);

	CT_BYTE     GetMemberPosition(CT_DWORD dwUserID);
	inline      CT_VOID SendMsgToAdministrator(CT_DWORD dwSubID, CT_VOID* pData, CT_DWORD dwDataSize);
	CT_VOID     UpdateIncomeTodb(CT_DWORD dwTodayInt, bool bCrossDays);

    tagGroupUserInfo* GetGroupUserInfo(CT_DWORD dwUserID);
    std::unique_ptr<tagGroupRoomConfig>* GetGroupRoomCfg(CT_WORD wGameID, CT_WORD wKindID);

	std::unique_ptr<tagGroupInfo>& GetGroupInfo() { return m_GroupInfo; }
    MapGroupUser& GetAllGroupUserInfo()     { return m_mapGroupUser; }
    //MapGroupApplyUser& GetAllApplyInfo()	{ return m_mapApplyUser; }
    MapGroupLinkUser& GetDirGroupUser()     { return m_mapDirGroupUser; }
    MapGroupLinkUser& GetSubGroupUser()     { return m_mapSubGroupUser; }
    MapGroupChat&     GetGroupChat()        { return m_mapGroupChat;}
    MapGroupUserDateRevenue&    GetGroupUserDateRevenue()      { return m_mapDateRevenue; }
    MapGroupAllUserDateIncome&  GetGroupUserDateDirIncome()    { return m_mapDateDirIncome; }
    MapGroupAllUserDateIncome&  GetGroupUserDateSubIncome()    { return m_mapDateSubIncome; }
    MapGroupParentUserTotalIncome& GetParentUserTotalIncome()  { return m_mapTotalIncome; }
    MapGroupPRoom&                 GetGroupPRoom()             { return m_mapPRoom; }
    MapGroupOpenUIUser&            GetGroupOpenUIUserID()      { return m_mapOpenGameUI; }
	//std::list<std::unique_ptr<stPRoomClearingInfo>>& GetClearingInfo()				{ return m_listPRoomClearing; }

private:
	std::unique_ptr<tagGroupInfo> 	m_GroupInfo;
    MapGroupUser		            m_mapGroupUser;
    //MapGroupApplyUser	            m_mapApplyUser;
    MapGroupLinkUser                m_mapDirGroupUser;  //直属成员
    MapGroupLinkUser                m_mapSubGroupUser;  //附属成员
    MapGroupChat                    m_mapGroupChat;     //聊天记录

    MapGroupUserDateRevenue         m_mapDateRevenue;   //按天保存的玩家税收
    MapGroupAllUserDateIncome       m_mapDateDirIncome; //直属成员的收入
    MapGroupAllUserDateIncome       m_mapDateSubIncome; //附属成员的收入

    MapGroupParentUserTotalIncome   m_mapTotalIncome;   //成员附属各级总收入

    MapGroupPRoom                   m_mapPRoom;         //开房间信息
    MapGroupOpenUIUser              m_mapOpenGameUI;    //打开UI的用户


	//std::vector<std::unique_ptr<tagGroupRoomConfig>>			m_vecPRoomCfg;
	//std::list<std::unique_ptr<stPRoomClearingInfo>>				m_listPRoomClearing;

	//std::map<CT_DWORD, std::vector<std::unique_ptr<tagGroupUserGameData>>> m_mapUserGameData;
};