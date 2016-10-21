/******************************************************
* Class Name: CFTTD
* Class Functional: CTP期货交易类实现
* Author: Ryan Sun
* Date: 2015/08/25
* Description: CTP期货交易类实现
*****************************************************/

#if !defined(FTTD_H__INCLUDED_)
#define FTTD_H__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <windows.h> 
#include <stdio.h>
#include <string>

#include "ThostFtdcTraderApi.h"
#include "logInfo.h"

class CFTTD : public CThostFtdcTraderSpi  
{
public:
	CFTTD();
	virtual ~CFTTD();

	bool bIsgetInst, bIsgetPosDetail;

	CThostFtdcInstrumentField g_pInstinfo[35000];
	int g_Instnum;
	int nRequestID;
    
    TThostFtdcBrokerIDType qh_BrokerID;
	TThostFtdcAddressType qh_TDAddress;
	TThostFtdcUserIDType qh_UserID;
    TThostFtdcPasswordType qh_Password;
    char systime[20];

	double round(double dVal, short iPlaces);
    void Init(const char* pi_BrokerID, const char* pi_TDAdress, const char* pi_User, const char* pi_pwd, logInfo *pi_log);
	void GetSysTime();
    void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
    void PassChange(const char* newpass);
	void CancelAll();
	void QryInstruments();
	void QueryPositionDetail();
    void QueryPosition();
    void QueryOrders();
    void QueryTrades();
    void QueryMD();
    void QueryAcct();
    void prtErr(const char* pFuncID, CThostFtdcRspInfoField *pRspInfo);

    void PlaceOrder(const char* pi_Instrument, 
                    const char* pi_BuyOrSell,
                    const char* pi_OpenOrClose,
                    double pi_Price,
                    int pi_Position);

	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询报单响应
	void OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询成交响应
	void OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询交易编码响应
	void OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询合约手续费率响应
	void OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓响应
	void OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询投资者持仓明细响应
	void OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询交易所响应
	void OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询行情响应
	void OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///请求查询资金账户响应
	void OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///错误应答
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
	///报单录入错误回报
	void OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo);

	///报单操作错误回报
	void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);

	///报单通知
	void OnRtnOrder(CThostFtdcOrderField *pOrder);

	///成交通知
	void OnRtnTrade(CThostFtdcTradeField *pTrade);

	///合约交易状态通知
	void OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus);
private:
	CThostFtdcTraderApi* m_pTdApi;
	logInfo *pLog;
};
#endif
