// ZQTD.cpp: implementation of the CZQTD class.
//
//////////////////////////////////////////////////////////////////////
#include <iostream>
#include "stdafx.h"
#include "ZQTD.h"

using namespace std;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CZQTD::CZQTD()
{
    
}

CZQTD::~CZQTD()
{

}

void CZQTD::Init(const char* pi_TDAdress, const char* pi_User, const char* pi_pwd, logInfo* pLog)
{
    zq_TDAddress = new char[strlen(pi_TDAdress)];
    strcpy(zq_TDAddress, pi_TDAdress);
    zq_UserID = new char[strlen(pi_User)];
    strcpy(zq_UserID, pi_User);
    zq_Password = new char[strlen(pi_pwd)];
    strcpy(zq_Password, pi_pwd);
	g_pLog = pLog;

    g_Instnum = 0;
	nRequestID = 0;
    bIsgetInst = false;
    bIsgetPosDetail = false;
	// 产生一个CThostFtdcTraderApi实例
	m_pTdApi = CZQThostFtdcTraderApi::CreateFtdcTraderApi("");

	// 注册一事件处理的实例
	m_pTdApi->RegisterSpi(this);

    // 订阅公共流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后公共流的内容
	m_pTdApi->SubscribePublicTopic(ZQTHOST_TERT_RESUME);//(ZQTHOST_TERT_RESTART);

    // 订阅私有流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后私有流的内容
	m_pTdApi->SubscribePrivateTopic(ZQTHOST_TERT_RESTART);
	
	// 设置交易托管系统服务的地址，可以注册多个地址备用
	m_pTdApi->RegisterFront(zq_TDAddress);//此处tcp连接方式需标明，不可直接用IP。

	// 使客户端开始与后台服务建立连接
	m_pTdApi->Init();
}
void CZQTD::GetSysTime()
{
    SYSTEMTIME sys;
    GetLocalTime(&sys); 
    sprintf(systime,"%02d:%02d:%02d.%03d",sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds);
}
void CZQTD::OnFrontConnected()
{
	CZQThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID,"2011");//此处2011不要改
	strcpy_s(reqUserLogin.UserID, zq_UserID);//输入自己的帐号
	strcpy_s(reqUserLogin.Password, zq_Password);//输入密码
	int login=m_pTdApi->ReqUserLogin(&reqUserLogin, 1);//登录
	g_pLog->printLog("交易登录请求完毕,交易登录请求返回值%d\n",login); 
}
void CZQTD::OnFrontDisconnected (int nReason)
{
    g_pLog->printLog("交易连接断开！原因代码%d\n",nReason);
}

