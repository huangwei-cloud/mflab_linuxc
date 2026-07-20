#include <stdarg.h>
#include "log.h"

static logLevelMap_t g_log_level_map[P_LOG_LEVEL_NUM] = {
    {P_LOG_LEVEL_DEBUG,   (uint8_t *)"P_LOG_DEBUG"},
    {P_LOG_LEVEL_INFO,    (uint8_t *)"P_LOG_INFO"},
    {P_LOG_LEVEL_WARNING, (uint8_t *)"P_LOG_WARNING"},
    {P_LOG_LEVEL_ERROR,   (uint8_t *)"P_LOG_ERROR"},
    {P_LOG_LEVEL_FATAL,   (uint8_t *)"P_LOG_FATAL"},
};

// 全局日志等级, 默认不打印Debug
static uint8_t g_log_level = P_LOG_LEVEL_INFO | P_LOG_LEVEL_WARNING | P_LOG_LEVEL_ERROR | P_LOG_LEVEL_FATAL;
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;

// 初始化日志系统
void plog_init(uint8_t level)
{
    pthread_mutex_lock(&g_log_mutex);
    g_log_level = 0xff & level;
    pthread_mutex_unlock(&g_log_mutex);
}

void plog_set_level(uint8_t level)
{
    pthread_mutex_lock(&g_log_mutex);
    g_log_level = 0xff & level;
    pthread_mutex_unlock(&g_log_mutex);
}

// 获取当前日志等级
void plog_get_level(void)
{
    pthread_mutex_lock(&g_log_mutex);
    printf("Platform log print level: ");
    for (uint8_t i = 0; i < P_LOG_LEVEL_NUM; ++i)
    {
        if (1 == (g_log_level & g_log_level_map[i].bit))
        {
            printf("%s ", g_log_level_map[i].name);
        }
    }
    printf("\r\n");
    pthread_mutex_unlock(&g_log_mutex);
}

// 日志打印实现
void _log_print(uint8_t level, const char *file, int line, const char *fmt, ...)
{
    // 检查日志等级是否需要打印
    if (0 == (level & g_log_level)) {
        return;
    }

    // 查找当前level对应的名称
    uint8_t *log_name = (uint8_t *)"UNKNOWN";
    for (uint8_t i = 0; i < P_LOG_LEVEL_NUM; ++i) {
        if (level == g_log_level_map[i].bit) {
            log_name = g_log_level_map[i].name;
            break;
        }
    }

    pthread_mutex_lock(&g_log_mutex);

    // 时间获取逻辑
    time_t now;
    struct tm tm_info;
    char time_str[20];
    time(&now);
    localtime_r(&now, &tm_info);
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_info);

    // 打印日志头部
    fprintf(stderr, "[%s] [%s] %s:%d: ",
            time_str, log_name, file, line);

    // 日志内容打印逻辑
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (level == P_LOG_LEVEL_FATAL) {
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    pthread_mutex_unlock(&g_log_mutex);
}
