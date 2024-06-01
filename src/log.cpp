#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"

#if defined(LOG_WIN32)
//#pragma warning (disable: 4244) // conversion loss of data
#include <windows.h>
#define hmutex_t            CRITICAL_SECTION
#define hmutex_init         InitializeCriticalSection
#define hmutex_destroy      DeleteCriticalSection
#define hmutex_lock         EnterCriticalSection
#define hmutex_unlock       LeaveCriticalSection
#elif defined(LOG_LINUX)
#include <sys/time.h>       // for gettimeofday
#include <pthread.h>
#define hmutex_t            pthread_mutex_t
#define hmutex_init(mutex)  pthread_mutex_init(mutex, NULL)
#define hmutex_destroy      pthread_mutex_destroy
#define hmutex_lock         pthread_mutex_lock
#define hmutex_unlock       pthread_mutex_unlock
#else
#if 1
#define hmutex_t                  void*
#define hmutex_init(mutex)        ((void)mutex)
#define hmutex_destroy(mutex)     ((void)mutex)
#define hmutex_lock(mutex)        ((void)mutex)
#define hmutex_unlock(mutex)      ((void)mutex)
#else
#include "rtthread.h"
#define hmutex_t                  struct rt_mutex
#define hmutex_init(mutex)        rt_mutex_init(mutex, NULL, RT_IPC_FLAG_FIFO)
#define hmutex_destroy(mutex)
#define hmutex_lock(mutex)        rt_mutex_take(mutex, RT_WAITING_FOREVER)
#define hmutex_unlock(mutex)      rt_mutex_release(mutex)
#endif
#endif

// =============================================================================
// private function

// =============================================================================
// types and definitions

typedef struct
{
    log_function_t fn;
    log_level_t threshold;
} subscriber_t;

// =============================================================================
// local storage

static subscriber_t s_subscribers[LOG_MAX_SUBSCRIBERS];
static char message[LOG_MAX_MESSAGE_LENGTH];

// =============================================================================
// thread-safe

static hmutex_t s_mutex;

// =============================================================================
// user-visible code

void log_init()
{
    memset(s_subscribers, 0, sizeof(s_subscribers));
    hmutex_init(&s_mutex);
#if defined(LOG_WIN32) || defined(LOG_LINUX)
    atexit(log_deinit);  // 程序退出自动调用log_deinit函数
#endif
}

void log_deinit()
{
    hmutex_destroy(&s_mutex);
}

// search the s_subscribers table to install or update fn
log_err_t log_subscribe(log_function_t fn, log_level_t threshold)
{
    int available_slot = -1;
    int i;
    hmutex_lock(&s_mutex);
    for(i = 0; i < LOG_MAX_SUBSCRIBERS; i++)
    {
        if(s_subscribers[i].fn == fn)
        {
            // already subscribed: update threshold and return immediately.
            s_subscribers[i].threshold = threshold;
            hmutex_unlock(&s_mutex);
            return LOG_ERR_NONE;
        }
        else if(s_subscribers[i].fn == NULL)
        {
            // found a free slot
            available_slot = i;
        }
    }
    // fn is not yet a subscriber.  assign if possible.
    if(available_slot == -1)
    {
    	hmutex_unlock(&s_mutex);
        return LOG_ERR_SUBSCRIBERS_EXCEEDED;
    }
    s_subscribers[available_slot].fn = fn;
    s_subscribers[available_slot].threshold = threshold;
    hmutex_unlock(&s_mutex);
    return LOG_ERR_NONE;
}

// search the s_subscribers table to remove
log_err_t log_unsubscribe(log_function_t fn)
{
    int i;
    hmutex_lock(&s_mutex);
    for(i = 0; i < LOG_MAX_SUBSCRIBERS; i++)
    {
        if(s_subscribers[i].fn == fn)
        {
            s_subscribers[i].fn = NULL;    // mark as empty
            hmutex_unlock(&s_mutex);
            return LOG_ERR_NONE;
        }
    }
    hmutex_unlock(&s_mutex);
    return LOG_ERR_NOT_SUBSCRIBED;
}

const char* log_level_name(log_level_t severity)
{
    switch(severity)
    {
        case LOG_TRACE_LEVEL:
            return "TRACE";
        case LOG_DEBUG_LEVEL:
            return "DEBUG";
        case LOG_INFO_LEVEL:
            return "INFO";
        case LOG_WARN_LEVEL:
            return "WARN";
        case LOG_ERROR_LEVEL:
            return "ERROR";
        case LOG_FATAL_LEVEL:
            return "FATAL";
        default:
            return "UNKNOW";
    }
}

