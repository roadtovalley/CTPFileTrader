#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <locale.h>
#include <assert.h>
#include <tchar.h>
#include <conio.h>
#include <time.h>
#include <Windows.h>

#include "stdafx.h"
#include "FTMD.h"
#include "FTTD.h"
#include "config.h"
#include "PathHelper.h"
#include "FileSystemWatcher.h"
#include "logInfo.h"

#define CONST_LINEBUF_SIZE 512

char *qh_MDAddress;
char *qh_TDAddress;
char *qh_BrokerID;
char *qh_UserID;
char *qh_Password;
char *qh_OrderPath;
char *qh_ArchivePath;
char *qh_LogPath;

CFTTD *pTdHandler=new CFTTD();
CFTMD *pMdHandler=new CFTMD();
logInfo* g_pLog = new logInfo();

void printtime()
{
    time_t t = time( 0 );   
    char tmpBuf[255];   
    strftime(tmpBuf, 255, "%Y%m%d%H%M%S", localtime(&t)); //format date and time. 
    g_pLog->printLog("%s|",tmpBuf); 
}

int chkMoveFile(LPCWSTR pi_ordrfile,char* po_ordrfile)
{
    time_t nowtime;  
    struct tm *local;  
     
    nowtime = time(NULL); //获取日历时间  
    local=localtime(&nowtime);  //获取当前系统时间  
  
    char filepre_tm[18];  
    strftime(filepre_tm,18,"%Y%m%d_%H%M%S",local);  
    
    int err_cd = 0;
    int len= WideCharToMultiByte(CP_ACP,0,pi_ordrfile,wcslen(pi_ordrfile),NULL,0,NULL,NULL);  
    char* filename = new char[len+1];  
    WideCharToMultiByte(CP_ACP,0,pi_ordrfile,wcslen(pi_ordrfile),filename,len,NULL,NULL);  
    filename[len]='\0';
    
    char* srcFile = new char[strlen(qh_OrderPath)+strlen(filename)+1];
    char* trgFile = new char[strlen(qh_ArchivePath)+strlen(filename)+1];
    sprintf(srcFile,"%s\\%s", qh_OrderPath, filename);
    sprintf(trgFile,"%s\\%s_%s", qh_ArchivePath, filepre_tm, filename);

    if(MoveFileEx(srcFile, trgFile,MOVEFILE_REPLACE_EXISTING))
    {
        strcpy(po_ordrfile, trgFile);
    }
    else
    {
        strcpy(po_ordrfile, "");
        err_cd = 1;
    }
    return err_cd;
}

void get_config(char **envp)
{
    std::string cfgfile = GetConfigDir() + GetBasicFileName() + ".ini";
    char *qh_cfgfile = new char[strlen(cfgfile.c_str())];
    strcpy(qh_cfgfile, cfgfile.c_str());
    printf("%s\n", qh_cfgfile);
    Config config(cfgfile, envp);
    qh_MDAddress = new char[strlen(config.pString("qh_MDAddress").c_str())];
    qh_TDAddress = new char[strlen(config.pString("qh_TDAddress").c_str())];
	qh_BrokerID = new char[strlen(config.pString("qh_BrokerID").c_str())];
	qh_UserID = new char[strlen(config.pString("qh_UserID").c_str())];
	qh_Password = new char[strlen(config.pString("qh_Password").c_str())];
    qh_OrderPath = new char[strlen(config.pString("qh_OrderPath").c_str())];
    qh_ArchivePath = new char[strlen(config.pString("qh_OrderArchive").c_str())];
    qh_LogPath = new char[strlen(config.pString("qh_OrderLog").c_str())];
    strcpy(qh_MDAddress, config.pString("qh_MDAddress").c_str());
    strcpy(qh_TDAddress, config.pString("qh_TDAddress").c_str());
	strcpy(qh_BrokerID, config.pString("qh_BrokerID").c_str());
    strcpy(qh_UserID, config.pString("qh_UserID").c_str());
    strcpy(qh_Password, config.pString("qh_Password").c_str());
    strcpy(qh_OrderPath, config.pString("qh_OrderPath").c_str());
    strcpy(qh_ArchivePath, config.pString("qh_OrderArchive").c_str());
    strcpy(qh_LogPath, config.pString("qh_OrderLog").c_str());
    printf ("qh_MDAddress=%s\n", qh_MDAddress);
    printf ("qh_TDAddress=%s\n", qh_TDAddress);
	printf("qh_BrokerID=%s\n", qh_BrokerID);
	printf("qh_UserID=%s\n", qh_UserID);
	printf("qh_Password=%s\n", qh_Password);
    printf ("qh_OrderPath=%s\n", qh_OrderPath);
    printf ("qh_ArchivePath=%s\n", qh_ArchivePath);
    printf ("qh_LogPath=%s\n", qh_LogPath);
}

