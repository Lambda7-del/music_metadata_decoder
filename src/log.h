#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

/* 全局使能 */
#define LOG_ENABLED

/* 控制台带颜色输出 */
#define LOG_CONSOLE_COLOR_ENABLE

/* 换行字符，windows下是\r\n，linux系统下是\n，macos下是\r */
#define LOG_LINEBREAK "\n"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define LOG_WIN32
#elif defined(linux) || defined(__linux) || defined(__linux__)
    #define LOG_LINUX
#elif defined(__unix__)
	#define LOG_UNIX
#endif

typedef enum
{
	LOG_FATAL_LEVEL = 1,
	LOG_ERROR_LEVEL,
	LOG_WARN_LEVEL,
	LOG_INFO_LEVEL,
	LOG_DEBUG_LEVEL,
	LOG_TRACE_LEVEL,
} log_level_t;

#if defined(LOG_WIN32)
#define DIR_SEPARATOR       '\\'
#define DIR_SEPARATOR_STR   "\\"
#elif defined(LOG_LINUX) || defined(LOG_UNIX)
#define DIR_SEPARATOR       '/'
#define DIR_SEPARATOR_STR   "/"
#else
#define DIR_SEPARATOR       '\\'
#define DIR_SEPARATOR_STR   "\\"
#endif

#ifndef __FILENAME__
#define __FILENAME__  (strrchr(__FILE__, DIR_SEPARATOR) ? strrchr(__FILE__, DIR_SEPARATOR) + 1 : __FILE__)
#endif

#define CLR_CLR         "\033[0m"       /* 恢复颜色 */
#define CLR_BLACK       "\033[30m"      /* 黑色字 */
#define CLR_RED         "\033[31m"      /* 红色字 */
#define CLR_GREEN       "\033[32m"      /* 绿色字 */
#define CLR_YELLOW      "\033[33m"      /* 黄色字 */
#define CLR_BLUE        "\033[34m"      /* 蓝色字 */
#define CLR_PURPLE      "\033[35m"      /* 紫色字 */
#define CLR_SKYBLUE     "\033[36m"      /* 天蓝字 */
#define CLR_WHITE       "\033[37m"      /* 白色字 */

#define CLR_BLK_WHT     "\033[40;37m"   /* 黑底白字 */
#define CLR_RED_WHT     "\033[41;37m"   /* 红底白字 */
#define CLR_GREEN_WHT   "\033[42;37m"   /* 绿底白字 */
#define CLR_YELLOW_WHT  "\033[43;37m"   /* 黄底白字 */
#define CLR_BLUE_WHT    "\033[44;37m"   /* 蓝底白字 */
#define CLR_PURPLE_WHT  "\033[45;37m"   /* 紫底白字 */
#define CLR_SKYBLUE_WHT "\033[46;37m"   /* 天蓝底白字 */
#define CLR_WHT_BLK     "\033[47;30m"   /* 白底黑字 */

#ifdef LOG_ENABLED
#define LOG_INIT() log_init()
#define LOG_SUBSCRIBE(a, b) log_subscribe(a, b)
#define LOG_UNSUBSCRIBE(a) log_unsubscribe(a)
#define LOG(severity, fmt, ...) log_printf(severity, fmt, ##__VA_ARGS__)
#define LOGT(fmt, ...)  log_printf(LOG_TRACE_LEVEL, "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_TRACE_LEVEL), __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGD(fmt, ...)  log_printf(LOG_DEBUG_LEVEL, "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_DEBUG_LEVEL), __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGI(fmt, ...)  log_printf(LOG_INFO_LEVEL,  "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_INFO_LEVEL),  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGW(fmt, ...)  log_printf(LOG_WARN_LEVEL,  "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_WARN_LEVEL),  __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGE(fmt, ...)  log_printf(LOG_ERROR_LEVEL, "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_ERROR_LEVEL), __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOGF(fmt, ...)  log_printf(LOG_FATAL_LEVEL, "%s%-5s " "<%s:%d:%s>: " fmt LOG_LINEBREAK, (const char *)log_timestamp(), log_level_name(LOG_FATAL_LEVEL), __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define LOG_INIT()
#define LOG_SUBSCRIBE(a, b)
#define LOG_UNSUBSCRIBE(a)
#define LOG(severity, fmt, ...)
#define LOG_HEXDUMP(TAG,buf, size)
#define LOGT(fmt, ...)
#define LOGD(fmt, ...)
#define LOGI(fmt, ...)
#define LOGW(fmt, ...)
#define LOGE(fmt, ...)
#define LOGF(fmt, ...)
#endif

typedef enum
{
	LOG_ERR_NONE = 0,
	LOG_ERR_SUBSCRIBERS_EXCEEDED,
	LOG_ERR_NOT_SUBSCRIBED,
} log_err_t;

// define the maximum number of concurrent subscribers
#ifndef LOG_MAX_SUBSCRIBERS
#define LOG_MAX_SUBSCRIBERS 6
#endif
// maximum length of formatted log message
#ifndef LOG_MAX_MESSAGE_LENGTH
#define LOG_MAX_MESSAGE_LENGTH 2048
#endif

/**
 * @brief: prototype for uLog subscribers.
 */
typedef void (*log_function_t)(const char *msg, unsigned int len);

void log_init(void);
void log_deinit(void);
log_err_t log_subscribe(log_function_t fn, log_level_t threshold);
log_err_t log_unsubscribe(log_function_t fn);
const char* log_level_name(log_level_t level);
const char* log_level_color(log_level_t severity);
void log_file(log_level_t severity, const char *msg, unsigned int len);
void log_console(log_level_t severity, const char *msg, unsigned int len);
int log_printf(log_level_t severity, const char* fmt, ...);

//
// 重新实现以下三个函数
//
const char* log_timestamp(void);
void log_file_output(const char *msg, uint32_t len);
void log_console_output(const char *msg, uint32_t len);

#ifdef __cplusplus
}
#endif

#include "singleton.h"

#define INITONLYLOGGER() static music_data::LogInitializer::ptr s_onlyLogger = music_data::OnlyLogger::GetInstance()

namespace music_data {

// 修改构造函数改变日志设置
class LogInitializer {
public: 
	typedef std::shared_ptr<LogInitializer> ptr; 
    LogInitializer(); 
}; 

typedef SingletonPtr<LogInitializer> OnlyLogger; 

}

#endif /* LOG_H_ */
