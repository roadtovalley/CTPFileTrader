#include "ConfigSettings.h"
#include "ConfigParser.h"

#include <string.h>
#include <assert.h>
#include "PathHelper.h"
#include <map>

#define SAFE_STR(str) ((str)?(str):"")

ConfigSettings::ConfigSettings()
{
    memset(this, 0, sizeof(*this));
    RestoreStatus();
}

void ConfigSettings::RestoreStatus()
{
    if (m_pConfigParser)
    {
        delete m_pConfigParser;
        m_pConfigParser = NULL;
    }
    memset(this, 0, sizeof(*this));
}

ConfigSettings::~ConfigSettings()
{
    RestoreStatus();
}

bool ConfigSettings::LoadSettings(const std::string& strConfigFile)
{
    RestoreStatus();
    m_pConfigParser = new ConfigParser;
    if (!m_pConfigParser->Load(strConfigFile))
    {
        delete m_pConfigParser;
        m_pConfigParser = NULL;
        return false;
    }
    int nLineOffset = 0;
    std::string strValue;
    nLineOffset = m_pConfigParser->GetConfigString(strValue, "IP", 0);

    if (nLineOffset==-1 ||!strValue.size())
    {
        fprintf(stderr, "Config: IP is null\n");
        delete m_pConfigParser;
        m_pConfigParser = NULL;
        return false;
    }
    strncpy(szIP, strValue.c_str(), sizeof(szIP)-1);

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "Port", 0);
    if (nLineOffset==-1 ||!strValue.size())
    {
        fprintf(stderr, "Config: Port is null\n");
        delete m_pConfigParser;
        m_pConfigParser = NULL;
        return false;
    }
    nPort = atoi(strValue.c_str());

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "User", 0);
    if (nLineOffset ==-1 ||!strValue.size())
    {
        fprintf(stderr, "Config: User is null\n");
    }
    strncpy(szUser, strValue.c_str(), sizeof(szUser)-1);

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "Password", 0);
    if (nLineOffset ==-1 ||!strValue.size())
    {
        fprintf(stderr, "Config: Password is null\n");
    }
    strncpy(szPwd, strValue.c_str(), sizeof(szPwd)-1);

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "Date", 0);
    if (nLineOffset ==-1 ||!strValue.size())
    {
        nDate = 0;
    }
    else
    {
        nDate = atoi(strValue.c_str());
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "Time", 0);
    if (nLineOffset ==-1 ||!strValue.size())
    {
        nTime = 0;
    }
    else
    {
        nTime = atoi(strValue.c_str());
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "StockList", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        assert(1);
    }
    else
    {
        unsigned int nLen = strValue.size();
        szSubscriptions = new char[nLen+1];
        memset(szSubscriptions, 0, nLen+1);
        strncpy(szSubscriptions, strValue.c_str(), nLen);
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "MarketList", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        assert(1);
    }
    else
    {
        strncpy(szMarketList, strValue.c_str(), sizeof(szMarketList)-1);
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "DataDir", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        std::string strConfigDir= GetConfigDir();
        strncpy(szDataDir, strValue.c_str(), sizeof(szDataDir)-1);
    }
    else
    {
        std::string strConfigDir= GetConfigDir();
        if (strValue.find('\\')==std::string::npos && strValue.find('/')==std::string::npos)
        {
            strValue = std::string(".\\") + strValue;
        }
        std::string strFull = GetFullPath(strConfigDir + strValue);
        PathHelper::NormalizePath(strFull, true);
        strncpy(szDataDir, strFull.c_str(), sizeof(szDataDir)-1);
        SafeCreateDir(strFull);
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "OutputDest", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nOutputDevice = OUTPUTTYPE_NO;
    }
    else
    {
        nOutputDevice = OUTPUTTYPE_NO;
        LineParser parser(strValue, ';');
        for (int i=0; i<parser.size(); i++)
        {
            const std::string& strDst = parser[i];
            if (stricmp(strDst.c_str(), "SCREEN")==0)
            {
                nOutputDevice |= OUTPUTTYPE_SCREN;
            }
            else if (stricmp(strDst.c_str(), "CSV")==0)
            {
                nOutputDevice |= OUTPUTTYPE_FILE;
            }
        }
    }


    nLineOffset = m_pConfigParser->GetConfigString(strValue, "OutputCodeTable", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        bOutputCodeTable = false;
    }
    else
    {
        bOutputCodeTable = atoi(strValue.c_str());
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "MaxMemBuf", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nMaxMemBuf = DEFAULT_MAX_MEM_BUF;
    }
    else
    {
        unsigned int nOutValue = atoi(strValue.c_str());
        nOutValue = nOutValue>= 256 ? 256 : nOutValue;
        nOutValue = nOutValue<= 1 ? 1 : nOutValue;
        nMaxMemBuf = nOutValue * 1024 * 1024;
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "MaxWriteTimeGap", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nMaxWriteTimeGap = DEFAULT_MAX_WRITE_TIME_GAP;
    }
    else
    {
        unsigned int nOutValue = atoi(strValue.c_str());
        nOutValue = nOutValue>= 3600 ? 3600 : nOutValue;
        nOutValue = nOutValue<= 1 ? 1 : nOutValue;
        nMaxWriteTimeGap = nOutValue;
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "ReconnectCount", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nReconnectCount = DEFAULT_RECONNECT_COUNT;
    }
    else
    {
        nReconnectCount = atoi(strValue.c_str());
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "nReconnectGap", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nReconnGap = DEFAULT_RECONNECT_GAP;
    }
    else
    {
        nReconnGap = atoi(strValue.c_str());
        nReconnGap = nReconnGap>3600?3600:nReconnGap;
        nReconnGap = nReconnGap<=1?1:nReconnGap;
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "DataType", 0);
    if (nLineOffset==-1 || !strValue.size())
    {
        nDataType = DATA_TYPE_ALL;
    }
    else
    {
        nDataType = 0;
        LineParser parser(strValue, ';');
        for (int i=0; i<parser.size(); i++)
        {
            const std::string& strDst = parser[i];
            if (stricmp(strDst.c_str(), "INDEX")==0)
            {
                nDataType |= DATA_TYPE_INDEX;
            }
            else if (stricmp(strDst.c_str(), "TRANSACTION")==0)
            {
                nDataType |= DATA_TYPE_TRANSACTION;
            }
            else if (stricmp(strDst.c_str(), "ORDER")==0)
            {
                nDataType |= DATA_TYPE_ORDER;
            }
            else if (stricmp(strDst.c_str(), "ORDERQUEUE")==0)
            {
                nDataType |= DATA_TYPE_ORDERQUEUE;
            }
        }
    }

     nLineOffset = m_pConfigParser->GetConfigString(strValue, "CommonLogGap", 0);
     if (nLineOffset==-1 || !strValue.size())
     {
         nCommonLogGap = 1;
     }
     else
     {
         nCommonLogGap = atoi(strValue.c_str());
         nCommonLogGap = nCommonLogGap? nCommonLogGap: 1 ;
     }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "EnableProxy", 0);
    if (nLineOffset == -1||!strValue.size())
    {
        bEnableProxy = false;
    }
    else
    {
        bEnableProxy = atoi(strValue.c_str());
        if (bEnableProxy)
        {
            nLineOffset = m_pConfigParser->GetConfigString(strValue, "ProxyType", 0);
            if (nLineOffset==-1 || !strValue.size())
            {
                fprintf(stderr, "ProxyType is not specified!\n");
                delete m_pConfigParser;
                m_pConfigParser = NULL;
                return false;
            }

            int nHasProxy = 0;
            nProxyType = ProxyTypeStr2TDF(&nHasProxy, strValue);
            if (!nHasProxy)
            {
                fprintf(stderr, "ProxyType is Invalid\n");
                delete m_pConfigParser;
                m_pConfigParser = NULL;
                return false;
            }

            nLineOffset = m_pConfigParser->GetConfigString(strValue, "ProxyHostIP", 0);
            if (nLineOffset==-1 || !strValue.size())
            {
                fprintf(stderr, "ProxyHostIP is not specified\n");
                delete m_pConfigParser;
                m_pConfigParser = NULL;
                return false;
            }
            strncpy(szProxyHostIp, strValue.c_str(), sizeof(szProxyHostIp));

            nLineOffset = m_pConfigParser->GetConfigString(strValue, "ProxyHostPort", 0);
            if (nLineOffset==-1 || !strValue.size())
            {
                fprintf(stderr, "ProxyHostPort is not specified\n");
                delete m_pConfigParser;
                m_pConfigParser = NULL;
                return false;
            }
            nProxyHostPort = atoi(strValue.c_str());

            nLineOffset = m_pConfigParser->GetConfigString(strValue, "ProxyUser", 0);
            if (nLineOffset==-1 || !strValue.size())
            {
                //fprintf(stderr, "ProxyUser is not specified\n");
                memset(szProxyUser, 0, sizeof(szProxyUser));
            }
            else
            {
                strncpy(szProxyUser, strValue.c_str(),  sizeof(szProxyUser));
            }

            nLineOffset = m_pConfigParser->GetConfigString(strValue, "ProxyPassword", 0);
            if (nLineOffset==-1 || !strValue.size())
            {
                //fprintf(stderr, "ProxyPassword is not specified\n");
                memset(szProxyPassword, 0, sizeof(szProxyPassword));
            }
            else
            {
                strncpy(szProxyPassword, strValue.c_str(),  sizeof(szProxyPassword));
            }
        }
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "OpenTimeOut", 0);
    if (nLineOffset == -1||!strValue.size())
    {
        nOpenTimeOut = DEFAULT_TDF_OPEN_TIME_OUT;
    }
    else
    {
        nOpenTimeOut = atoi(strValue.c_str());
        nOpenTimeOut = (nOpenTimeOut ? nOpenTimeOut : 1);
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "HeartBeatGap", 0);
    if (nLineOffset == -1||!strValue.size())
    {
        nHeartBeatGap = DEFAULT_HEART_BEAT_GAP;
    }
    else
    {
        nHeartBeatGap = atoi(strValue.c_str());
        nHeartBeatGap = (nHeartBeatGap ? nHeartBeatGap : 1);
    }

    nLineOffset = m_pConfigParser->GetConfigString(strValue, "MissHeartBeatCount", 0);
    if (nLineOffset == -1||!strValue.size())
    {
        nMissedHeartBeatCount = DEFAULT_MISSED_HEAT_BEAT_COUNT;
    }
    else
    {
        nMissedHeartBeatCount = atoi(strValue.c_str());
        nMissedHeartBeatCount = (nMissedHeartBeatCount ? nMissedHeartBeatCount : 1);
    }


    //after config loading over, clear the config parser;
    delete m_pConfigParser;
    m_pConfigParser = NULL;
    return true;
}

