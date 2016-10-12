// ZQTD.h: interface for the CZQTD class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZQTD_H__3CA7A2E1_A491_47BE_A447_669B265F00B9__INCLUDED_)
#define AFX_ZQTD_H__3CA7A2E1_A491_47BE_A447_669B265F00B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ThostFtdcTraderApiSSE.h"
#include "logInfo.h"
#include <windows.h> 
#include <stdio.h>

class CZQTD : public CZQThostFtdcTraderSpi  
{
public:
	bool bIsgetInst, bIsgetPosDetail;
	CZQThostFtdcInstrumentField g_pInstinfo[35000];
	int g_Instnum;
    int nRequestID;
    
    char *zq_TDAddress;
    char *zq_UserID;
    char *zq_Password;
    char systime[20];

    void Init(const char* pi_TDAdress, const char* pi_User, const char* pi_pwd, logInfo* pLog);
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
    void prtErr(const char* pFuncID, CZQThostFtdcRspInfoField *pRspInfo);

    void PlaceOrder(const char* pi_Instrument, 
                           const char* pi_ExchangeID,
                           const char* pi_BuyOrSell,
                           const char* pi_Price,
                           int pi_Position);

	void OnRspUserLogin(CZQThostFtdcRspUserLoginField *pRspUserLogin, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInstrument(CZQThostFtdcInstrumentField *pInstrument, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspOrderInsert(CZQThostFtdcInputOrderField *pInputOrder, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryOrder(CZQThostFtdcOrderField *pOrder, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryTrade(CZQThostFtdcTradeField *pTrade, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryTradingCode(CZQThostFtdcTradingCodeField *pTradingCode, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryInstrumentCommissionRate(CZQThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPosition(CZQThostFtdcInvestorPositionField *pInvestorPosition, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    void OnRspQryInvestorPositionDetail(CZQThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspQryExchange(CZQThostFtdcExchangeField *pExchange, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);	///请求查询交易所响应
	void OnRspQryDepthMarketData(CZQThostFtdcDepthMarketDataField *pDepthMarketData, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast); ///请求查询行情响应
	void OnRspQryTradingAccount(CZQThostFtdcTradingAccountField *pTradingAccount, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	
    void OnRspError(CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnErrRtnOrderInsert(CZQThostFtdcInputOrderField *pInputOrder, CZQThostFtdcRspInfoField *pRspInfo);
	void OnErrRtnOrderAction(CZQThostFtdcOrderActionField *pOrderAction, CZQThostFtdcRspInfoField *pRspInfo);

	void OnRtnOrder(CZQThostFtdcOrderField *pOrder);
	void OnRtnTrade(CZQThostFtdcTradeField *pTrade);
	void OnRtnInstrumentStatus(CZQThostFtdcInstrumentStatusField *pInstrumentStatus);

	CZQTD();
	virtual ~CZQTD();

private:
	double getPrice(TZQThostFtdcStockPriceType price);
	
	CZQThostFtdcTraderApi* m_pTdApi;
	logInfo* g_pLog;
};
#endif // !defined(AFX_ZQTD_H__3CA7A2E1_A491_47BE_A447_669B265F00B9__INCLUDED_)
