#include "reg.h"
#include <string.h>
#include "../clk/hmc7043.h"
#include "../pm/param.h"

//REG(0, SCOPE_TRANS_SPEED,	       0x0000, "NULL",		   "int"),
//REG(1, SCOPE_TRANS_SOURCE,	   0x0004, "NULL",		   "ADC1|ADC2|EXT"),
//REG(2, SCOPE_TRANS_EN,		   0x0008, "NULL",		   "0:close|1:open"),
//REG(3, SCOPE_TRIG_TYPE, 	       0x001C, "NULL",		   "RISING|FALLING|LEVEL"),
//REG(4, SCOPE_TRIG_LEVEL,	       0x0020, "NULL",		   "int:0-1023"),
//REG(5, SCOPE_TRIG_SINGLE_EN,     0x0034, "NULL",		   "bool"),
#define BASE_ADDR (0x4C3C0000)

#define  SYSTEM_CONFIG_BASE 0x0000

#define  AUXIN_CONFIG_BASE 0x0300

#define  AUXOUT1_CONFIG_BASE 0x0E00
#define  AUXOUT2_CONFIG_BASE 0x0F00
#define  AUXOUT3_CONFIG_BASE 0x1000
#define  AUXOUT4_CONFIG_BASE 0x1100

//L500K = GUI AUXOUT
#define  CHOUT1_CONFIG_BASE 0x1000
#define  CHOUT2_CONFIG_BASE 0x1010
#define  CHOUT3_CONFIG_BASE 0x1020
#define  CHOUT4_CONFIG_BASE 0x1030

#define  SIGNALOUT1_CONFIG_BASE 0x0C00
#define  SIGNALOUT2_CONFIG_BASE 0x0D00

#define  TRIG_CONFIG_BASE 0x1400

#define  PLL_CONFIG_BASE 0x0700

#define  PID1_CONFIG_BASE 0x1600
#define  PID2_CONFIG_BASE 0x1700
#define  PID3_CONFIG_BASE 0x1800
#define  PID4_CONFIG_BASE 0x1900

#define  OSC1_CONFIG_BASE 0x2200
#define  OSC2_CONFIG_BASE 0x2250
#define  OSC3_CONFIG_BASE 0x22A0
#define  OSC4_CONFIG_BASE 0x22F0
#define  OSC5_CONFIG_BASE 0x2340
#define  OSC6_CONFIG_BASE 0x2390
#define  OSC7_CONFIG_BASE 0x23E0
#define  OSC8_CONFIG_BASE 0x2430

#define  DEMOD1_CONFIG_BASE 0x0800
#define  DEMOD2_CONFIG_BASE 0x0850
#define  DEMOD3_CONFIG_BASE 0x08A0
#define  DEMOD4_CONFIG_BASE 0x08F0
#define  DEMOD5_CONFIG_BASE 0x0940
#define  DEMOD6_CONFIG_BASE 0x0990
#define  DEMOD7_CONFIG_BASE 0x09E0
#define  DEMOD8_CONFIG_BASE 0x0A30


#define  UP_CH1_CONFIG_BASE 0x1A00
#define  UP_CH2_CONFIG_BASE 0x1B00

#define  PCNT1_CONFIG_BASE 0x1E00
#define  PCNT2_CONFIG_BASE 0x1F00
#define  PCNT3_CONFIG_BASE 0x2000
#define  PCNT4_CONFIG_BASE 0x2100

#define  I_CARD_CONFIG_BASE 0x1C00

#define  TRIM_CONFIG_BASE   0x2600

#define  AWG1_CONFIG_BASE 0x2700
#define  AWG2_CONFIG_BASE 0x3700
#define  AWG2_L500K_CONFIG_BASE 0x2800

#define REG_CNT_MAX (1000)

static reg_entry_t reg_table[REG_CNT_MAX];

