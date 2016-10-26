#include "stdafx.h"
#include "FTMD.h"
#include <iostream>
CFTMD::CFTMD()
{

}

CFTMD::~CFTMD()
{
	
}

void CFTMD::Init(const char* pi_BrokerID, const char* pi_MDAdress, const char* pi_User, const char* pi_pwd, CFTTD* pTdHandler, logInfo* pLog)
{
	// 产生一个CThostFtdcMdApi实例
    memset(qh_BrokerID, 0, sizeof(qh_BrokerID));
    memset(qh_MDAddress, 0, sizeof(qh_MDAddress));
    memset(qh_UserID, 0, sizeof(qh_UserID));
    memset(qh_Password, 0, sizeof(qh_Password));
    strcpy(qh_BrokerID, pi_BrokerID);
    strcpy(qh_MDAddress, pi_MDAdress);
    strcpy(qh_UserID, pi_User);
    strcpy(qh_Password, pi_pwd);
    g_pTdHandler = pTdHandler;
	g_pLog = pLog;

	m_pMdApi=CThostFtdcMdApi::CreateFtdcMdApi("");
	m_pMdApi->RegisterSpi(this);
	m_pMdApi->RegisterFront(qh_MDAddress);
	m_pMdApi->Init();
}

void CFTMD::OnFrontConnected()
{
	CThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID, qh_BrokerID);
	strcpy_s(reqUserLogin.UserID, qh_UserID);
	strcpy_s(reqUserLogin.Password, qh_Password);
	g_pLog->printLog("行情登陆请求...\n");
	m_pMdApi->ReqUserLogin(&reqUserLogin,1);
}
void CFTMD::OnFrontDisconnected(int nReason)
{
	g_pLog->printLog("行情连接断开!\n");
}
void CFTMD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	g_pLog->printLog("行情登陆成功!\n");
    initSubMD();
}
void CFTMD::SubscribeMD(char* Instrument)
{
	//订阅行情
	char** Instruments;
    Instruments=new char*[0];
    Instruments[0]= new char[strlen(Instrument)];
    strcpy(Instruments[0], Instrument);
	CThostFtdcDepthMarketDataField tmpMD;
	memset(&tmpMD, 0, sizeof(tmpMD));
	LastDepth[Instrument] = tmpMD;
	int rtn_cd = m_pMdApi->SubscribeMarketData (Instruments, 1);
	g_pLog->printLog("订阅%s行情, 返回代码:%d\n", Instrument, rtn_cd);
}
void CFTMD::initSubMD()
{
        //等待交易类中订阅合约列表刷新完成
	while (!g_pTdHandler->bIsgetInst)
	{
	    Sleep(100);
	}
    //根据合约列表订阅行情
	for (int i=0; i<g_pTdHandler->g_Instnum; i++)
	{
		SubscribeMD(g_pTdHandler->g_pInstinfo[i].InstrumentID);
	}
    //建立事件并等待3秒行情反馈
    g_pLog->printLog("等待行情反馈...\n");
    HANDLE g_hEvent = CreateEvent(NULL, true, false, NULL);	
	//WaitForSingleObject(g_hEvent, INFINITE);
    WaitForSingleObject(g_hEvent, 1000);
}

void CFTMD::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData) 
{
	CThostFtdcDepthMarketDataField *pMD;
	LastDepth[pDepthMarketData->InstrumentID] = *pDepthMarketData;
	pMD = &LastDepth[pDepthMarketData->InstrumentID];

	pMD->LastPrice = (pMD->LastPrice > 10000000.0) ? 0 : pMD->LastPrice;                          ///最新价
	pMD->OpenPrice = (pMD->OpenPrice > 10000000.0) ? pMD->LastPrice : pMD->OpenPrice;             ///今开盘
	pMD->HighestPrice = (pMD->HighestPrice > 10000000.0) ? pMD->LastPrice : pMD->HighestPrice;    ///最高价
	pMD->LowestPrice = (pMD->LowestPrice > 10000000.0) ? pMD->LastPrice : pMD->LowestPrice;       ///最低价
	pMD->BidPrice1 = (pMD->BidPrice1 > 10000000.0) ? pMD->LastPrice : pMD->BidPrice1;             ///申买价一
	pMD->AskPrice1 = (pMD->AskPrice1 > 10000000.0) ? pMD->LastPrice : pMD->AskPrice1;             ///申卖价一
	pMD->AveragePrice = (pMD->AveragePrice > 10000000.0) ? pMD->LastPrice : pMD->AveragePrice;    ///当日均价
	/*
	if (pDepthMarketData->LastPrice == 0)
	{
		g_pLog->printLog("MarketData:%s,%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n", pDepthMarketData->InstrumentID,
			pDepthMarketData->UpdateTime,
			pDepthMarketData->TradingDay,
			pDepthMarketData->PreClosePrice,
			pDepthMarketData->OpenPrice,
			pDepthMarketData->LastPrice,
			pDepthMarketData->UpperLimitPrice,
			pDepthMarketData->LowerLimitPrice,
			pDepthMarketData->AskPrice1,
			pDepthMarketData->BidPrice1,
			pDepthMarketData->Volume);
	}
	*/
}
