#include "regctrl.h"

#include "regctrl.h"
#include <errno.h>
#include <stdio.h>


#define RANGE_R1_MASK   0x0F  /* REG1[0:3] */
#define RANGE_R2_MASK   0x03  /* REG2[6:7] */
#define RANGE_R3_MASK   0x0F  /* REG3[0:3] */

#define F100_SHIFT      4     /* REG1[4:5] */
#define F50_SHIFT       6     /* REG1[6:7] */

#define GND_SHIFT       0     /* REG2[0:1] */
#define COUP_SHIFT      2     /* REG2[2:3] */
#define INMODE_SHIFT    4     /* REG2[4:5] */
#define RANGE_R2_SHIFT  6     /* REG2[6:7] */

#define Z_LOW_SHIFT     4     /* REG3[4:5] */
#define Z_HIGH_SHIFT    6     /* REG3[6:7]*/

/* ----------  ----------
 * 
 *  g_range_r1_low4[] -> REG1[0:3]
 *  g_range_r2_hi2[]  -> REG2[6:7]
 *  g_range_r3_low4[] -> REG3[0:3]
 */
static const uint8_t g_range_r1_low4[8] = {  /* REG1[0:3] */
    0x05, 0x06, 0x05, 0x06, 0x05, 0x06, 0x09, 0x0A
};
static const uint8_t g_range_r2_hi2[8]  = {  /* REG2[6:7] */
    0x01, 0x1, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02
};
static const uint8_t g_range_r3_low4[8] = {  /* REG3[0:3] */
    0x05, 0x05, 0x05, 0x05, 0x0A, 0x0A, 0x0A, 0x0A
};

/*--------------------------------------*/
#define CUR_COUP_SHIFT   0                            /* REG1[0:1] */
#define CUR_RANGE_R2_SHIFT  2     					  /* REG2[2:3] */
#define CUR_RANGE_R1_MASK   0xFF  					  /* REG1[0:7] */
#define CUR_RANGE_R2_MASK   0x03  					  /* REG2[2:3] 2bit */

static const uint8_t g_cur_range_r1_low8[8] = {       /* REG1[0:7] */
    0x55, 0x55, 0x69, 0x69, 0x96, 0x96, 0xAA, 0xAA
};
static const uint8_t g_cur_range_r2_low4_hi2[8]  = {  /* REG2[2:3] */
    0x01, 0x02, 0x01, 0x02, 0x01, 0x02, 0x01, 0x02
};

/* -------------------- */
static inline uint8_t u2(uint8_t v) { return (uint8_t)(v & 0x3); }
static inline uint8_t u4(uint8_t v) { return (uint8_t)(v & 0xF); }

/* -------------------- */
SiVolCtrol si_cfg_default(void)
{
    SiVolCtrol c;
    c.range_idx   = 0;          /**/
    c.filt100     = FILT_OFF;   /* 100 Hz*/
    c.filt50      = FILT_OFF;   /* 50 Hz*/
    c.gnd_sel     = GND_FLOAT;  /**/
    c.coup_sel    = COUP_DC;    /* */
    c.in_mode     = IN_SINGLE;  /**/
    c.z_for_low4  = Z_10M;      /**/
    c.z_for_high4 = Z_10M;      /**/
    return c;
}

/* ------------------- */
int si_pack_regs(const SiVolCtrol *cfg, SiVolCtrolRegs *out)
{
    if (!cfg || !out) return -EINVAL;

    uint8_t i = (uint8_t)(cfg->range_idx & 0x7);

  
    uint8_t r1_low4 = u4(g_range_r1_low4[i]);                  /* REG1[0:3] */
    uint8_t r2_hi2  = (uint8_t)(g_range_r2_hi2[i] & RANGE_R2_MASK); /* REG2 */
    uint8_t r3_low4 = u4(g_range_r3_low4[i]);                  /* REG3[0:3] */

    /* REG1 */
    out->reg1  = 0;
    out->reg1 |= r1_low4;
    out->reg1 |= (uint8_t)(u2(cfg->filt100) << F100_SHIFT);
    out->reg1 |= (uint8_t)(u2(cfg->filt50 ) << F50_SHIFT);

    /* REG2 */
    out->reg2  = 0;
    out->reg2 |= (uint8_t)(u2(cfg->gnd_sel)  << GND_SHIFT);
    out->reg2 |= (uint8_t)(u2(cfg->coup_sel) << COUP_SHIFT);
    out->reg2 |= (uint8_t)(u2(cfg->in_mode)  << INMODE_SHIFT);
    out->reg2 |= (uint8_t)(r2_hi2 << RANGE_R2_SHIFT);          /* [6:7] */

    /* REG3 */
    out->reg3  = 0;
    out->reg3 |= r3_low4;                                      /* [0:3] */
    out->reg3 |= (uint8_t)(u2(cfg->z_for_low4)  << Z_LOW_SHIFT);   /* [5:4] */
    out->reg3 |= (uint8_t)(u2(cfg->z_for_high4) << Z_HIGH_SHIFT);  /* [7:6] */

	printf("REG1:%08x\r\n", out->reg1);
	printf("REG2:%08x\r\n", out->reg2);
	printf("REG3:%08x\r\n", out->reg3);
	
    return 0;
}

/* -------------------- */
int si_pack_bytes(const SiVolCtrol *cfg, uint8_t out3[3])
{
    if (!cfg || !out3) return -EINVAL;
    SiVolCtrolRegs r;
    int rc = si_pack_regs(cfg, &r);
    if (rc) return rc;
    out3[0] = r.reg1; out3[1] = r.reg2; out3[2] = r.reg3;
    return 0;
}


int si_pack_cur_regs(const SiCurCtrol *cfg, SiCurCtrolRegs *out)
{
	if (!cfg || !out) return -EINVAL;
	
   uint8_t i = (uint8_t)(cfg->range_idx & 0x7);

   uint8_t r1_low8 = g_cur_range_r1_low8[i];				  
   uint8_t r2_low4_hi2  = g_cur_range_r2_low4_hi2[i]; 


	/* REG1 */
	out->reg1  = r1_low8 & CUR_RANGE_R1_MASK;

	/* REG2 */
	out->reg2 = 0;
	out->reg2 |= (uint8_t)(cfg->coup_sel << CUR_COUP_SHIFT);
	out->reg2 |= (uint8_t)(r2_low4_hi2 << CUR_RANGE_R2_SHIFT);

	printf("REG1:%08x\r\n", out->reg1);
	printf("REG2:%08x\r\n", out->reg2);

	return 0;

}

















