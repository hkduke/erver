
#include "PaseIni.h"
#include "glog_wrapper.h"
#include "exam_assert.h"


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif



#ifdef _MSC_VER
#pragma warning(disable:4996)
#pragma warning(disable:4800)
#endif

template<> CPaseIni* CCSingleton<CPaseIni>::ms_Singleton = NULL;
CPaseIni::CPaseIni() : m_fp(NULL), m_pBuf(NULL)
{
}

CPaseIni::~CPaseIni()
{
}

CT_BOOL CPaseIni::Open(const std::string& inifile)
{
	LOG(INFO) << "Begin to Open inifile.";
	if (inifile == "")
	{
		LOG(ERROR) << "IniFile is empty.";
		exit(0);
		return CT_FALSE;
	}
	m_fp = fopen(inifile.c_str(), "r");
	if (m_fp == 0)
	{
		LOG(ERROR) << "Failed to Open INI file.";
		exit(0);
		return CT_FALSE;
	}
	return CT_TRUE;
}

CT_BOOL CPaseIni::Read()
{
	 fseek(m_fp,0L,SEEK_END);   
	 CT_DWORD len = ftell(m_fp);           
	 m_pBuf = new char[len +1];
	 _EXAM_ASSERT(m_pBuf != NULL);
	 if (m_pBuf == NULL)
	 {
		 Close();
		 LOG(ERROR) << "Failed to molloc memery to m_pBuf.";
		 exit(0);
		 return CT_FALSE;
	 }
	 memset(m_pBuf, 0, len+1);
	 fseek(m_fp, 0L, SEEK_SET);

	 len = (CT_DWORD)fread(m_pBuf, sizeof(char), len+1,  m_fp);
	 if (len <= 0)
	 {
		 Close();
		 LOG(ERROR) << "Failed to fread().";
		 exit(0);
		 return CT_FALSE;
	 }
	 return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_DWORD& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atoi(buf);
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_BYTE&  value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atoi(buf);
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_INT64& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}

#ifdef _WIN32
	value = _atoi64(buf);
#else
	value = atoll(buf);
#endif
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_FLOAT& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = (CT_FLOAT)atof(buf);
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, std::string& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = buf;
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_BOOL&   value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = static_cast<CT_BOOL>(atoi(buf));
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_DOUBLE& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atof(buf);
	return CT_TRUE;
}
CT_BOOL CPaseIni::GetValue(const std::string& key, CT_CHAR&   value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atoi(buf);
	return CT_TRUE;
}

CT_BOOL CPaseIni::GetValue(const std::string& key, CT_WORD& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atoi(buf);
	return CT_TRUE;
}

CT_BOOL CPaseIni::GetValue(const std::string& key, CT_INT32& value)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	value = atoi(buf);
	return CT_TRUE;
}


CT_BOOL CPaseIni::GetValue(const std::string& key, CT_CHAR* value, CT_WORD len)
{
	CT_CHAR buf[BUFSIZ] = {0}; 
	if (!GetData(key, buf, BUFSIZ -1))
	{
		return CT_FALSE;
	}
	strncpy(value, buf, len-1);
	return CT_TRUE;
}

CT_VOID CPaseIni::Close()
{
	fclose(m_fp);
	delete [] m_pBuf;
	m_pBuf = NULL;
	LOG(INFO) << "Begin to close inifile.";
}

CT_BOOL CPaseIni::GetData(const std::string& key, CT_CHAR* pBuff, CT_WORD len)
{
	CT_CHAR buf[BUFSIZ]= {0};
	sprintf(buf, "Begin to read  KEY=%s", key.c_str());
	LOG(INFO) << buf;

	char* pTag = strstr(m_pBuf, key.c_str());
	if (pTag == 0)
	{
		sprintf(buf, "GetData : can't find key = %s", key.c_str());
		LOG(ERROR) << buf;
		return CT_FALSE;
	}
	char* pBeginTag = strstr(pTag, "=");
#ifdef _WIN32
	char* pEndTag1 = strstr(pTag, "\n");
#else
	char* pEndTag1 = strstr(pTag, "\r\n");
#endif
	
	char* pEndTag2 = strstr(pTag, "#");

	if(pBeginTag == 0)
	{
		CT_CHAR  bbuf[BUFSIZ] = {0};
		memcpy(bbuf, pTag, pEndTag1-pTag);
		sprintf(buf, "GetData : parse ini line error , can't find = in %s",  bbuf);
		LOG(ERROR) << buf;
		return CT_FALSE;
	}
	pBeginTag+=1;

	char* pEndTag = 0;
	
	if (pEndTag2 == 0)
	{
		pEndTag = pEndTag1;
	}else
	{
		if (pEndTag2 < pEndTag1 )
		{
			pEndTag = pEndTag2;
		}else
		{
			pEndTag = pEndTag1;
		}
	}

	--pEndTag;
	
	if (pEndTag - pBeginTag > len )
	{
		sprintf(buf, "GetData : buf is too small to read key : %s", key.c_str());
		LOG(ERROR) << buf;
		return CT_FALSE;
	}

	if (pEndTag - pBeginTag < 0)
	{
		sprintf(buf, "GetData : Read Key:%s, pos is wrong pEndTag=%s, pTag=%s", key.c_str(), pEndTag, pTag);
		LOG(ERROR) << buf;
		return CT_FALSE;
	}

	
	while(pBeginTag != pEndTag)
	{
		if (*pBeginTag == ' ')
		{
			++pBeginTag;
		}

		if (*pEndTag == ' ')
		{
			--pEndTag;
		}

		if (*pBeginTag != ' ' && *pEndTag != ' ')
		{
			break;
		}
	}


	memcpy(pBuff, pBeginTag, (pEndTag - pBeginTag)+1);

	sprintf(buf, "End to read  %s=%s", key.c_str(), pBuff);
	LOG(INFO) << buf;
	return CT_TRUE;
}



#ifdef SOCKETS_NAMESPACE
}
#endif












