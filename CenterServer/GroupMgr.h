#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include <memory>
#include "CTType.h"
#include "GlobalStruct.h"
#include "Group.h"
#include "CMD_Plaza.h"
#include "CMD_Dip.h"

#define MAX_GROUP_COUNT		 1
#define MAX_GROUP_USER_COUNT 300
#define CAN_CREATE_GROUP_VIP 6
#define PAGE_ITEM_COUNT      20 //每页显示条目数量

//玩家与群组的绑定关系
typedef std::map<CT_DWORD, CT_DWORD> MapUserGroup;

//typedef std::vector<std::shared_ptr<tagDefaultRoomConfig>> VecDefaultPRoomCfg;
typedef std::map<CT_DWORD, std::unique_ptr<CGroup>> MapGroup;
//typedef std::map<CT_DWORD, std::vector<std::shared_ptr<tagGroupPRoomInfo>>> MapGroupPRoomNum;
using MapGroupPRoomNum = std::map<CT_DWORD, CT_DWORD>;          //格式roomNum, groupId
using MapGroupLevelCfg = std::map<CT_BYTE, tagGroupLevelCfg>;

enum en_GroupPosition
{
    en_Master = 1,
    en_Deputy = 2,
    en_Member = 3,
};

class CGroupMgr : public acl::singleton<CGroupMgr>
{
public:
	CGroupMgr();
	~CGroupMgr();

public:
	CT_VOID		SetRedisPtr(acl::redis* pRedis);
	CT_VOID		InsertGroupInfo(tagGroupInfo* pGroupInfo);
	CT_VOID		InsertGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
	CT_VOID     InsertGroupUserTodayRevenue(CT_DWORD dwGroupID, tagGroupUserRevenue* pGroupUserRevenue);
	CT_VOID     InsertGroupUserDateDirIncome(CT_DWORD dwGroupID, tagGroupUserDateIncomeDetail* pGroupUserDateIncomeDetail);
    CT_VOID     InsertGroupUserDateSubIncome(CT_DWORD dwGroupID, tagGroupUserDateIncomeDetail* pGroupUserDateIncomeDetail);
    CT_VOID     InsertGroupUserTotalIncome(CT_DWORD dwGroupID, tagGroupUserTotalIncome* pGroupUserDateIncomeDetail);
	CT_VOID     InsertGroupChatMsg(CT_DWORD dwGroupID, tagGroupChat* pGroupChat);
	CT_VOID		RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID		InsertUserGroup(CT_DWORD dwUserID, CT_DWORD dwGroupID);
	CT_BOOL 	IsUserHasGroup(CT_DWORD dwUserID);
	CT_DWORD    GetUserGroupID(CT_DWORD dwUserID);
	CT_VOID		RemoveUserGroup(CT_DWORD dwUserID, CT_DWORD dwGroupID);
	CT_VOID		AddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
	CT_VOID		InsertApplyGroupInfo(CT_DWORD dwGroupID, CT_DWORD dwApplyUser, tagGlobalUserInfo* pUserInfo);
	//CT_VOID	InsertApplyGroupInfo(CT_DWORD dwGroupID, tagApplyAddGroupInfo* pApplyInfo);
	CT_DWORD	GetGroupMasterID(CT_DWORD dwGroupID);
	CT_VOID		OperateApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwApplyUserID, CT_DWORD dwMasterID, CT_BYTE cbOperateCode);
	CT_VOID		QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID		TickOutGroupUser(CT_DWORD dwGroupID, CT_DWORD dwMasterID, CT_DWORD dwUserID);
	CT_VOID		RemoveGroup(CT_DWORD dwGroupID, CT_DWORD dwMasterID);
	CT_BOOL		IsGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_BOOL		IsApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_BOOL		HasGroup(CT_DWORD dwGroupID);
	CT_DWORD	GetGroupUserCount(CT_DWORD dwGroupID);

