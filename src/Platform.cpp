#include "Platform.h"

#ifdef __PLATFORM_LINUX__
//#include <atomic.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

void Sleep(long long nMiniSecods)
{
    //struct timeval delay;
    //delay.tv_sec = 0;
    //delay.tv_usec = nMiniSecods * 1000; // 20 ms
    //select(0, NULL, NULL, NULL, &delay);
    if (nMiniSecods > 1800*1000)
    {
        sleep(nMiniSecods/1000);
    }
    else
    {
        usleep(nMiniSecods*1000);
    }
}

// static atomic_t atomic_number;

#else
#include <windows.h>
//#include "pthread.h"

void gettimeofday(struct timeval* tp)
{
#ifdef NGX_WIN32

    DWORD dt = timeGetTime(); // 错误代码项  
    tp->tv_sec = (long) (dt / 1000);  
    tp->tv_usec = (long) (dt*1000);  
#else  
    ULONGLONG   usec;  
    FILETIME    ft;  
    SYSTEMTIME  st;  

    GetSystemTime(&st);  
    SystemTimeToFileTime(&st, &ft);  

    usec = ft.dwHighDateTime;  
    usec <<= 32;  
    usec |= ft.dwLowDateTime;  
    usec /= 10;  
    usec -= 11644473600000000LL;  

    tp->tv_sec = (long) (usec / 1000000);  
    tp->tv_usec = (long) (usec % 1000000); 
#endif // NGX_WIN32  
}


#endif

#if 0
static pthread_once_t counter_init = PTHREAD_ONCE_INIT;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
static int nCounter=1;

void init_counter_mutex()
{
    pthread_mutex_init(&counter_mutex, NULL);
}

class uninit_counter_mutex
{
public:
    static uninit_counter_mutex m_uninit_mutex_obj;
    ~uninit_counter_mutex()
    {
        pthread_mutex_destroy(&counter_mutex);
    }
private:
    uninit_counter_mutex(){}

};
uninit_counter_mutex uninit_counter_mutex:: m_uninit_mutex_obj;

int get_uniq_id()
{
    pthread_once(&counter_init, init_counter_mutex);
    pthread_mutex_lock(&counter_mutex);
    int nRet = nCounter ++;
    pthread_mutex_unlock(&counter_mutex);
    return nRet;
}
#endif

static int idUniq;

#if 0
void gettimeofday_spec(struct timespec* pTime)
{
    if (pTime)
    {
        timeval val;
#ifdef __PLATFORM_LINUX__
        gettimeofday(&val, NULL);
#else
        gettimeofday(&val);
#endif
        pTime->tv_sec = val.tv_sec;
        pTime->tv_nsec = (long long)val.tv_usec*1000;
    }
}
#endif