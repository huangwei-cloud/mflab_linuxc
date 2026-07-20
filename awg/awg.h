#ifndef _AWG_H_
#define _AWG_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define AWG_L500K_DDR_CH1_BASE  0x80000000U
#define AWG_L500K_DDR_CH2_BASE  0x88000000U
#define AWG_L500K_DDR_CH_SIZE   0x08000000U

#define AWG_QUEUE_MAX_STEPS  64    // 队列里最多 32 段

typedef enum {
    AWG_CH1 = 0,
    AWG_CH2 = 1,
    AWG_CH_MAX
} awg_channel_t;

typedef enum {
    AWG_PLAY_SINGLE = 1,   // 单次/连续播放一个波形（由 repeat 决定）
    AWG_PLAY_QUEUE  = 2,   // 队列播放
} awg_play_mode_t;


/* 队列中的一个播放段 */
typedef struct {
    uint32_t start_addr;   // AWG 物理起始地址
    uint32_t length;       // 波形长度（点数 or 字节）
    uint32_t repeat;       // 播放次数，0 表示无限循环（对单次模式可以用 0=连续）
    uint32_t wait_us;      // 当前段结束后等待时间（微秒）
	uint32_t play_num;      // 当前序列的播放号，例如第一个序列可能放最后播放
} awg_queue_step_t;

/* 当前 AWG 播放配置 */
typedef struct {
    awg_play_mode_t  mode;                // AWG_PLAY_SINGLE / AWG_PLAY_QUEUE
    uint32_t         queue_count;         // 队列有效个数
    awg_queue_step_t queue[AWG_QUEUE_MAX_STEPS];
} awg_play_config_t;

typedef struct {
    awg_play_config_t cfg[AWG_CH_MAX];
} awg_ctx_t;

void init_awg_ctx();

void awg_data_send(uint32_t chnum, uint8_t *buff, uint32_t size);

#endif

