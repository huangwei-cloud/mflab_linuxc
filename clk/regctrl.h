#ifndef _REGCTRL_H_
#define _REGCTRL_H_

/*
 * REG 打包说明（按你最终确认的位定义）：
 *
 *  REG1[0:3]  范围（range 低 4 位片段）
 *  REG1[4:5]  100 Hz 滤波选择（2bit）
 *  REG1[6:7]  50  Hz 滤波选择（2bit）
 *
 *  REG2[0:1]  接地/浮地（2bit）
 *  REG2[2:3]  直流/交流耦合（2bit）
 *  REG2[4:5]  单端/差分（2bit）
 *  REG2[6:7]  范围（range 高 2 位片段）
 *
 *  REG3[0:3]  范围（range 扩展片段，4bit）
 *  REG3[4:5]  阻抗（前 4 档范围有效，2bit）
 *  REG3[6:7]  阻抗（后 4 档范围有效，2bit）
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 2bit 控制量（可按需扩展到 0..3 四档） */
typedef enum { FILT_OFF = 0x02, FILT_ON  = 0x01 } filt_sel_t;    /* ON:FREQ OFF:旁路 */
typedef enum { GND_FLOAT= 0x01, GND_EARTH= 0x02 } gnd_sel_t;      /* 浮地/接地 */
typedef enum { COUP_DC   = 0x01, COUP_AC  = 0x02 } coupling_t;   /* 直流/交流 */
typedef enum { IN_SINGLE = 0x02, IN_DIFF  = 0x01 } input_mode_t; /* 单端/差分 */
typedef enum { Z_10M     = 0x02, Z_50M    = 0x01 } impedance_t;  /* 10M / 50M */

/* 配置结构体：支持直接改单个字段再统一打包 */
typedef struct {
    uint8_t     range_idx;     /* 0..7：8 种范围索引（驱动三张查表） */

    /* REG1 控制 */
    uint8_t     filt100;       /* 2bit -> REG1[5:4] */
    uint8_t     filt50;        /* 2bit -> REG1[7:6] */

    /* REG2 控制 */
    uint8_t     gnd_sel;       /* 2bit -> REG2[1:0] */
    uint8_t     coup_sel;      /* 2bit -> REG2[3:2] */
    uint8_t     in_mode;       /* 2bit -> REG2[5:4] */

    /* REG3 阻抗 */
	uint8_t     z_index;       /* save z index */
    uint8_t     z_for_low4;    /* 2bit -> REG3[5:4]：range∈{0..3} 时生效 */
    uint8_t     z_for_high4;   /* 2bit -> REG3[7:6]：range∈{4..7} 时生效 */
} SiVolCtrol;

typedef struct {
	uint8_t     range_idx;    /* 0..7：8 种范围索引（驱动二张查表） */

	/* REG2 控制*/
	uint8_t     coup_sel;     /* 2bit -> REG2[1:0]*/
	
} SiCurCtrol;

/* 打包后的三个寄存器字节 */
typedef struct {
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
} SiVolCtrolRegs;

/* 打包后的三个寄存器字节 */
typedef struct {
    uint8_t reg1;
    uint8_t reg2;
} SiCurCtrolRegs;

/* 默认配置（安全初值），可直接 SiVolCtrol cfg = so_cfg_default(); 然后改字段 */
SiVolCtrol si_cfg_default(void);

/* 打包：从配置生成 REG1/REG2/REG3 三个寄存器值；成功返回 0，参数错误返回 -EINVAL */
int si_pack_regs(const SiVolCtrol *cfg, SiVolCtrolRegs *out);

/* 打包为 3 字节数组（便于 SPI/I2C 直接发送） */
int si_pack_bytes(const SiVolCtrol *cfg, uint8_t out3[3]);

int si_pack_cur_regs(const SiCurCtrol *cfg, SiCurCtrolRegs *out);

/* —— 可选的细粒度 setter（你也可以直接改结构体字段） —— */
static inline void si_set_range(SiVolCtrol *c, uint8_t idx) { c->range_idx = (uint8_t)(idx & 0x7); }
static inline void si_set_filt100(SiVolCtrol *c, uint8_t v) { c->filt100   = (uint8_t)(v & 0x3); }
static inline void si_set_filt50 (SiVolCtrol *c, uint8_t v) { c->filt50    = (uint8_t)(v & 0x3); }
static inline void si_set_gnd    (SiVolCtrol *c, uint8_t v) { c->gnd_sel   = (uint8_t)(v & 0x3); }
static inline void si_set_coup   (SiVolCtrol *c, uint8_t v) { c->coup_sel  = (uint8_t)(v & 0x3); }
static inline void si_set_inmode (SiVolCtrol *c, uint8_t v) { c->in_mode   = (uint8_t)(v & 0x3); }
static inline void si_set_z_low4 (SiVolCtrol *c, uint8_t v) { c->z_for_low4  = (uint8_t)(v & 0x3); }
static inline void si_set_z_high4(SiVolCtrol *c, uint8_t v) { c->z_for_high4 = (uint8_t)(v & 0x3); }

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

