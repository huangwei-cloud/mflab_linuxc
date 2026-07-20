#ifndef _SICALIB_H_
#define _SICALIB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SI_CALIB_VVVI_VOLCH1_PATH "/home/root/app/param/liam005mvvvi/sicalibvolch1.csv"
#define SI_CALIB_VVVI_VOLCH2_PATH "/home/root/app/param/liam005mvvvi/sicalibvolch2.csv"
#define SI_CALIB_VVVI_CURCH2_PATH "/home/root/app/param/liam005mvvvi/sicalibcurch2.csv"

/* -------------------- 组合 → ID 编码规则（8 bit, 0..256） --------------------
 * bit 7 : in_mode     0=single, 1=diff
 * bit 6 : coupling    0=AC,     1=DC
 * bit 5 : impedance   0=10MΩ,   1=50Ω
 * bit 4-3: filter     0=none,1=50Hz,2=100Hz,3=both
 * bit 2-0: range      0..7（8 档）
 * --------------------------------------------------------------------------- */

#define CALIB_MAX_IDS 256

typedef struct {
    double vouta, voutb, voutc, voutd;  /* DAC VOUTA-D calibration values */
    int    valid;                       /* 1=valid, 0=unset */
} calib_params_t;

typedef struct {
    calib_params_t table[CALIB_MAX_IDS];
} caldb_t;

/* 加载 CSV 文件（UTF-8/ASCII），格式：
 *   id,vouta,voutb,voutc,voutd
 *   0,  0,  0,  0,  0
 *   ...
 * 返回：0 成功；负值失败
 */
int caldb_load_csv(caldb_t *db, const char *csv_path);

/* 按 ID 查询一行：成功返回 0，失败（未加载/越界/无效）返回 -1 */
int caldb_get_by_id(const caldb_t *db, unsigned id, calib_params_t *out);

/* 组合字段编码为 ID（见上方 bit 定义） */
static inline unsigned encode_id(unsigned in_mode, unsigned coupling,
                                 unsigned impedance, unsigned filter, unsigned range)
{
    return  ((range    & 7u) << 5) |
			((in_mode  & 1u) << 4) |
            ((coupling & 1u) << 3) |
            ((impedance& 1u) << 2) |
            ((filter   & 3u) << 0);
            
}

/* 从 ID 反解各字段（指针可为 NULL 表示不取） */
static inline void decode_id(unsigned id, unsigned *in_mode, unsigned *coupling,
                             unsigned *impedance, unsigned *filter, unsigned *range)
{
	if (range)	   *range	  = (id >> 5) & 7u;
    if (in_mode)   *in_mode   = (id >> 4) & 1u;
    if (coupling)  *coupling  = (id >> 3) & 1u;
    if (impedance) *impedance = (id >> 2) & 1u;
    if (filter)    *filter    = (id >> 0) & 3u;
    
}

void set_si_calibration(int ch);
void set_si_cur_calibration(int id);
int init_sicalib_param();

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* CALIB_H */