const char* log_level_color(log_level_t severity)
{
    switch(severity)
    {
        case LOG_TRACE_LEVEL:
            return CLR_SKYBLUE;
        case LOG_DEBUG_LEVEL:
            return CLR_WHITE;
        case LOG_INFO_LEVEL:
            return CLR_GREEN;
        case LOG_WARN_LEVEL:
            return CLR_YELLOW;
        case LOG_ERROR_LEVEL:
            return CLR_RED;
        case LOG_FATAL_LEVEL:
            return CLR_RED_WHT;
        default:
            return CLR_CLR;
    }
}

int log_printf(log_level_t severity, const char* fmt, ...)
{
    va_list ap;
    int i = 0, len = 0;

    if(severity < LOG_FATAL_LEVEL || severity > LOG_TRACE_LEVEL)
        return -1;

    hmutex_lock(&s_mutex);

    va_start(ap, fmt);
    len += vsnprintf(message + len, LOG_MAX_MESSAGE_LENGTH - len, fmt, ap);
    va_end(ap);

    for(i = 0; i < LOG_MAX_SUBSCRIBERS; i++)
    {
        if(s_subscribers[i].fn != NULL)
        {
            if(severity <= s_subscribers[i].threshold)
            {
#ifndef LOG_CONSOLE_COLOR_ENABLE
            	s_subscribers[i].fn(message, len);
#else
				const char *ps = NULL, *pe = NULL;
				ps = log_level_color(severity);
				pe = CLR_CLR;
				s_subscribers[i].fn(ps, strlen(ps));
				s_subscribers[i].fn(message, len);
				s_subscribers[i].fn(pe, strlen(pe));
#endif
            }
        }
    }

    hmutex_unlock(&s_mutex);

    return len;
}

/*============================================== 自定义实现以下三个函数 ====================================================*/

const char* log_timestamp(void)
{
    static char buf[64];
    hmutex_lock(&s_mutex);
#if defined(LOG_WIN32)
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0, us = 0;
    SYSTEMTIME tm;
    GetLocalTime(&tm);
    year     = tm.wYear;
    month    = tm.wMonth;
    day      = tm.wDay;
    hour     = tm.wHour;
    min      = tm.wMinute;
    sec      = tm.wSecond;
    us       = tm.wMilliseconds * 1000;
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d ", year, month, day, hour, min, sec, us / 1000);
#elif defined(LOG_LINUX)
    int year = 0, month = 0, day = 0, hour = 0, min = 0, sec = 0, us = 0;
    struct timeval tv;
    struct tm* tm = NULL;
    gettimeofday(&tv, NULL);
    time_t tt = tv.tv_sec;
    tm = localtime(&tt);
    year     = tm->tm_year + 1900;
    month    = tm->tm_mon  + 1;
    day      = tm->tm_mday;
    hour     = tm->tm_hour;
    min      = tm->tm_min;
    sec      = tm->tm_sec;
    us       = tv.tv_usec;
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d.%03d ", year, month, day, hour, min, sec, us / 1000);
#else
    buf[0] = 0;
	{
		rl_time_t t = {0};
		uint32_t ts = rl_time(NULL);
		rl_timepack(ts, &t);
		snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d ", t.year, t.mon, t.mday, t.hour, t.min, t.sec);
	}
#endif
    hmutex_unlock(&s_mutex);
    return buf;
}

#if (!defined(LOG_WIN32)) && (!defined(LOG_LINUX))
#include "usart.h"
#endif

void log_console_output(const char *msg, uint32_t len)
{
#if defined(LOG_WIN32) || defined(LOG_LINUX)
	fputs(msg, stdout);
	fflush(stdout);
#else
	USARTx_SendData(USART1, (uint8_t *)msg, len);
#endif
}

#if defined(LOG_WIN32) || defined(LOG_LINUX)

void log_file_output(const char *msg, uint32_t len)
{
    FILE* fp;
    char fname[64];
    time_t t = time(NULL);
    struct tm* ts = localtime(&t);
    sprintf(fname, "runlog-%04d%02d%02d.log", ts->tm_year + 1900, ts->tm_mon + 1, ts->tm_mday);
    fp = fopen(fname, "a+");
    if(fp)
    {
        fwrite(msg, 1, len, fp);
        fclose(fp);
    }
}

namespace music_data {

LogInitializer::LogInitializer() {
    LOG_INIT();
	LOG_SUBSCRIBE(log_console_output, LOG_TRACE_LEVEL);
}

}

#endif

/*
	LOG_INIT();
	LOG_SUBSCRIBE(log_console_output, LOG_DEBUG_LEVEL);
*/
