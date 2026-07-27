#ifndef _AWG_H_
#define _AWG_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define AWG_QUEUE_MAX_STEPS  64    // 队列里最多 64 段

typedef enum {
    AWG_CH1 = 0,
    AWG_CH2 = 1,
    AWG_CH_MAX
} awg_channel_t;

/* 队列中的一个播放段 */
typedef struct {
    uint32_t start_addr;   // AWG 起始地址
    uint32_t length;       // 波形长度
    uint32_t repeat;       // 播放次数，0 表示无限循环
    uint32_t wait_us;      // 当前段结束后等待时间（微秒）
    uint32_t play_num;     // 当前队列播放顺序编号，从 0 开始
} awg_queue_step_t;

/* 当前 AWG 播放配置 */
typedef struct {
    uint32_t         mode;        // 寄存器值：1 连续播放，2 触发全部队列，3 触发一个队列，4 触发单次按序播放
    uint32_t         queue_count; // 队列有效个数
    awg_queue_step_t queue[AWG_QUEUE_MAX_STEPS];
} awg_play_config_t;

typedef struct {
    awg_play_config_t cfg[AWG_CH_MAX];
} awg_ctx_t;

void init_awg_ctx();

void awg_data_send(uint32_t chnum, uint8_t *buff, uint32_t size);

#endif
