#ifndef		___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_PDK_16.h"
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
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID* pDataBuffer, CT_DWORD dwDataSize);

	//�û��ӿ�
public:
	//�û�����
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//�û�׼��
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//�û��뿪
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0) {};
	//�û�����
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//�û�����
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};

public:
	//����ָ��
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//������Ϸ����
	virtual void ClearGameData();
	//˽�˷���Ϸ����Ƿ����
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//��ȡ��ֽ�����ܷ�
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);

private:
	//��������
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = true);


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
	//�Ƿ������3
	CT_BYTE IsMustOutThree(CT_DWORD dwChairID);

	//��Ϸ�¼�
private:
	//��ҳ���
	CT_INT32 OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout = CT_FALSE);
	//��Ҳ���
	CT_INT32	OnUserPassCard(CT_DWORD dwChairID);
	//֪ͨ��һ����ҳ���
	CT_INT32 NotifyNextOutCard(CT_DWORD dwChairID);
	//������һ���������
	CT_DWORD FindNextOutCardUser(CT_DWORD dwChairID);
	//֪ͨ����
	CT_BOOL NotifyOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCardTime, CT_BYTE cbOutCardFlag, CT_BYTE cbNewTurn);
	//�Ƿ���ը��
	CT_BOOL IsHaveRomb(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//�Ƿ��к���3
	CT_BOOL IsHaveThree(CT_BYTE cbCardData[], CT_BYTE cbCardCount);

	//ϴ��
	void XiPai();

private:
	//��Ϸ��Ϣ
	CT_DWORD				m_dwSysTime;										//ϵͳʱ��
	CT_BYTE					m_cbGameStatus;										//��Ϸ״̬

																				//��ұ�ʶ
	CT_DWORD				m_dwFirstUser;										//�׳��������
	CT_DWORD				m_dwCurrentUser;									//��ǰ�������
	CT_DWORD				m_dwResumeUser;										//��ԭ�û�
	CT_DWORD				m_dwTurnWiner;										//ʤ�����
	CT_BYTE					m_cbTurnCardType;									//�ϼ�����
	CT_BYTE					m_cbTurnCardCount;									//������Ŀ
	CT_BYTE					m_cbTurnCardData[MAX_COUNT];						//��������
	CT_BYTE					m_cbRombCount[GAME_PLAYER];							//ը������
	CT_BOOL					m_bMustThree;										//�س�����3
	CT_BYTE					m_cbIsOutThree;										//�Ƿ�Ҫ������3
	CT_BYTE					m_cbSelfCardCount;

	//�˿���Ϣ
	CT_BYTE					m_cbLeftCardCount;									//ʣ����Ŀ
	CT_BYTE					m_cbRepertoryCard[MAX_CARD_TOTAL];					//����˿�

	CT_BYTE					m_cbCardCount[GAME_PLAYER];							//���������
	CT_BYTE					m_cbCardData[GAME_PLAYER][MAX_COUNT];				//���������

	CT_BYTE					m_cbDiscardCount[GAME_PLAYER];						//������Ŀ
	CT_BYTE					m_cbDiscardCard[GAME_PLAYER][30];					//������¼

	CT_BYTE					m_cbOutCardCount[GAME_PLAYER];						//���ƴ���

	CT_BYTE					m_cbUserSingle[GAME_PLAYER];						//��ұ���
	CT_BYTE					m_cbUserBaoPei[GAME_PLAYER];						//��Ұ���

	PDK_16_CMD_S_GameEnd	m_GameEnd;											//��Ϸ����

																				//��������
private:
	//ʱ����Ϣ
	CT_BYTE					m_cbOutCardTime;									//����ʱ��

private:
    //������Ϣ
    CT_DWORD				m_dwGameType;										//��Ϸ����.0:��Ϸ��;1:������.
    CT_DWORD				m_dwServiceCost;									//�������.(%).
    CT_DWORD				m_dwlCellScore;										//��������

private:
	ITableFrame*			m_pGameDeskPtr;										//��Ϸָ��
	CGameLogic				m_GameLogic;										//��Ϸ�߼�

	PrivateTableInfo		m_PrivateTableInfo;									//��ȡ˽�˷���Ϣ
	CT_INT32				m_iTotalLWScore[GAME_PLAYER];						//���������Ӯ��
	CT_BYTE					m_cbTotalQuanGuan[GAME_PLAYER];						//�����ȫ�ش���
	CT_BYTE					m_cbTotalBeiGuan[GAME_PLAYER];						//����ұ��ش���
	CT_BYTE					m_cbTotalRomb[GAME_PLAYER];							//�����ը������
	CT_BYTE					m_cbTotalTongPei[GAME_PLAYER];						//�����ͨ�����
private:
	CT_BYTE					m_cbIsLookCardCount;								//�Ƿ���ʾ����
};

#endif	 