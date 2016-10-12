//#include "stdafx.h"
#include <algorithm>
#include "PathHelper.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <string.h>
#include <fstream>
using namespace std;

#ifdef __PLATFORM_WINDOWS__
#include <windows.h>
#else
#include <sys/time.h>
#include <stdarg.h>
#endif

extern std::ofstream g_fsLog;


#ifdef __PLATFORM_WINDOWS__
void PathHelper::GetFileList(std::vector<std::string>&vecFiles, const std::string& strDir, 
    const std::string& strSuffix, bool bRecursive)
{
    std::string strDirInner = strDir;
    NormalizePath(strDirInner, true, '\\');
    WIN32_FIND_DATAA findData;

    std::string strFindFile = strDirInner;
    strFindFile[strFindFile.size()-1] = 0;//remove the back slash character,for the FindFirstFile function will not accept a folder with backslash.
    strFindFile = strFindFile.c_str();
    HANDLE hFindFile = FindFirstFileA(strFindFile.c_str(), &findData);
    if (hFindFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        strFindFile += "\\*";
        WIN32_FIND_DATAA findDataSubDir;
        HANDLE hFindSubDir = FindFirstFileA(strFindFile.c_str(), &findDataSubDir);

        while (FindNextFileA(hFindSubDir, &findDataSubDir))
        {
            if ((findDataSubDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && bRecursive)
            {
                if (strcmp(findDataSubDir.cFileName, ".")==0||
                    strcmp(findDataSubDir.cFileName, "..")==0)
                {
                    continue;
                }
                std::string strSubDir = strDirInner;
                strSubDir += findDataSubDir.cFileName;
                NormalizePath(strSubDir, true, '\\');
                GetFileList(vecFiles, strSubDir, strSuffix, true);
            }
            else if((findDataSubDir.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
            {
                if(HasSuffix(strDirInner+ findDataSubDir.cFileName, strSuffix))
                {
                    vecFiles.push_back(strDirInner + findDataSubDir.cFileName);
                }
            }
        }

        FindClose(hFindSubDir);
    }
    else
    {
        if (HasSuffix(strDirInner+ findData.cFileName, strSuffix))
        {
            vecFiles.push_back(strDirInner + findData.cFileName);
        }
    }

    FindClose(hFindFile);
}
#endif

//isDi:false means file
void PathHelper::NormalizePath(std::string& strPath, bool isDir, char chSep/*='\\'*/)
{
    char chOrg = ((chSep == '\\') ? '/':'\\');
    std::replace(strPath.begin(), strPath.end(),chOrg, chSep);
    
    std::string strTemp;
    strTemp.reserve(strPath.size() +1);
    for (int i=0; i<strPath.size(); )
    {
        int nBegin = strPath.find_first_of("\\/", i);
        if (nBegin!= string::npos)
        {
            std::string strSub = strPath.substr(i, nBegin - i);
            strTemp += trim(strSub);
            strTemp.append(1, chSep);
        }
        else
        {
            std::string strSub = strPath.substr(i, strPath.size() -i);
            strTemp += trim(strSub);
        }
        //nBegin = strPath.find_first_not_of(chSep, nBegin);
        nBegin = strPath.find_first_not_of("\\/", nBegin);
        i = nBegin;
    }
    if (isDir && strTemp[strTemp.size()-1]!=chSep)
    {
        std::string strSlash = std::string(1, chSep);
        strTemp += strSlash;
    }

    strPath = strTemp;
}

bool PathHelper::HasSuffix(const std::string &lhs, const std::string& strSuffix)
{
    if (lhs.size()< strSuffix.size())
    {
        return 0;
    }

    std::string str1 = lhs;
    std::string str2 = strSuffix;
    std::transform(str1.begin(), str1.end(), str1.begin(), ::toupper);
    std::transform(str2.begin(), str2.end(), str2.begin(), ::toupper);

    return str1.rfind(str2) == str1.size() - str2.size();
    //str1.substr(str1.size()-str2.size(), str2.size()) == str2
}

std::string PathHelper::fully_replace(const std::string& str, const std::string& sub_str, const std::string& new_str)
{
    if (!sub_str.size())
    {
        return str;
    }

    std::string str_copy = str;

    int nfirstfind=0;
    while ((nfirstfind = str_copy.find(sub_str, nfirstfind)) != string::npos)
    {
        str_copy.replace(nfirstfind, sub_str.size(), new_str);
        nfirstfind += new_str.size();
    }
    return str_copy;
}

char* strupr_n(char* src, int n)
{
    if (n)
    {
        std::string strCopy(src, 0, n);
        for (int i=0; i<strCopy.size()&& i<n; i++)
        {
            if (src[i] >='a' && src[i] <='z')
            {
                src[i]-=32;
            }
        }
    }
    return src;
}

char* strlwr_n(char* src, int n)
{
    if (n)
    {
        std::string strCopy(src, 0, n);
        for (int i=0; i<strCopy.size()&& i<n; i++)
        {
            if (src[i] >='A' && src[i] <='Z')
            {
                src[i]+=32;
            }
        }
    }
    return src;
}

bool PathHelper::StartsWith(const std::string& str, const std::string& strPrefix)
 {
     if (str.size()<strPrefix.size())
     {
         return false;
     }
     std::string str_cpy = str;
     std::transform(str_cpy.begin(), str_cpy.end(), str_cpy.begin(), ::toupper);
     std::string prefix_cpy = strPrefix;
     std::transform(prefix_cpy.begin(), prefix_cpy.end(), prefix_cpy.begin(), ::toupper);
     
     if (str_cpy.find(prefix_cpy)==0)
     {
         return true;
     }
     return false;
 }

 /*static */
bool PathHelper::IFind(const std::string & strText, const std::string& strSub)
 {
     if (strText.size()<strSub.size())
     {
         return false;
     }
     std::string str1 = strText;
     strUpper(str1);
     std::string str2 = strSub;
     strUpper(str2);

     return str1.find(str2)!=std::string::npos;
 }


std::string& strUpper(std::string& str)
 {
     std::transform(str.begin(), str.end(), str.begin(), ::toupper);
     return str;

 }
std::string& strLower(std::string& str)
 {
     std::transform(str.begin(), str.end(), str.begin(), ::tolower);
     return str;
 }

#ifdef __PLATFORM_WINDOWS__
 void SafeCreateDir(const std::string& strDir)
 {
     std::string strCmd = "if not exist \"" + strDir + "\" mkdir \"" + strDir +"\"";
     system(strCmd.c_str());
 }

 //绝对路径文件名转换成相对路径
 std::string GetFullPath(const std::string& strFile)
 {
     char szTemp[1024]={0};
     int n = GetFullPathNameA(strFile.c_str(), sizeof(szTemp)/sizeof(szTemp[0])-1, szTemp, NULL);
     return std::string(szTemp, 0, sizeof(szTemp)/sizeof(szTemp[0])-1);
 }

 std::string GetConfigDir()
 {
     char szPath[MAX_PATH+1];
     GetModuleFileNameA(NULL, szPath, sizeof(szPath)/sizeof(szPath[0]));
     std::string strFile = szPath;
     int nPos = strFile.find_last_of('\\');
     strFile.erase(strFile.begin() + nPos +1, strFile.end());
     return strFile;
 }

 std::string GetBasicFileName()
 {
     char szPath[MAX_PATH+1];
     GetModuleFileNameA(NULL, szPath, sizeof(szPath)/sizeof(szPath[0]));
     std::string strFile = szPath;
     int nSlashPos = strFile.find_last_of('\\');
     int nDotPos = strFile.find_last_of('.');
     assert(nDotPos != std::string::npos);
     std::string strBasic = strFile.substr(nSlashPos+1, nDotPos-(nSlashPos+1));
     return strBasic;
 }

#ifdef _DEBUG

 int __cdecl DebugPrint(const char* szFormat, ...)
 {

     const int MAX_OUTPUT_LEN = 65534;//经测试，OutputDebugStringA能输出的最大字符串长度为65534，超出之后将会导致整个字符串显示异常。

     const int nBufSize = MAX_OUTPUT_LEN + 1; //多申请一个字符用于放置结尾的空字符
     va_list vArgs;
     va_start(vArgs, szFormat);



     char* szBuf = new char [nBufSize];
     char* szMoreBuf = new char[nBufSize];

     vsnprintf_s(szBuf, nBufSize,nBufSize-1 , szFormat, vArgs);


     //SYSTEMTIME st;
     //GetLocalTime(&st);
     DateTimeInfo timeInfo;
     int nYYMMDD = MAKE_YMD(timeInfo.nYear, timeInfo.nMonth, timeInfo.nDay);

     _snprintf_s(szMoreBuf, nBufSize, nBufSize-1, "TID:%04ld, Time:%d %02d:%02d-%02d-%03d: %s", GetCurrentThreadId(), nYYMMDD, timeInfo.nHour, timeInfo.nMinute, timeInfo.nSecond, timeInfo.nMiniSecond, szBuf);

    OutputDebugStringA(szMoreBuf);

     delete [] szBuf;
     delete [] szMoreBuf;

     va_end(vArgs);

     return 0;
 }
#endif //_DEBUG


#endif


 //当没有定义 PRINT_USE_HEAP_BUFFER 时候，
 //为了从栈中分配空间，提高效率，这个函数输出不大于20K的文本
 int Print(const char* szFormat, ...)
 {
#ifdef PRINT_USE_HEAP_BUFFER
     const int MAX_OUTPUT_LEN = 65534;//经测试，OutputDebugStringA能输出的最大字符串长度为65534，超出之后将会导致整个字符串显示异常。
#else
     const int MAX_OUTPUT_LEN = 20480;
#endif

     const int nBufSize = MAX_OUTPUT_LEN + 1; //多申请一个字符用于放置结尾的空字符
     va_list vArgs;
     va_start(vArgs, szFormat);



#ifdef PRINT_USE_HEAP_BUFFER
     char* szBuf = new char [nBufSize];
     char* szMoreBuf = new char[nBufSize];
#else
     char szBuf[nBufSize];
     char szMoreBuf[nBufSize];
#endif

     vsnprintf_s(szBuf, nBufSize,nBufSize-1 , szFormat, vArgs);


     //SYSTEMTIME st;
     //GetLocalTime(&st);
     DateTimeInfo timeInfo;

     int nYYMMDD = MAKE_YMD(timeInfo.nYear, timeInfo.nMonth, timeInfo.nDay);
     _snprintf_s(szMoreBuf, nBufSize, nBufSize-1, "TID:%04ld, Time:%d %02d:%02d:%02d.%03d: %s", GetCurrentThreadId(),nYYMMDD,timeInfo.nHour, timeInfo.nMinute, timeInfo.nSecond, timeInfo.nMiniSecond, szBuf);
     fprintf(stdout, (const char*)szMoreBuf);

  /*   if (g_fsLog.is_open())
     {
         g_fsLog << szMoreBuf;
     }
*/
     DebugPrint(szMoreBuf);


#ifdef PRINT_USE_HEAP_BUFFER
     delete [] szBuf;
     delete [] szMoreBuf;
#else
#endif

     va_end(vArgs);

     return 0;
 }

 int ErrPrint(const char* szFormat, ...)
 {
#if 1

     const int MAX_OUTPUT_LEN = 65534;//经测试，OutputDebugStringA能输出的最大字符串长度为65534，超出之后将会导致整个字符串显示异常。
     const int nBufSize = MAX_OUTPUT_LEN + 1; //多申请一个字符用于放置结尾的空字符
     va_list vArgs;
     va_start(vArgs, szFormat);

     char* szBuf = new char [nBufSize];
     vsnprintf_s(szBuf, nBufSize,nBufSize-1 , szFormat, vArgs);

#if 1

     char* szMoreInfo = new char [nBufSize];

     //SYSTEMTIME st;
     //GetLocalTime(&st);
     DateTimeInfo timeInfo;
     int nYYMMDD = MAKE_YMD(timeInfo.nYear, timeInfo.nMonth, timeInfo.nDay);

     _snprintf_s(szMoreInfo, nBufSize, nBufSize-1, "TID:%04ld, Time:%d %02d:%02d-%02d-%03d: %s", GetCurrentThreadId(), nYYMMDD, timeInfo.nHour, timeInfo.nMinute, timeInfo.nSecond, timeInfo.nMiniSecond, szBuf);

#ifdef __PLATFORM_WINDOWS__ 
     DebugPrint(szMoreInfo);
#endif
     delete [] szMoreInfo;
#else

#ifdef __PLATFORM_WINDOWS__ 
     DebugPrint(szBuf);
#endif

#endif
     fprintf(stderr, (const char*)szBuf);
     delete [] szBuf;
     va_end(vArgs);
#endif //_DEBUG
     return 0;
 }

 bool IsFileExist(const std::string& strFile)
 {
     FILE* fp = fopen(strFile.c_str(), "rb");
     bool nRet = (fp != NULL);
     if (fp)
     {
         fclose(fp);
     }
     return nRet;
 }


 /* static */
 int PathHelper::IFindStrInVec(const std::vector<std::string>& vecStrs, const std::string& str)
 {
     int nRet = -1;

     for (int i=0; i<vecStrs.size(); i++)
     {
         if (stricmp(vecStrs[i].c_str(), str.c_str()) == 0)
         {
             nRet = i;
             break;
         }
     }
     return nRet;
 }

 std::string int2str(int n)
 {
     char szBuf[32]={0};
     _snprintf(szBuf, sizeof(szBuf)-1, "%d", n);
     return std::string(szBuf, 0, sizeof(szBuf)-1);
 }

 void DateTimeInfo::GetCurDateTime()
 {
#ifdef __PLATFORM_WINDOWS__
     SYSTEMTIME st;
     GetLocalTime(&st);
     nYear = st.wYear;
     nMonth = st.wMonth;
     nDay = st.wDay;
     nHour = st.wHour;
     nMinute = st.wMinute;
     nSecond = st.wSecond;
     nMiniSecond = st.wMilliseconds;
#else
     timeval val;
     gettimeofday(&val, NULL);
     tm detailed;
     gmtime_r(&val.tv_sec, &detailed);
     nYear = detailed.tm_year + 1900;
     nMonth = detailed.tm_mon + 1;
     nDay = detailed.tm_mday;
     nHour = detailed.tm_hour;
     nMinute = detailed.tm_min;
     nSecond = detailed.tm_sec;
     nMiniSecond = val.tv_usec/1000;
#endif
 }


 long long DateTimeInfo::ToTick()
 {
     long long nRet = nDay*24*3600*1000+ nHour*3600*1000 + nMinute*60*1000 + nSecond*1000 + nMiniSecond;
     return nRet;
 }