void CZQTD::OnRspUserLogin(CZQThostFtdcRspUserLoginField *pRspUserLogin, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    cerr<<"--------------交易登录响应---------------"<<endl;
    g_pLog->printLog("交易登录响应及结果ID:%d,Msg:%s,Date:%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg,pRspUserLogin->TradingDay);
    QryInstruments();
    //CZQThostFtdcQryExchangeField pQryExchange;
    //memset(&pQryExchange,0,sizeof(pQryExchange));
    //int ex_rtn=m_pTdApi->ReqQryExchange(&pQryExchange,0);
    //printf("查询交易所信息请求完毕，返回值%d\n",ex_rtn);
}

void CZQTD::PassChange(const char* newpass)
{
    char l_setpass[8];
    memset(&l_setpass,0,sizeof(l_setpass));
    if (strcmp(newpass,"")==0)
    {
        printf("Please Input new password:");
        scanf("%8s",l_setpass);
    }
    else
    {
        strcpy_s(l_setpass, newpass);
    }
    printf("\nThe new passowrd is %s", l_setpass);
    //修改登录口令
	CZQThostFtdcUserPasswordUpdateField password;
	memset(&password,0,sizeof(password));
	strcpy_s(password.BrokerID,"2011");
	strcpy_s(password.UserID, zq_UserID);
	strcpy_s(password.OldPassword, zq_Password);
	strcpy_s(password.NewPassword, l_setpass);
	int ex_rtn=m_pTdApi->ReqUserPasswordUpdate(&password,1);
    g_pLog->printLog("更改密码请求完毕，返回值%d\n",ex_rtn);
}

void CZQTD::CancelAll()
{

}
void CZQTD::QryInstruments()
{
    //初始化合约列表
    g_Instnum = 0;
    memset(&g_pInstinfo, 0, sizeof(g_pInstinfo));
    //获得合约列表
	CZQThostFtdcQryInstrumentField qryField;
    memset(&qryField, 0, sizeof(qryField));
    int resCode = m_pTdApi-> ReqQryInstrument(&qryField, 0);
	g_pLog->printLog("合约列表订阅请求完毕，返回值：%d\n",resCode);	
}

void CZQTD::OnRspQryInstrument(CZQThostFtdcInstrumentField *pInstrument, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryInstrument", pRspInfo);
	if (
		(pInstrument!=NULL) 
     && ((((pInstrument->ProductClass == '6') ||
         (pInstrument->ProductClass == '8') ||
         (pInstrument->ProductClass == '9'))
     && ((strcmp(pInstrument->ProductID, "SHA") == 0) ||
		 (strcmp(pInstrument->ProductID, "SZA") == 0) ||
		 (strcmp(pInstrument->ProductID, "SHCYB") == 0) ||
		 (strcmp(pInstrument->ProductID, "SZCYB") == 0) ||
         (strcmp(pInstrument->ProductID, "SHETF") == 0) ||
         (strcmp(pInstrument->ProductID, "SZETF") == 0) ||
         (strcmp(pInstrument->ProductID, "SHBONDS") == 0) ||
         (strcmp(pInstrument->ProductID, "SZBONDS") == 0)))
         || (strcmp(pInstrument->InstrumentID, "204001") == 0)
         || (strcmp(pInstrument->InstrumentID, "131810") == 0)
         || (strcmp(pInstrument->InstrumentID, "000001") == 0)
         || (strcmp(pInstrument->InstrumentID, "159901") == 0)
         || (strcmp(pInstrument->InstrumentID, "510050") == 0)
         || (strcmp(pInstrument->InstrumentID, "510051") == 0)
         || (strcmp(pInstrument->InstrumentID, "502048") == 0)
        )
	   )
	{
    	g_Instnum++;
	    g_pInstinfo[g_Instnum-1] = *pInstrument;
        if (strcmp(pInstrument->InstrumentID, "510051") == 0) g_pLog->printLog("Instrument:%s;Name:%s;TypeID:%s;Type:%c;Max:%d;Status:%c;PriceTick=%.5f;VolumeMultiple=%d\n",g_pInstinfo[g_Instnum-1].InstrumentID,g_pInstinfo[g_Instnum-1].InstrumentName,g_pInstinfo[g_Instnum-1].ProductID,g_pInstinfo[g_Instnum-1].ProductClass,g_pInstinfo[g_Instnum-1].MaxMarketOrderVolume,g_pInstinfo[g_Instnum-1].InstrumentStatusFlag,g_pInstinfo[g_Instnum-1].PriceTick,g_pInstinfo[g_Instnum-1].VolumeMultiple);
	}
	if(bIsLast)
	{
		g_pLog->printLog("取得沪深A股股票列表数量:%d\n", g_Instnum);
		bIsgetInst = true;
	}
    return;
}
void CZQTD::OnRspQryInvestorPosition(CZQThostFtdcInvestorPositionField *pInvestorPosition, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryInvestorPosition", pRspInfo);
    if (pInvestorPosition != NULL)
    {
        g_pLog->printLog("合约代码:%s", pInvestorPosition->InstrumentID);
        g_pLog->printLog("持仓多空方向:%c", pInvestorPosition->PosiDirection);
        g_pLog->printLog("投机套保标志:%c", pInvestorPosition->HedgeFlag);
        g_pLog->printLog("持仓日期:%c", pInvestorPosition->PositionDate);
        g_pLog->printLog("上日持仓:%d", pInvestorPosition->YdPosition);
        g_pLog->printLog("今日持仓:%d", pInvestorPosition->Position);
        g_pLog->printLog("多头冻结:%d", pInvestorPosition->LongFrozen);
        g_pLog->printLog("空头冻结:%d", pInvestorPosition->ShortFrozen);
        g_pLog->printLog("开仓量:%d", pInvestorPosition->OpenVolume);
        g_pLog->printLog("平仓量:%d", pInvestorPosition->CloseVolume);
        g_pLog->printLog("开仓金额:%.2f", pInvestorPosition->OpenAmount);
        g_pLog->printLog("平仓金额:%.2f", pInvestorPosition->CloseAmount);
        g_pLog->printLog("持仓成本:%.3f", pInvestorPosition->PositionCost);
        g_pLog->printLog("手续费:%.2f", pInvestorPosition->Commission);
        g_pLog->printLog("平仓盈亏:%.2f", pInvestorPosition->CloseProfit);
        g_pLog->printLog("持仓盈亏:%.2f", pInvestorPosition->PositionProfit);
        g_pLog->printLog("交易日:%s", pInvestorPosition->TradingDay);
        g_pLog->printLog("开仓成本:%.2f", pInvestorPosition->OpenCost);
        g_pLog->printLog("今日持仓:%d", pInvestorPosition->TodayPosition);
        g_pLog->printLog("过户费:%.3f", pInvestorPosition->TransferFee);
        g_pLog->printLog("印花税:%.3f", pInvestorPosition->StampTax);
        g_pLog->printLog("证券价值:%.2f", pInvestorPosition->StockValue);
        g_pLog->printLog("交易所代码:%s\n", pInvestorPosition->ExchangeID);
    }
    if(bIsLast)
	{
		bIsgetInst = true;
	}
    return;
}
void CZQTD::OnRspQryInvestorPositionDetail(CZQThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pInvestorPositionDetail != NULL)
    {
        g_pLog->printLog("合约代码:%s;", pInvestorPositionDetail->InstrumentID);
        g_pLog->printLog("买卖:%c;", pInvestorPositionDetail->Direction);
        g_pLog->printLog("开仓日期:%s;",pInvestorPositionDetail->OpenDate);
        g_pLog->printLog("成交编号:%s;",pInvestorPositionDetail->TradeID);
        g_pLog->printLog("数量:%d;",pInvestorPositionDetail->Volume);
        g_pLog->printLog("开仓价:%f;",pInvestorPositionDetail->OpenPrice);
        g_pLog->printLog("交易日:%s;",pInvestorPositionDetail->TradingDay);
        g_pLog->printLog("成交类型:%c;",pInvestorPositionDetail->TradeType);
        g_pLog->printLog("交易所代码:%s;",pInvestorPositionDetail->ExchangeID);
        g_pLog->printLog("投资者保证金:%f;",pInvestorPositionDetail->Margin);
        g_pLog->printLog("交易所保证金:%f;",pInvestorPositionDetail->ExchMargin);
        g_pLog->printLog("昨结算价:%f;",pInvestorPositionDetail->LastSettlementPrice);
        g_pLog->printLog("结算价:%f;",pInvestorPositionDetail->SettlementPrice);
        g_pLog->printLog("平仓量:%d;",pInvestorPositionDetail->CloseVolume);
        g_pLog->printLog("平仓金额:%f;",pInvestorPositionDetail->CloseAmount);
        g_pLog->printLog("过户费:%f;",pInvestorPositionDetail->TransferFee);
        g_pLog->printLog("印花税:%f;",pInvestorPositionDetail->StampTax);
        g_pLog->printLog("手续费:%f;\n",pInvestorPositionDetail->Commission);
    }
    else
    {
        g_pLog->printLog("持仓明细查询未能返回数据。\n");
    }
	if(bIsLast)
	{
		bIsgetInst = true;
	}
    return;
}
void CZQTD::OnRspQryExchange(CZQThostFtdcExchangeField *pExchange, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pExchange != NULL)
    {
        g_pLog->printLog("交易所代码:%s;", pExchange->ExchangeID);
        g_pLog->printLog("交易所名称:%s;", pExchange->ExchangeName);
        g_pLog->printLog("交易所属性:%c;\n",pExchange->ExchangeProperty);
    }
    else
    {
        g_pLog->printLog("交易所查询未能返回数据。\n");
    }
    return;
}