void init_reg()
{
	int i = 0;
	int model = get_device_model();

	/*module 1 start*/

    reg_table[i++] = REG(0, soft_trig                ,   SYSTEM_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1, osc_work_all             ,   SYSTEM_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2, fan_pwm_val              ,   SYSTEM_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3, pll_freq_l               ,   SYSTEM_CONFIG_BASE + 0x0100, "NULL",         "NULL");
    reg_table[i++] = REG(4, pll_freq_h               ,   SYSTEM_CONFIG_BASE + 0x0101, "NULL",         "NULL");
    reg_table[i++] = REG(5, sync_ok                  ,   SYSTEM_CONFIG_BASE + 0x0102, "NULL",         "NULL");
    /*module 1 end*/

	/*module 2 start*/
	reg_table[i++] = REG(0, auxin_impedance          ,   AUXIN_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    /*module 2 end*/

    /*module 3 start*/
    reg_table[i++] = REG(0, auxout1_en               ,   AUXOUT1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1, auxout1_source           ,   AUXOUT1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2, auxout1_scaling_l        ,   AUXOUT1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3, auxout1_scaling_h        ,   AUXOUT1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4, auxout1_offset           ,   AUXOUT1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5, auxout1_preoffset        ,   AUXOUT1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6, auxout1_decimation       ,   AUXOUT1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7, auxout1_lower_limit      ,   AUXOUT1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8, auxout1_upper_limit      ,   AUXOUT1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9, auxout1_con				 ,	 AUXOUT1_CONFIG_BASE + 0x0009, "NULL",		  "NULL");
	reg_table[i++] = REG(10,auxout1_dcen             ,   AUXOUT1_CONFIG_BASE + 0x000A, "NULL",         "NULL");
    /*module 3 end*/

    /*module 4 start*/
    reg_table[i++] = REG(0, auxout2_en               ,   AUXOUT2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1, auxout2_source           ,   AUXOUT2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2, auxout2_scaling_l        ,   AUXOUT2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3, auxout2_scaling_h        ,   AUXOUT2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4, auxout2_offset           ,   AUXOUT2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5, auxout2_preoffset        ,   AUXOUT2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6, auxout2_decimation       ,   AUXOUT2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7, auxout2_lower_limit      ,   AUXOUT2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8, auxout2_upper_limit      ,   AUXOUT2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
	reg_table[i++] = REG(9, auxout2_con				 ,	AUXOUT2_CONFIG_BASE + 0x0009, "NULL",		  "NULL");
	reg_table[i++] = REG(10,auxout2_dcen             ,   AUXOUT2_CONFIG_BASE + 0x000A, "NULL",         "NULL");
	/*module 4 end*/
    if( (model == DEVICE_MODEL_M005M_VV) || (model == DEVICE_MODEL_M005M_VI) || (model == DEVICE_MODEL_M005M) ) {
        /*module 5 start*/
        reg_table[i++] = REG(0, auxout3_en               ,   AUXOUT3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, auxout3_source           ,   AUXOUT3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, auxout3_scaling_l        ,   AUXOUT3_CONFIG_BASE + 0x0002, "NULL",         "NULL");
        reg_table[i++] = REG(3, auxout3_scaling_h        ,   AUXOUT3_CONFIG_BASE + 0x0003, "NULL",         "NULL");
        reg_table[i++] = REG(4, auxout3_offset           ,   AUXOUT3_CONFIG_BASE + 0x0004, "NULL",         "NULL");
        reg_table[i++] = REG(5, auxout3_preoffset        ,   AUXOUT3_CONFIG_BASE + 0x0005, "NULL",         "NULL");
        reg_table[i++] = REG(6, auxout3_decimation       ,   AUXOUT3_CONFIG_BASE + 0x0006, "NULL",         "NULL");
        reg_table[i++] = REG(7, auxout3_lower_limit      ,   AUXOUT3_CONFIG_BASE + 0x0007, "NULL",         "NULL");
        reg_table[i++] = REG(8, auxout3_upper_limit      ,   AUXOUT3_CONFIG_BASE + 0x0008, "NULL",         "NULL");
        reg_table[i++] = REG(9, auxout3_con				 ,	 AUXOUT3_CONFIG_BASE + 0x0009, "NULL",		  "NULL");
        reg_table[i++] = REG(10,auxout3_dcen             ,   AUXOUT3_CONFIG_BASE + 0x000A, "NULL",         "NULL");
        /*module 6 end*

        /*module 6 start*/
        reg_table[i++] = REG(0, auxout4_en               ,   AUXOUT4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, auxout4_source           ,   AUXOUT4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, auxout4_scaling_l        ,   AUXOUT4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
        reg_table[i++] = REG(3, auxout4_scaling_h        ,   AUXOUT4_CONFIG_BASE + 0x0003, "NULL",         "NULL");
        reg_table[i++] = REG(4, auxout4_offset           ,   AUXOUT4_CONFIG_BASE + 0x0004, "NULL",         "NULL");
        reg_table[i++] = REG(5, auxout4_preoffset        ,   AUXOUT4_CONFIG_BASE + 0x0005, "NULL",         "NULL");
        reg_table[i++] = REG(6, auxout4_decimation       ,   AUXOUT4_CONFIG_BASE + 0x0006, "NULL",         "NULL");
        reg_table[i++] = REG(7, auxout4_lower_limit      ,   AUXOUT4_CONFIG_BASE + 0x0007, "NULL",         "NULL");
        reg_table[i++] = REG(8, auxout4_upper_limit      ,   AUXOUT4_CONFIG_BASE + 0x0008, "NULL",         "NULL");
        reg_table[i++] = REG(9, auxout4_con				 ,	 AUXOUT4_CONFIG_BASE + 0x0009, "NULL",		  "NULL");
        reg_table[i++] = REG(10,auxout4_dcen             ,   AUXOUT4_CONFIG_BASE + 0x000A, "NULL",         "NULL");
        /*module 6 end*/
    }else if (model == DEVICE_MODEL_L500K) {
        /*GUI-AUXOUT*/
        reg_table[i++] = REG(0, chout1_en                  ,   CHOUT1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, choutdc1_value             ,   CHOUT1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, choutdc1_switch            ,   CHOUT1_CONFIG_BASE + 0x0002, "NULL",         "NULL");

        reg_table[i++] = REG(0, chout2_en                  ,   CHOUT2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, choutdc2_value             ,   CHOUT2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, choutdc2_switch            ,   CHOUT2_CONFIG_BASE + 0x0002, "NULL",         "NULL");

        reg_table[i++] = REG(0, chout3_en                  ,   CHOUT3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, choutdc3_value             ,   CHOUT3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, choutdc3_switch            ,   CHOUT3_CONFIG_BASE + 0x0002, "NULL",         "NULL");

        reg_table[i++] = REG(0, chout4_en                  ,   CHOUT4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
        reg_table[i++] = REG(1, choutdc4_value             ,   CHOUT4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
        reg_table[i++] = REG(2, choutdc4_switch            ,   CHOUT4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    }
    /*module 7 start*/
    reg_table[i++] = REG(0 , signalout1_osc_enable     ,   SIGNALOUT1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , signalout1_osc_mode       ,   SIGNALOUT1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , signalout1_osc_rst        ,   SIGNALOUT1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , signalout1_osc_phase_l    ,   SIGNALOUT1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , signalout1_osc_phase_h    ,   SIGNALOUT1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , signalout1_osc_freq_word_l,   SIGNALOUT1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , signalout1_osc_freq_word_h,   SIGNALOUT1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , signalout1_enable         ,   SIGNALOUT1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , signalout1_source1        ,   SIGNALOUT1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , signalout1_source2        ,   SIGNALOUT1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, signalout1_inverted_enable,   SIGNALOUT1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, signalout1_subtract       ,   SIGNALOUT1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, signalout1_scaling1_l     ,   SIGNALOUT1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, signalout1_scaling1_h     ,   SIGNALOUT1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, signalout1_scaling2_l     ,   SIGNALOUT1_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    reg_table[i++] = REG(15, signalout1_scaling2_h     ,   SIGNALOUT1_CONFIG_BASE + 0x000f, "NULL",         "NULL");
    reg_table[i++] = REG(16, signalout1_add_en         ,   SIGNALOUT1_CONFIG_BASE + 0x0010, "NULL",         "NULL");
    reg_table[i++] = REG(17, singalout1_offset_en      ,   SIGNALOUT1_CONFIG_BASE + 0x0011, "NULL",         "NULL");
    reg_table[i++] = REG(18, singalout1_offset         ,   SIGNALOUT1_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    // REG(19, signalout1_vos_trim_en    ,   SIGNALOUT1_CONFIG_BASE + 0x0013, "NULL",         "NULL"),
    // REG(20, signalout1_vos_trim       ,   SIGNALOUT1_CONFIG_BASE + 0x0014, "NULL",         "NULL"),
    // REG(21, signalout1_blazex_trim_en ,   SIGNALOUT1_CONFIG_BASE + 0x0015, "NULL",         "NULL"),
    // REG(22, signalout1_blazex_trim    ,   SIGNALOUT1_CONFIG_BASE + 0x0016, "NULL",         "NULL"),
    reg_table[i++] = REG(23, signalout1_range          ,   SIGNALOUT1_CONFIG_BASE + 0x0017, "NULL",         "NULL");
    reg_table[i++] = REG(24, signalout1_ttl            ,   SIGNALOUT1_CONFIG_BASE + 0x0018, "NULL",         "NULL");
    /*module 7 end*/

    /*module 8 start*/
    reg_table[i++] = REG(0 , signalout2_osc_enable     ,   SIGNALOUT2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , signalout2_osc_mode       ,   SIGNALOUT2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , signalout2_osc_rst        ,   SIGNALOUT2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , signalout2_osc_phase_l    ,   SIGNALOUT2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , signalout2_osc_phase_h    ,   SIGNALOUT2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , signalout2_osc_freq_word_l,   SIGNALOUT2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , signalout2_osc_freq_word_h,   SIGNALOUT2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , signalout2_enable         ,   SIGNALOUT2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , signalout2_source1        ,   SIGNALOUT2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , signalout2_source2        ,   SIGNALOUT2_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, signalout2_inverted_enable,   SIGNALOUT2_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, signalout2_subtract       ,   SIGNALOUT2_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, signalout2_scaling1_l     ,   SIGNALOUT2_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, signalout2_scaling1_h     ,   SIGNALOUT2_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, signalout2_scaling2_l     ,   SIGNALOUT2_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    reg_table[i++] = REG(15, signalout2_scaling2_h     ,   SIGNALOUT2_CONFIG_BASE + 0x000f, "NULL",         "NULL");
    reg_table[i++] = REG(16, signalout2_add_en         ,   SIGNALOUT2_CONFIG_BASE + 0x0010, "NULL",         "NULL");
    reg_table[i++] = REG(17, singalout2_offset_en      ,   SIGNALOUT2_CONFIG_BASE + 0x0011, "NULL",         "NULL");
    reg_table[i++] = REG(18, singalout2_offset         ,   SIGNALOUT2_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    // REG(19, signalout2_vos_trim_en    ,   SIGNALOUT2_CONFIG_BASE + 0x0013, "NULL",         "NULL"),
    // REG(20, signalout2_vos_trim       ,   SIGNALOUT2_CONFIG_BASE + 0x0014, "NULL",         "NULL"),
    // REG(21, signalout2_blazex_trim_en ,   SIGNALOUT2_CONFIG_BASE + 0x0015, "NULL",         "NULL"),
    // REG(22, signalout2_blazex_trim    ,   SIGNALOUT2_CONFIG_BASE + 0x0016, "NULL",         "NULL"),
    reg_table[i++] = REG(23, signalout2_range          ,   SIGNALOUT2_CONFIG_BASE + 0x0017, "NULL",         "NULL");
    reg_table[i++] = REG(24, signalout2_ttl            ,   SIGNALOUT2_CONFIG_BASE + 0x0018, "NULL",         "NULL");
    /*module 8 end*/


    /*module 9 start*/
	reg_table[i++] = REG( 0, clk_sync_mode            ,   TRIG_CONFIG_BASE + 0x0000, "NULL",         "NULL");
	reg_table[i++] = REG( 1, sync_trig                ,   TRIG_CONFIG_BASE + 0x0001, "NULL",         "NULL");
	reg_table[i++] = REG( 2, sync_en                  ,   TRIG_CONFIG_BASE + 0x0002, "NULL",         "NULL");
	reg_table[i++] = REG( 3, trig_out_mode            ,   TRIG_CONFIG_BASE + 0x0003, "NULL",         "NULL");
	reg_table[i++] = REG( 4, trig_out_plus_period     ,   TRIG_CONFIG_BASE + 0x0004, "NULL",         "NULL");
	reg_table[i++] = REG( 5, trig_out_plus_width      ,   TRIG_CONFIG_BASE + 0x0005, "NULL",         "NULL");
	reg_table[i++] = REG( 6, trig_out_plus_loop       ,   TRIG_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG( 7, trig_in_mode             ,   TRIG_CONFIG_BASE + 0x0007, "NULL",         "NULL");
	reg_table[i++] = REG( 8, trig_in_source           ,   TRIG_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG( 9, trig_en                  ,   TRIG_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, trig_source              ,   TRIG_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, trig_mode                ,   TRIG_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, trig_delay               ,   TRIG_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, trig_release_restrain    ,   TRIG_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, trig_plus_width          ,   TRIG_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    reg_table[i++] = REG(15, hysteresis_range_high_l  ,   TRIG_CONFIG_BASE + 0x000f, "NULL",         "NULL");
    reg_table[i++] = REG(16, hysteresis_range_high_h  ,   TRIG_CONFIG_BASE + 0x0010, "NULL",         "NULL");
    reg_table[i++] = REG(17, hysteresis_range_low_l   ,   TRIG_CONFIG_BASE + 0x0011, "NULL",         "NULL");
    reg_table[i++] = REG(18, hysteresis_range_low_h   ,   TRIG_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    reg_table[i++] = REG(19, trigout1_mode            ,   TRIG_CONFIG_BASE + 0x0013, "NULL",         "NULL");
	reg_table[i++] = REG(20, trigout1_period          ,   TRIG_CONFIG_BASE + 0x0014, "NULL",         "NULL");
	reg_table[i++] = REG(21, trigout1_width           ,   TRIG_CONFIG_BASE + 0x0015, "NULL",         "NULL");
	reg_table[i++] = REG(22, trigout1_loop            ,   TRIG_CONFIG_BASE + 0x0016, "NULL",         "NULL");
	reg_table[i++] = REG(23, trigout2_mode            ,   TRIG_CONFIG_BASE + 0x0017, "NULL",         "NULL");
	reg_table[i++] = REG(24, trigout2_period          ,   TRIG_CONFIG_BASE + 0x0018, "NULL",         "NULL");
	reg_table[i++] = REG(25, trigout2_width           ,   TRIG_CONFIG_BASE + 0x0019, "NULL",         "NULL");
	reg_table[i++] = REG(26, trigout2_loop            ,   TRIG_CONFIG_BASE + 0x001a, "NULL",         "NULL");
	reg_table[i++] = REG(27, trigout3_mode            ,   TRIG_CONFIG_BASE + 0x001b, "NULL",         "NULL");
	reg_table[i++] = REG(28, trigout3_period          ,   TRIG_CONFIG_BASE + 0x001c, "NULL",         "NULL");
	reg_table[i++] = REG(29, trigout3_width           ,   TRIG_CONFIG_BASE + 0x001d, "NULL",         "NULL");
	reg_table[i++] = REG(30, trigout3_loop            ,   TRIG_CONFIG_BASE + 0x001e, "NULL",         "NULL");
	reg_table[i++] = REG(31, trigout4_mode            ,   TRIG_CONFIG_BASE + 0x001f, "NULL",         "NULL");
	reg_table[i++] = REG(32, trigout4_period          ,   TRIG_CONFIG_BASE + 0x0020, "NULL",         "NULL");
	reg_table[i++] = REG(33, trigout4_width           ,   TRIG_CONFIG_BASE + 0x0021, "NULL",         "NULL");
	reg_table[i++] = REG(34, trigout4_loop            ,   TRIG_CONFIG_BASE + 0x0022, "NULL",         "NULL");
    /*module 9 end*/

    /*module 10 start*/
	reg_table[i++] = REG(0, pll_enable               ,   PLL_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1, pll_source               ,   PLL_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2, pll_ref_hysteresis_high  ,   PLL_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3, pll_ref_hysteresis_low   ,   PLL_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4, pll_mov_fir_ratio        ,   PLL_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5, pll_calc_time_l          ,   PLL_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6, pll_calc_time_h          ,   PLL_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7, ref_mov_fir_ratio        ,   PLL_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    /*module 10 end*/

    /*module 11 start*/
    reg_table[i++] = REG(0 , pid1_enable             ,   PID1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , pid1_ki_enable          ,   PID1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , pid1_moving_aver_ratio  ,   PID1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , pid1_source             ,   PID1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , pid1_set_point_l        ,   PID1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , pid1_set_point_h        ,   PID1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , pid1_Kp_l               ,   PID1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , pid1_Kp_h               ,   PID1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , pid1_Ki_l               ,   PID1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , pid1_Ki_h               ,   PID1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, pid1_Kd_l               ,   PID1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, pid1_Kd_h               ,   PID1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, pid1_decimation         ,   PID1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, pid1_normaliztion_v_l        ,   PID1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, pid1_normaliztion_v_h        ,   PID1_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    /*module 11 end*/

    /*module 12 start*/
    reg_table[i++] = REG(0 , pid2_enable             ,   PID2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , pid2_ki_enable          ,   PID2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , pid2_moving_aver_ratio  ,   PID2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , pid2_source             ,   PID2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , pid2_set_point_l        ,   PID2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , pid2_set_point_h        ,   PID2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , pid2_Kp_l               ,   PID2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , pid2_Kp_h               ,   PID2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , pid2_Ki_l               ,   PID2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , pid2_Ki_h               ,   PID2_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, pid2_Kd_l               ,   PID2_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, pid2_Kd_h               ,   PID2_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, pid2_decimation         ,   PID2_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, pid2_normaliztion_v_l        ,   PID2_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, pid2_normaliztion_v_h        ,   PID2_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    /*module 12 end*/


    /*module 13 start*/
    reg_table[i++] = REG(0 , pid3_enable             ,   PID3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , pid3_ki_enable          ,   PID3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , pid3_moving_aver_ratio  ,   PID3_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , pid3_source             ,   PID3_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , pid3_set_point_l        ,   PID3_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , pid3_set_point_h        ,   PID3_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , pid3_Kp_l               ,   PID3_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , pid3_Kp_h               ,   PID3_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , pid3_Ki_l               ,   PID3_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , pid3_Ki_h               ,   PID3_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, pid3_Kd_l               ,   PID3_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, pid3_Kd_h               ,   PID3_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, pid3_decimation         ,   PID3_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, pid3_normaliztion_v_l        ,   PID3_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, pid3_normaliztion_v_h        ,   PID3_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    /*module 13 end*/

    /*module 14 start*/
    reg_table[i++] = REG(0 , pid4_enable             ,   PID4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , pid4_ki_enable          ,   PID4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , pid4_moving_aver_ratio  ,   PID4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , pid4_source             ,   PID4_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , pid4_set_point_l        ,   PID4_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , pid4_set_point_h        ,   PID4_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , pid4_Kp_l               ,   PID4_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , pid4_Kp_h               ,   PID4_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , pid4_Ki_l               ,   PID4_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , pid4_Ki_h               ,   PID4_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, pid4_Kd_l               ,   PID4_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, pid4_Kd_h               ,   PID4_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, pid4_decimation         ,   PID4_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, pid4_normaliztion_v_l        ,   PID4_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    reg_table[i++] = REG(14, pid4_normaliztion_v_h        ,   PID4_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    /*module 14 end*/
    
    /*module 15 start*/
    reg_table[i++] = REG(0 , oscillators1_en          ,   OSC1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators1_mode        ,   OSC1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators1_rst         ,   OSC1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators1_phase_l     ,   OSC1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators1_phase_h     ,   OSC1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators1_freq_word_l ,   OSC1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators1_freq_word_h ,   OSC1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7 , am_fm_enable             ,   OSC1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8 , am_fm_mode               ,   OSC1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9 , modulation_multi_1       ,   OSC1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, am_n1                    ,   OSC1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, am_n2                    ,   OSC1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, am_n3                    ,   OSC1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    /*module 15 end*/

    /*module 16 start*/
    reg_table[i++] = REG(0 , oscillators2_en          ,   OSC2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators2_mode        ,   OSC2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators2_rst         ,   OSC2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators2_phase_l     ,   OSC2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators2_phase_h     ,   OSC2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators2_freq_word_l ,   OSC2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators2_freq_word_h ,   OSC2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 16 end*/


    /*module 17 start*/
    reg_table[i++] = REG(0 , oscillators3_en          ,   OSC3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators3_mode        ,   OSC3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators3_rst         ,   OSC3_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators3_phase_l     ,   OSC3_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators3_phase_h     ,   OSC3_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators3_freq_word_l ,   OSC3_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators3_freq_word_h ,   OSC3_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 17 end*/

    /*module 18 start*/
    reg_table[i++] = REG(0 , oscillators4_en          ,   OSC4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators4_mode        ,   OSC4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators4_rst         ,   OSC4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators4_phase_l     ,   OSC4_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators4_phase_h     ,   OSC4_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators4_freq_word_l ,   OSC4_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators4_freq_word_h ,   OSC4_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(0 , oscillators5_en          ,   OSC5_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators5_mode        ,   OSC5_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators5_rst         ,   OSC5_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators5_phase_l     ,   OSC5_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators5_phase_h     ,   OSC5_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators5_freq_word_l ,   OSC5_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators5_freq_word_h ,   OSC5_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(0 , oscillators6_en          ,   OSC6_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators6_mode        ,   OSC6_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators6_rst         ,   OSC6_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators6_phase_l     ,   OSC6_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators6_phase_h     ,   OSC6_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators6_freq_word_l ,   OSC6_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators6_freq_word_h ,   OSC6_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(0 , oscillators7_en          ,   OSC7_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators7_mode        ,   OSC7_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators7_rst         ,   OSC7_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators7_phase_l     ,   OSC7_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators7_phase_h     ,   OSC7_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators7_freq_word_l ,   OSC7_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators7_freq_word_h ,   OSC7_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(0 , oscillators8_en          ,   OSC8_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1 , oscillators8_mode        ,   OSC8_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2 , oscillators8_rst         ,   OSC8_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3 , oscillators8_phase_l     ,   OSC8_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4 , oscillators8_phase_h     ,   OSC8_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5 , oscillators8_freq_word_l ,   OSC8_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6 , oscillators8_freq_word_h ,   OSC8_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 18 end*/

    /*module 19 start*/
	reg_table[i++] = REG(0,  demod1_sinc_dec_ratio_en ,   DEMOD1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod1_sinc_dec_ratio_m1 ,   DEMOD1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod1_sinc_aver_ratio   ,   DEMOD1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod1_sinc_dec_ratio_m2 ,   DEMOD1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod1_enable            ,   DEMOD1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod1_source            ,   DEMOD1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod1_sinc_en           ,   DEMOD1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod1_fir_order         ,   DEMOD1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod1_alpha_l           ,   DEMOD1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod1_alpha_m           ,   DEMOD1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod1_alpha_h           ,   DEMOD1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod1_constant_reg      ,   DEMOD1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod1_psd_scaling_l     ,   DEMOD1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod1_psd_scaling_h     ,   DEMOD1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    /*module 19 end*/


    /*module 20 start*/
	reg_table[i++] = REG(0,  demod2_sinc_dec_ratio_en ,   DEMOD2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod2_sinc_dec_ratio_m1 ,   DEMOD2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod2_sinc_aver_ratio   ,   DEMOD2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod2_sinc_dec_ratio_m2 ,   DEMOD2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod2_enable            ,   DEMOD2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod2_source            ,   DEMOD2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod2_sinc_en           ,   DEMOD2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod2_fir_order         ,   DEMOD2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod2_alpha_l           ,   DEMOD2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod2_alpha_m           ,   DEMOD2_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod2_alpha_h           ,   DEMOD2_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod2_constant_reg      ,   DEMOD2_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod2_psd_scaling_l     ,   DEMOD2_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod2_psd_scaling_h     ,   DEMOD2_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    /*module 20 end*/

    /*module 21 start*/
	reg_table[i++] = REG(0,  demod3_sinc_dec_ratio_en ,   DEMOD3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod3_sinc_dec_ratio_m1 ,   DEMOD3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod3_sinc_aver_ratio   ,   DEMOD3_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod3_sinc_dec_ratio_m2 ,   DEMOD3_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod3_enable            ,   DEMOD3_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod3_source            ,   DEMOD3_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod3_sinc_en           ,   DEMOD3_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod3_fir_order         ,   DEMOD3_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod3_alpha_l           ,   DEMOD3_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod3_alpha_m           ,   DEMOD3_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod3_alpha_h           ,   DEMOD3_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod3_constant_reg      ,   DEMOD3_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod3_psd_scaling_l     ,   DEMOD3_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod3_psd_scaling_h     ,   DEMOD3_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    /*module 21 end*/

    /*module 22 start*/
	reg_table[i++] = REG(0,  demod4_sinc_dec_ratio_en ,   DEMOD4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod4_sinc_dec_ratio_m1 ,   DEMOD4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod4_sinc_aver_ratio   ,   DEMOD4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod4_sinc_dec_ratio_m2 ,   DEMOD4_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod4_enable            ,   DEMOD4_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod4_source            ,   DEMOD4_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod4_sinc_en           ,   DEMOD4_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod4_fir_order         ,   DEMOD4_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod4_alpha_l           ,   DEMOD4_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod4_alpha_m           ,   DEMOD4_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod4_alpha_h           ,   DEMOD4_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod4_constant_reg      ,   DEMOD4_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod4_psd_scaling_l     ,   DEMOD4_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod4_psd_scaling_h     ,   DEMOD4_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(0,  demod5_sinc_dec_ratio_en ,   DEMOD5_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod5_sinc_dec_ratio_m1 ,   DEMOD5_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod5_sinc_aver_ratio   ,   DEMOD5_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod5_sinc_dec_ratio_m2 ,   DEMOD5_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod5_enable            ,   DEMOD5_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod5_source            ,   DEMOD5_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod5_sinc_en           ,   DEMOD5_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod5_fir_order         ,   DEMOD5_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod5_alpha_l           ,   DEMOD5_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod5_alpha_m           ,   DEMOD5_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod5_alpha_h           ,   DEMOD5_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod5_constant_reg      ,   DEMOD5_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod5_psd_scaling_l     ,   DEMOD5_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod5_psd_scaling_h     ,   DEMOD5_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(0,  demod6_sinc_dec_ratio_en ,   DEMOD6_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod6_sinc_dec_ratio_m1 ,   DEMOD6_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod6_sinc_aver_ratio   ,   DEMOD6_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod6_sinc_dec_ratio_m2 ,   DEMOD6_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod6_enable            ,   DEMOD6_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod6_source            ,   DEMOD6_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod6_sinc_en           ,   DEMOD6_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod6_fir_order         ,   DEMOD6_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod6_alpha_l           ,   DEMOD6_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod6_alpha_m           ,   DEMOD6_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod6_alpha_h           ,   DEMOD6_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod6_constant_reg      ,   DEMOD6_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod6_psd_scaling_l     ,   DEMOD6_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod6_psd_scaling_h     ,   DEMOD6_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(0,  demod7_sinc_dec_ratio_en ,   DEMOD7_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod7_sinc_dec_ratio_m1 ,   DEMOD7_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod7_sinc_aver_ratio   ,   DEMOD7_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod7_sinc_dec_ratio_m2 ,   DEMOD7_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod7_enable            ,   DEMOD7_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod7_source            ,   DEMOD7_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod7_sinc_en           ,   DEMOD7_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod7_fir_order         ,   DEMOD7_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod7_alpha_l           ,   DEMOD7_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod7_alpha_m           ,   DEMOD7_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod7_alpha_h           ,   DEMOD7_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod7_constant_reg      ,   DEMOD7_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod7_psd_scaling_l     ,   DEMOD7_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod7_psd_scaling_h     ,   DEMOD7_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(0,  demod8_sinc_dec_ratio_en ,   DEMOD8_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  demod8_sinc_dec_ratio_m1 ,   DEMOD8_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  demod8_sinc_aver_ratio   ,   DEMOD8_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  demod8_sinc_dec_ratio_m2 ,   DEMOD8_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  demod8_enable            ,   DEMOD8_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  demod8_source            ,   DEMOD8_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  demod8_sinc_en           ,   DEMOD8_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  demod8_fir_order         ,   DEMOD8_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  demod8_alpha_l           ,   DEMOD8_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  demod8_alpha_m           ,   DEMOD8_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, demod8_alpha_h           ,   DEMOD8_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, demod8_constant_reg      ,   DEMOD8_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    reg_table[i++] = REG(12, demod8_psd_scaling_l     ,   DEMOD8_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    reg_table[i++] = REG(13, demod8_psd_scaling_h     ,   DEMOD8_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    /*module 22 end*/

    /*module 23 start*/
	reg_table[i++] = REG(0,  ch1_scpoe_up_en             ,   UP_CH1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
	reg_table[i++] = REG(1,  ch1_scpoe_average_en        ,   UP_CH1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
	reg_table[i++] = REG(2,  ch1_scope_up_data_mux       ,   UP_CH1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
	reg_table[i++] = REG(3,  ch1_scope_decimation_value  ,   UP_CH1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
	reg_table[i++] = REG(4,  ch1_daq_up_en               ,   UP_CH1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
	reg_table[i++] = REG(5,  ch1_daq_up_data_mux_l       ,   UP_CH1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
	reg_table[i++] = REG(6,  ch1_daq_up_data_mux_h       ,   UP_CH1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(7,  ch1_daq_decimation_value    ,   UP_CH1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
	reg_table[i++] = REG(8,  ch1_up_en                   ,   UP_CH1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
	reg_table[i++] = REG(9,  ch1_up_data_mode            ,   UP_CH1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
	reg_table[i++] = REG(10,  ch1_updata_soft_trig       ,   UP_CH1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
	reg_table[i++] = REG(11,  ch1_storag_cnt             ,   UP_CH1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
	reg_table[i++] = REG(12,  ch1_block_size             ,   UP_CH1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
	reg_table[i++] = REG(13,  ch1_up_data_cache_length   ,   UP_CH1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(14,  ch1_up_data_write_length   ,   UP_CH1_CONFIG_BASE + 0x000e, "NULL",         "NULL");
	reg_table[i++] = REG(15,  ch1_read_block             ,   UP_CH1_CONFIG_BASE + 0x000f, "NULL",         "NULL");
	reg_table[i++] = REG(16,  ch1_dma_up_en              ,   UP_CH1_CONFIG_BASE + 0x0010, "NULL",         "NULL");
	reg_table[i++] = REG(17,  ch1_dma_up_data_clr        ,   UP_CH1_CONFIG_BASE + 0x0011, "NULL",         "NULL");
	reg_table[i++] = REG(18,  ch1_dma_rd_length          ,   UP_CH1_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    /*module 23 end*/

    /*module 24 start*/
	reg_table[i++] = REG(0,  ch2_scpoe_up_en             ,   UP_CH2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
	reg_table[i++] = REG(1,  ch2_scpoe_average_en        ,   UP_CH2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
	reg_table[i++] = REG(2,  ch2_scope_up_data_mux       ,   UP_CH2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
	reg_table[i++] = REG(3,  ch2_scope_decimation_value  ,   UP_CH2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
	reg_table[i++] = REG(4,  ch2_daq_up_en               ,   UP_CH2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
	reg_table[i++] = REG(5,  ch2_daq_up_data_mux_l       ,   UP_CH2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
	reg_table[i++] = REG(6,  ch2_daq_up_data_mux_h       ,   UP_CH2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
	reg_table[i++] = REG(7,  ch2_daq_decimation_value    ,   UP_CH2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
	reg_table[i++] = REG(8,  ch2_up_en                   ,   UP_CH2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
	reg_table[i++] = REG(9,  ch2_up_data_mode            ,   UP_CH2_CONFIG_BASE + 0x0009, "NULL",         "NULL");
	reg_table[i++] = REG(10,  ch2_updata_soft_trig       ,   UP_CH2_CONFIG_BASE + 0x000a, "NULL",         "NULL");
	reg_table[i++] = REG(11,  ch2_storag_cnt             ,   UP_CH2_CONFIG_BASE + 0x000b, "NULL",         "NULL");
	reg_table[i++] = REG(12,  ch2_block_size             ,   UP_CH2_CONFIG_BASE + 0x000c, "NULL",         "NULL");
	reg_table[i++] = REG(13,  ch2_up_data_cache_length   ,   UP_CH2_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	reg_table[i++] = REG(14,  ch2_up_data_write_length   ,   UP_CH2_CONFIG_BASE + 0x000e, "NULL",         "NULL");
	reg_table[i++] = REG(15,  ch2_read_block             ,   UP_CH2_CONFIG_BASE + 0x000f, "NULL",         "NULL");
	reg_table[i++] = REG(16,  ch2_dma_up_en              ,   UP_CH2_CONFIG_BASE + 0x0010, "NULL",         "NULL");
	reg_table[i++] = REG(17,  ch2_dma_up_data_clr        ,   UP_CH2_CONFIG_BASE + 0x0011, "NULL",         "NULL");
	reg_table[i++] = REG(18,  ch2_dma_rd_length          ,   UP_CH2_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    /*module 24 end*/
    
    /*module 25 start*/
    reg_table[i++] = REG(0,  pulsecounter1_enable     ,   PCNT1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  pulsecounter1_mode       ,   PCNT1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  pulsecounter1_edge_mode  ,   PCNT1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  pulsecounter1_source     ,   PCNT1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  pulsecounter1_period_l   ,   PCNT1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  pulsecounter1_period_h   ,   PCNT1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  pulsecounter1_gate_source,   PCNT1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 25 end*/   

    /*module 26 start*/
    reg_table[i++] = REG(0,  pulsecounter2_enable     ,   PCNT2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  pulsecounter2_mode       ,   PCNT2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  pulsecounter2_edge_mode  ,   PCNT2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  pulsecounter2_source     ,   PCNT2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  pulsecounter2_period_l   ,   PCNT2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  pulsecounter2_period_h   ,   PCNT2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  pulsecounter2_gate_source,   PCNT2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 26 end*/ 

    /*module 27 start*/
    reg_table[i++] = REG(0,  pulsecounter3_enable     ,   PCNT3_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  pulsecounter3_mode       ,   PCNT3_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  pulsecounter3_edge_mode  ,   PCNT3_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  pulsecounter3_source     ,   PCNT3_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  pulsecounter3_period_l   ,   PCNT3_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  pulsecounter3_period_h   ,   PCNT3_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  pulsecounter3_gate_source,   PCNT3_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 27 end*/ 

    /*module 28 start*/
    reg_table[i++] = REG(0,  pulsecounter4_enable     ,   PCNT4_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  pulsecounter4_mode       ,   PCNT4_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  pulsecounter4_edge_mode  ,   PCNT4_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  pulsecounter4_source     ,   PCNT4_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  pulsecounter4_period_l   ,   PCNT4_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  pulsecounter4_period_h   ,   PCNT4_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  pulsecounter4_gate_source,   PCNT4_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    /*module 28 end*/ 

    /*module 29 start*/
    reg_table[i++] = REG(0,  auxin_relays_addr        ,   I_CARD_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  auxin_range_relays       ,   I_CARD_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  auxin_range_relays_en    ,   I_CARD_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  trigin_relays_addr       ,   I_CARD_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  trigin_range_relays      ,   I_CARD_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  trigin_range_relays_en   ,   I_CARD_CONFIG_BASE + 0x0005, "NULL",         "NULL");
	if( (model == DEVICE_MODEL_M005M_VV) || (model == DEVICE_MODEL_M005M_VI) ) {
		reg_table[i++] = REG(6,  i_relays_addr            	,   I_CARD_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    	reg_table[i++] = REG(7,  i_range_relays           	,   I_CARD_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    	reg_table[i++] = REG(8,  i_range_relays_en        	,   I_CARD_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    	reg_table[i++] = REG(9,  v_relays_addr            	,   I_CARD_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    	reg_table[i++] = REG(10, v_range_relays           	,   I_CARD_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    	reg_table[i++] = REG(11, v_range_relays_en        	,   I_CARD_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    	reg_table[i++] = REG(12, sine_relays_addr        	,   I_CARD_CONFIG_BASE + 0x000c, "NULL",         "NULL");
    	reg_table[i++] = REG(13, sine_range_relays        	,   I_CARD_CONFIG_BASE + 0x000d, "NULL",         "NULL");
    	reg_table[i++] = REG(14, sine_range_relays_en     	,   I_CARD_CONFIG_BASE + 0x000e, "NULL",         "NULL");
    	reg_table[i++] = REG(15, sinemux_relays_addr      	,   I_CARD_CONFIG_BASE + 0x000f, "NULL",         "NULL");
    	reg_table[i++] = REG(16, sinemux_range_relays     	,   I_CARD_CONFIG_BASE + 0x0010, "NULL",         "NULL");
	    reg_table[i++] = REG(17, sinemux_range_relays_en  	,   I_CARD_CONFIG_BASE + 0x0011, "NULL",         "NULL");
	} else if(model == DEVICE_MODEL_M005M) {
		reg_table[i++] = REG(6,  sine_relays_addr         	,   I_CARD_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    	reg_table[i++] = REG(7,  sine_range_relays        	,   I_CARD_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    	reg_table[i++] = REG(8,  sine_range_relays_en     	,   I_CARD_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    	reg_table[i++] = REG(9,  sinemux_relays_addr      	,   I_CARD_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    	reg_table[i++] = REG(10, sinemux_range_relays     	,   I_CARD_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    	reg_table[i++] = REG(11, sinemux_range_relays_en  	,   I_CARD_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    	reg_table[i++] = REG(12, v1_ctrl_en         		,   I_CARD_CONFIG_BASE + 0x0010, "NULL",         "NULL");
    	reg_table[i++] = REG(13, v1_ctrl_low        		,   I_CARD_CONFIG_BASE + 0x0011, "NULL",         "NULL");
    	reg_table[i++] = REG(14, v1_ctrl_high     			,   I_CARD_CONFIG_BASE + 0x0012, "NULL",         "NULL");
    	reg_table[i++] = REG(15, v2_ctrl_en      			,   I_CARD_CONFIG_BASE + 0x0013, "NULL",         "NULL");
    	reg_table[i++] = REG(16, v2_ctrl_low     			,   I_CARD_CONFIG_BASE + 0x0014, "NULL",         "NULL");
	    reg_table[i++] = REG(17, v2_ctrl_high  				,   I_CARD_CONFIG_BASE + 0x0015, "NULL",         "NULL");
        reg_table[i++] = REG(17, v2_ctrl_high  				,   I_CARD_CONFIG_BASE + 0x0015, "NULL",         "NULL");
        reg_table[i++] = REG(17, test1        				,   I_CARD_CONFIG_BASE + 0x0016, "NULL",         "NULL");
        reg_table[i++] = REG(18, test2        				,   I_CARD_CONFIG_BASE + 0x0017, "NULL",         "NULL");
        reg_table[i++] = REG(19, test3  		    		,   I_CARD_CONFIG_BASE + 0x0020, "NULL",         "NULL");
        reg_table[i++] = REG(20, test4  				    ,   I_CARD_CONFIG_BASE + 0x0021, "NULL",         "NULL");
        reg_table[i++] = REG(21, test5 				        ,   I_CARD_CONFIG_BASE + 0x0022, "NULL",         "NULL");
        reg_table[i++] = REG(22, test6 				        ,   I_CARD_CONFIG_BASE + 0x0023, "NULL",         "NULL");
	} else if(model == DEVICE_MODEL_L500K) {
        //signal output
        reg_table[i++] = REG(15, sine_relays_addr           ,   I_CARD_CONFIG_BASE + 0x000F, "NULL",         "NULL");
        reg_table[i++] = REG(16, sine_range_relays          ,   I_CARD_CONFIG_BASE + 0x0010, "NULL",         "NULL");
        reg_table[i++] = REG(17, sine_range_relays_en       ,   I_CARD_CONFIG_BASE + 0x0011, "NULL",         "NULL");
        reg_table[i++] = REG(18, sinemux_relays_addr        ,   I_CARD_CONFIG_BASE + 0x0012, "NULL",         "NULL");
        reg_table[i++] = REG(19, sinemux_range_relays       ,   I_CARD_CONFIG_BASE + 0x0013, "NULL",         "NULL");
        reg_table[i++] = REG(20, sinemux_range_relays_en    ,   I_CARD_CONFIG_BASE + 0x0014, "NULL",         "NULL");
        //signal input
        reg_table[i++] = REG(6,  sinput2_reg_addr           ,   I_CARD_CONFIG_BASE + 0x0006, "NULL",         "NULL");
        reg_table[i++] = REG(7,  sinput2_reg_value          ,   I_CARD_CONFIG_BASE + 0x0007, "NULL",         "NULL");
        reg_table[i++] = REG(8,  sinput2_reg_enable         ,   I_CARD_CONFIG_BASE + 0x0008, "NULL",         "NULL");
        reg_table[i++] = REG(9,  sinput1_reg_addr           ,   I_CARD_CONFIG_BASE + 0x0009, "NULL",         "NULL");
        reg_table[i++] = REG(10, sinput1_reg_value          ,   I_CARD_CONFIG_BASE + 0x000a, "NULL",         "NULL");
        reg_table[i++] = REG(11, sinput1_reg_enable         ,   I_CARD_CONFIG_BASE + 0x000b, "NULL",         "NULL");
	}else {

    }
	
    /*module 29 end*/ 


    /*module 30 start*/
    reg_table[i++] = REG(0,  trigin_trim_en           ,   TRIM_CONFIG_BASE + 0x0000, "NULL",         "NULL");
    reg_table[i++] = REG(1,  trigin_trim_ch           ,   TRIM_CONFIG_BASE + 0x0001, "NULL",         "NULL");
    reg_table[i++] = REG(2,  trigin_trim              ,   TRIM_CONFIG_BASE + 0x0002, "NULL",         "NULL");
    reg_table[i++] = REG(3,  signalout_trim_en        ,   TRIM_CONFIG_BASE + 0x0003, "NULL",         "NULL");
    reg_table[i++] = REG(4,  signalout_trim_ch        ,   TRIM_CONFIG_BASE + 0x0004, "NULL",         "NULL");
    reg_table[i++] = REG(5,  signalout_trim           ,   TRIM_CONFIG_BASE + 0x0005, "NULL",         "NULL");
    reg_table[i++] = REG(6,  i_trim_en                ,   TRIM_CONFIG_BASE + 0x0006, "NULL",         "NULL");
    reg_table[i++] = REG(7,  i_trim_ch                ,   TRIM_CONFIG_BASE + 0x0007, "NULL",         "NULL");
    reg_table[i++] = REG(8,  i_trim                   ,   TRIM_CONFIG_BASE + 0x0008, "NULL",         "NULL");
    reg_table[i++] = REG(9,  v_trim_en                ,   TRIM_CONFIG_BASE + 0x0009, "NULL",         "NULL");
    reg_table[i++] = REG(10, v_trim_ch                ,   TRIM_CONFIG_BASE + 0x000a, "NULL",         "NULL");
    reg_table[i++] = REG(11, v_trim                   ,   TRIM_CONFIG_BASE + 0x000b, "NULL",         "NULL");
    /*module 30 end*/

    /*module 31 start*/
	if( (model == DEVICE_MODEL_M005M_VV) || (model == DEVICE_MODEL_M005M_VI) || (model == DEVICE_MODEL_M005M) ) {
		reg_table[i++] = REG(0,  awg1_addr_en              ,   AWG1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
		reg_table[i++] = REG(1,  awg1_addr                 ,   AWG1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
		reg_table[i++] = REG(2,  awg1_play_en              ,   AWG1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
		reg_table[i++] = REG(3,  awg1_play_mode            ,   AWG1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
		reg_table[i++] = REG(4,  awg1_play_seq_cnt         ,   AWG1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
		reg_table[i++] = REG(5,  awg1_play_start_addr      ,   AWG1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
		reg_table[i++] = REG(6,  awg1_play_length          ,   AWG1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
		reg_table[i++] = REG(7,  awg1_play_cnt             ,   AWG1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
		reg_table[i++] = REG(8,  awg1_play_waittime        ,   AWG1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
		reg_table[i++] = REG(9,  awg1_param_addr_en        ,   AWG1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
		reg_table[i++] = REG(10, awg1_param_addr           ,   AWG1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
		reg_table[i++] = REG(11, awg1_seq_param_addr_en    ,   AWG1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
		reg_table[i++] = REG(12, awg1_seq_param_addr       ,   AWG1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
		reg_table[i++] = REG(13, awg1_seq_param            ,   AWG1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
		reg_table[i++] = REG(0,  awg2_addr_en              ,   AWG2_CONFIG_BASE + 0x0000, "NULL",         "NULL");
		reg_table[i++] = REG(1,  awg2_addr                 ,   AWG2_CONFIG_BASE + 0x0001, "NULL",         "NULL");
		reg_table[i++] = REG(2,  awg2_play_en              ,   AWG2_CONFIG_BASE + 0x0002, "NULL",         "NULL");
		reg_table[i++] = REG(3,  awg2_play_mode            ,   AWG2_CONFIG_BASE + 0x0003, "NULL",         "NULL");
		reg_table[i++] = REG(4,  awg2_play_seq_cnt         ,   AWG2_CONFIG_BASE + 0x0004, "NULL",         "NULL");
		reg_table[i++] = REG(5,  awg2_play_start_addr      ,   AWG2_CONFIG_BASE + 0x0005, "NULL",         "NULL");
		reg_table[i++] = REG(6,  awg2_play_length          ,   AWG2_CONFIG_BASE + 0x0006, "NULL",         "NULL");
		reg_table[i++] = REG(7,  awg2_play_cnt             ,   AWG2_CONFIG_BASE + 0x0007, "NULL",         "NULL");
		reg_table[i++] = REG(8,  awg2_play_waittime        ,   AWG2_CONFIG_BASE + 0x0008, "NULL",         "NULL");
		reg_table[i++] = REG(9,  awg2_param_addr_en        ,   AWG2_CONFIG_BASE + 0x0009, "NULL",         "NULL");
		reg_table[i++] = REG(10, awg2_param_addr           ,   AWG2_CONFIG_BASE + 0x000a, "NULL",         "NULL");
		reg_table[i++] = REG(11, awg2_seq_param_addr_en    ,   AWG2_CONFIG_BASE + 0x000b, "NULL",         "NULL");
		reg_table[i++] = REG(12, awg2_seq_param_addr       ,   AWG2_CONFIG_BASE + 0x000c, "NULL",         "NULL");
		reg_table[i++] = REG(13, awg2_seq_param            ,   AWG2_CONFIG_BASE + 0x000d, "NULL",         "NULL");
	} else if(model == DEVICE_MODEL_L500K) {
		reg_table[i++] = REG(0,  awg1_ch_en                ,   AWG1_CONFIG_BASE + 0x0000, "NULL",         "NULL");
		reg_table[i++] = REG(1,  awg1_play_mode            ,   AWG1_CONFIG_BASE + 0x0001, "NULL",         "NULL");
		reg_table[i++] = REG(2,  awg1_start_read           ,   AWG1_CONFIG_BASE + 0x0002, "NULL",         "NULL");
		reg_table[i++] = REG(3,  awg1_play_en              ,   AWG1_CONFIG_BASE + 0x0003, "NULL",         "NULL");
		reg_table[i++] = REG(4,  awg1_sample_time          ,   AWG1_CONFIG_BASE + 0x0004, "NULL",         "NULL");
		reg_table[i++] = REG(5,  awg1_play_seq_cnt         ,   AWG1_CONFIG_BASE + 0x0005, "NULL",         "NULL");
		reg_table[i++] = REG(6,  awg1_param_addr_en        ,   AWG1_CONFIG_BASE + 0x0006, "NULL",         "NULL");
		reg_table[i++] = REG(7,  awg1_param_addr           ,   AWG1_CONFIG_BASE + 0x0007, "NULL",         "NULL");
		reg_table[i++] = REG(8,  awg1_seq_param_addr_en    ,   AWG1_CONFIG_BASE + 0x0008, "NULL",         "NULL");
		reg_table[i++] = REG(9,  awg1_seq_param_addr       ,   AWG1_CONFIG_BASE + 0x0009, "NULL",         "NULL");
		reg_table[i++] = REG(10, awg1_seq_param            ,   AWG1_CONFIG_BASE + 0x000a, "NULL",         "NULL");
		reg_table[i++] = REG(11, awg1_play_start_addr      ,   AWG1_CONFIG_BASE + 0x000b, "NULL",         "NULL");
		reg_table[i++] = REG(12, awg1_play_length          ,   AWG1_CONFIG_BASE + 0x000c, "NULL",         "NULL");
		reg_table[i++] = REG(13, awg1_play_cnt             ,   AWG1_CONFIG_BASE + 0x000d, "NULL",         "NULL");
		reg_table[i++] = REG(14, awg1_play_waittime        ,   AWG1_CONFIG_BASE + 0x000e, "NULL",         "NULL");
		reg_table[i++] = REG(0,  awg2_ch_en                ,   AWG2_L500K_CONFIG_BASE + 0x0000, "NULL",         "NULL");
		reg_table[i++] = REG(1,  awg2_play_mode            ,   AWG2_L500K_CONFIG_BASE + 0x0001, "NULL",         "NULL");
		reg_table[i++] = REG(2,  awg2_start_read           ,   AWG2_L500K_CONFIG_BASE + 0x0002, "NULL",         "NULL");
		reg_table[i++] = REG(3,  awg2_play_en              ,   AWG2_L500K_CONFIG_BASE + 0x0003, "NULL",         "NULL");
		reg_table[i++] = REG(4,  awg2_sample_time          ,   AWG2_L500K_CONFIG_BASE + 0x0004, "NULL",         "NULL");
		reg_table[i++] = REG(5,  awg2_play_seq_cnt         ,   AWG2_L500K_CONFIG_BASE + 0x0005, "NULL",         "NULL");
		reg_table[i++] = REG(6,  awg2_param_addr_en        ,   AWG2_L500K_CONFIG_BASE + 0x0006, "NULL",         "NULL");
		reg_table[i++] = REG(7,  awg2_param_addr           ,   AWG2_L500K_CONFIG_BASE + 0x0007, "NULL",         "NULL");
		reg_table[i++] = REG(8,  awg2_seq_param_addr_en    ,   AWG2_L500K_CONFIG_BASE + 0x0008, "NULL",         "NULL");
		reg_table[i++] = REG(9,  awg2_seq_param_addr       ,   AWG2_L500K_CONFIG_BASE + 0x0009, "NULL",         "NULL");
		reg_table[i++] = REG(10, awg2_seq_param            ,   AWG2_L500K_CONFIG_BASE + 0x000a, "NULL",         "NULL");
		reg_table[i++] = REG(11, awg2_play_start_addr      ,   AWG2_L500K_CONFIG_BASE + 0x000b, "NULL",         "NULL");
		reg_table[i++] = REG(12, awg2_play_length          ,   AWG2_L500K_CONFIG_BASE + 0x000c, "NULL",         "NULL");
		reg_table[i++] = REG(13, awg2_play_cnt             ,   AWG2_L500K_CONFIG_BASE + 0x000d, "NULL",         "NULL");
		reg_table[i++] = REG(14, awg2_play_waittime        ,   AWG2_L500K_CONFIG_BASE + 0x000e, "NULL",         "NULL");
	}
    /*module 31 end*/
};


#define REG_TABLE_SIZE (sizeof(reg_table) / sizeof(reg_table[0]))

void print_reg_info()
{
	unsigned int i;
    for (i = 0; i < REG_TABLE_SIZE; ++i) {
        printf("%-25s [0x%04X]  %s  (%s)\n",
               reg_table[i].name,
               reg_table[i].offset,
               reg_table[i].desc,
               reg_table[i].options);
    }
}

 uint32_t reg_read(uint32_t offset)
 {
 	AxiDevice *pdev = NULL;
 	
 	get_plreg_intance(&pdev);
 
 	return pdev->reg_base[offset];
 }

void reg_write(uint32_t offset, uint32_t value)
{
    AxiDevice *pdev = NULL;
	
	get_plreg_intance(&pdev);

	//printf("offset: 0x%08x value: 0x%08x\r\n", offset, value);
	pdev->reg_base[offset] = value;
}


int find_reg_offset(const char* name, uint32_t* offset_out)
{
	unsigned int i;
    for (i = 0; i < REG_TABLE_SIZE; ++i) {
        if (strcmp(name, reg_table[i].name) == 0) {
            *offset_out = reg_table[i].offset;
            return 0;
        }
    }
	
    return -1;  // no find
}


int reg_read_by_name(const char* name, uint32_t* value_out)
{
    uint32_t offset;
    if (find_reg_offset(name, &offset) != 0) {
        return -1;
    }
    *value_out = reg_read(offset);
    return 0;
}


int reg_write_by_name(const char* name, uint32_t value)
{
    uint32_t offset;
    if (find_reg_offset(name, &offset) != 0) {
		printf("Param: %s no find\r\n", name);
		return -1;
    }
    reg_write(offset, value);
    return 0;
}


const reg_entry_t* get_reg_entry(const char* name) 
{
	unsigned int i;
    for (i = 0; i < REG_TABLE_SIZE; ++i) {
        if (strcmp(name, reg_table[i].name) == 0) {
            return &reg_table[i];
        }
    }
    return NULL;
}


// ??????????��?????
int write_reg_by_name(const char* name, const char* str_val)
{
    uint32_t offset;
    if (find_reg_offset(name, &offset) != 0)
        return -1;

    const reg_entry_t* reg = get_reg_entry(name);
    if (!reg)
        return -1;

    uint32_t val = 0;

    // ?????? options???????
    if (strcmp(reg->options, "bool") == 0) {
        val = (!strcmp(str_val, "1") || !strcasecmp(str_val, "true")) ? 1 : 0;
    } else if (strncmp(reg->options, "int", 3) == 0) {
        val = (uint32_t)atoi(str_val);
    } else if (strstr(reg->options, "|")) {
        // ?? "ADC1|ADC2|EXT"
        const char* opts = reg->options;
        int idx = 0;
        char buf[64];
        while (*opts) {
            const char* end = strchr(opts, '|');
            size_t len = end ? (size_t)(end - opts) : strlen(opts);
            strncpy(buf, opts, len); buf[len] = 0;
            if (strcasecmp(buf, str_val) == 0) {
                val = idx;
                break;
            }
            idx++;
            if (!end) break;
            opts = end + 1;
        }
    } else {
        // ????????
        val = (uint32_t)atoi(str_val);
    }

    reg_write(offset, val);
    return 0;
}


#if 0
void example()
{
	uint32_t val;
	if (reg_read_by_name("SCOPE_TRANS_SPEED", &val) == 0) {
	    printf("???????: %u\n", val);
	}

	if (reg_write_by_name("SCOPE_TRANS_SPEED", 100) == 0) {
	    printf("��????\n");
	}
}
#endif

