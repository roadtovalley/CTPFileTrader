#include <iostream>
#include "stdafx.h"
#include "FTTD.h"

using namespace std;

CFTTD::CFTTD()
{
    
}

CFTTD::~CFTTD()
{

}

void CFTTD::Init(const char* pi_BrokerID, const char* pi_TDAdress, const char* pi_User, const char* pi_pwd, logInfo *pi_log)
{
    memset(qh_BrokerID, 0, sizeof(qh_BrokerID));
    memset(qh_TDAddress, 0, sizeof(qh_TDAddress));
    memset(qh_UserID, 0, sizeof(qh_UserID));
    memset(qh_Password, 0, sizeof(qh_Password));
	strcpy(qh_BrokerID, pi_BrokerID);
    strcpy(qh_TDAddress, pi_TDAdress);
    strcpy(qh_UserID, pi_User);
    strcpy(qh_Password, pi_pwd);
	pLog = pi_log;

    g_Instnum = 0;
	nRequestID = 0;
    bIsgetInst = false;
    bIsgetPosDetail = false;
	// 产生一个CThostFtdcTraderApi实例
	m_pTdApi = CThostFtdcTraderApi::CreateFtdcTraderApi("");

	// 注册一事件处理的实例
	m_pTdApi->RegisterSpi(this);

    // 订阅公共流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后公共流的内容
	m_pTdApi->SubscribePublicTopic(THOST_TERT_RESUME);//(THOST_TERT_RESTART);

    // 订阅私有流
	//        TERT_RESTART:从本交易日开始重传
	//        TERT_RESUME:从上次收到的续传
	//        TERT_QUICK:只传送登录后私有流的内容
	m_pTdApi->SubscribePrivateTopic(THOST_TERT_RESTART);
	
	// 设置交易托管系统服务的地址，可以注册多个地址备用
	m_pTdApi->RegisterFront(qh_TDAddress);//此处tcp连接方式需标明，不可直接用IP。

	// 使客户端开始与后台服务建立连接
	m_pTdApi->Init();
}
void CFTTD::GetSysTime()
{
    SYSTEMTIME sys;
    GetLocalTime(&sys); 
    sprintf(systime,"%02d:%02d:%02d.%03d",sys.wHour,sys.wMinute, sys.wSecond,sys.wMilliseconds);
}
void CFTTD::OnFrontConnected()
{
	CThostFtdcReqUserLoginField reqUserLogin;
	memset(&reqUserLogin,0,sizeof(reqUserLogin));
	strcpy_s(reqUserLogin.BrokerID,qh_BrokerID);//此处2011不要改
	strcpy_s(reqUserLogin.UserID, qh_UserID);//输入自己的帐号
	strcpy_s(reqUserLogin.Password, qh_Password);//输入密码
	int login=m_pTdApi->ReqUserLogin(&reqUserLogin, 1);//登录
	pLog->printLog("交易登录请求完毕,交易登录请求返回值%d\n",login); 
}
void CFTTD::OnFrontDisconnected (int nReason)
{
    pLog->printLog("OnFrontDisconnected: 交易连接断开！原因代码:%d\n",nReason);
}

void CFTTD::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    pLog->printLog("OnRspUserLogin: 交易登录响应及结果ID:%d,Msg:%s,Date:%s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg,pRspUserLogin->TradingDay);
    QryInstruments();
}

void CFTTD::PassChange(const char* newpass)
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
	CThostFtdcUserPasswordUpdateField password;
	memset(&password,0,sizeof(password));
	strcpy_s(password.BrokerID,qh_BrokerID);
	strcpy_s(password.UserID, qh_UserID);
	strcpy_s(password.OldPassword, qh_Password);
	strcpy_s(password.NewPassword, l_setpass);
	int ex_rtn=m_pTdApi->ReqUserPasswordUpdate(&password,1);
    pLog->printLog("更改密码请求完毕，返回值%d\n",ex_rtn);
}

