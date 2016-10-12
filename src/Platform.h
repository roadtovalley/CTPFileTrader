#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_WINDOWS)
  #define __PLATFORM_WINDOWS__
#else
  #define __PLATFORM_LINUX__
#endif


#ifdef __PLATFORM_LINUX__
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
#include <sys/time.h>
#include <stdio.h>

void Sleep(long long nMiniSecods);
#define strtok_s strtok_r
#define stricmp strcasecmp
#define strnicmp strncasecmp
#define _snprintf snprintf
#define _snprintf_s(szBuf, len1, len2, ...) _snprintf(szBuf, len1, __VA_ARGS__)
#define __int64 long long
#define GetCurrentThreadId gettid
#define vsnprintf_s(szBuf, len1, len2, ...) vsnprintf(szBuf, len1, __VA_ARGS__)
#define DebugPrint
#else
void gettimeofday(struct timeval* pTime);
#endif

struct timespec;
void gettimeofday_spec(struct timespec* pTime);
int get_uniq_id();
#endif
