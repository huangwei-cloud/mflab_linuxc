#include "reg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "../clk/hmc7043.h"

#define SYSTEM_CONFIG_BASE      0x0000
#define PLL_CONFIG_BASE         0x0700
#define DEMOD_CONFIG_BASE       0x0800
#define SIGNALOUT_CONFIG_BASE   0x0C00
#define TRIG_CONFIG_BASE        0x1400
#define PID_CONFIG_BASE         0x1600
#define UP_DATA_CONFIG_BASE     0x1A00
#define I_CARD_CONFIG_BASE      0x1C00
#define OSC_CONFIG_BASE         0x2200
#define  AWG1_CONFIG_BASE       0x2700
#define  AWG2_CONFIG_BASE       0x3700

#define REG_CNT_MAX             1000

static reg_entry_t reg_table[REG_CNT_MAX];
static unsigned int reg_table_size;

void init_reg()
{
    int i = 0;

    memset(reg_table, 0, sizeof(reg_table));

    /* system ctrl */
    reg_table[i++] = REG(0,  soft_trig                 , SYSTEM_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  osc_work_all              , SYSTEM_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  fan_pwm_val               , SYSTEM_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  load_delay                , SYSTEM_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  ltc2000_1_odelay          , SYSTEM_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  ltc2000_2_odelay          , SYSTEM_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  ad9684_idelay             , SYSTEM_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  pll_freq_word_l           , SYSTEM_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(8,  pll_freq_word_h           , SYSTEM_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(9,  sync_ok                   , SYSTEM_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(10, zam6228_temp_data1        , SYSTEM_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(11, zam6229_temp_data2        , SYSTEM_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(12, zam6230_temp_data3        , SYSTEM_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(13, daq_data_source           , SYSTEM_CONFIG_BASE + 0x0200, "NULL", "NULL");

    /* signalout1 config */
    reg_table[i++] = REG(0,  signalout1_osc_enable     , SIGNALOUT_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  signalout1_osc_mode       , SIGNALOUT_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  signalout1_osc_rst        , SIGNALOUT_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  signalout1_osc_phase_l    , SIGNALOUT_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  signalout1_osc_phase_h    , SIGNALOUT_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  signalout1_osc_freq_word_l, SIGNALOUT_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  signalout1_osc_freq_word_h, SIGNALOUT_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  signalout1_enable         , SIGNALOUT_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  signalout1_source1        , SIGNALOUT_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  signalout1_scaling1_l     , SIGNALOUT_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, signalout1_scaling1_h     , SIGNALOUT_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, singalout1_offset_en      , SIGNALOUT_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, singalout1_offset         , SIGNALOUT_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, signalout1_limit_en       , SIGNALOUT_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, signalout1_upper_limit    , SIGNALOUT_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, signalout1_lower_limit    , SIGNALOUT_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, signalout1_constant1      , SIGNALOUT_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, signalout1_osc_type       , SIGNALOUT_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, signalout1_osc_pluse_duty , SIGNALOUT_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(19, signalout1_osc_scaling    , SIGNALOUT_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(20, signalout1_add_osc1_en    , SIGNALOUT_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(21, signalout1_add_osc2_en    , SIGNALOUT_CONFIG_BASE + 0x0015, "NULL", "NULL");

    /* signalout2 config */
    reg_table[i++] = REG(0,  signalout2_osc_enable     , SIGNALOUT_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(1,  signalout2_osc_mode       , SIGNALOUT_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(2,  signalout2_osc_rst        , SIGNALOUT_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(3,  signalout2_osc_phase_l    , SIGNALOUT_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(4,  signalout2_osc_phase_h    , SIGNALOUT_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(5,  signalout2_osc_freq_word_l, SIGNALOUT_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(6,  signalout2_osc_freq_word_h, SIGNALOUT_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(7,  signalout2_enable         , SIGNALOUT_CONFIG_BASE + 0x0107, "NULL", "NULL");
    reg_table[i++] = REG(8,  signalout2_source1        , SIGNALOUT_CONFIG_BASE + 0x0108, "NULL", "NULL");
    reg_table[i++] = REG(9,  signalout2_scaling1_l     , SIGNALOUT_CONFIG_BASE + 0x0109, "NULL", "NULL");
    reg_table[i++] = REG(10, signalout2_scaling1_h     , SIGNALOUT_CONFIG_BASE + 0x010A, "NULL", "NULL");
    reg_table[i++] = REG(11, singalout2_offset_en      , SIGNALOUT_CONFIG_BASE + 0x010B, "NULL", "NULL");
    reg_table[i++] = REG(12, singalout2_offset         , SIGNALOUT_CONFIG_BASE + 0x010C, "NULL", "NULL");
    reg_table[i++] = REG(13, signalout2_limit_en       , SIGNALOUT_CONFIG_BASE + 0x010D, "NULL", "NULL");
    reg_table[i++] = REG(14, signalout2_upper_limit    , SIGNALOUT_CONFIG_BASE + 0x010E, "NULL", "NULL");
    reg_table[i++] = REG(15, signalout2_lower_limit    , SIGNALOUT_CONFIG_BASE + 0x010F, "NULL", "NULL");
    reg_table[i++] = REG(16, signalout2_constant1      , SIGNALOUT_CONFIG_BASE + 0x0110, "NULL", "NULL");
    reg_table[i++] = REG(17, signalout2_osc_type       , SIGNALOUT_CONFIG_BASE + 0x0111, "NULL", "NULL");
    reg_table[i++] = REG(18, signalout2_osc_pluse_duty , SIGNALOUT_CONFIG_BASE + 0x0112, "NULL", "NULL");
    reg_table[i++] = REG(19, signalout2_osc_scaling    , SIGNALOUT_CONFIG_BASE + 0x0113, "NULL", "NULL");
    reg_table[i++] = REG(20, signalout2_add_osc1_en    , SIGNALOUT_CONFIG_BASE + 0x0114, "NULL", "NULL");
    reg_table[i++] = REG(21, signalout2_add_osc2_en    , SIGNALOUT_CONFIG_BASE + 0x0115, "NULL", "NULL");

    /* trig config */
    reg_table[i++] = REG(0,  clk_sync_mode             , TRIG_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  sync_trig                 , TRIG_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  sync_en                   , TRIG_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  trig_in_mode              , TRIG_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(4,  trig_in_source            , TRIG_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(5,  trig_en                   , TRIG_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(6,  trig_source               , TRIG_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(7,  trig_mode                 , TRIG_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(8,  trig_delay                , TRIG_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(9,  trig_release_restrain     , TRIG_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(10, trig_plus_width           , TRIG_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(11, hysteresis_range_high_l   , TRIG_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(12, hysteresis_range_high_h   , TRIG_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(13, hysteresis_range_low_l    , TRIG_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(14, hysteresis_range_low_h    , TRIG_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(15, trig_out_mode             , TRIG_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(16, trig_out_plus_period      , TRIG_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(17, trig_out_plus_width       , TRIG_CONFIG_BASE + 0x0015, "NULL", "NULL");
    reg_table[i++] = REG(18, trig_out_plus_loop        , TRIG_CONFIG_BASE + 0x0016, "NULL", "NULL");

    /* PLL config */
    reg_table[i++] = REG(0, pll_enable                 , PLL_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1, pll_source                 , PLL_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2, pll_ref_hysteresis_high    , PLL_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3, pll_ref_hysteresis_low     , PLL_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4, pll_mov_fir_ratio          , PLL_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5, pll_calc_time_l            , PLL_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6, pll_calc_time_h            , PLL_CONFIG_BASE + 0x0006, "NULL", "NULL");

    /* PID config */
    reg_table[i++] = REG(0,  pid1_enable               , PID_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  pid1_ki_enable            , PID_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  pid1_moving_aver_ratio    , PID_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  pid1_source               , PID_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  pid1_set_point_l          , PID_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  pid1_set_point_h          , PID_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  pid1_Kp_l                 , PID_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  pid1_Kp_h                 , PID_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  pid1_Ki_l                 , PID_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  pid1_Ki_h                 , PID_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, pid1_Kd_l                 , PID_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid1_Kd_h                 , PID_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid1_decimation           , PID_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid1_invert               , PID_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid1_normaliztion_v_l     , PID_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid1_normaliztion_v_h     , PID_CONFIG_BASE + 0x000F, "NULL", "NULL");

    reg_table[i++] = REG(0,  pid2_enable               , PID_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(1,  pid2_ki_enable            , PID_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(2,  pid2_moving_aver_ratio    , PID_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(3,  pid2_source               , PID_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(4,  pid2_set_point_l          , PID_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(5,  pid2_set_point_h          , PID_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(6,  pid2_Kp_l                 , PID_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(7,  pid2_Kp_h                 , PID_CONFIG_BASE + 0x0107, "NULL", "NULL");
    reg_table[i++] = REG(8,  pid2_Ki_l                 , PID_CONFIG_BASE + 0x0108, "NULL", "NULL");
    reg_table[i++] = REG(9,  pid2_Ki_h                 , PID_CONFIG_BASE + 0x0109, "NULL", "NULL");
    reg_table[i++] = REG(10, pid2_Kd_l                 , PID_CONFIG_BASE + 0x010A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid2_Kd_h                 , PID_CONFIG_BASE + 0x010B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid2_decimation           , PID_CONFIG_BASE + 0x010C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid2_invert               , PID_CONFIG_BASE + 0x010D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid2_normaliztion_v_l     , PID_CONFIG_BASE + 0x010E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid2_normaliztion_v_h     , PID_CONFIG_BASE + 0x010F, "NULL", "NULL");

    reg_table[i++] = REG(0,  pid3_enable               , PID_CONFIG_BASE + 0x0200, "NULL", "NULL");
    reg_table[i++] = REG(1,  pid3_ki_enable            , PID_CONFIG_BASE + 0x0201, "NULL", "NULL");
    reg_table[i++] = REG(2,  pid3_moving_aver_ratio    , PID_CONFIG_BASE + 0x0202, "NULL", "NULL");
    reg_table[i++] = REG(3,  pid3_source               , PID_CONFIG_BASE + 0x0203, "NULL", "NULL");
    reg_table[i++] = REG(4,  pid3_set_point_l          , PID_CONFIG_BASE + 0x0204, "NULL", "NULL");
    reg_table[i++] = REG(5,  pid3_set_point_h          , PID_CONFIG_BASE + 0x0205, "NULL", "NULL");
    reg_table[i++] = REG(6,  pid3_Kp_l                 , PID_CONFIG_BASE + 0x0206, "NULL", "NULL");
    reg_table[i++] = REG(7,  pid3_Kp_h                 , PID_CONFIG_BASE + 0x0207, "NULL", "NULL");
    reg_table[i++] = REG(8,  pid3_Ki_l                 , PID_CONFIG_BASE + 0x0208, "NULL", "NULL");
    reg_table[i++] = REG(9,  pid3_Ki_h                 , PID_CONFIG_BASE + 0x0209, "NULL", "NULL");
    reg_table[i++] = REG(10, pid3_Kd_l                 , PID_CONFIG_BASE + 0x020A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid3_Kd_h                 , PID_CONFIG_BASE + 0x020B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid3_decimation           , PID_CONFIG_BASE + 0x020C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid3_invert               , PID_CONFIG_BASE + 0x020D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid3_normaliztion_v_l     , PID_CONFIG_BASE + 0x020E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid3_normaliztion_v_h     , PID_CONFIG_BASE + 0x020F, "NULL", "NULL");

    reg_table[i++] = REG(0,  pid4_enable               , PID_CONFIG_BASE + 0x0300, "NULL", "NULL");
    reg_table[i++] = REG(1,  pid4_ki_enable            , PID_CONFIG_BASE + 0x0301, "NULL", "NULL");
    reg_table[i++] = REG(2,  pid4_moving_aver_ratio    , PID_CONFIG_BASE + 0x0302, "NULL", "NULL");
    reg_table[i++] = REG(3,  pid4_source               , PID_CONFIG_BASE + 0x0303, "NULL", "NULL");
    reg_table[i++] = REG(4,  pid4_set_point_l          , PID_CONFIG_BASE + 0x0304, "NULL", "NULL");
    reg_table[i++] = REG(5,  pid4_set_point_h          , PID_CONFIG_BASE + 0x0305, "NULL", "NULL");
    reg_table[i++] = REG(6,  pid4_Kp_l                 , PID_CONFIG_BASE + 0x0306, "NULL", "NULL");
    reg_table[i++] = REG(7,  pid4_Kp_h                 , PID_CONFIG_BASE + 0x0307, "NULL", "NULL");
    reg_table[i++] = REG(8,  pid4_Ki_l                 , PID_CONFIG_BASE + 0x0308, "NULL", "NULL");
    reg_table[i++] = REG(9,  pid4_Ki_h                 , PID_CONFIG_BASE + 0x0309, "NULL", "NULL");
    reg_table[i++] = REG(10, pid4_Kd_l                 , PID_CONFIG_BASE + 0x030A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid4_Kd_h                 , PID_CONFIG_BASE + 0x030B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid4_decimation           , PID_CONFIG_BASE + 0x030C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid4_invert               , PID_CONFIG_BASE + 0x030D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid4_normaliztion_v_l     , PID_CONFIG_BASE + 0x030E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid4_normaliztion_v_h     , PID_CONFIG_BASE + 0x030F, "NULL", "NULL");

    /* OSC config */
    reg_table[i++] = REG(0,  oscillators1_en           , OSC_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators1_mode         , OSC_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators1_rst          , OSC_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators1_phase_l      , OSC_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators1_phase_h      , OSC_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators1_freq_word_l  , OSC_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators1_freq_word_h  , OSC_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  am_fm_enable              , OSC_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  am_fm_mode                , OSC_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  modulation_multi_1        , OSC_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, am_n1                     , OSC_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, am_n2                     , OSC_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, am_n3                     , OSC_CONFIG_BASE + 0x000C, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators2_en           , OSC_CONFIG_BASE + 0x0050, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators2_mode         , OSC_CONFIG_BASE + 0x0051, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators2_rst          , OSC_CONFIG_BASE + 0x0052, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators2_phase_l      , OSC_CONFIG_BASE + 0x0053, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators2_phase_h      , OSC_CONFIG_BASE + 0x0054, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators2_freq_word_l  , OSC_CONFIG_BASE + 0x0055, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators2_freq_word_h  , OSC_CONFIG_BASE + 0x0056, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators3_en           , OSC_CONFIG_BASE + 0x00A0, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators3_mode         , OSC_CONFIG_BASE + 0x00A1, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators3_rst          , OSC_CONFIG_BASE + 0x00A2, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators3_phase_l      , OSC_CONFIG_BASE + 0x00A3, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators3_phase_h      , OSC_CONFIG_BASE + 0x00A4, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators3_freq_word_l  , OSC_CONFIG_BASE + 0x00A5, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators3_freq_word_h  , OSC_CONFIG_BASE + 0x00A6, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators4_en           , OSC_CONFIG_BASE + 0x00F0, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators4_mode         , OSC_CONFIG_BASE + 0x00F1, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators4_rst          , OSC_CONFIG_BASE + 0x00F2, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators4_phase_l      , OSC_CONFIG_BASE + 0x00F3, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators4_phase_h      , OSC_CONFIG_BASE + 0x00F4, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators4_freq_word_l  , OSC_CONFIG_BASE + 0x00F5, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators4_freq_word_h  , OSC_CONFIG_BASE + 0x00F6, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators5_en           , OSC_CONFIG_BASE + 0x0140, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators5_mode         , OSC_CONFIG_BASE + 0x0141, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators5_rst          , OSC_CONFIG_BASE + 0x0142, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators5_phase_l      , OSC_CONFIG_BASE + 0x0143, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators5_phase_h      , OSC_CONFIG_BASE + 0x0144, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators5_freq_word_l  , OSC_CONFIG_BASE + 0x0145, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators5_freq_word_h  , OSC_CONFIG_BASE + 0x0146, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators6_en           , OSC_CONFIG_BASE + 0x0190, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators6_mode         , OSC_CONFIG_BASE + 0x0191, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators6_rst          , OSC_CONFIG_BASE + 0x0192, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators6_phase_l      , OSC_CONFIG_BASE + 0x0193, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators6_phase_h      , OSC_CONFIG_BASE + 0x0194, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators6_freq_word_l  , OSC_CONFIG_BASE + 0x0195, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators6_freq_word_h  , OSC_CONFIG_BASE + 0x0196, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators7_en           , OSC_CONFIG_BASE + 0x01E0, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators7_mode         , OSC_CONFIG_BASE + 0x01E1, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators7_rst          , OSC_CONFIG_BASE + 0x01E2, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators7_phase_l      , OSC_CONFIG_BASE + 0x01E3, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators7_phase_h      , OSC_CONFIG_BASE + 0x01E4, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators7_freq_word_l  , OSC_CONFIG_BASE + 0x01E5, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators7_freq_word_h  , OSC_CONFIG_BASE + 0x01E6, "NULL", "NULL");

    reg_table[i++] = REG(0,  oscillators8_en           , OSC_CONFIG_BASE + 0x0230, "NULL", "NULL");
    reg_table[i++] = REG(1,  oscillators8_mode         , OSC_CONFIG_BASE + 0x0231, "NULL", "NULL");
    reg_table[i++] = REG(2,  oscillators8_rst          , OSC_CONFIG_BASE + 0x0232, "NULL", "NULL");
    reg_table[i++] = REG(3,  oscillators8_phase_l      , OSC_CONFIG_BASE + 0x0233, "NULL", "NULL");
    reg_table[i++] = REG(4,  oscillators8_phase_h      , OSC_CONFIG_BASE + 0x0234, "NULL", "NULL");
    reg_table[i++] = REG(5,  oscillators8_freq_word_l  , OSC_CONFIG_BASE + 0x0235, "NULL", "NULL");
    reg_table[i++] = REG(6,  oscillators8_freq_word_h  , OSC_CONFIG_BASE + 0x0236, "NULL", "NULL");

    /* demod config */
    reg_table[i++] = REG(0,  demod1_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod1_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod1_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod1_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod1_enable             , DEMOD_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod1_source             , DEMOD_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod1_sinc_en            , DEMOD_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod1_fir_order          , DEMOD_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod1_alpha_l            , DEMOD_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod1_alpha_m            , DEMOD_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, demod1_alpha_h            , DEMOD_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod1_constant_reg       , DEMOD_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod1_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod1_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, ref_source                , DEMOD_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, ref_constant              , DEMOD_CONFIG_BASE + 0x000F, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod2_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x0050, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod2_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x0051, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod2_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x0052, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod2_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x0053, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod2_enable             , DEMOD_CONFIG_BASE + 0x0054, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod2_source             , DEMOD_CONFIG_BASE + 0x0055, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod2_sinc_en            , DEMOD_CONFIG_BASE + 0x0056, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod2_fir_order          , DEMOD_CONFIG_BASE + 0x0057, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod2_alpha_l            , DEMOD_CONFIG_BASE + 0x0058, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod2_alpha_m            , DEMOD_CONFIG_BASE + 0x0059, "NULL", "NULL");
    reg_table[i++] = REG(10, demod2_alpha_h            , DEMOD_CONFIG_BASE + 0x005A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod2_constant_reg       , DEMOD_CONFIG_BASE + 0x005B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod2_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x005C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod2_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x005D, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod3_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x00A0, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod3_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x00A1, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod3_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x00A2, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod3_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x00A3, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod3_enable             , DEMOD_CONFIG_BASE + 0x00A4, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod3_source             , DEMOD_CONFIG_BASE + 0x00A5, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod3_sinc_en            , DEMOD_CONFIG_BASE + 0x00A6, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod3_fir_order          , DEMOD_CONFIG_BASE + 0x00A7, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod3_alpha_l            , DEMOD_CONFIG_BASE + 0x00A8, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod3_alpha_m            , DEMOD_CONFIG_BASE + 0x00A9, "NULL", "NULL");
    reg_table[i++] = REG(10, demod3_alpha_h            , DEMOD_CONFIG_BASE + 0x00AA, "NULL", "NULL");
    reg_table[i++] = REG(11, demod3_constant_reg       , DEMOD_CONFIG_BASE + 0x00AB, "NULL", "NULL");
    reg_table[i++] = REG(12, demod3_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x00AC, "NULL", "NULL");
    reg_table[i++] = REG(13, demod3_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x00AD, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod4_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x00F0, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod4_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x00F1, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod4_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x00F2, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod4_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x00F3, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod4_enable             , DEMOD_CONFIG_BASE + 0x00F4, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod4_source             , DEMOD_CONFIG_BASE + 0x00F5, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod4_sinc_en            , DEMOD_CONFIG_BASE + 0x00F6, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod4_fir_order          , DEMOD_CONFIG_BASE + 0x00F7, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod4_alpha_l            , DEMOD_CONFIG_BASE + 0x00F8, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod4_alpha_m            , DEMOD_CONFIG_BASE + 0x00F9, "NULL", "NULL");
    reg_table[i++] = REG(10, demod4_alpha_h            , DEMOD_CONFIG_BASE + 0x00FA, "NULL", "NULL");
    reg_table[i++] = REG(11, demod4_constant_reg       , DEMOD_CONFIG_BASE + 0x00FB, "NULL", "NULL");
    reg_table[i++] = REG(12, demod4_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x00FC, "NULL", "NULL");
    reg_table[i++] = REG(13, demod4_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x00FD, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod5_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x0140, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod5_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x0141, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod5_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x0142, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod5_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x0143, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod5_enable             , DEMOD_CONFIG_BASE + 0x0144, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod5_source             , DEMOD_CONFIG_BASE + 0x0145, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod5_sinc_en            , DEMOD_CONFIG_BASE + 0x0146, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod5_fir_order          , DEMOD_CONFIG_BASE + 0x0147, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod5_alpha_l            , DEMOD_CONFIG_BASE + 0x0148, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod5_alpha_m            , DEMOD_CONFIG_BASE + 0x0149, "NULL", "NULL");
    reg_table[i++] = REG(10, demod5_alpha_h            , DEMOD_CONFIG_BASE + 0x014A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod5_constant_reg       , DEMOD_CONFIG_BASE + 0x014B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod5_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x014C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod5_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x014D, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod6_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x0190, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod6_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x0191, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod6_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x0192, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod6_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x0193, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod6_enable             , DEMOD_CONFIG_BASE + 0x0194, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod6_source             , DEMOD_CONFIG_BASE + 0x0195, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod6_sinc_en            , DEMOD_CONFIG_BASE + 0x0196, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod6_fir_order          , DEMOD_CONFIG_BASE + 0x0197, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod6_alpha_l            , DEMOD_CONFIG_BASE + 0x0198, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod6_alpha_m            , DEMOD_CONFIG_BASE + 0x0199, "NULL", "NULL");
    reg_table[i++] = REG(10, demod6_alpha_h            , DEMOD_CONFIG_BASE + 0x019A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod6_constant_reg       , DEMOD_CONFIG_BASE + 0x019B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod6_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x019C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod6_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x019D, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod7_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x01E0, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod7_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x01E1, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod7_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x01E2, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod7_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x01E3, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod7_enable             , DEMOD_CONFIG_BASE + 0x01E4, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod7_source             , DEMOD_CONFIG_BASE + 0x01E5, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod7_sinc_en            , DEMOD_CONFIG_BASE + 0x01E6, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod7_fir_order          , DEMOD_CONFIG_BASE + 0x01E7, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod7_alpha_l            , DEMOD_CONFIG_BASE + 0x01E8, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod7_alpha_m            , DEMOD_CONFIG_BASE + 0x01E9, "NULL", "NULL");
    reg_table[i++] = REG(10, demod7_alpha_h            , DEMOD_CONFIG_BASE + 0x01EA, "NULL", "NULL");
    reg_table[i++] = REG(11, demod7_constant_reg       , DEMOD_CONFIG_BASE + 0x01EB, "NULL", "NULL");
    reg_table[i++] = REG(12, demod7_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x01EC, "NULL", "NULL");
    reg_table[i++] = REG(13, demod7_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x01ED, "NULL", "NULL");

    reg_table[i++] = REG(0,  demod8_sinc_dec_ratio_en  , DEMOD_CONFIG_BASE + 0x0230, "NULL", "NULL");
    reg_table[i++] = REG(1,  demod8_sinc_dec_ratio_m1  , DEMOD_CONFIG_BASE + 0x0231, "NULL", "NULL");
    reg_table[i++] = REG(2,  demod8_sinc_aver_ratio    , DEMOD_CONFIG_BASE + 0x0232, "NULL", "NULL");
    reg_table[i++] = REG(3,  demod8_sinc_dec_ratio_m2  , DEMOD_CONFIG_BASE + 0x0233, "NULL", "NULL");
    reg_table[i++] = REG(4,  demod8_enable             , DEMOD_CONFIG_BASE + 0x0234, "NULL", "NULL");
    reg_table[i++] = REG(5,  demod8_source             , DEMOD_CONFIG_BASE + 0x0235, "NULL", "NULL");
    reg_table[i++] = REG(6,  demod8_sinc_en            , DEMOD_CONFIG_BASE + 0x0236, "NULL", "NULL");
    reg_table[i++] = REG(7,  demod8_fir_order          , DEMOD_CONFIG_BASE + 0x0237, "NULL", "NULL");
    reg_table[i++] = REG(8,  demod8_alpha_l            , DEMOD_CONFIG_BASE + 0x0238, "NULL", "NULL");
    reg_table[i++] = REG(9,  demod8_alpha_m            , DEMOD_CONFIG_BASE + 0x0239, "NULL", "NULL");
    reg_table[i++] = REG(10, demod8_alpha_h            , DEMOD_CONFIG_BASE + 0x023A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod8_constant_reg       , DEMOD_CONFIG_BASE + 0x023B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod8_psd_scaling_l      , DEMOD_CONFIG_BASE + 0x023C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod8_psd_scaling_h      , DEMOD_CONFIG_BASE + 0x023D, "NULL", "NULL");

    /* Up data config */
    reg_table[i++] = REG(0,  ch1_scpoe_up_en           , UP_DATA_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  ch1_scpoe_average_en      , UP_DATA_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  ch1_scope_up_data_mux     , UP_DATA_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  ch1_scope_decimation_value, UP_DATA_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  ch1_daq_up_en             , UP_DATA_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  ch1_daq_up_data_mux_l     , UP_DATA_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  ch1_daq_up_data_mux_h     , UP_DATA_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  ch1_daq_decimation_value  , UP_DATA_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  ch1_up_en                 , UP_DATA_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  ch1_up_data_mode          , UP_DATA_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, ch1_updata_soft_trig      , UP_DATA_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, ch1_storag_cnt            , UP_DATA_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, ch1_block_size            , UP_DATA_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, ch1_up_data_cache_length  , UP_DATA_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, ch1_up_data_write_length  , UP_DATA_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, ch1_read_block            , UP_DATA_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, ch1_dma_up_en             , UP_DATA_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, ch1_dma_up_data_clr       , UP_DATA_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, ch1_dma_rd_length         , UP_DATA_CONFIG_BASE + 0x0012, "NULL", "NULL");

    reg_table[i++] = REG(0,  ch2_scpoe_up_en           , UP_DATA_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(1,  ch2_scpoe_average_en      , UP_DATA_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(2,  ch2_scope_up_data_mux     , UP_DATA_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(3,  ch2_scope_decimation_value, UP_DATA_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(4,  ch2_daq_up_en             , UP_DATA_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(5,  ch2_daq_up_data_mux_l     , UP_DATA_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(6,  ch2_daq_up_data_mux_h     , UP_DATA_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(7,  ch2_daq_decimation_value  , UP_DATA_CONFIG_BASE + 0x0107, "NULL", "NULL");
    reg_table[i++] = REG(8,  ch2_up_en                 , UP_DATA_CONFIG_BASE + 0x0108, "NULL", "NULL");
    reg_table[i++] = REG(9,  ch2_up_data_mode          , UP_DATA_CONFIG_BASE + 0x0109, "NULL", "NULL");
    reg_table[i++] = REG(10, ch2_updata_soft_trig      , UP_DATA_CONFIG_BASE + 0x010A, "NULL", "NULL");
    reg_table[i++] = REG(11, ch2_storag_cnt            , UP_DATA_CONFIG_BASE + 0x010B, "NULL", "NULL");
    reg_table[i++] = REG(12, ch2_block_size            , UP_DATA_CONFIG_BASE + 0x010C, "NULL", "NULL");
    reg_table[i++] = REG(13, ch2_up_data_cache_length  , UP_DATA_CONFIG_BASE + 0x010D, "NULL", "NULL");
    reg_table[i++] = REG(14, ch2_up_data_write_length  , UP_DATA_CONFIG_BASE + 0x010E, "NULL", "NULL");
    reg_table[i++] = REG(15, ch2_read_block            , UP_DATA_CONFIG_BASE + 0x010F, "NULL", "NULL");
    reg_table[i++] = REG(16, ch2_dma_up_en             , UP_DATA_CONFIG_BASE + 0x0110, "NULL", "NULL");
    reg_table[i++] = REG(17, ch2_dma_up_data_clr       , UP_DATA_CONFIG_BASE + 0x0111, "NULL", "NULL");
    reg_table[i++] = REG(18, ch2_dma_rd_length         , UP_DATA_CONFIG_BASE + 0x0112, "NULL", "NULL");

    /* IV card */
    reg_table[i++] = REG(0,  relays1_addr              , I_CARD_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1,  range_relays1             , I_CARD_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2,  range_relays1_en          , I_CARD_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3,  relays2_addr              , I_CARD_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4,  range_relays2             , I_CARD_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5,  range_relays2_en          , I_CARD_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6,  relays3_addr              , I_CARD_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7,  range_relays3             , I_CARD_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8,  range_relays3_en          , I_CARD_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9,  relays4_addr              , I_CARD_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, range_relays4             , I_CARD_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, range_relays4_en          , I_CARD_CONFIG_BASE + 0x000B, "NULL", "NULL");

    /* AWG config */
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

    reg_table_size = (unsigned int)i;
}




#define REG_TABLE_SIZE (reg_table_size)

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


int write_reg_by_name(const char* name, const char* str_val)
{
    uint32_t offset;
    if (find_reg_offset(name, &offset) != 0)
        return -1;

    const reg_entry_t* reg = get_reg_entry(name);
    if (!reg)
        return -1;

    uint32_t val = 0;

    if (strcmp(reg->options, "bool") == 0) {
        val = (!strcmp(str_val, "1") || !strcasecmp(str_val, "true")) ? 1 : 0;
    } else if (strncmp(reg->options, "int", 3) == 0) {
        val = (uint32_t)atoi(str_val);
    } else if (strstr(reg->options, "|")) {
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