CThostFtdcInstrumentField getInstInfo(const char* InstrumentID)
{
    int  i = 0, instnum = pTdHandler->g_Instnum;
    CThostFtdcInstrumentField Inst;
    memset(&Inst, 0, sizeof(CThostFtdcInstrumentField));
    while (i<instnum)
    {
        if (strcmp(pTdHandler->g_pInstinfo[i].InstrumentID, InstrumentID)==0)
        {
            Inst = pTdHandler->g_pInstinfo[i];
            break;
        }
        i++;
    }
    return Inst;
}

double getOrderPrice(const char* InstrumentID, char* pi_BOS, int pi_Pricelvl = 1)
{
    CThostFtdcDepthMarketDataField LastMD;
    memset(&LastMD, 0, sizeof(CThostFtdcDepthMarketDataField));

    if ((pMdHandler->LastDepth.find(InstrumentID) != pMdHandler->LastDepth.end()) &&
        strcmp(InstrumentID,"") != 0)
    {
        LastMD = pMdHandler->LastDepth[InstrumentID];
        g_pLog->printLog("合约:%s;最新:%.2lf;昨收:%.2lf;今开:%.2lf;最高:%.2lf;最低:%.2lf;涨停:%.2lf;跌停:%.2lf;\n",
            InstrumentID, LastMD.LastPrice, LastMD.PreClosePrice,LastMD.OpenPrice,LastMD.HighestPrice,LastMD.LowestPrice,
            LastMD.UpperLimitPrice,LastMD.LowerLimitPrice);
		if (strcmp(pi_BOS, "B") == 0 ||
			strcmp(pi_BOS, "BUY") == 0)
		{
			if (pi_Pricelvl >= 1)
			{
				if ((LastMD.UpperLimitPrice > 0) && (LastMD.UpperLimitPrice < 99999999))
				{
					return LastMD.UpperLimitPrice;
				}
				else if ((LastMD.AskPrice1 > 0) && (LastMD.AskPrice1 < 99999999))
				{
					return LastMD.AskPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
			else if (pi_Pricelvl == 0)
			{
				if ((LastMD.AskPrice1 > 0) && (LastMD.AskPrice1 < 99999999))
				{
					return LastMD.AskPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
			else if (pi_Pricelvl <= -1)
			{
				if ((LastMD.BidPrice1 > 0) && (LastMD.BidPrice1 < 99999999))
				{
					return LastMD.BidPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
		}
		else if (strcmp(pi_BOS, "S") == 0 ||
			strcmp(pi_BOS, "SELL") == 0)
		{
			if (pi_Pricelvl >= 1)
			{
				if ((LastMD.LowerLimitPrice > 0) && (LastMD.LowerLimitPrice < 99999999))
				{
					return LastMD.LowerLimitPrice;
				}
				else if ((LastMD.BidPrice1 > 0) && (LastMD.BidPrice1 < 99999999))
				{
					return LastMD.BidPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
			else if (pi_Pricelvl == 0)
			{
				if ((LastMD.BidPrice1 > 0) && (LastMD.BidPrice1 < 99999999))
				{
					return LastMD.BidPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
			else if (pi_Pricelvl <= -1)
			{
				if ((LastMD.AskPrice1 > 0) && (LastMD.AskPrice1 < 99999999))
				{
					return LastMD.AskPrice1;
				}
				else
				{
					return LastMD.LastPrice;
				}
			}
		}
    }
    else
    {
        return 0;
    }
}

void readfile(const char* pi_ordrfile)
{
    ifstream infile;
    infile.open(pi_ordrfile, ifstream::in);
    string Inststr, BoSstr, OoCstr;
    char linebuf[CONST_LINEBUF_SIZE]={0};
    char InstrumentID[6];
    char BuyOrSell[3];
	char OpenOrClose[5];
    TThostFtdcExchangeIDType ExID;
    double OrdrPrice;
    double PriceBuf;
    int Pos, Pricelvl;
    while (infile.getline(linebuf,sizeof(linebuf)))
    {
        std::stringstream words(linebuf);
        memset(ExID, 0, sizeof(ExID));
		OrdrPrice = 0;
        PriceBuf = 0;
        Inststr.clear();
        BoSstr.clear();
		OoCstr.clear();
        Pos = 0;
        Pricelvl = 5;
        words>>Inststr;
        words>>BoSstr;
		words>>OoCstr;
        words>>Pos;
        words>>Pricelvl;
        strcpy(InstrumentID, Inststr.c_str());
		strcpy(BuyOrSell, BoSstr.c_str());
		strcpy(OpenOrClose, OoCstr.c_str());
		if ((strcmp(InstrumentID, "") != 0) &&
			(strcmp(BuyOrSell, "") != 0) &&
			(strcmp(OpenOrClose, "") != 0))
		{
            OrdrPrice = getOrderPrice(InstrumentID, BuyOrSell, Pricelvl);
			printtime(); g_pLog->printLog("取得合约价格：%f\n", OrdrPrice);
			printtime(); g_pLog->printLog("读取:合约=%s;买卖=%s;开平=%s;价格=%.4f;数量=%d;\n", InstrumentID, BuyOrSell, OpenOrClose, OrdrPrice, Pos);
            pTdHandler->PlaceOrder(InstrumentID, 
                                   BuyOrSell,
								   OpenOrClose,
                                   OrdrPrice,
                                   Pos);
        }
    }
}

void __stdcall MyDeal( FileSystemWatcher::ACTION act, LPCWSTR filename, LPVOID lParam )
{
    char* ordrfile = new char[500];
    int rtnMoveFile = 0;
    static FileSystemWatcher::ACTION pre = FileSystemWatcher::ACTION_ERRSTOP;
    switch( act )
    {
        case FileSystemWatcher::ACTION_ADDED:
            printtime();wprintf_s(L"新增:%s\n", filename);
            rtnMoveFile = chkMoveFile(filename, ordrfile);
            if (rtnMoveFile == 0)
            {
                printtime();g_pLog->printLog("文件移动至Archive成功！\n");
                printtime();g_pLog->printLog("读取文件:%s\n", ordrfile);
                readfile(ordrfile);
            }
            else
            {
                printtime();g_pLog->printLog("文件移动至Archive失败！\n");
            }
            break;
        case FileSystemWatcher::ACTION_REMOVED:
            printtime();wprintf_s(L"删除:%s\n", filename);
            break;
        case FileSystemWatcher::ACTION_MODIFIED:
            printtime();wprintf_s(L"修改:%s\n", filename);
            rtnMoveFile = chkMoveFile(filename, ordrfile);
            if (rtnMoveFile == 0)
            {
                printtime();g_pLog->printLog("文件移动至Archive成功！\n");
                printtime();g_pLog->printLog("读取文件:%s\n", ordrfile);
                readfile(ordrfile);
            }
            else
            {
                printtime();g_pLog->printLog("文件移动至Archive失败！\n");
            }
            break;
        case FileSystemWatcher::ACTION_RENAMED_OLD:
            printtime();wprintf_s(L"更名(原名称):%s\n", filename);
            break;
        case FileSystemWatcher::ACTION_RENAMED_NEW:
            assert( pre == FileSystemWatcher::ACTION_RENAMED_OLD );
            printtime();wprintf_s(L"更名(新名称):%s\n", filename);
            break;
        case FileSystemWatcher::ACTION_ERRSTOP:
        default:
            printtime();wprintf_s(L"--错误--%s\n", filename);
            break;
    }
    pre = act;
}

int main(int argc, char* argv[], char *envp[])
{
    //取得INI文件配置信息
    get_config(envp);

	//log类实例
	g_pLog->SetLogPath(qh_LogPath);
	
	//初始化行情和交易类
	pTdHandler->Init(qh_BrokerID, qh_TDAddress, qh_UserID, qh_Password, g_pLog);
	pMdHandler->Init(qh_BrokerID, qh_MDAddress, qh_UserID, qh_Password, pTdHandler, g_pLog);
    
    //定义扫描文件夹名称和过滤器
    LPCTSTR sDir= TEXT(qh_OrderPath);
    DWORD dwNotifyFilter = FileSystemWatcher::FILTER_FILE_NAME|
                           FileSystemWatcher::FILTER_DIR_NAME|
                           FileSystemWatcher::FILTER_LAST_WRITE_NAME|
                           FileSystemWatcher::FILTER_SIZE_NAME;

    //定义文件夹监控类并初始化
    FileSystemWatcher fsw;
    bool r = fsw.Run( sDir, true, dwNotifyFilter, &MyDeal, 0 );
    if( !r ) return -1;
    _tsetlocale( LC_CTYPE, TEXT("chs") );
    _tprintf_s(TEXT("成功监控文件夹:%s\n"),sDir); 
    _tprintf_s(TEXT("按<p>查询持仓，按<q>退出程序\n"));
    char pressKey = '\0';
    while(pressKey!='q') 
    {
        pressKey = _getch();
        if (pressKey == 'p')
        {
            pTdHandler->QueryPosition();
        }
        else if (pressKey == 'd')
        {
            pTdHandler->QueryPositionDetail();
        }
        else if (pressKey == 'o')
        {
            pTdHandler->QueryOrders();
        }
        else if (pressKey == 't')
        {
            pTdHandler->QueryTrades();
        }
        else if (pressKey == 'm')
        {
            pTdHandler->QueryMD();
        }
        else if (pressKey == 'a')
        {
            pTdHandler->QueryAcct();
        }
        else if (pressKey == 's')
        {
            pTdHandler->PassChange("");
        }
		else if (pressKey == '1')
		{
			g_pLog->setLogType(1);
		}
		else if (pressKey == '2')
		{
			g_pLog->setLogType(2);
		}
		else if (pressKey == '3')
		{
			g_pLog->setLogType(3);
		}
		else if (pressKey == '0')
		{
			g_pLog->setLogType(0);
		}
    }
    fsw.Close(1000);
	return 0;
}