public:
	CT_VOID		ModifyGroupInfo(MSG_CS_Modify_GroupInfo* pGroupInfo);
	CT_VOID     ModifyGroupName(MSG_CS_Modify_GroupName* pGroupName);
	CT_VOID     ModifyGroupNotice(MSG_CS_Modify_GroupNotice* pGroupNotice);
	CT_VOID     ModifyGroupIcon(MSG_CS_Modify_GroupIcon* pGroupIcon);
    CT_VOID     UpgradeGroup(MSG_CS_Upgrade_Group* pUpgradeGroup);
    CT_VOID     ModifyGroupUserRemarks(MSG_CS_DirGroupUser_Remarks* pGroupUserRemarks);
	CT_VOID     UpgradeGroupUserIncomeRate(MSG_CS_Upgrade_GroupUser_IncomeRate* pGroupUserIncomeRate);
	CT_DWORD	CalUserGroupCount(CT_DWORD dwUserID);
	CT_VOID		ModifyGroupOptions(CT_DWORD dwGroupID, CT_WORD wBindUserPlay, CT_DWORD dwDefaultKindID); //由web页面设置
	CT_VOID     ChangeGroupMemberPosition(MSG_CS_Change_GroupPosition* pChangeGroupPosition);
	CT_VOID     SetGroupUserOnline(CT_DWORD dwGroupID, CT_DWORD dwUserID, CT_BOOL bOnline);
	CT_VOID     QueryDirGroupUserList(MSG_CS_Query_DirGroupUser_List* pQueryDirGroupUser);
    CT_VOID     QueryDirGroupUserInfo(MSG_CS_Query_DirGroupUser_Info* pQueryDirGroupUserInfo);
    CT_VOID     QueryOneDirGroupUser(const CT_VOID * pData, CT_DWORD dwDataSize);
    CT_VOID     GiftScoreToDirGroupUser(const CT_VOID * pData, CT_DWORD dwDataSize);
    CT_VOID     QuerySubGroupUserList(MSG_CS_Query_SubGroupUser_List* pQuerySubGroupUser);
    CT_VOID     QueryGroupUserIncome(MSG_CS_Query_Group_Income* pQueryGroupUserIncome);
    inline      CT_LONGLONG FindDateIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwDirUserID, CT_DWORD dwSubUserID, CT_DWORD dwDate);
    inline      CT_LONGLONG FindDateTotalIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwDate1, CT_DWORD dwDate2);
    inline      CT_LONGLONG FindTotalIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwSubUserID);
    CT_VOID     QueryGroupUserYesterdayIncome(MSG_CS_Query_Group_Yesterday_Income* pQueryGroupUserYesterdayIncome);
    CT_VOID     QueryGroupUserIncomeDetail(MSG_CS_Query_Group_Income_Detail* pQueryGroupUserIncomeDetail);
    CT_VOID     QueryGroupUserSettleRecord(MSG_CS_Query_Group_Settle_Record* pQuerySettle);
    CT_VOID     QueryGroupUserSettleRecord(CT_DWORD dwUserID, MSG_SC_Query_Group_Settle_Record* pSettleResult, CT_DWORD dwDataSize);
    CT_VOID     SendGroupChatMsg(MSG_CS_Send_GroupChatMsg* pGroupChatMsg);
    CT_VOID     UserReadMsg(const CT_VOID * pData, CT_DWORD dwDataSize);
    CT_VOID     QuerySettleInfo(MSG_CS_Query_Group_Settle_Info* pGroupSettleInfo);
    CT_VOID     SettleIncome(MSG_CS_Settle_Income* pSettleIncome);
    CT_VOID     CreatePRoom(MSG_CS_Create_Group_PRoom * pCreatePRoom);
    CT_VOID     OpenGameUI(MSG_CS_Open_Group_Game * pOpenGroupGameUI);
    CT_VOID     CloseGameUI(CT_DWORD dwUserID);

    CT_VOID     UpdateGroupContribution(CT_DWORD dwGroupID, CT_DWORD dwAddContribution);
    CT_VOID     UpdateGroupPRoomUser(MSG_G2CS_GroupPRoom_Update* pRoomUpdate);
    CT_VOID     RemoveGroupPRoom(MSG_G2CS_Remove_GroupPRoom* pRemovePRoom);
    CT_VOID     UpdateGroupInfoForDip(const CT_VOID * pData, CT_DWORD dwDataSize);
    CT_VOID     UpdateGroupUserInfoForDip(const CT_VOID * pData, CT_DWORD dwDataSize);
    CT_VOID     UpdateGroupLevelConfigForDip(const CT_VOID * pData, CT_DWORD dwDataSize);

	CT_VOID     InsertGroupLevelConfig(tagGroupLevelCfg* pGroupLevel);
	CT_VOID		InsertGroupPRoomClearing(MSG_G2CS_GroupPRoom_Clearing* pClearing);
    //CT_VOID		InsertDefaultPRoomConfig(tagDefaultRoomConfig* pDefaultConfig);
    //CT_VOID		InsertGroupPRoomConfig(tagGroupRoomConfig* pRoomConfig);

	//std::unique_ptr<tagGroupRoomConfig>* GetGroupRoomCfg(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID);
	//std::shared_ptr<tagDefaultRoomConfig>* GetDefaultRoomCfg(CT_WORD wGameID, CT_WORD wKindID);

	std::unique_ptr<CGroup>* GetGroup(CT_DWORD dwGroupID);

	CT_DWORD    GetLevelMaxContribution(CT_BYTE cbLevel);
	tagGroupLevelCfg* GetLevelCfg(CT_BYTE cbLevel);
	CT_WORD     GetBaseIncomeRate();
	CT_BYTE     GetBaseSettleDays();
	CT_VOID     SetChatMsgId(CT_DWORD dwMsgId) { m_dwChatMsgId = dwMsgId; }

	CT_VOID		SendGroupToGameServer(acl::aio_socket_stream* pSocket, CMD_GameServer_Info* pGameServerInfo);
	CT_VOID		UpdateGroupInfoToGameServer(acl::aio_socket_stream* pSocket, CT_DWORD dwGroupID);
	CT_VOID		SendAllGroupInfoToClient(CT_DWORD dwUserID);
	CT_VOID		SendGroupInfoToClient(CT_DWORD dwUserID);
	CT_VOID		SendOneGroupInfoToClient(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID		SendAllGameServerAddGroup(tagGroupInfo* pGroupInfo);
	CT_VOID		SendAllGameSserverAddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
	CT_VOID		SendAllGameServerRemoveGroup(CT_DWORD dwGroupID);
	CT_VOID		SendAllGameServerRemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID     SendAllGameServerAddPRoom(CT_DWORD dwGroupID, tagGroupPRoomInfo* pGroupPRoomInfo);
    CT_VOID     SendAllGameServerRemovePRoom(tagGroupPRoomInfo* pGroupPRoomInfo);
    CT_VOID     SendAllProxyServerAddPRoom(tagGroupPRoomInfo* pGroupPRoomInfo);
	CT_VOID     SendAllPRoomToProxyServer(acl::aio_socket_stream* pSocket);
	CT_VOID     SendAllProxyServerRemovePRoom(CT_DWORD dwRoomNum);

	//房间相关
	CT_VOID		AddPRoomNum(CT_DWORD dwGroupID, std::shared_ptr<tagGroupPRoomInfo> pRoomInfo);
	CT_VOID		RemovePRoomNum(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum);
	CT_BOOL		FindPRoomInfo(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID,  CT_DWORD& dwRoomNum, CT_WORD& userCount);

	//收入相关
    CT_VOID     CollectUserGameData(MSG_G2CS_PlayGame* pPlayGame);
    CT_VOID     CalcGroupUserIncome(bool bCrossDays);
    CT_VOID     CheckRemoveEmptyPRoom();
    CT_VOID     OnUserRecharge(CT_DWORD dwUserID, CT_DWORD dwGroupID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash, CT_BYTE cbVip2);
    inline      CT_DWORD CalcDayInt(int nBeforeDays);

    CT_VOID		SendGroupErrorMsg(CT_DWORD dwUserID, CT_BYTE cbErrorCode);

public:
    inline      CT_VOID UpdateGroupInfoToDB(std::unique_ptr<tagGroupInfo>& pGroupInfo);
    inline      CT_VOID UpdateGroupUserInfoToDB(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
    inline      CT_VOID InsertGroupChatToDB(MSG_CS2DB_Insert_GroupChat* pGroupChat);
    inline      CT_VOID UpdateGroupChatStatusToDB(MSG_CS2DB_Update_GroupChat* pGroupChat);

private:
	MapGroup				m_mapGroup;
	MapUserGroup			m_mapUserGroup;

	MapGroupLevelCfg		m_mapGroupLevelCfg;
	CT_DWORD                m_dwMaxGroupContribution;       //公会最大等级的
	//VecDefaultPRoomCfg		m_vecRoomDefaultCfg;
	MapGroupPRoomNum		m_mapRoomNum;

	CT_DWORD                m_dwChatMsgId;

	acl::redis*				m_pRedis;
}; 