void CFTTD::CancelAll()
{

}
void CFTTD::QryInstruments()
{
    //初始化合约列表
    g_Instnum = 0;
    memset(&g_pInstinfo, 0, sizeof(g_pInstinfo));
    //获得合约列表
	CThostFtdcQryInstrumentField qryField;
    memset(&qryField, 0, sizeof(qryField));
    int resCode = m_pTdApi-> ReqQryInstrument(&qryField, 0);
	pLog->printLog("合约列表订阅请求完毕，返回值：%d\n",resCode);	
}

void CFTTD::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryInstrument", pRspInfo);
	if (
		(pInstrument!=NULL) 
     && ((strcmp(pInstrument->ExchangeID, "CFFEX") == 0)
	  || (strcmp(pInstrument->ExchangeID, "CZCE") == 0)
	  || (strcmp(pInstrument->ExchangeID, "SHFE") == 0)
	  || (strcmp(pInstrument->ExchangeID, "DCE") == 0)
	    )
     && ((pInstrument->ProductClass == '1')
	    )
	   )
	{
    	g_Instnum++;
	    g_pInstinfo[g_Instnum-1] = *pInstrument;
	}
	if(bIsLast)
	{
		pLog->printLog("取得期货合约列表数量:%d\n", g_Instnum);
		bIsgetInst = true;
	}
    return;
}
void CFTTD::OnRspQryInvestorPosition(CThostFtdcInvestorPositionField *pInvestorPosition, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryInvestorPosition", pRspInfo);
    if (pInvestorPosition != NULL)
    {
        pLog->printLog("合约代码:%s", pInvestorPosition->InstrumentID);
        pLog->printLog("持仓多空方向:%c", pInvestorPosition->PosiDirection);
        pLog->printLog("投机套保标志:%c", pInvestorPosition->HedgeFlag);
        pLog->printLog("持仓日期:%c", pInvestorPosition->PositionDate);
        pLog->printLog("上日持仓:%d", pInvestorPosition->YdPosition);
        pLog->printLog("今日持仓:%d", pInvestorPosition->Position);
        pLog->printLog("多头冻结:%d", pInvestorPosition->LongFrozen);
        pLog->printLog("空头冻结:%d", pInvestorPosition->ShortFrozen);
		pLog->printLog("开仓量:%d", pInvestorPosition->OpenVolume);
        pLog->printLog("平仓量:%d", pInvestorPosition->CloseVolume);
        pLog->printLog("开仓金额:%.2f", pInvestorPosition->OpenAmount);
        pLog->printLog("平仓金额:%.2f", pInvestorPosition->CloseAmount);
        pLog->printLog("持仓成本:%.3f", pInvestorPosition->PositionCost);
        pLog->printLog("手续费:%.2f", pInvestorPosition->Commission);
        pLog->printLog("平仓盈亏:%.2f", pInvestorPosition->CloseProfit);
        pLog->printLog("持仓盈亏:%.2f", pInvestorPosition->PositionProfit);
        pLog->printLog("交易日:%s", pInvestorPosition->TradingDay);
        pLog->printLog("开仓成本:%.2f", pInvestorPosition->OpenCost);
        pLog->printLog("今日持仓:%d", pInvestorPosition->TodayPosition);
    }
    if(bIsLast)
	{
		bIsgetInst = true;
	}
    return;
}
void CFTTD::OnRspQryInvestorPositionDetail(CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pInvestorPositionDetail != NULL)
    {
        pLog->printLog("合约代码:%s;", pInvestorPositionDetail->InstrumentID);
        pLog->printLog("买卖:%c;", pInvestorPositionDetail->Direction);
        pLog->printLog("开仓日期:%s;",pInvestorPositionDetail->OpenDate);
        pLog->printLog("成交编号:%s;",pInvestorPositionDetail->TradeID);
        pLog->printLog("数量:%d;",pInvestorPositionDetail->Volume);
        pLog->printLog("开仓价:%.4f;",pInvestorPositionDetail->OpenPrice);
        pLog->printLog("交易日:%s;",pInvestorPositionDetail->TradingDay);
        pLog->printLog("成交类型:%c;",pInvestorPositionDetail->TradeType);
        pLog->printLog("交易所代码:%s;",pInvestorPositionDetail->ExchangeID);
        pLog->printLog("投资者保证金:%f;",pInvestorPositionDetail->Margin);
        pLog->printLog("交易所保证金:%f;",pInvestorPositionDetail->ExchMargin);
        pLog->printLog("昨结算价:%.4f;",pInvestorPositionDetail->LastSettlementPrice);
        pLog->printLog("结算价:%.4f;",pInvestorPositionDetail->SettlementPrice);
        pLog->printLog("平仓量:%d;",pInvestorPositionDetail->CloseVolume);
        pLog->printLog("平仓金额:%.4f;",pInvestorPositionDetail->CloseAmount);
    }
    else
    {
        pLog->printLog("持仓明细查询未能返回数据。\n");
    }
	if(bIsLast)
	{
		bIsgetInst = true;
	}
    return;
}
void CFTTD::OnRspQryExchange(CThostFtdcExchangeField *pExchange, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (pExchange != NULL)
    {
        pLog->printLog("交易所代码:%s;", pExchange->ExchangeID);
        pLog->printLog("交易所名称:%s;", pExchange->ExchangeName);
        pLog->printLog("交易所属性:%c;\n",pExchange->ExchangeProperty);
    }
    else
    {
        pLog->printLog("交易所查询未能返回数据。\n");
    }
    return;
}

