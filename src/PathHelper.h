#ifndef __PATH_HELPER_H__
#define __PATH_HELPER_H__

#include "Platform.h"
#include <cctype>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <functional>

#ifdef __PLATFORM_WINDOWS__

#define LAST_DURATION(Topic) \
    static int nLast##Topic##Time_ltzhou; \
    int nCur##Topic##Time_ltzhou = GetTickCount(); \
    nLast##Topic##Time_ltzhou = nLast##Topic##Time_ltzhou ? nLast##Topic##Time_ltzhou : nCur##Topic##Time_ltzhou; \
    char szBuf##Topic##Time_ltzhou[128];\
    sprintf(szBuf##Topic##Time_ltzhou, "Topic:%s time-sep:%d\n", #Topic, nCur##Topic##Time_ltzhou - nLast##Topic##Time_ltzhou); \
    DebugPrint(szBuf##Topic##Time_ltzhou);\
    nLast##Topic##Time_ltzhou = nCur##Topic##Time_ltzhou;

#define TICK_BEGIN(Topic) \
    int nBefore##Topic##Tick_ = GetTickCount();

#define TICK_END(Topic) \
    int nAfter##Topic##Tick_ = GetTickCount(); \
    DebugPrint("Topic:%s time-sep:%d\n", #Topic, nAfter##Topic##Tick_ - nBefore##Topic##Tick_);

std::string GetConfigDir();
std::string GetBasicFileName();

//支持相对路径文件夹的创建
void SafeCreateDir(const std::string& strDir);
//绝对路径文件名转换成相对路径
std::string GetFullPath(const std::string& strFile);

#ifdef _DEBUG
int __cdecl DebugPrint(const char* szFormat, ...);
#else
#define DebugPrint(expr)
#endif

#else

#define LAST_DURATION(Topic)
#define TICK_BEGIN(Topic)
#define TICK_END(Topic)

#endif

#define MAKE_YMD(y, m, d) ((y)*10000 + (m)*100 + (d))

struct DateTimeInfo
{
    unsigned int nYear;
    unsigned int nMonth;
    unsigned int nDay;
    unsigned int nHour;
    unsigned int nMinute;
    unsigned int nSecond;
    unsigned int nMiniSecond;
    DateTimeInfo()
    {
        GetCurDateTime();
    }
    void GetCurDateTime();
    long long ToTick();
};


#ifdef NDEBUG
#define AssertEx(expr) expr;
#else
#define AssertEx(expr) {int n = (int)(expr); assert(n);} 
#endif

class PathHelper
{
public:

	static inline std::string &ltrim(std::string &s) { 
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace)))); 
        return s; 
    } 

    // trim from end 
    static inline std::string &rtrim(std::string &s) { 
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end()); 
        return s; 
    } 

    static inline std::string &trim(std::string &s) { 
        return ltrim(rtrim(s)); 
    }
    static std::string fully_replace(const std::string& str, const std::string& sub_str, const std::string& new_str);


    static void NormalizePath(std::string& strPath, bool isDir, char chSep='\\');
    #ifdef __PLATFORM_WINDOWS__
    static void GetFileList(std::vector<std::string>&vecFiles, const std::string& strDir, const std::string& strSuffix, bool bRecursive);
    #endif
    static bool HasSuffix(const std::string &lhs, const std::string& strSuffix);
    static bool IFind(const std::string & strText, const std::string& strSub);
    static bool StartsWith(const std::string& str, const std::string& strPrefix);
    static int IFindStrInVec(const std::vector<std::string>& vecStrs, const std::string& str);

};

std::string& strUpper(std::string& str);
std::string& strLower(std::string& str);
char* strupr_n(char* src, int n);
char* strlwr_n(char* src, int n);



std::string int2str(int n);

 template <class Container> void GetStrVecInStr(/*std::vector<std::string>&*/Container& vecResult, const char* szInput, const char* szDelim=";", bool bEraseSpace=true)
 {
     if (vecResult.size())
     {
         vecResult.clear();
     }
     if (!szInput)
     {
         return ;
     }

     unsigned int nLen = strlen(szInput);

     if (nLen>0x7fffffff)
     {
         assert(0);
         return ;
     }

     char* szCpy = new char[nLen+1];
     if (!szCpy)
     {
         assert(0);
         return;
     }

     memcpy(szCpy, szInput, nLen);
     szCpy[nLen]=0;

     char* pContext = NULL;
     char* pToken = strtok_s(szCpy, szDelim, &pContext);
     while(pToken)
     {
         std::string strToken(pToken);
         if (bEraseSpace)
         {
             PathHelper::trim(strToken);
             if (strToken.size())
             {
                 vecResult.push_back(strToken);
             }
         }
         else
         {
             vecResult.push_back(strToken);
         }
         
         pToken = strtok_s(NULL, szDelim, &pContext);
     }

     delete [] szCpy;
 }




 bool IsFileExist(const std::string& strFile);
 int ErrPrint(const char* szFormat, ...);
 int Print(const char* szFormat, ...);


 //这种函数效率很低,bFullMath要求两者必须完全一样，否则vec2的为空可以匹配vec1
 template <class StrContainer> StrContainer StrVecAnd(const StrContainer& vec1, const  StrContainer& vec2)
 {
     StrContainer vecRes;
     for (typename StrContainer::const_iterator it1 = vec1.begin(); it1 != vec1.end(); it1++)
     {
         const std::string& str1 = *it1;
         for (typename StrContainer::const_iterator it2 =vec2.begin(); it2!=vec2.end(); it2++)
         {
             const std::string& str2 = *it2;
             if (stricmp(str1.c_str(), str2.c_str()) == 0)
             {
                 vecRes.push_back(str1);
                 break;
             }
         }
     }

     return vecRes;
 }

 template <class StrContainer> StrContainer StrVecOr(const StrContainer& vec1, const  StrContainer& vec2)
 {
     StrContainer vecRes = vec1;
     for (typename StrContainer::const_iterator it2 =vec2.begin(); it2!=vec2.end(); it2++)
     {
         const std::string& str2 = *it2;
         bool bNotFound = true;
         for (typename StrContainer::const_iterator it1 = vec1.begin(); it1 != vec1.end(); it1++)
         {
             const std::string& str1 = *it1;
             if (stricmp(str1.c_str(), str2.c_str()) == 0)
             {
                 bNotFound = false;
                 break;
             }
         }

         if (bNotFound)
         {
             vecRes.push_back(str2);
         }
         
     }
     return vecRes;
 }

 //vec1 -vec2即在vec1中不在vec2中的
 template <class StrContainer> StrContainer StrVecSubstraction(const StrContainer& vec1, const StrContainer& vec2)
 {
     if (vec2.size() == 0)
     {
         return vec1;
     }

     StrContainer vecRes;
     for (typename StrContainer::const_iterator it1 = vec1.begin(); it1 != vec1.end(); it1++)
     {
         const std::string& str1 = *it1;
         bool bEqual = false;
         for (typename StrContainer::const_iterator it2 =vec2.begin(); it2!=vec2.end(); it2++)
         {
             const std::string& str2 = *it2;
             if (stricmp(str1.c_str(), str2.c_str()) == 0)
             {
                 bEqual = true;
                 break;
             }
         }

         if (!bEqual)
         {
             vecRes.push_back(str1);
         }
     }

     return vecRes;
 }


#endif

