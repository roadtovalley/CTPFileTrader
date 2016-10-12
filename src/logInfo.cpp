#include "logInfo.h"

logInfo::logInfo(void)
{
    m_pfLogFile = NULL;
    memset(m_cInfo, NULL, sizeof(m_cInfo));
}

logInfo::~logInfo(void)
{
    if (NULL != m_pfLogFile)
    {
        fclose(m_pfLogFile);
        m_pfLogFile = NULL;
    }
}

int logInfo::SetLogPath(const char *pLogPath)
{
    time_t nowtime;
    struct tm *local;
    nowtime = time(NULL); //获取日历时间
    local=localtime(&nowtime);  //获取当前系统时间

    char filepre_tm[18];
    strftime(filepre_tm,18,"%Y%m%d_%H%M%S",local);

    memset(m_logFileName, 0, sizeof(m_logFileName));
    sprintf(m_logFileName,"%s\\log_%s.txt", pLogPath, filepre_tm);

	printf("Open Log File:%s\n", m_logFileName);
    m_pfLogFile = fopen(m_logFileName, "a");
    if (!m_pfLogFile) {
		m_pfLogFile = fopen(m_logFileName, "w");
		if (!m_pfLogFile) {
			cerr << "cannot open log file '" << m_logFileName << "'" << endl;
		}
    }
    return 0;
}

int logInfo::WriteLogInfo(const char *pInfo)
{
    if(NULL != m_pfLogFile)
    {
        fprintf(m_pfLogFile,"%s",pInfo);
        return 0;
    }
    return 1;
}

void logInfo::flushLog(void)
{
    if(NULL != m_pfLogFile)
        fflush(m_pfLogFile);
    return;
}

void logInfo::GetSysTime(void)
{
	SYSTEMTIME sys;
	GetLocalTime(&sys);
	sprintf(systime, "%04d%02d%02d%02d%02d%02d%03d", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
}

void logInfo::printLog(const char *format, ...)
{
    va_list args;
    memset(m_cInfo, 0, sizeof(m_cInfo));
    va_start(args, format);
    vsprintf(m_cInfo, format, args);
    va_end(args);
	if ((g_logType == 2) || (g_logType == 3)) printf("%s", m_cInfo);
	if ((g_logType == 1) || (g_logType == 3))
	{
		WriteLogInfo(m_cInfo);
		flushLog();
	}
}
void logInfo::setLogType(int logType)
{
	g_logType = logType;
}