void CZQTD::QueryPositionDetail()
{
   CZQThostFtdcQryInvestorPositionDetailField qryPosDetail;
   memset(&qryPosDetail, 0, sizeof(qryPosDetail));
   bIsgetPosDetail = false;
   strcpy_s(qryPosDetail.BrokerID,"2011");
   strcpy_s(qryPosDetail.ExchangeID,"");
   strcpy_s(qryPosDetail.InstrumentID,"");
   strcpy_s(qryPosDetail.InvestorID,zq_UserID);
   int resCode = m_pTdApi-> ReqQryInvestorPositionDetail(&qryPosDetail, 0);
   g_pLog->printLog("持仓明细查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::QueryPosition()
{
   CZQThostFtdcQryInvestorPositionField qryPos;
   memset(&qryPos, 0, sizeof(qryPos));
   strcpy_s(qryPos.BrokerID,"2011");
   strcpy_s(qryPos.InvestorID, zq_UserID);
   strcpy_s(qryPos.InstrumentID,"");
   int resCode = m_pTdApi-> ReqQryInvestorPosition(&qryPos, 0);
   g_pLog->printLog("持仓查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::QueryOrders()
{
   CZQThostFtdcQryOrderField qryOrder;
   memset(&qryOrder,0,sizeof(qryOrder));
   strcpy_s(qryOrder.BrokerID,"2011");
   strcpy_s(qryOrder.InvestorID, zq_UserID);
   strcpy_s(qryOrder.ExchangeID, "");
   strcpy_s(qryOrder.InstrumentID,"");
   strcpy_s(qryOrder.OrderSysID,"");
   strcpy_s(qryOrder.InsertTimeStart,"");
   strcpy_s(qryOrder.InsertTimeEnd,"");
   //strcpy_s(qryOrder.ExchangeID, "SSE");
   //strcpy_s(qryOrder.InstrumentID,"600016");
   //strcpy_s(qryOrder.OrderSysID,"N537");
   //strcpy_s(qryOrder.InsertTimeStart,"09:00:00");
   //strcpy_s(qryOrder.InsertTimeEnd,"15:00:00");

   int resCode = m_pTdApi->ReqQryOrder(&qryOrder, 0);
   g_pLog->printLog("委托查询:ExchangeID:%s;InstrumentID:%s;OrderSysID:%s;InsertTimeStart:%s;InsertTimeEnd:%s\n",
          qryOrder.ExchangeID,qryOrder.InstrumentID,qryOrder.OrderSysID,qryOrder.InsertTimeStart,qryOrder.InsertTimeEnd);	
   g_pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::QueryTrades()
{
   CZQThostFtdcQryTradeField qryTrade;
   memset(&qryTrade,0,sizeof(qryTrade));
   strcpy_s(qryTrade.BrokerID,"2011");
   strcpy_s(qryTrade.InvestorID, zq_UserID);
   strcpy_s(qryTrade.InstrumentID,"");
   strcpy_s(qryTrade.ExchangeID, "");
   strcpy_s(qryTrade.TradeID,"");
   strcpy_s(qryTrade.TradeTimeStart,"");
   strcpy_s(qryTrade.TradeTimeEnd,"");
   //strcpy_s(qryOrder.ExchangeID, "SSE");
   //strcpy_s(qryOrder.InstrumentID,"600016");
   //strcpy_s(qryOrder.OrderSysID,"N537");
   //strcpy_s(qryOrder.InsertTimeStart,"09:00:00");
   //strcpy_s(qryOrder.InsertTimeEnd,"15:00:00");

   int resCode = m_pTdApi->ReqQryTrade(&qryTrade, 0);
   g_pLog->printLog("成交查询:ExchangeID:%s;InstrumentID:%s;TradeID:%s;TradeTimeStart:%s;TradeTimeEnd:%s\n",
          qryTrade.ExchangeID,qryTrade.InstrumentID,qryTrade.TradeID,qryTrade.TradeTimeStart,qryTrade.TradeTimeEnd);	
   g_pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::QueryMD()
{
   CZQThostFtdcQryDepthMarketDataField qryDepthMarketData;
   memset(&qryDepthMarketData,0,sizeof(qryDepthMarketData));
   strcpy_s(qryDepthMarketData.InstrumentID ,"002734");
   int resCode = m_pTdApi->ReqQryDepthMarketData(&qryDepthMarketData, 0);
   g_pLog->printLog("行情查询:InstrumentID:%s\n",
          qryDepthMarketData.InstrumentID);	
   g_pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::QueryAcct()
{
   CZQThostFtdcQryTradingAccountField qryTradingAccount;
   memset(&qryTradingAccount,0,sizeof(qryTradingAccount));
   strcpy_s(qryTradingAccount.BrokerID, "2011");
   strcpy_s(qryTradingAccount.InvestorID, zq_UserID);
   int resCode = m_pTdApi->ReqQryTradingAccount(&qryTradingAccount, 0);
   g_pLog->printLog("账户查询:InvestorID:%s\n",
          qryTradingAccount.InvestorID);	
   g_pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CZQTD::prtErr(const char* pFuncID, CZQThostFtdcRspInfoField *pRspInfo)
{
    if (pRspInfo==NULL)
    {
        return; //g_pLog->printLog("%s: pRspInfo = NULL\n", pFuncID);
    }
    else
    {
        g_pLog->printLog("%s:返回代码:%d,Msg:%s\n", pFuncID, pRspInfo->ErrorID,pRspInfo->ErrorMsg);
    }
    return;
}
void CZQTD::PlaceOrder(const char* pi_Instrument, 
                       const char* pi_ExchangeID,
                       const char* pi_BuyOrSell,
                       const char* pi_Price,
                       int pi_Position)
{
	CZQThostFtdcInputOrderField pInputOrder;
    memset(&pInputOrder,0,sizeof(pInputOrder));
    CZQThostFtdcInputOrderField * pIptOrdFld=&pInputOrder;

    strcpy_s(pIptOrdFld->BrokerID,"2011");    //经纪商代码
	strcpy_s(pIptOrdFld->InvestorID, zq_UserID); //投资者代码
	//strcpy_s(pIptOrdFld->UserID, zq_UserID); //投资者代码
	strcpy_s(pIptOrdFld->InstrumentID, pi_Instrument); //合约代码
    strcpy_s(pIptOrdFld->ExchangeID, pi_ExchangeID); //交易所代码 //SSE上交所 SZE深交所
    pIptOrdFld->OrderPriceType=THOST_FTDC_OPT_LimitPrice; //报单价格条件 //这里注意，SSE或者SZE不支持AnyPrice
    if (strcmp(pi_BuyOrSell, "B")==0||
        strcmp(pi_BuyOrSell, "BUY")==0)
    {
        pIptOrdFld->Direction=THOST_FTDC_D_Buy; //买卖方向
    }
    else if (strcmp(pi_BuyOrSell, "S")==0||
        strcmp(pi_BuyOrSell, "SELL")==0)
    {
        pIptOrdFld->Direction=THOST_FTDC_D_Sell; //买卖方向
    }
    else if (strcmp(pi_BuyOrSell, "P")==0||
        strcmp(pi_BuyOrSell, "ETFPUR")==0)
    {
        pIptOrdFld->Direction=THOST_FTDC_D_ETFPur; //买卖方向
    }
    else if (strcmp(pi_BuyOrSell, "R")==0||
        strcmp(pi_BuyOrSell, "ETFRED")==0)
    {
        pIptOrdFld->Direction=THOST_FTDC_D_ETFRed; //买卖方向
    }

    pIptOrdFld->VolumeTotalOriginal=pi_Position; //数量
    pIptOrdFld->TimeCondition = THOST_FTDC_TC_GFD;  //有效期类型 //当日有效
    pIptOrdFld->VolumeCondition=THOST_FTDC_VC_AV; //成交量类型
    pIptOrdFld->ContingentCondition = THOST_FTDC_CC_Immediately; //触发条件
    pIptOrdFld->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;  //强平原因
    strcpy_s(pIptOrdFld->LimitPrice, pi_Price);
    //如果一次控制台程序启动后，多次使用ReqOrderInsert报单，此处的nRequestID要写成单调递增的，如nRequestID++，初始为1
	nRequestID++;

    SYSTEMTIME sys;
    GetLocalTime(&sys); 
    sprintf(systime,"%02d%02d%02d%03d",sys.wHour,sys.wMinute, sys.wSecond, sys.wMilliseconds);
    
    //pIptOrdFld->RequestID = nRequestID;
    sprintf(pIptOrdFld->OrderRef,"%s%03d", systime, nRequestID);

    int resCode=m_pTdApi->ReqOrderInsert(pIptOrdFld, nRequestID);
	GetSysTime();
	g_pLog->printLog("报单-%s:合约:%s;交易所:%s;买卖:%c;价格:%s;数量:%d;报单初始返回值:%d;OrderRef:%s;RequestID:%d\n",
           systime,
           pIptOrdFld->InstrumentID,
           pIptOrdFld->ExchangeID,
           pIptOrdFld->Direction,
           pIptOrdFld->LimitPrice,
           pIptOrdFld->VolumeTotalOriginal,
           resCode,
		   pIptOrdFld->OrderRef,
           pIptOrdFld->RequestID);	
}

void CZQTD::OnRspOrderInsert(CZQThostFtdcInputOrderField *pInputOrder, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
     prtErr("OnRspOrderInsert", pRspInfo);
     GetSysTime();
     g_pLog->printLog("OnRspOrderInsert-%s:合约:%s;报单返回:%s;Ref:%s;RequestID:%d\n",systime,pInputOrder->InstrumentID,pRspInfo->ErrorMsg,pInputOrder->OrderRef,pInputOrder->RequestID);
     return;
}
void CZQTD::OnRspQryOrder(CZQThostFtdcOrderField *pOrder, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryOrder", pRspInfo);
    if(pOrder!=NULL)
    {
		GetSysTime();
		g_pLog->printLog("OnRspQryOrder-%s:合约:%s;买卖:%c;报单状态:%s;RequestID:%d;OrderRef:%s;LocalOrderID:%s\n", systime, pOrder->InstrumentID, pOrder->Direction, pOrder->StatusMsg, pOrder->RequestID, pOrder->OrderRef, pOrder->OrderLocalID);
        g_pLog->printLog("OnRspQryOrder:BrokerID:%s;InvestorID:%s;ExchangeID:%s;SessionID:%d;OrderSysID:%s;InsertTime:%s\n",pOrder->BrokerID,pOrder->InvestorID,pOrder->ExchangeID,pOrder->SessionID,pOrder->OrderSysID,pOrder->InsertTime);
    }
	return;
}
void CZQTD::OnRspQryTrade(CZQThostFtdcTradeField *pTrade, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryTrade", pRspInfo);
    if(pTrade!=NULL)
    {
		GetSysTime();
		g_pLog->printLog("OnRspQryTrade-%s:合约:%s;成交价:%s;成交时间:%s\n", systime, pTrade->InstrumentID, pTrade->Price, pTrade->TradeTime);
    }
	return;
}
void CZQTD::OnRspQryDepthMarketData(CZQThostFtdcDepthMarketDataField *pDepthMarketData, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryDepthMarketData", pRspInfo);
    if(pDepthMarketData!=NULL)
    {
        g_pLog->printLog("OnRspQryDepthMarketData:合约:%s;开盘价:%f\n",pDepthMarketData->InstrumentID,pDepthMarketData->OpenPrice);
    }
	return;
}
void CZQTD::OnRspQryTradingAccount(CZQThostFtdcTradingAccountField *pTradingAccount, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryTradingAccount", pRspInfo);
    if(pTradingAccount!=NULL)
    {
        g_pLog->printLog("OnRspQryTradingAccount:StockValue=%.2f;Available=%.2f;WithdrawQuota=%.2f\n",pTradingAccount->StockValue,pTradingAccount->Available,pTradingAccount->WithdrawQuota);
    }
    return;
}
void CZQTD::OnRspQryTradingCode(CZQThostFtdcTradingCodeField *pTradingCode, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	g_pLog->printLog("InvestorID:%d   ClientID:%d\n",pTradingCode->InvestorID,pTradingCode->ClientID);
	return;
}
void CZQTD::OnRspQryInstrumentCommissionRate(CZQThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	return;
}
//ErrRSP&Rtn/////////////////////////////////////////////////////////////////////
void CZQTD::OnRspError(CZQThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	return;
}
void CZQTD::OnErrRtnOrderInsert(CZQThostFtdcInputOrderField *pInputOrder, CZQThostFtdcRspInfoField *pRspInfo)
{
	return;
}
void CZQTD::OnErrRtnOrderAction(CZQThostFtdcOrderActionField *pOrderAction, CZQThostFtdcRspInfoField *pRspInfo)
{
	return;
}
//Rtn/////////////////////////////////////////////////////////////////////
void CZQTD::OnRtnOrder(CZQThostFtdcOrderField *pOrder)
{
    if(pOrder!=NULL)
    {
        GetSysTime();
        g_pLog->printLog("OnRtnOrder-%s:合约:%s;买卖:%c;报单状态:%s;OrderStatus:%c;RequestID:%d;OrderRef:%s;LocalOrderID:%s\n",systime,pOrder->InstrumentID,pOrder->Direction,pOrder->StatusMsg,pOrder->OrderStatus,pOrder->RequestID,pOrder->OrderRef,pOrder->OrderLocalID);
        g_pLog->printLog("OnRtnOrder-%s:BrokerID:%s;InvestorID:%s;ExchangeID:%s;SessionID:%d;OrderSysID:%s;InsertTime:%s\n",systime,pOrder->BrokerID,pOrder->InvestorID,pOrder->ExchangeID,pOrder->SessionID,pOrder->OrderSysID,pOrder->InsertTime);
    }
	return;
}
void CZQTD::OnRtnTrade(CZQThostFtdcTradeField *pTrade)
{
	if (pTrade != NULL)
	{
		GetSysTime();
		//g_pLog->printLog("OnRtnTrade-%s:合约:%s;价格:%s;OrderLocalID:%s\n",systime,pTrade->InstrumentID,pTrade->Price,pTrade->OrderLocalID);
		g_pLog->printLog("OnRtnTrade-%s:", systime);
		g_pLog->printLog("经纪公司代码:%s;", pTrade->BrokerID);
		g_pLog->printLog("投资者代码:%s;", pTrade->InvestorID);
		g_pLog->printLog("合约代码:%s;", pTrade->InstrumentID);
		g_pLog->printLog("报单引用:%s;", pTrade->OrderRef);
		g_pLog->printLog("用户代码:%s;", pTrade->UserID);
		g_pLog->printLog("交易所代码:%s;", pTrade->ExchangeID);
		g_pLog->printLog("成交编号:%s;", pTrade->TradeID);
		g_pLog->printLog("买卖方向:%c;", pTrade->Direction);
		g_pLog->printLog("报单编号:%s;", pTrade->OrderSysID);
		g_pLog->printLog("会员代码:%s;", pTrade->ParticipantID);
		g_pLog->printLog("客户代码:%s;", pTrade->ClientID);
		g_pLog->printLog("交易角色:%c;", pTrade->TradingRole);
		g_pLog->printLog("合约在交易所的代码:%s;", pTrade->ExchangeInstID);
		g_pLog->printLog("开平标志:%c;", pTrade->OffsetFlag);
		g_pLog->printLog("投机套保标志:%c;", pTrade->HedgeFlag);
		g_pLog->printLog("价格:%s;", pTrade->Price);
		g_pLog->printLog("数量:%d;", pTrade->Volume);
		g_pLog->printLog("成交时期:%s;", pTrade->TradeDate);
		g_pLog->printLog("成交时间:%s;", pTrade->TradeTime);
		g_pLog->printLog("成交类型:%c;", pTrade->TradeType);
		g_pLog->printLog("成交价来源:%c;", pTrade->PriceSource);
		g_pLog->printLog("交易所交易员代码:%s;", pTrade->TraderID);
		g_pLog->printLog("本地报单编号:%s;", pTrade->OrderLocalID);
		g_pLog->printLog("结算会员编号:%s;", pTrade->ClearingPartID);
		g_pLog->printLog("业务单元:%s;", pTrade->BusinessUnit);
		g_pLog->printLog("序号:%d;", pTrade->SequenceNo);
		g_pLog->printLog("交易日:%s;", pTrade->TradingDay);
		g_pLog->printLog("结算编号:%d;", pTrade->SettlementID);
		g_pLog->printLog("经纪公司报单编号:%d;\n", pTrade->BrokerOrderSeq);
	}
	return;
}
void CZQTD::OnRtnInstrumentStatus(CZQThostFtdcInstrumentStatusField *pInstrumentStatus)
{
    g_pLog->printLog("OnRtnInstrumentStatus:%s,%c",pInstrumentStatus->InstrumentID,pInstrumentStatus->InstrumentStatus);
    return;
}
/////////////////////////////////////////////////////////////////////////////////////
double CZQTD::getPrice(TZQThostFtdcStockPriceType price)
{
	return sizeof(price) / sizeof(price[0]);
}