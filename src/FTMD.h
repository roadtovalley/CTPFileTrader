/******************************************************
* Class Name: CFTMD
* Class Functional: CTP期货行情类实现
* Author: Ryan Sun
* Date: 2015/08/25
* Description: CTP期货行情类实现
*****************************************************/
#if !defined(FTMD__INCLUDED_)
#define FTMD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <map>
#include <string>

#include "ThostFtdcMdApi.h"
#include "FTTD.h"
#include "logInfo.h"

using namespace std;


class CFTMD : public CThostFtdcMdSpi  
{
public:
	CFTMD();
	virtual ~CFTMD();
	typedef map<string, CThostFtdcDepthMarketDataField> TYP_QUTOE;
	TYP_QUTOE LastDepth;

	TThostFtdcBrokerIDType qh_BrokerID;
	TThostFtdcAddressType qh_MDAddress;
	TThostFtdcUserIDType qh_UserID;
    TThostFtdcPasswordType qh_Password;

    void Init(const char* pi_BrokerID, const char* pi_MDAdress, const char* pi_User, const char* pi_pwd, CFTTD* pTdHandler, logInfo* pLog);
	void initSubMD();
	void SubscribeMD(char* Instrument);

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	void OnFrontConnected();
	///登录请求响应
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重新连接，客户端可不做处理。
	///@param nReason 错误原因
	///        0x1001 网络读失败
	///        0x1002 网络写失败
	///        0x2001 接收心跳超时
	///        0x2002 发送心跳失败
	///        0x2003 收到错误报文
	void OnFrontDisconnected(int nReason);
	///深度行情通知
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);


private:
	CThostFtdcMdApi *m_pMdApi;
    CFTTD *g_pTdHandler;
	logInfo *g_pLog;
};
#endif 
