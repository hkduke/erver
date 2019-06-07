#include "VerificationCodeThread.h"
#include "CTType.h"
#include "Utility.h"
#include "glog_wrapper.h"
#include "ServerCfg.h"
#include "httplib.h"

#define MAXLINE 4096
#define MAXSUB  2000
#define MAXPARAM 2048

#define LISTENQ         1024

 std::string CVerificationCodeThread::m_sHostname = "smssh1.253.com";
 std::string CVerificationCodeThread::m_sSend_sms_url = "/msg/send/json";
 std::string CVerificationCodeThread::m_Query_balance_url = "/msg/balance/json";

std::string CVerificationCodeThread::m_sHostname2 = "www.stongnet.com";
std::string CVerificationCodeThread::m_sSend_sms_url2 = "/sdkhttp/sendsms.aspx";

std::string CVerificationCodeThread::m_sHostname3 = "sms.yunpian.com";
std::string CVerificationCodeThread::m_sSend_sms_url3 = "/v2/sms/tpl_single_send.json";
std::string CVerificationCodeThread::m_sSend_sms_url3Single = "/v2/sms/single_send.json";
std::string CVerificationCodeThread::m_sYunPianKey = "9d3e1a737d5a31786422f4a081436fa5";
int CVerificationCodeThread::m_sTplId1 = 2890356;
int CVerificationCodeThread::m_sTplId2 = 2808518;
int CVerificationCodeThread::m_sTplId3 = 2808520;
int CVerificationCodeThread::m_sTplId4 = 2889002;

unsigned int CVerificationCodeThread::m_dwUseIndex = en_253;

CVerificationCodeThread::CVerificationCodeThread(bool bAutoDestroy)
	: m_bAutoDestroy(bAutoDestroy)
{
    //LOG(WARNING) << "CVerificationCodeThread";
}

CVerificationCodeThread::~CVerificationCodeThread()
{
    //LOG(WARNING) << "~CVerificationCodeThread";

}

/*std::shared_ptr<tagVerificationCode> CVerificationCodeThread::GetFreeVerificationCodeQue()
{
	if (m_verificationCodeFreeQue.GetSize() == 0)
	{
		return std::make_shared<tagVerificationCode>();
	}

	return m_verificationCodeFreeQue.GetHead();
}


void CVerificationCodeThread::InsertVerificationCodeQue(std::shared_ptr<tagVerificationCode>& loginInfoPtr)
{
	m_VerificationCodeQue.PushBack(loginInfoPtr);
}*/

void CVerificationCodeThread::SetVerificationCode(std::shared_ptr<tagVerificationCode>& verificationCodePtr)
{
    m_VerifycationCode = verificationCodePtr;
}

/*void* CVerificationCodeThread::run()
{
	while (true)
	{
		if (m_VerificationCodeQue.GetSize() == 0)
		{
			Utility::Sleep(10);
			continue;
		}

		std::shared_ptr<tagVerificationCode> VerificationCodePtr = m_VerificationCodeQue.GetHead();

		char msg[1024] = { 0 };
		//LOG(WARNING) << "Get Verification Code platform id: " << m_nPlatformId;

		bool bSendSucc = false;
		if (m_dwUseIndex == en_yunpian)
		{
			if (VerificationCodePtr->nPlatformId == 1)
			{
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), VerificationCodePtr->strMobileNum.c_str(), VerificationCodePtr->nVerificationCode, m_sTplId1);
			}
			else if (VerificationCodePtr->nPlatformId == 2)
			{
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), VerificationCodePtr->strMobileNum.c_str(), VerificationCodePtr->nVerificationCode, m_sTplId2);
			}
			else if (VerificationCodePtr->nPlatformId == 3)
			{
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), VerificationCodePtr->strMobileNum.c_str(), VerificationCodePtr->nVerificationCode, m_sTplId3);
			}
			else
			{
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), VerificationCodePtr->strMobileNum.c_str(), VerificationCodePtr->nVerificationCode, m_sTplId1);
			}
		}
		else if (m_dwUseIndex == en_253)
		{
			if (VerificationCodePtr->nPlatformId == 1)
			{
				_snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和娱乐祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else if (VerificationCodePtr->nPlatformId == 2)
			{
				_snprintf_info(msg, 1024, "【将军娱乐】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。将军娱乐祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else if (VerificationCodePtr->nPlatformId == 3)
			{
				_snprintf_info(msg, 1024, "【同花顺娱乐】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。同花顺娱乐祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else
			{
				_snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和娱乐祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			bSendSucc = send_sms(CServerCfg::m_SmsAccount, CServerCfg::m_SmsPwd, VerificationCodePtr->strMobileNum.c_str(), Utility::Ansi2Utf8(msg).c_str());
		}
		else if (m_dwUseIndex == en_STONG)
		{
			if (VerificationCodePtr->nPlatformId == 1)
			{
				_snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else if (VerificationCodePtr->nPlatformId == 2)
			{
				_snprintf_info(msg, 1024, "【将军】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。将军祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else if (VerificationCodePtr->nPlatformId == 3)
			{
				_snprintf_info(msg, 1024, "【同花顺】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。同花顺祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			else
			{
				_snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和祝你生活愉快！", VerificationCodePtr->nVerificationCode);
			}
			bSendSucc = send_sms2(CServerCfg::m_SmsAccount1, CServerCfg::m_SmsPwd1, VerificationCodePtr->strMobileNum.c_str(), Utility::Ansi2Utf8(msg).c_str());
		}

		if (!bSendSucc)
		{
			LOG(WARNING) << "send verify code fail. mobile num: " << VerificationCodePtr->strMobileNum.c_str();
		}

        //插入空闲队列
        VerificationCodePtr->Reset();
        m_verificationCodeFreeQue.PushBack(VerificationCodePtr);
		//if (bSendSucc)
		//{
			//插入空闲队列
		//	VerificationCodePtr->Reset();
		//	m_verificationCodeFreeQue.PushBack(VerificationCodePtr);
		//}
		//else
		//{
			//重新插入列表再发一次
		//	m_VerificationCodeQue.PushBack(VerificationCodePtr);
		//}
	}

	if (m_bAutoDestroy)
		delete this;

	return NULL;
}*/

