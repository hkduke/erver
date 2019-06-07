#pragma once
#include "AndroidUserItem.h"
#include <map>
#include <set>
#include <memory>

typedef std::map<CT_DWORD, std::shared_ptr<tagAndroidUserParameter>> mapAndroidParam;
typedef std::list<std::shared_ptr<tagAndroidUserParameter>> listAndroidParam;
typedef std::set<std::shared_ptr<tagAndroidUserParameter>> setAndroidParam;
typedef std::map<CT_DWORD, GS_UserBaseData>	MapAndroidUserBaseData;

class CGameServerThread;
class CAndroidUserMgr : public acl::singleton<CAndroidUserMgr>
{
public:
	CAndroidUserMgr();
	~CAndroidUserMgr();

public:
	void Initialization(tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, CGameServerThread* pGameserverThread);

	//ʱ������
	void OnTimePulse();
	//�������Ƿ���Ҫ����
	void OnTimeCheckUserIn();
	//��������˲���
	void InsertAndroidUserParam(tagAndroidUserParameter* pAndroidParam);
	//��ջ����˲���
	void ClearAndroidUserParam();

	//�����û�
	CT_BOOL CreateAndroidUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID);
	// ɾ���û�
	CT_BOOL	DeleteUserItem(CServerUserItem* pServerUserItem);
	// �����û�
	CServerUserItem* FindAndroidUserItem(CT_DWORD dwUserID);

	//���û����û�
	CT_VOID SetAndroidUserScore(CT_DWORD dwUserID, CT_LONGLONG llScore);

	//���û����û�����
	CT_VOID SetActiveAndroidCount(CT_DWORD dwActiveAndroidCount);
	//��ȡ�����û�����
	CT_DWORD GetActiveAndroidCount();

	//���»�����
	CT_VOID UpdateAndroidStatus(CT_DWORD dwUserID, CT_BYTE cbStatus);

public:
	GS_UserBaseData* GetAndroidUserBaseData(CT_DWORD dwUserID);

public:
	CServerUserItemMap			m_AndroidUserMap;				
	CServerUserItemArray		m_AndroidUserItemSitting;		//���µĻ�����
	CServerUserItemArray		m_AndroidUserItemFree;			//���еĻ�������

	//���������ñ�
	setAndroidParam				m_setAndroidParamAll;			//ȫ���������б�
	listAndroidParam			m_listAndroidParamStore;		//��û��ʹ�õĻ����˲���
	mapAndroidParam				m_mapAndroidParamUsed;			//�Ѿ�ʹ�õĻ����˲���
	MapAndroidUserBaseData		m_mapAndroidUserData;			//�Ѿ�ʹ�õĻ����˻�������

	tagGameKind*				m_pGameKindInfo;
	tagGameRoomKind*			m_pRoomKindInfo;
	CGameServerThread*			m_pGameServerThread;

	CT_DWORD					m_dwActiveAndroidCount;			//����Ļ���������

	LpDllNewAndroid				m_pFunNewAndroidSink;
};