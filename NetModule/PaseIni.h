#ifndef CCPASEINI_H_
#define CCPASEINI_H_
#include "CTType.h"
#include "Singleton.h"
#include <string>
#include <stdio.h>

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif


//ini解析文件
class CPaseIni : public CCSingleton<CPaseIni>
{
	FILE*     m_fp;
	CT_CHAR*  m_pBuf;
public:
	CT_BOOL Open(const std::string& inifile);
	CT_BOOL Read();
	CT_BOOL GetValue(const std::string& key, CT_DWORD& value);
	CT_BOOL GetValue(const std::string& key, CT_BYTE&  value);
	CT_BOOL GetValue(const std::string& key, CT_INT64& value);
	CT_BOOL GetValue(const std::string& key, CT_FLOAT& value);
	CT_BOOL GetValue(const std::string& key, std::string& value);
	CT_BOOL GetValue(const std::string& key, CT_BOOL&   value);
	CT_BOOL GetValue(const std::string& key, CT_DOUBLE& value);
	CT_BOOL GetValue(const std::string& key, CT_CHAR&   value);
	CT_BOOL GetValue(const std::string& key, CT_WORD& value);
	CT_BOOL GetValue(const std::string& key, CT_INT32& value);
    CT_BOOL GetValue(const std::string& key, CT_CHAR* value, CT_WORD len);

	CT_VOID Close();
private:
	CT_BOOL GetData(const std::string& key, CT_CHAR* pBuff, CT_WORD len);
public:
	CPaseIni();
	~CPaseIni();
};




#ifdef SOCKETS_NAMESPACE
}
#endif


#endif