void CFTTD::QueryPositionDetail()
{
   CThostFtdcQryInvestorPositionDetailField qryPosDetail;
   memset(&qryPosDetail, 0, sizeof(qryPosDetail));
   bIsgetPosDetail = false;
   strcpy_s(qryPosDetail.BrokerID,qh_BrokerID);
   strcpy_s(qryPosDetail.InstrumentID,"");
   strcpy_s(qryPosDetail.InvestorID,qh_UserID);
   int resCode = m_pTdApi-> ReqQryInvestorPositionDetail(&qryPosDetail, 0);
   pLog->printLog("持仓明细查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CFTTD::QueryPosition()
{
   CThostFtdcQryInvestorPositionField qryPos;
   memset(&qryPos, 0, sizeof(qryPos));
   strcpy_s(qryPos.BrokerID,qh_BrokerID);
   strcpy_s(qryPos.InvestorID, qh_UserID);
   strcpy_s(qryPos.InstrumentID,"");
   int resCode = m_pTdApi-> ReqQryInvestorPosition(&qryPos, 0);
   pLog->printLog("持仓查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CFTTD::QueryOrders()
{
   CThostFtdcQryOrderField qryOrder;
   memset(&qryOrder,0,sizeof(qryOrder));
   strcpy_s(qryOrder.BrokerID,qh_BrokerID);
   strcpy_s(qryOrder.InvestorID, qh_UserID);
   strcpy_s(qryOrder.ExchangeID, "");
   strcpy_s(qryOrder.InstrumentID,"");
   strcpy_s(qryOrder.OrderSysID,"");
   strcpy_s(qryOrder.InsertTimeStart,"");
   strcpy_s(qryOrder.InsertTimeEnd,"");


   int resCode = m_pTdApi->ReqQryOrder(&qryOrder, 0);
   pLog->printLog("委托查询:ExchangeID:%s;InstrumentID:%s;OrderSysID:%s;InsertTimeStart:%s;InsertTimeEnd:%s\n",
          qryOrder.ExchangeID,qryOrder.InstrumentID,qryOrder.OrderSysID,qryOrder.InsertTimeStart,qryOrder.InsertTimeEnd);	
   pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CFTTD::QueryTrades()
{
   CThostFtdcQryTradeField qryTrade;
   memset(&qryTrade,0,sizeof(qryTrade));
   strcpy_s(qryTrade.BrokerID,qh_BrokerID);
   strcpy_s(qryTrade.InvestorID, qh_UserID);
   strcpy_s(qryTrade.InstrumentID,"");
   strcpy_s(qryTrade.ExchangeID, "");
   strcpy_s(qryTrade.TradeID,"");
   strcpy_s(qryTrade.TradeTimeStart,"");
   strcpy_s(qryTrade.TradeTimeEnd,"");


   int resCode = m_pTdApi->ReqQryTrade(&qryTrade, 0);
   pLog->printLog("成交查询:ExchangeID:%s;InstrumentID:%s;TradeID:%s;TradeTimeStart:%s;TradeTimeEnd:%s\n",
          qryTrade.ExchangeID,qryTrade.InstrumentID,qryTrade.TradeID,qryTrade.TradeTimeStart,qryTrade.TradeTimeEnd);	
   pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CFTTD::QueryMD()
{
   CThostFtdcQryDepthMarketDataField qryDepthMarketData;
   memset(&qryDepthMarketData,0,sizeof(qryDepthMarketData));
   strcpy_s(qryDepthMarketData.InstrumentID, "IF1612");
   int resCode = m_pTdApi->ReqQryDepthMarketData(&qryDepthMarketData, 0);
   pLog->printLog("行情查询:InstrumentID:%s\n", qryDepthMarketData.InstrumentID);	
   pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}
void CFTTD::QueryAcct()
{
   CThostFtdcQryTradingAccountField qryTradingAccount;
   memset(&qryTradingAccount,0,sizeof(qryTradingAccount));
   strcpy_s(qryTradingAccount.BrokerID, qh_BrokerID);
   strcpy_s(qryTradingAccount.InvestorID, qh_UserID);
   int resCode = m_pTdApi->ReqQryTradingAccount(&qryTradingAccount, 0);
   pLog->printLog("账户查询:InvestorID:%s\n", qryTradingAccount.InvestorID);	
   pLog->printLog("委托查询请求完毕，返回值：%d\n",resCode);	
   return;
}

void CFTTD::prtErr(const char* pFuncID, CThostFtdcRspInfoField *pRspInfo)
{
    if (pRspInfo==NULL)
    {
        return; //printf("%s: pRspInfo = NULL\n", pFuncID);
    }
    else
    {
        pLog->printLog("%s:返回代码:%d,Msg:%s\n", pFuncID, pRspInfo->ErrorID,pRspInfo->ErrorMsg);
    }
    return;
}

double CFTTD::round(double dVal, short iPlaces)
{
	double dRetval;
	double dMod = 0.0000001;
	if (dVal<0.0) dMod = -0.0000001;
	dRetval = dVal;
	dRetval += (5.0 / pow(10.0, iPlaces + 1.0));
	dRetval *= pow(10.0, iPlaces);
	dRetval = floor(dRetval + dMod);
	dRetval /= pow(10.0, iPlaces);
	return(dRetval);
}

void CFTTD::PlaceOrder(const char* pi_Instrument, 
                       const char* pi_BuyOrSell,
					   const char* pi_OpenOrClose,
					   double pi_Price,
                       int pi_Position)
{
	CThostFtdcInputOrderField pInputOrder;
    memset(&pInputOrder,0,sizeof(pInputOrder));
    CThostFtdcInputOrderField * pIptOrdFld=&pInputOrder;

    strcpy_s(pIptOrdFld->BrokerID,qh_BrokerID);    //经纪商代码
	strcpy_s(pIptOrdFld->InvestorID, qh_UserID); //投资者代码
	//strcpy_s(pIptOrdFld->UserID, qh_UserID); //投资者代码
	strcpy_s(pIptOrdFld->InstrumentID, pi_Instrument); //合约代码
    pIptOrdFld->OrderPriceType=THOST_FTDC_OPT_LimitPrice; //报单价格条件 //这里注意，SSE或者SZE不支持AnyPrice
	if (strcmp(pi_BuyOrSell, "B") == 0 ||
		strcmp(pi_BuyOrSell, "BUY") == 0)
	{
		pIptOrdFld->Direction = THOST_FTDC_D_Buy; //买卖方向
	}
	else if (strcmp(pi_BuyOrSell, "S") == 0 ||
		strcmp(pi_BuyOrSell, "SELL") == 0)
	{
		pIptOrdFld->Direction = THOST_FTDC_D_Sell; //买卖方向
	}

	///开仓 THOST_FTDC_OF_Open '0'
	///平仓 THOST_FTDC_OF_Close '1'
	///平今 THOST_FTDC_OF_CloseToday '3'
	///平昨 THOST_FTDC_OF_CloseYesterday '4'
	if (strcmp(pi_OpenOrClose, "O") == 0 ||
		strcmp(pi_OpenOrClose, "OPEN") == 0)
	{
		strcpy(pIptOrdFld->CombOffsetFlag, "0"); //买卖方向
	}
	else if (strcmp(pi_OpenOrClose, "C") == 0 ||
		strcmp(pi_OpenOrClose, "CLOSE") == 0)
	{
		strcpy(pIptOrdFld->CombOffsetFlag, "1"); //买卖方向
	}

	++nRequestID;
	pIptOrdFld->VolumeTotalOriginal = pi_Position; //数量
    pIptOrdFld->TimeCondition = THOST_FTDC_TC_GFD;  //有效期类型 //当日有效
    pIptOrdFld->VolumeCondition=THOST_FTDC_VC_AV; //成交量类型
    pIptOrdFld->ContingentCondition = THOST_FTDC_CC_Immediately; //触发条件
    pIptOrdFld->ForceCloseReason = THOST_FTDC_FCC_NotForceClose;  //强平原因
	strcpy(pIptOrdFld->CombHedgeFlag, "1"); //投机套保
	pIptOrdFld->LimitPrice = round(pi_Price,4);
    //如果一次控制台程序启动后，多次使用ReqOrderInsert报单，此处的nRequestID要写成单调递增的，如nRequestID++，初始为1
	
    SYSTEMTIME sys;
    GetLocalTime(&sys); 
    sprintf(systime,"%02d%02d%02d%03d",sys.wHour,sys.wMinute, sys.wSecond, sys.wMilliseconds);
    
    pIptOrdFld->RequestID = nRequestID;
    sprintf(pIptOrdFld->OrderRef,"%s%03d", systime, nRequestID);
	GetSysTime();
	pLog->printLog("报单-%s:合约:%s;买卖:%c;开平:%s;价格:%.2f;数量:%d;OrderRef:%s;RequestID:%d\n",
		systime,
		pIptOrdFld->InstrumentID,
		pIptOrdFld->Direction,
		pIptOrdFld->CombOffsetFlag,
		pIptOrdFld->LimitPrice,
		pIptOrdFld->VolumeTotalOriginal,
		pIptOrdFld->OrderRef,
		pIptOrdFld->RequestID);
    int resCode=m_pTdApi->ReqOrderInsert(pIptOrdFld, nRequestID);
	pLog->printLog("报单-%s:报单初始返回值:%d\n", systime, resCode);
}

void CFTTD::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
     prtErr("OnRspOrderInsert", pRspInfo);
     GetSysTime();
     pLog->printLog("OnRspOrderInsert-%s:合约:%s;报单返回:%s;Ref:%s;RequestID:%d\n",systime,pInputOrder->InstrumentID,pRspInfo->ErrorMsg,pInputOrder->OrderRef,pInputOrder->RequestID);
     return;
}
void CFTTD::OnRspQryOrder(CThostFtdcOrderField *pOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryOrder", pRspInfo);
    if(pOrder!=NULL)
    {
		GetSysTime();
		pLog->printLog("OnRspQryOrder-%s:合约:%s;买卖:%c;报单状态:%s;RequestID:%d;OrderRef:%s;LocalOrderID:%s\n", systime, pOrder->InstrumentID, pOrder->Direction, pOrder->StatusMsg, pOrder->RequestID, pOrder->OrderRef, pOrder->OrderLocalID);
        pLog->printLog("OnRspQryOrder:BrokerID:%s;InvestorID:%s;ExchangeID:%s;SessionID:%d;OrderSysID:%s;InsertTime:%s\n",pOrder->BrokerID,pOrder->InvestorID,pOrder->ExchangeID,pOrder->SessionID,pOrder->OrderSysID,pOrder->InsertTime);
    }
	return;
}
void CFTTD::OnRspQryTrade(CThostFtdcTradeField *pTrade, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryTrade", pRspInfo);
    if(pTrade!=NULL)
    {
		GetSysTime();
		pLog->printLog("OnRspQryTrade-%s:合约:%s;成交价:%.4f;成交时间:%s\n", systime, pTrade->InstrumentID, pTrade->Price, pTrade->TradeTime);
    }
	return;
}
void CFTTD::OnRspQryDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
 {
    prtErr("OnRspQryDepthMarketData", pRspInfo);
    if(pDepthMarketData!=NULL)
    {
        pLog->printLog("OnRspQryDepthMarketData:合约:%s;开盘价:%.4f\n",pDepthMarketData->InstrumentID,pDepthMarketData->OpenPrice);
    }
	return;
}
void CFTTD::OnRspQryTradingAccount(CThostFtdcTradingAccountField *pTradingAccount, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    prtErr("OnRspQryTradingAccount", pRspInfo);
    if(pTradingAccount!=NULL)
    {
		pLog->printLog("OnRspQryTradingAccount:");
		pLog->printLog("入金金额:%.2f;", pTradingAccount->Deposit);
		pLog->printLog("出金金额:%.2f;", pTradingAccount->Withdraw);
		pLog->printLog("冻结的保证金:%.2f;", pTradingAccount->FrozenMargin);
		pLog->printLog("冻结的资金:%.2f;", pTradingAccount->FrozenCash);
		pLog->printLog("冻结的手续费:%.2f;", pTradingAccount->FrozenCommission);
		pLog->printLog("当前保证金总额:%.2f;", pTradingAccount->CurrMargin);
		pLog->printLog("资金差额:%.2f;", pTradingAccount->CashIn);
		pLog->printLog("手续费:%.2f;", pTradingAccount->Commission);
		pLog->printLog("平仓盈亏:%.2f;", pTradingAccount->CloseProfit);
		pLog->printLog("持仓盈亏:%.2f;", pTradingAccount->PositionProfit);
		pLog->printLog("期货结算准备金:%.2f;", pTradingAccount->Balance);
		pLog->printLog("可用资金:%.2f;", pTradingAccount->Available);
		pLog->printLog("可取资金:%.2f;", pTradingAccount->WithdrawQuota);
		pLog->printLog("基本准备金:%.2f;", pTradingAccount->Reserve);
		pLog->printLog("交易日:%s;", pTradingAccount->TradingDay);
		pLog->printLog("质押金额:%.2f;", pTradingAccount->Mortgage);
		pLog->printLog("交易所保证金:%.2f\n", pTradingAccount->ExchangeMargin);
    }
    return;
}
void CFTTD::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	pLog->printLog("InvestorID:%d   ClientID:%d\n",pTradingCode->InvestorID,pTradingCode->ClientID);
	return;
}
void CFTTD::OnRspQryInstrumentCommissionRate(CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	return;
}
//ErrRSP&Rtn/////////////////////////////////////////////////////////////////////
void CFTTD::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	return;
}
void CFTTD::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo)
{
	return;
}
void CFTTD::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
	return;
}
//Rtn/////////////////////////////////////////////////////////////////////
void CFTTD::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    if(pOrder!=NULL)
    {
        GetSysTime();
        pLog->printLog("OnRtnOrder-%s:合约:%s;买卖:%c;报单状态:%s;OrderStatus:%c;RequestID:%d;OrderRef:%s;LocalOrderID:%s\n",systime,pOrder->InstrumentID,pOrder->Direction,pOrder->StatusMsg,pOrder->OrderStatus,pOrder->RequestID,pOrder->OrderRef,pOrder->OrderLocalID);
        pLog->printLog("OnRtnOrder-%s:BrokerID:%s;InvestorID:%s;ExchangeID:%s;SessionID:%d;OrderSysID:%s;InsertTime:%s\n",systime,pOrder->BrokerID,pOrder->InvestorID,pOrder->ExchangeID,pOrder->SessionID,pOrder->OrderSysID,pOrder->InsertTime);
    }
	return;
}
void CFTTD::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
	if(pTrade!=NULL)
        GetSysTime();
        pLog->printLog("OnRtnTrade-%s:合约:%s;价格:%.4f;OrderLocalID:%s\n",systime,pTrade->InstrumentID,pTrade->Price,pTrade->OrderLocalID);
	return;
}
void CFTTD::OnRtnInstrumentStatus(CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
    //pLog->printLog("OnRtnInstrumentStatus:%s,%c",pInstrumentStatus->InstrumentID,pInstrumentStatus->InstrumentStatus);
    return;
}