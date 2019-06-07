#ifndef CCDBCONNECTOR_H_
#define CCDBCONNECTOR_H_


#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include "CTType.h"
#include "Mutex.h"

#define DB_NET_ERROR    (1)
#define DB_USER_ERROR   (2)

class CCDBResult
{
private:
	MYSQL_RES*  m_pRes;
	MYSQL_ROW	m_sqlRow;
	MYSQL*      m_pMysql;
	CT_DWORD    m_rowNum;
public:
	CCDBResult();
	void Init(MYSQL* pMySql);
	void GetValue(CT_WORD index, std::string& value);
	void GetValue(CT_WORD index, CT_WORD&  value);
	void GetValue(CT_WORD index, CT_DWORD& value);
	void GetValue(CT_WORD index, CT_SHORT& value);
	void GetValue(CT_WORD index, CT_CHAR&  value);
	void GetValue(CT_WORD index, CT_UCHAR& value);
	void GetValue(CT_WORD index, CT_INT32& value);
	void GetValue(CT_WORD index, CT_FLOAT& value);
	void GetValue(CT_WORD index, CT_DOUBLE& value);


	void GetValue(CT_WORD index, CT_INT64& value);
	void GetValue(CT_WORD index, CT_CHAR*  value, CT_DWORD bufLen);
	void GetValue(CT_WORD index, CT_BOOL&  value);

	bool GetResult();
	bool MoveNextRow();
	void Release();
};

class CCDBConnector
{
	class CCDBProcParm
	{
	public:
		std::string m_command;
		void SetPcName(const std::string& spName);
		void Reset();
		void AddParm(const std::string& parmName, const std::string& parmValue );
		void AddParm(const std::string& parmName, CT_DWORD parmValue);
		void AddParm(const std::string& parmName, CT_CHAR parmValue);
		void AddParm(const std::string& parmName, CT_INT32 parmValue);
		void AddParm(const std::string& parmName, CT_WORD parmValue);
		void AddParm(const std::string& parmName, CT_SHORT parmValue);
		void AddParm(const std::string& parmName, CT_UCHAR parmValue);
		void AddParm(const std::string& parmName, CT_FLOAT parmValue);
		void AddParm(const std::string& parmName, CT_DOUBLE parmValue);
		void AddParm(const std::string& parmName, CT_CHAR* parmValue);
		void AddParm(const std::string& parmName, CT_BOOL& parmValue);
		void AddParm(const std::string& parmName, CT_INT64& parmValue);
		void Exec();
	};

private:
	MYSQL			m_mySql;
	CCDBProcParm	m_procParm; 
	CCDBResult      m_result;
	//CTMutex         m_mutex;

public:
	bool Init();
	bool Connect(const std::string& host, const std::string& user, const std::string& key, 
		const std::string& dbName, CT_DWORD port);
	void SetPcName(const std::string& spName);
	void Reset();
	void AddParm(const std::string& parmName, const std::string& parmValue );
	void AddParm(const std::string& parmName, CT_DWORD parmValue);
	void AddParm(const std::string& parmName, CT_CHAR parmValue);
	void AddParm(const std::string& parmName, CT_INT32 parmValue);
	void AddParm(const std::string& parmName, CT_WORD parmValue);
	void AddParm(const std::string& parmName, CT_SHORT parmValue);
	void AddParm(const std::string& parmName, CT_UCHAR parmValue);
	void AddParm(const std::string& parmName, CT_FLOAT parmValue);
	void AddParm(const std::string& parmName, CT_DOUBLE parmValue);
	void AddParm(const std::string& parmName, CT_INT64& value);
	void AddParm(const std::string& parmName, CT_CHAR* parmValue);
	void AddParm(const std::string& parmName, CT_BOOL& parmValue);
	
public:
	CT_BOOL ExecSqlStr(const CT_CHAR* pStr, CT_DWORD dwLen);
	bool Exec();
	CCDBResult* GetResult() {return &m_result;}
	CT_DWORD    GetErrno();

	static char* ConvertBinaryToString(MYSQL* pMysqlConn, char* pBinaryData, int nLen)
	{
		static char s_BinaryData[81920];
		if (nLen > 40960)
		{
			nLen = 40960;
		}
		mysql_real_escape_string(pMysqlConn, s_BinaryData, pBinaryData, nLen);
		return s_BinaryData;
	}

public:
	CCDBConnector(void);
	~CCDBConnector(void);
};

#endif