std::string ConfigSettings::ToString()
{
    const int nMaxBufSize = 512*1024;
    char* szTemp = new char[nMaxBufSize];
    int nOffset = 0;
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "IP:%s, Port:%d, User:%s, Password:%s, Date:%d, Time:%d\n", szIP, nPort, szUser, szPwd, nDate, nTime);
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "szSubscriptions:%s\n", SAFE_STR(szSubscriptions));
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "szMarketList:%s\n", szMarketList);
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "szDataDir:%s\n", szDataDir);
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "nOutputDevice:%s, bOutputCodeTable:%d, nMaxMemBuf:%d(MB), nMaxWriteTimeGap:%d(s)\n", OutputDeviceToStr((OUTPUTTYPE)nOutputDevice).c_str(), bOutputCodeTable, nMaxMemBuf/1024/1024, nMaxWriteTimeGap);
    nOffset += _snprintf(szTemp+nOffset, nMaxBufSize-1, "nReconnectCount:%d, nReconnGap:%d(s), nDataType:%s,nCommonLogGap:%d\n", nReconnectCount, nReconnGap, DataTypeToStr((DATA_TYPE_FLAG)nDataType).c_str(), nCommonLogGap);

    szTemp[nMaxBufSize-1]=0;
    std::string strRet(szTemp);
    delete [] szTemp;
    return strRet;
}

