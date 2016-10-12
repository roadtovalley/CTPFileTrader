// ZQMD.h: interface for the CZQMD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZQMD_H__E1C8C421_0DB0_4129_AC7E_7DF1BB1C6DFC__INCLUDED_)
#define AFX_ZQMD_H__E1C8C421_0DB0_4129_AC7E_7DF1BB1C6DFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ThostFtdcMdApiSSE.h"
#include "ZQTD.h"
#include "logInfo.h"
#include <map>
#include <string>
using namespace std;

class CZQMD : public CZQThostFtdcMdSpi  
{
public:
    map<string, CZQThostFtdcDepthMarketDataField> LastDepth;
    char *zq_MDAddress;
    char *zq_UserID;
    char *zq_Password;

    void OnFrontConnected();
	void OnRspUserLogin(CZQThostFtdcRspUserLoginField *pRspUserLogin, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void initSubMD();
    void SubscribeMD(char* Instrument, char* ExchangeID);
	void OnFrontDisconnected(int nReason);
	void Init(const char* pi_MDAdress, const char* pi_User, const char* pi_pwd, CZQTD* pTdHandler,logInfo* pLog);
	void OnRtnDepthMarketData(CZQThostFtdcDepthMarketDataField *pDepthMarketData);
	CZQMD();
	virtual ~CZQMD();

private:
	CZQThostFtdcMdApi * m_pMdApi;
    CZQTD* g_pTdHandler;
	logInfo* g_pLog;
};

#endif // !defined(AFX_ZQMD_H__E1C8C421_0DB0_4129_AC7E_7DF1BB1C6DFC__INCLUDED_)