void* CVerificationCodeThread::run()
{
    //LOG(WARNING) << "Get Verification Code platform id: " << m_nPlatformId;

    bool bSendSucc = false;
    if (m_dwUseIndex == en_yunpian)
    {
        if (m_VerifycationCode->nPlatformId == 1)
        {
            if (m_VerifycationCode->strCountryCode == "86")
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), m_VerifycationCode->strMobileNum.c_str(), m_VerifycationCode->nVerificationCode, m_sTplId1);
            else
            {
                char msg[1024] = { 0 };
                _snprintf_info(msg, 1024, "【HeLe】Your verification code is %d,please input your verification code in 5 minutes.", m_VerifycationCode->nVerificationCode);
                std::string strFullMobileNum = "+"+m_VerifycationCode->strCountryCode+m_VerifycationCode->strMobileNum;
                bSendSucc = send_sms3International(m_sYunPianKey.c_str(), strFullMobileNum.c_str(), msg);
            }
        }
        else if (m_VerifycationCode->nPlatformId == 2)
        {
            if (m_VerifycationCode->strCountryCode == "86")
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), m_VerifycationCode->strMobileNum.c_str(), m_VerifycationCode->nVerificationCode, m_sTplId2);
            else
            {
                char msg[1024] = { 0 };
                _snprintf_info(msg, 1024, "【JiangJun】Your verification code is %d,please input your verification code in 5 minutes.", m_VerifycationCode->nVerificationCode);
                std::string strFullMobileNum = "+"+m_VerifycationCode->strCountryCode+m_VerifycationCode->strMobileNum;
                bSendSucc = send_sms3International(m_sYunPianKey.c_str(), strFullMobileNum.c_str(), msg);
            }
        }
        else if (m_VerifycationCode->nPlatformId == 3)
        {
            if (m_VerifycationCode->strCountryCode == "86")
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), m_VerifycationCode->strMobileNum.c_str(), m_VerifycationCode->nVerificationCode, m_sTplId3);
            else
            {
                char msg[1024] = { 0 };
                _snprintf_info(msg, 1024, "【TongHuaShun】Your verification code is %d,please input your verification code in 5 minutes.", m_VerifycationCode->nVerificationCode);
                std::string strFullMobileNum = "+"+m_VerifycationCode->strCountryCode+m_VerifycationCode->strMobileNum;
                bSendSucc = send_sms3International(m_sYunPianKey.c_str(), strFullMobileNum.c_str(), msg);
            }
        }
        else if (m_VerifycationCode->nPlatformId == 4)
        {
            if (m_VerifycationCode->strCountryCode == "86")
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), m_VerifycationCode->strMobileNum.c_str(), m_VerifycationCode->nVerificationCode, m_sTplId4);
            else
            {
                char msg[1024] = { 0 };
                _snprintf_info(msg, 1024, "【QiLe】Your verification code is %d,please input your verification code in 5 minutes.", m_VerifycationCode->nVerificationCode);
                std::string strFullMobileNum = "+"+m_VerifycationCode->strCountryCode+m_VerifycationCode->strMobileNum;
                bSendSucc = send_sms3International(m_sYunPianKey.c_str(), strFullMobileNum.c_str(), msg);
            }
        }
        else
        {
            if (m_VerifycationCode->strCountryCode == "86")
                bSendSucc = send_sms3(m_sYunPianKey.c_str(), m_VerifycationCode->strMobileNum.c_str(), m_VerifycationCode->nVerificationCode, m_sTplId1);
            else
            {
                char msg[1024] = { 0 };
                _snprintf_info(msg, 1024, "【HeLe】Your verification code is %d,please input your verification code in 5 minutes.", m_VerifycationCode->nVerificationCode);
                std::string strFullMobileNum = "+"+m_VerifycationCode->strCountryCode+m_VerifycationCode->strMobileNum;
                bSendSucc = send_sms3International(m_sYunPianKey.c_str(), strFullMobileNum.c_str(), msg);
            }
        }
    }
    else if (m_dwUseIndex == en_253)
    {
        char msg[1024] = { 0 };
        if (m_VerifycationCode->nPlatformId == 1)
        {
            _snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和娱乐祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else if (m_VerifycationCode->nPlatformId == 2)
        {
            _snprintf_info(msg, 1024, "【将军娱乐】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。将军娱乐祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else if (m_VerifycationCode->nPlatformId == 3)
        {
            _snprintf_info(msg, 1024, "【同花顺娱乐】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。同花顺娱乐祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else
        {
            _snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和娱乐祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        bSendSucc = send_sms(CServerCfg::m_SmsAccount, CServerCfg::m_SmsPwd, m_VerifycationCode->strMobileNum.c_str(), Utility::Ansi2Utf8(msg).c_str());
    }
    else if (m_dwUseIndex == en_STONG)
    {
        char msg[1024] = { 0 };
        if (m_VerifycationCode->nPlatformId == 1)
        {
            _snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else if (m_VerifycationCode->nPlatformId == 2)
        {
            _snprintf_info(msg, 1024, "【将军】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。将军祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else if (m_VerifycationCode->nPlatformId == 3)
        {
            _snprintf_info(msg, 1024, "【同花顺】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。同花顺祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        else
        {
            _snprintf_info(msg, 1024, "【万和】您的验证码是：%d。5分钟内有效，如非本人操作，请忽略本短信。万和祝你生活愉快！", m_VerifycationCode->nVerificationCode);
        }
        bSendSucc = send_sms2(CServerCfg::m_SmsAccount1, CServerCfg::m_SmsPwd1, m_VerifycationCode->strMobileNum.c_str(), Utility::Ansi2Utf8(msg).c_str());
    }

    if (!bSendSucc)
    {
        LOG(WARNING) << "send verify code fail. mobile num: (+" << m_VerifycationCode->strCountryCode << ")" << m_VerifycationCode->strMobileNum.c_str();
    }
    if (m_bAutoDestroy)
        delete this;

    return NULL;
}

bool CVerificationCodeThread::send_sms(const char *un, const char *pw, const char *phone, const char *msg)
{
	char params[MAXPARAM + 1];
	char *cp = params;

	_snprintf_info(params, MAXPARAM, "{\"account\":\"%s\",\"password\":\"%s\",\"phone\":\"%s\",\"msg\":\"%s\",\"report\":\"false\"}", un, pw, phone, msg);
	return http_post(m_sSend_sms_url.c_str(), cp);
}

bool CVerificationCodeThread::send_sms2(const char *un, const char *pw, const char *phone, const char *msg)
{
	char params[MAXPARAM + 1];
	char *cp = params;

	_snprintf_info(params, MAXPARAM, "reg=%s&pwd=%s&sourceadd=&phone=%s&content=%s", un, pw, phone, msg);
	return http_post2(m_sSend_sms_url2.c_str(), cp);
}

bool CVerificationCodeThread::send_sms3(const char *apikey, const char *mobile, int nCode, int nTplId)
{
    const char *tmp;
    char tpl_value[MAXPARAM + 1];
    bzero(tpl_value, sizeof(char)*(MAXPARAM + 1));

    // 模板短信发送需要编码两次，第一次URL编码转换
    int len = 0;
    std::string strTmp = Utility::urlEncodeCPP("#code#");
    tmp = strTmp.c_str();
    memcpy(tpl_value+len,tmp,strlen(tmp));
    len += strlen(tmp);
    tpl_value[len++] = '=';

    strTmp = Utility::urlEncodeCPP(std::to_string(nCode));
    tmp = strTmp.c_str();
    memcpy(tpl_value+len,tmp,strlen(tmp));
    len += strlen(tmp);
    tpl_value[len++] = '&';

    tmp=tpl_value;
    // 第二次URL编码
    strTmp = Utility::urlEncodeCPP(tpl_value);

    _snprintf_info(tpl_value, sizeof(tpl_value), "apikey=%s&mobile=%s&tpl_id=%d&tpl_value=%s", apikey, mobile, nTplId, strTmp.c_str());
    LOG(INFO) << "send verification Code to: "<< mobile << ", code: " << nCode;
    return http_post3(tpl_value, m_sSend_sms_url3);
}

bool CVerificationCodeThread::send_sms3International(const char *apikey, const char *mobile, const char *msg)
{
    char params[MAXPARAM + 1];
    sprintf(params,"apikey=%s&mobile=%s&text=%s", apikey, Utility::urlEncodeCPP(mobile).c_str(), msg);
    return http_post3(params, m_sSend_sms_url3Single);
}

bool CVerificationCodeThread::get_balance(const char *un, const char *pw)
{
	char params[MAXPARAM + 1];
	char *cp = params;

	_snprintf_info(params, MAXPARAM, "account=%s&password=%s", un, pw);

	return http_post(m_Query_balance_url.c_str(), cp);
}

bool CVerificationCodeThread::http_post(const char *page, const char *poststr)
{
	acl::socket_stream m_conn;
	acl::string serverAddr;
	serverAddr.format("%s:%d", m_sHostname.c_str(), 80);

	// 连接远程服务器
	if (m_conn.open(serverAddr, 10, 10) == false)
	{
		LOG(WARNING) << "connect " << serverAddr.c_str() << " error: " << acl::last_serror();

		if (m_bAutoDestroy)
			delete this;

		return false;
	}

	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	_snprintf_info(sendline, MAXSUB,
		"POST %s HTTP/1.1\r\n"
		"Host: vsms.253.com\r\n"
		"Content-type: application/json\r\n"
		"Content-length: %zu\r\n\r\n"
		"%s", page, strlen(poststr), poststr);

	LOG(INFO) << "post content: \n" << sendline;
	if (m_conn.write(sendline, strlen(sendline)) == (-1))
	{
		LOG(INFO) << "http pose msg fail.";
	}

	ssize_t n;
	bool bSucc = false;
	while ((n = m_conn.read(recvline, MAXLINE, false)) > 0)
	{
		recvline[n] = '\0';
		LOG(INFO) << "recv reply message: \n " << Utility::Utf82Ansi(recvline).c_str();

		//判断发送短信是否成功
		std::string strRecvMsg(recvline);
		auto it = strRecvMsg.find("{");
		if (it != std::string::npos)
		{
			strRecvMsg = strRecvMsg.substr(it);
			acl::json Json(strRecvMsg.c_str());
			const acl::json_node* pCode = Json.getFirstElementByTagName("code");
			if (pCode != NULL)
			{
				int nCode = atoi(pCode->get_string());
				if (nCode != 0)
				{
					//切换备用帐号
					//m_dwUseIndex = en_STONG;
					LOG(ERROR) << "send verification code fail. msg: " << poststr;
				}
				else
                {
					bSucc = true;
				}
			}
		}
	}

	m_conn.close();
	return bSucc;
}

bool CVerificationCodeThread::http_post2(const char *page, const char *poststr)
{
	acl::socket_stream m_conn;
	acl::string serverAddr;
	serverAddr.format("%s:%d", m_sHostname2.c_str(), 80);

	// 连接远程服务器
	if (m_conn.open(serverAddr, 10, 10) == false)
	{
		printf("connect %s error: %s\r\n", serverAddr.c_str(),
			   acl::last_serror());

		if (m_bAutoDestroy)
			delete this;

		return false;
	}

	char sendline[MAXLINE + 1], recvline[MAXLINE + 1];
	_snprintf_info(sendline, MAXSUB,
				   "POST %s?%s HTTP/1.1\r\n"
				   "Host: www.stongnet.com\r\n"
				   "Content-type: text/html;charset=utf-8\r\n"
				   "Content-length: %u\r\n\r\n", page, poststr, 0);

	LOG(INFO) << "post content: \n" << sendline;
	if (m_conn.write(sendline, strlen(sendline)) == (-1))
	{
		LOG(INFO) << "http pose msg fail.";
	}

	ssize_t n;
	bool bSucc = false;
	while ((n = m_conn.read(recvline, MAXLINE, false)) > 0)
	{
		recvline[n] = '\0';
		LOG(INFO) << "recv reply message: \n " << Utility::Utf82Ansi(recvline).c_str();

		//判断发送短信是否成功
		std::string strRecvMsg(recvline);
		auto it = strRecvMsg.rfind("result=0");
		if (it == std::string::npos)
		{
			//m_dwUseIndex = en_253;
			LOG(ERROR) << "send verification code fail. msg: " << poststr;
		}
		else
		{
			bSucc = true;
		}
	}

	m_conn.close();
	return bSucc;
}

bool CVerificationCodeThread::http_post3(const char *data, std::string& sms3Url)
{
	httplib::Client cli(m_sHostname3.c_str());
	auto res = cli.Post(sms3Url.c_str(), data, "application/x-www-form-urlencoded;charset=utf-8");
	if (res)
	{
		LOG(INFO) << "recv reply message: " << res->body;
		if (res->status == 200)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}