/*static */
std::string ConfigSettings::OutputDeviceToStr(OUTPUTTYPE nDevice)
{
    std::string strOut;
    if (nDevice & OUTPUTTYPE_SCREN)
    {
        strOut += "Screen;";
    }

    if (nDevice & OUTPUTTYPE_FILE)
    {
        strOut += "CSV;";
    }

    if (!nDevice)
    {
        strOut += "None;";
    }

    return strOut;
}

/*static */
std::string ConfigSettings::DataTypeToStr(DATA_TYPE_FLAG nFlag)
{
    std::string strOut;
    if (!nFlag)
    {
        strOut = "DATA_TYPE_ALL";
    }
    else
    {
        if (nFlag & DATA_TYPE_INDEX)
        {
            strOut += "INDEX;";
        }
        
        if (nFlag & DATA_TYPE_TRANSACTION)
        {
            strOut += "TRANSACTION;";
        }
        
        if (nFlag & DATA_TYPE_ORDER)
        {
            strOut += "ORDER;";
        }
        
        if (DATA_TYPE_ORDERQUEUE)
        {
            strOut += "ORDERQUEUE";
        }
    }

    return strOut;
}

/*static*/ 
TDF_PROXY_TYPE ConfigSettings::ProxyTypeStr2TDF(int* nIsValid, const std::string& strProxy)
{
    std::map<std::string, TDF_PROXY_TYPE> typeMap;
    typeMap.insert(std::make_pair<std::string, TDF_PROXY_TYPE>("SOCK4", TDF_PROXY_SOCK4));
    typeMap.insert(std::make_pair<std::string, TDF_PROXY_TYPE>("SOCK4A", TDF_PROXY_SOCK4A));
    typeMap.insert(std::make_pair<std::string, TDF_PROXY_TYPE>("SOCK5", TDF_PROXY_SOCK5));
    typeMap.insert(std::make_pair<std::string, TDF_PROXY_TYPE>("HTTP1.1", TDF_PROXY_HTTP11));
    std::string strProxyCpy(strProxy);
    strUpper(strProxyCpy);
    
    if (typeMap.find(strProxyCpy) == typeMap.end())
    {
        if(nIsValid)
        {
            *nIsValid = 0;
        }
        return TDF_PROXY_SOCK4;
    }
    else
    {
        if(nIsValid)
        {
            *nIsValid = 1;
        }
        return typeMap[strProxyCpy];
    }
}
