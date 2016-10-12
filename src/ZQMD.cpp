#include "stdafx.h"
#include "ZQMD.h"
#include <iostream>

CZQMD::CZQMD()
{

}

CZQMD::~CZQMD()
{
	
}

void CZQMD::Init(const char* pi_MDAdress, const char* pi_User, const char* pi_pwd, CZQTD* pTdHandler, logInfo* pLog)
{
	// 产生一个CThostFtdcMdApi实例
    zq_MDAddress = new char[strlen(pi_MDAdress)];
    strcpy(zq_MDAddress, pi_MDAdress);
    zq_UserID = new char[strlen(pi_User)];
    strcpy(zq_UserID, pi_User);
    zq_Password = new char[strlen(pi_pwd)];
    strcpy(zq_Password, pi_pwd);
    g_pTdHandler = pTdHandler;
	g_pLog = pLog;

	m_pMdApi=CZQThostFtdcMdApi::CreateFtdcMdApi("");
	m_pMdApi->RegisterSpi(this);
	m_pMdApi->RegisterFront(zq_MDAddress);
	m_pMdApi->Init();
}

void CZQMD::OnFrontConnected()
{
	CZQThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID, "2011");
	strcpy_s(reqUserLogin.UserID, zq_UserID);
	strcpy_s(reqUserLogin.Password, zq_Password);
	g_pLog->printLog("行情登陆请求...\n");
	m_pMdApi->ReqUserLogin(&reqUserLogin,1);
}
void CZQMD::OnFrontDisconnected(int nReason)
{
	g_pLog->printLog("行情连接断开!\n");
}
void CZQMD::OnRspUserLogin(CZQThostFtdcRspUserLoginField *pRspUserLogin, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
	g_pLog->printLog("行情登陆成功!\n");
    initSubMD();
}
void CZQMD::SubscribeMD(char* Instrument, char* ExchangeID)
{
	//订阅行情
	char** Instruments;
    Instruments=new char*[0];
    Instruments[0]= new char[strlen(Instrument)];
    strcpy(Instruments[0], Instrument);
	m_pMdApi->SubscribeMarketData (Instruments, 1, ExchangeID);
}
void CZQMD::initSubMD()
{
        //等待交易类中订阅合约列表刷新完成
	while (!g_pTdHandler->bIsgetInst)
	{
	    Sleep(100);
	}
    //根据合约列表订阅行情
	for (int i=0; i<g_pTdHandler->g_Instnum; i++)
	{
        SubscribeMD(g_pTdHandler->g_pInstinfo[i].InstrumentID, g_pTdHandler->g_pInstinfo[i].ExchangeID);
	}
    //建立事件并等待3秒行情反馈
    g_pLog->printLog("等待行情反馈...\n");
    g_pLog->printLog("MarketData:合约,日期,时间,昨日收盘价,开盘价,最新价,涨停价,跌停价,买一价,卖一价\n");
    HANDLE g_hEvent = CreateEvent(NULL, true, false, NULL);	
	//WaitForSingleObject(g_hEvent, INFINITE);
    WaitForSingleObject(g_hEvent, 3000);
}

void CZQMD::OnRtnDepthMarketData(CZQThostFtdcDepthMarketDataField *pDepthMarketData) 
{
	memcpy(&LastDepth[pDepthMarketData->InstrumentID], pDepthMarketData, sizeof(*pDepthMarketData));
    //g_pLog->printLog("%s,%s\n", pDepthMarketData->InstrumentID, pDepthMarketData->TradingDay);
    if (strcmp(pDepthMarketData->InstrumentID,"159901") == 0 ||
		strcmp(pDepthMarketData->InstrumentID, "000001") == 0 ||
        strcmp(pDepthMarketData->InstrumentID, "510050") == 0 ||
        strcmp(pDepthMarketData->InstrumentID, "510051") == 0 ||
        strcmp(pDepthMarketData->InstrumentID, "600030") == 0 ||
		strcmp(pDepthMarketData->InstrumentID, "601377") == 0 ||
		strcmp(pDepthMarketData->InstrumentID, "132002") == 0 ||
		strcmp(pDepthMarketData->InstrumentID, "502048") == 0 ||
		strcmp(pDepthMarketData->InstrumentID, "002241") == 0)
    {
        g_pLog->printLog("MarketData:%s,%s,%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%d\n",pDepthMarketData->InstrumentID,
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
   /* g_pLog->printLog("MarketData:%s,%s,%.3f,%.3f,%.3f,%.3f\n",pDepthMarketData->InstrumentID,
              pDepthMarketData->UpdateTime,
              pDepthMarketData->OpenPrice,
              pDepthMarketData->LastPrice,
              pDepthMarketData->AskPrice1,
              pDepthMarketData->BidPrice1);
    */
    //printf(".");
}
