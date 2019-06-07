#ifndef _verification_code_h_
#define _verification_code_h_

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "DataQueue.h"
#include <memory>
#include <iostream>

struct tagVerificationCode
{
    std::string  strCountryCode;
	std::string	 strMobileNum;
	int			 nPlatformId;
	int			 nVerificationCode;

	tagVerificationCode()
	{
		Reset();
	}
    tagVerificationCode(std::string& countryCode, std::string& mobileNum, int nPid, int nCode)
    : strCountryCode(countryCode)
    , strMobileNum(mobileNum)
    , nPlatformId(nPid)
    , nVerificationCode(nCode)
    {
        //std::cout << "tagVerificationCode" << std::endl;
    }

    ~tagVerificationCode()
    {
	    //std::cout << "~tagVerificationCode" << std::endl;
    }

	void Reset()
	{
        strCountryCode.clear();
		strMobileNum.clear();
		nPlatformId = 0;
		nVerificationCode = 0;
	}
};

class CVerificationCodeThread : public acl::thread
{
	friend class CLoginServerThread;
public:
	CVerificationCodeThread(bool bAutoDestroy);
	~CVerificationCodeThread();

	virtual void* run();

public:
	/************
	插入验证码数据
	*************/
	//std::shared_ptr<tagVerificationCode> GetFreeVerificationCodeQue();
	//void InsertVerificationCodeQue(std::shared_ptr<tagVerificationCode>& verificationCodePtr);

	void SetVerificationCode(std::shared_ptr<tagVerificationCode>& verificationCodePtr);

private:
	//CDataQueue<std::shared_ptr<tagVerificationCode>>	m_VerificationCodeQue;		//需要处理的验证码信息
	//CDataQueue<std::shared_ptr<tagVerificationCode>>	m_verificationCodeFreeQue;	//空闲的验证码信息

    std::shared_ptr<tagVerificationCode>                m_VerifycationCode;         //验证码信息

private:
	/**
	* * 发送短信
	* */
	inline bool send_sms(const char *un, const char *pw, const char *phone, const char *msg);
	inline bool send_sms2(const char *un, const char *pw, const char *phone, const char *msg);
	//云片新接口
	inline bool send_sms3(const char *apikey, const char *mobile, int nCode, int nTplId);
	inline bool send_sms3International(const char *apikey, const char *mobile, const char *msg);

	/**
	* * 查账户余额
	* */
	inline bool get_balance(const char *un, const char *pw);

	/**
	* * 发http post请求
	* */
	inline bool http_post(const char *page, const char *poststr);
	inline bool http_post2(const char *page, const char *poststr);
	inline bool http_post3(const char *data, std::string& sms3Url);

private:
	static std::string m_sHostname;
	static std::string m_sSend_sms_url;
	static std::string m_Query_balance_url;
	static unsigned int m_dwUseIndex;

	static std::string m_sHostname2;
	static std::string m_sSend_sms_url2;

	//云片
	static std::string m_sHostname3;
	static std::string m_sSend_sms_url3;      //用于发国内短信
	static std::string m_sSend_sms_url3Single;//用于发国际短信
	static std::string m_sYunPianKey;
	static int         m_sTplId1;
    static int         m_sTplId2;
    static int         m_sTplId3;
    static int         m_sTplId4;
    //std::string	 m_MobileNum;
	//int			 m_nPlatformId;
	//int			 m_nVerificationCode;
	//acl::socket_stream m_conn;
	bool		 m_bAutoDestroy;

	enum enSMSId
	{
		en_253 		= 1,
		en_STONG 	= 2,
		en_yunpian  = 3,
	};
};





#endif