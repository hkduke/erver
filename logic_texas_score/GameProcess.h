#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_PDK.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>

//��Ϸ����
class CGameProcess : public ITableFrameSink
{
public:
	CGameProcess(void);
	~CGameProcess(void);
    
public:
	//��Ϸ��ʼ
	virtual void	OnEventGameStart();
	//��Ϸ����
	virtual void OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag);
	//���ͳ���
	virtual void OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
    
	//�¼��ӿ�
public:
	//��ʱ���¼�
	virtual void	OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//��Ϸ��Ϣ
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);

	//�û��ӿ�
public:
	//�û�����
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//�û�׼��
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//�û��뿪
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0);
	//�û�����
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//�û�����
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//�û�����
	virtual void OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce) {}
	//�û���ֵ llRechargeScore��ֵ��� ��λ: ��
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) {}

public:
	//����ָ��
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//������Ϸ����
	virtual void ClearGameData();
	//˽�˷���Ϸ����Ƿ����
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//��ȡ��ֽ�����ܷ�
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);
	//�������ӿ��
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {}
	//�������ӿ������
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) {}
	//���û����˿��
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {}
	//���û����˿������
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) {}
	//����ϵͳͨɱ����
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) {}
	//���ý��տ��ֵ
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) {}
	//���ý��տ������
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) {}
	//���û��Ƹ���
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) {}
	//���û��Ƹ���
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio) {}
    //������Ϸ������
    virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList) {}
	//���û����������
	virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) {}
	//���ò���������
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) { return true; }
	//����ը�𻨵��ƿ����
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio) {}
    //�����ȡ��������ҵ���Ӯֵ
    virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) {}
    //����Ѫ��ϵͳ����(��)
    virtual CT_VOID SetStockControlInfo(tagStockControlInfo* pStockControlInfo) {}
	//��ȡ���ӵ�����ͳ����Ϣ
	virtual CT_VOID GetAllPlayerFishStatisticInfo() {}
    
private:
	//��������
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false);


    //���ܺ���
private:
    //��ʼ����Ϸ����
    void InitGameData();
	//������Ϸ����
	void UpdateGameConfig();
	//������ж�ʱ��
	void ClearAllTimer();
    //��ͨ����Ϸ����
    CT_VOID NormalGameEnd(CT_DWORD dwWinChairID);
	//�����ܽ���
	void SendTotalClearing(std::string strTime);

	//����˽�˳�����
	void ParsePrivateData(CT_CHAR *pBuf);

    //��Ϸ�¼�
private:
	//��ҳ���
	CT_BOOL OnUserOutCard(CT_WORD wChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout = CT_FALSE);
	//��Ҳ���
	CT_BOOL OnUserPassCard(CT_WORD wChairID);
	//�����һ�����
	CT_DWORD GetNextUser(CT_DWORD dwChairID);
protected:
	//�й�
	CT_BOOL IsTrustee(CT_DWORD dwTime = TIME_OUT_CARD);
	//�����й�
	CT_VOID SetTrustee(CT_DWORD dwChairID, CT_BYTE cbTrustee);
	//�Զ��е���
	CT_BOOL OnAutoCallLand();
	//�Զ��ӱ�
	CT_BOOL OnAutoDouble(CT_DWORD dwChairID);
	//�Զ�����
	CT_BOOL OnAutoOutCard();

private:
	CT_DWORD						m_dwSysTime;						//ϵͳʱ��
	CT_BYTE							m_cbGameStatus;						//��Ϸ״̬
    //��Ϸ��Ϣ
protected:
	CT_DWORD						m_dwBankerUser;						//��3�û�
	CT_DWORD						m_dwCurrentUser;					//��ǰ���
	CT_BYTE							m_cbOutCardCount[GAME_PLAYER];		//���ƴ���
	CT_BYTE							m_cbWaitTime;						//�ȴ�ʱ��
	CT_BYTE							m_cbTrustee[GAME_PLAYER];			//�Ƿ��й�
	CT_BYTE							m_cbTimeOutCount[GAME_PLAYER];		//��ʱ����
	CT_BOOL 						m_bIsPassCard;						//�Ƿ����

	//������Ϣ
protected:
	CT_DWORD						m_dwBombChairID;						//ը����λ��
	CT_BYTE							m_cbEachBombCount[GAME_PLAYER];		//ը������
    CT_DOUBLE						m_dEachBombScore[GAME_PLAYER];		//ը������
	CT_DOUBLE						m_dEachBombRevenue[GAME_PLAYER];	//ը��˰��
	//CT_DWORD						m_dwChunTianbTime[GAME_PLAYER];					//���챶��

	//������Ϣ
protected:
	CT_DWORD						m_dwTurnWiner;						//ʤ�����
	CT_BYTE							m_cbTurnCardCount;					//������Ŀ
	CT_BYTE							m_cbTurnCardData[MAX_COUNT];		//��������

	CT_BYTE							m_cbOutCardListCount[GAME_PLAYER][MAX_COUNT];			//ÿ�����ÿ�γ��Ƶ�
	CT_BYTE							m_cbOutCardList[GAME_PLAYER][MAX_COUNT][MAX_COUNT];		//ÿ����ҵĳ����б�
	//CT_BYTE							m_cbRemainCard[MAX_CARD_VALUE];//��ǰʣ��������

	//�˿���Ϣ
protected:
	CT_BYTE							m_cbHandCardCount[GAME_PLAYER];		//�˿���Ŀ
	CT_BYTE							m_cbHandCardData[GAME_PLAYER][MAX_COUNT];	//�����˿�


	//�������
protected:
	PDK_CMD_S_GameEnd				m_GameEnd;							//��Ϸ����

//��������
private:
	//ʱ����Ϣ
	CT_BYTE							m_cbOpTotalTime;					//����ʱ��
	CT_DWORD						m_dwOpStartTime;					//����ʱ���

private:
	//������Ϣ
	//CT_DWORD						m_dwGameType;						//��Ϸ����.0:��Ϸ��;1:������.
	CT_DWORD						m_dwlCellScore;						//��������

private:
	ITableFrame*					m_pGameDeskPtr;						//��Ϸָ��
	CGameLogic						m_GameLogic;						//��Ϸ�߼�

	tagGameRoomKind*				m_pGameRoomKindInfo;				//��������
	PrivateTableInfo				m_PrivateTableInfo;					//��ȡ˽�˷���Ϣ
	RecordScoreInfo					m_RecordScoreInfo[GAME_PLAYER];		//��¼����
	//CT_INT32						m_iTotalLWScore[GAME_PLAYER];		//���������Ӯ��
	//CT_BYTE							m_cbTotalQuanGuan[GAME_PLAYER];		//�����ȫ�ش���
	//CT_BYTE							m_cbTotalBeiGuan[GAME_PLAYER];		//����ұ��ش���
	//CT_BYTE							m_cbTotalRomb[GAME_PLAYER];			//�����ը������
	//CT_BYTE							m_cbTotalTongPei[GAME_PLAYER];		//�����ͨ�����
	//CT_WORD						    m_wPlayWinCount[GAME_PLAYER];							//Ӯ����
	//CT_WORD						    m_wPlayLoseCount[GAME_PLAYER];							//�����
};


#endif	//___DEF_GAME_PROCESS_H___
