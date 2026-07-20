#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

#define P_LOG_VERSION "0.2.0"

// 必须确保每个等级对应1个独立位，无重叠
#define P_LOG_LEVEL_DEBUG (1 << 0)   // 0x01
#define P_LOG_LEVEL_INFO (1 << 1)    // 0x02
#define P_LOG_LEVEL_WARNING (1 << 2) // 0x04
#define P_LOG_LEVEL_ERROR (1 << 3)   // 0x08
#define P_LOG_LEVEL_FATAL (1 << 4)   // 0x10
#define P_LOG_LEVEL_NUM 5      // 等级数量

typedef struct
{
    uint8_t bit;   // 等级对应的位标识
    uint8_t *name; // 等级名称
} logLevelMap_t;

// 初始化日志系统（设置默认等级）
void plog_init(uint8_t level);

// 动态修改日志等级
void plog_set_level(uint8_t level);

// 获取当前日志等级
void plog_get_level(void);

// 日志打印函数（内部使用，用户通过宏调用）
void _log_print(uint8_t level, const char *file, int line, const char *fmt, ...);

// 日志打印宏（自动填充文件名和行号）
#define P_LOG_DEBUG(fmt, ...) _log_print(P_LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define P_LOG_INFO(fmt, ...) _log_print(P_LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define P_LOG_WARNING(fmt, ...) _log_print(P_LOG_LEVEL_WARNING, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define P_LOG_ERROR(fmt, ...) _log_print(P_LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define P_LOG_FATAL(fmt, ...) _log_print(P_LOG_LEVEL_FATAL, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif // __PLATFORM_LOG_H__
