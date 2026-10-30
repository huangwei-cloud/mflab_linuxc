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
#define AMFM_CONFIG_BASE        0x2400
#define  AWG1_CONFIG_BASE       0x2700
#define  AWG2_CONFIG_BASE       0x2800
#define BOXCAR_CONFIG_BASE       0x2900

#define REG_CNT_MAX             1000

static reg_entry_t reg_table[REG_CNT_MAX];
static unsigned int reg_table_size;

void init_reg()
{
    int i = 0;

    memset(reg_table, 0, sizeof(reg_table));

    /* system ctrl */
    reg_table[i++] = REG(0 , fpga_version                     , SYSTEM_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , compilation_time                 , SYSTEM_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , pll_freq_word_l                  , SYSTEM_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(3 , pll_freq_word_h                  , SYSTEM_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(4 , osc1_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(5 , osc1_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(6 , osc2_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(7 , osc2_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x0015, "NULL", "NULL");
    reg_table[i++] = REG(8 , osc3_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x0016, "NULL", "NULL");
    reg_table[i++] = REG(9 , osc3_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x0017, "NULL", "NULL");
    reg_table[i++] = REG(10, osc4_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x0018, "NULL", "NULL");
    reg_table[i++] = REG(11, osc4_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x0019, "NULL", "NULL");
    reg_table[i++] = REG(12, osc5_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x001A, "NULL", "NULL");
    reg_table[i++] = REG(13, osc5_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x001B, "NULL", "NULL");
    reg_table[i++] = REG(14, osc6_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x001C, "NULL", "NULL");
    reg_table[i++] = REG(15, osc6_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x001D, "NULL", "NULL");
    reg_table[i++] = REG(16, osc7_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x001E, "NULL", "NULL");
    reg_table[i++] = REG(17, osc7_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x001F, "NULL", "NULL");
    reg_table[i++] = REG(18, osc8_freq_word_l                 , SYSTEM_CONFIG_BASE + 0x0020, "NULL", "NULL");
    reg_table[i++] = REG(19, osc8_freq_word_h                 , SYSTEM_CONFIG_BASE + 0x0021, "NULL", "NULL");
    reg_table[i++] = REG(20, pwa_mem_free                     , SYSTEM_CONFIG_BASE + 0x0030, "NULL", "NULL");
    reg_table[i++] = REG(21, pwa_sample_num_now               , SYSTEM_CONFIG_BASE + 0x0031, "NULL", "NULL");
    reg_table[i++] = REG(22, boxcar_mem_free                  , SYSTEM_CONFIG_BASE + 0x0032, "NULL", "NULL");
    reg_table[i++] = REG(23, boxcar_sample_num_now            , SYSTEM_CONFIG_BASE + 0x0033, "NULL", "NULL");
    reg_table[i++] = REG(24, sync_ok                          , SYSTEM_CONFIG_BASE + 0x0034, "NULL", "NULL");
    reg_table[i++] = REG(25, fpga_temp_out                    , SYSTEM_CONFIG_BASE + 0x0040, "NULL", "NULL");
    reg_table[i++] = REG(26, soft_trig                        , SYSTEM_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(27, osc_work_all                     , SYSTEM_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(28, fan_pwm_val                      , SYSTEM_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(29, load_delay                       , SYSTEM_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(30, ltc2000_1_odelay                 , SYSTEM_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(31, ltc2000_2_odelay                 , SYSTEM_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(32, ad9684_idelay                    , SYSTEM_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(33, daq_data_source                  , SYSTEM_CONFIG_BASE + 0x0200, "NULL", "NULL");

    /* signalout1 config */
    reg_table[i++] = REG(0 , signalout1_enable                , SIGNALOUT_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , signalout1_source1               , SIGNALOUT_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , signalout1_scaling1_l            , SIGNALOUT_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , signalout1_scaling1_h            , SIGNALOUT_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , singalout1_offset_en             , SIGNALOUT_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , singalout1_offset                , SIGNALOUT_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , signalout1_limit_en              , SIGNALOUT_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , signalout1_upper_limit           , SIGNALOUT_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , signalout1_lower_limit           , SIGNALOUT_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , signalout1_constant1             , SIGNALOUT_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, signalout1_bigwidth_write_en     , SIGNALOUT_CONFIG_BASE + 0x000A, "Pulse 1 then 0 after writing all words", "NULL");

    /* signalout2 config */
    reg_table[i++] = REG(0 , signalout2_enable                , SIGNALOUT_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(1 , signalout2_source1               , SIGNALOUT_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(2 , signalout2_scaling1_l            , SIGNALOUT_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(3 , signalout2_scaling1_h            , SIGNALOUT_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(4 , singalout2_offset_en             , SIGNALOUT_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(5 , singalout2_offset                , SIGNALOUT_CONFIG_BASE + 0x0015, "NULL", "NULL");
    reg_table[i++] = REG(6 , signalout2_limit_en              , SIGNALOUT_CONFIG_BASE + 0x0016, "NULL", "NULL");
    reg_table[i++] = REG(7 , signalout2_upper_limit           , SIGNALOUT_CONFIG_BASE + 0x0017, "NULL", "NULL");
    reg_table[i++] = REG(8 , signalout2_lower_limit           , SIGNALOUT_CONFIG_BASE + 0x0018, "NULL", "NULL");
    reg_table[i++] = REG(9 , signalout2_constant1             , SIGNALOUT_CONFIG_BASE + 0x0019, "NULL", "NULL");
    reg_table[i++] = REG(10, signalout2_bigwidth_write_en     , SIGNALOUT_CONFIG_BASE + 0x001A, "Pulse 1 then 0 after writing all words", "NULL");

    /* trig config */
    reg_table[i++] = REG(0 , clk_sync_mode                    , TRIG_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , sync_trig                        , TRIG_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , sync_en                          , TRIG_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , trig_in_mode                     , TRIG_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(4 , trig_in_source                   , TRIG_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(5 , trig_en                          , TRIG_CONFIG_BASE + 0x0020, "NULL", "NULL");
    reg_table[i++] = REG(6 , trig_source                      , TRIG_CONFIG_BASE + 0x0021, "NULL", "NULL");
    reg_table[i++] = REG(7 , trig_mode                        , TRIG_CONFIG_BASE + 0x0022, "NULL", "NULL");
    reg_table[i++] = REG(8 , trig_delay                       , TRIG_CONFIG_BASE + 0x0023, "NULL", "NULL");
    reg_table[i++] = REG(9 , trig_Nth_event                   , TRIG_CONFIG_BASE + 0x0024, "NULL", "NULL");
    reg_table[i++] = REG(10, trig_release_restrain            , TRIG_CONFIG_BASE + 0x0025, "NULL", "NULL");
    reg_table[i++] = REG(11, trig_plus_width                  , TRIG_CONFIG_BASE + 0x0026, "NULL", "NULL");
    reg_table[i++] = REG(12, hysteresis_range_high_l          , TRIG_CONFIG_BASE + 0x0027, "NULL", "NULL");
    reg_table[i++] = REG(13, hysteresis_range_high_h          , TRIG_CONFIG_BASE + 0x0028, "NULL", "NULL");
    reg_table[i++] = REG(14, hysteresis_range_low_l           , TRIG_CONFIG_BASE + 0x0029, "NULL", "NULL");
    reg_table[i++] = REG(15, hysteresis_range_low_h           , TRIG_CONFIG_BASE + 0x002A, "NULL", "NULL");
    reg_table[i++] = REG(16, trig_bigwidth_write_en           , TRIG_CONFIG_BASE + 0x002B, "Pulse 1 then 0 after writing all words", "NULL");
    reg_table[i++] = REG(17, trig_out_mode                    , TRIG_CONFIG_BASE + 0x0030, "NULL", "NULL");
    reg_table[i++] = REG(18, trig_out_plus_period             , TRIG_CONFIG_BASE + 0x0031, "NULL", "NULL");
    reg_table[i++] = REG(19, trig_out_plus_width              , TRIG_CONFIG_BASE + 0x0032, "NULL", "NULL");
    reg_table[i++] = REG(20, trig_out_plus_loop               , TRIG_CONFIG_BASE + 0x0033, "NULL", "NULL");

    /* PLL config */
    reg_table[i++] = REG(0 , pll_enable                       , PLL_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , pll_source                       , PLL_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , pll_ref_hysteresis_high          , PLL_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , pll_ref_hysteresis_low           , PLL_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , pll_mov_fir_ratio                , PLL_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , pll_calc_time_l                  , PLL_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , pll_calc_time_h                  , PLL_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , pll_calc_stopdog_l               , PLL_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , pll_calc_stopdog_h               , PLL_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , ref_mov_fir_ratio                , PLL_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, pll_bigwidth_write_en            , PLL_CONFIG_BASE + 0x000A, "Pulse 1 then 0 after writing all words", "NULL");

    /* PID1 config */
    reg_table[i++] = REG(0 , pid1_enable                      , PID_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , pid1_ki_enable                   , PID_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , pid1_moving_aver_ratio           , PID_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , pid1_source                      , PID_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , pid1_set_point_l                 , PID_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , pid1_set_point_h                 , PID_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , pid1_Kp_l                        , PID_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , pid1_Kp_h                        , PID_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , pid1_Ki_l                        , PID_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , pid1_Ki_h                        , PID_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, pid1_Kd_l                        , PID_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid1_Kd_h                        , PID_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid1_decimation                  , PID_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid1_output_offset_l             , PID_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid1_output_offset_h             , PID_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid1_lower_limit_l               , PID_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, pid1_lower_limit_h               , PID_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, pid1_upper_limit_l               , PID_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, pid1_upper_limit_h               , PID_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(19, pid1_invert                      , PID_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(20, pid1_bigwidth_write_en           , PID_CONFIG_BASE + 0x0014, "NULL", "NULL");

    /* PID2 config */
    reg_table[i++] = REG(0 , pid2_enable                      , PID_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(1 , pid2_ki_enable                   , PID_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(2 , pid2_moving_aver_ratio           , PID_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(3 , pid2_source                      , PID_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(4 , pid2_set_point_l                 , PID_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(5 , pid2_set_point_h                 , PID_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(6 , pid2_Kp_l                        , PID_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(7 , pid2_Kp_h                        , PID_CONFIG_BASE + 0x0107, "NULL", "NULL");
    reg_table[i++] = REG(8 , pid2_Ki_l                        , PID_CONFIG_BASE + 0x0108, "NULL", "NULL");
    reg_table[i++] = REG(9 , pid2_Ki_h                        , PID_CONFIG_BASE + 0x0109, "NULL", "NULL");
    reg_table[i++] = REG(10, pid2_Kd_l                        , PID_CONFIG_BASE + 0x010A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid2_Kd_h                        , PID_CONFIG_BASE + 0x010B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid2_decimation                  , PID_CONFIG_BASE + 0x010C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid2_output_offset_l             , PID_CONFIG_BASE + 0x010D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid2_output_offset_h             , PID_CONFIG_BASE + 0x010E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid2_lower_limit_l               , PID_CONFIG_BASE + 0x010F, "NULL", "NULL");
    reg_table[i++] = REG(16, pid2_lower_limit_h               , PID_CONFIG_BASE + 0x0110, "NULL", "NULL");
    reg_table[i++] = REG(17, pid2_upper_limit_l               , PID_CONFIG_BASE + 0x0111, "NULL", "NULL");
    reg_table[i++] = REG(18, pid2_upper_limit_h               , PID_CONFIG_BASE + 0x0112, "NULL", "NULL");
    reg_table[i++] = REG(19, pid2_invert                      , PID_CONFIG_BASE + 0x0113, "NULL", "NULL");
    reg_table[i++] = REG(20, pid2_bigwidth_write_en           , PID_CONFIG_BASE + 0x0114, "NULL", "NULL");

    /* PID3 config */
    reg_table[i++] = REG(0 , pid3_enable                      , PID_CONFIG_BASE + 0x0200, "NULL", "NULL");
    reg_table[i++] = REG(1 , pid3_ki_enable                   , PID_CONFIG_BASE + 0x0201, "NULL", "NULL");
    reg_table[i++] = REG(2 , pid3_moving_aver_ratio           , PID_CONFIG_BASE + 0x0202, "NULL", "NULL");
    reg_table[i++] = REG(3 , pid3_source                      , PID_CONFIG_BASE + 0x0203, "NULL", "NULL");
    reg_table[i++] = REG(4 , pid3_set_point_l                 , PID_CONFIG_BASE + 0x0204, "NULL", "NULL");
    reg_table[i++] = REG(5 , pid3_set_point_h                 , PID_CONFIG_BASE + 0x0205, "NULL", "NULL");
    reg_table[i++] = REG(6 , pid3_Kp_l                        , PID_CONFIG_BASE + 0x0206, "NULL", "NULL");
    reg_table[i++] = REG(7 , pid3_Kp_h                        , PID_CONFIG_BASE + 0x0207, "NULL", "NULL");
    reg_table[i++] = REG(8 , pid3_Ki_l                        , PID_CONFIG_BASE + 0x0208, "NULL", "NULL");
    reg_table[i++] = REG(9 , pid3_Ki_h                        , PID_CONFIG_BASE + 0x0209, "NULL", "NULL");
    reg_table[i++] = REG(10, pid3_Kd_l                        , PID_CONFIG_BASE + 0x020A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid3_Kd_h                        , PID_CONFIG_BASE + 0x020B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid3_decimation                  , PID_CONFIG_BASE + 0x020C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid3_output_offset_l             , PID_CONFIG_BASE + 0x020D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid3_output_offset_h             , PID_CONFIG_BASE + 0x020E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid3_lower_limit_l               , PID_CONFIG_BASE + 0x020F, "NULL", "NULL");
    reg_table[i++] = REG(16, pid3_lower_limit_h               , PID_CONFIG_BASE + 0x0210, "NULL", "NULL");
    reg_table[i++] = REG(17, pid3_upper_limit_l               , PID_CONFIG_BASE + 0x0211, "NULL", "NULL");
    reg_table[i++] = REG(18, pid3_upper_limit_h               , PID_CONFIG_BASE + 0x0212, "NULL", "NULL");
    reg_table[i++] = REG(19, pid3_invert                      , PID_CONFIG_BASE + 0x0213, "NULL", "NULL");
    reg_table[i++] = REG(20, pid3_bigwidth_write_en           , PID_CONFIG_BASE + 0x0214, "NULL", "NULL");

    /* PID4 config */
    reg_table[i++] = REG(0 , pid4_enable                      , PID_CONFIG_BASE + 0x0300, "NULL", "NULL");
    reg_table[i++] = REG(1 , pid4_ki_enable                   , PID_CONFIG_BASE + 0x0301, "NULL", "NULL");
    reg_table[i++] = REG(2 , pid4_moving_aver_ratio           , PID_CONFIG_BASE + 0x0302, "NULL", "NULL");
    reg_table[i++] = REG(3 , pid4_source                      , PID_CONFIG_BASE + 0x0303, "NULL", "NULL");
    reg_table[i++] = REG(4 , pid4_set_point_l                 , PID_CONFIG_BASE + 0x0304, "NULL", "NULL");
    reg_table[i++] = REG(5 , pid4_set_point_h                 , PID_CONFIG_BASE + 0x0305, "NULL", "NULL");
    reg_table[i++] = REG(6 , pid4_Kp_l                        , PID_CONFIG_BASE + 0x0306, "NULL", "NULL");
    reg_table[i++] = REG(7 , pid4_Kp_h                        , PID_CONFIG_BASE + 0x0307, "NULL", "NULL");
    reg_table[i++] = REG(8 , pid4_Ki_l                        , PID_CONFIG_BASE + 0x0308, "NULL", "NULL");
    reg_table[i++] = REG(9 , pid4_Ki_h                        , PID_CONFIG_BASE + 0x0309, "NULL", "NULL");
    reg_table[i++] = REG(10, pid4_Kd_l                        , PID_CONFIG_BASE + 0x030A, "NULL", "NULL");
    reg_table[i++] = REG(11, pid4_Kd_h                        , PID_CONFIG_BASE + 0x030B, "NULL", "NULL");
    reg_table[i++] = REG(12, pid4_decimation                  , PID_CONFIG_BASE + 0x030C, "NULL", "NULL");
    reg_table[i++] = REG(13, pid4_output_offset_l             , PID_CONFIG_BASE + 0x030D, "NULL", "NULL");
    reg_table[i++] = REG(14, pid4_output_offset_h             , PID_CONFIG_BASE + 0x030E, "NULL", "NULL");
    reg_table[i++] = REG(15, pid4_lower_limit_l               , PID_CONFIG_BASE + 0x030F, "NULL", "NULL");
    reg_table[i++] = REG(16, pid4_lower_limit_h               , PID_CONFIG_BASE + 0x0310, "NULL", "NULL");
    reg_table[i++] = REG(17, pid4_upper_limit_l               , PID_CONFIG_BASE + 0x0311, "NULL", "NULL");
    reg_table[i++] = REG(18, pid4_upper_limit_h               , PID_CONFIG_BASE + 0x0312, "NULL", "NULL");
    reg_table[i++] = REG(19, pid4_invert                      , PID_CONFIG_BASE + 0x0313, "NULL", "NULL");
    reg_table[i++] = REG(20, pid4_bigwidth_write_en           , PID_CONFIG_BASE + 0x0314, "NULL", "NULL");

    /* OSC0 config: base = 0x2200 + 0x20 * 0 */
    reg_table[i++] = REG(0 , oscillators1_en                  , OSC_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators1_freq_en             , OSC_CONFIG_BASE + 0x0001, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators1_source              , OSC_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators1_mode                , OSC_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators1_harm                , OSC_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators1_phase_l             , OSC_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators1_phase_h             , OSC_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators1_freq_word_l         , OSC_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators1_freq_word_h         , OSC_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators1_osc_type            , OSC_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators1_osc_pluse_duty      , OSC_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators1_sinout1_scaling1    , OSC_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators1_sinout2_scaling1    , OSC_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators1_sinout1_en          , OSC_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators1_sinout2_en          , OSC_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators1_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators1_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators1_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators1_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x0012, "NULL", "NULL");

    /* OSC1 config: base = 0x2200 + 0x20 * 1 */
    reg_table[i++] = REG(0 , oscillators2_en                  , OSC_CONFIG_BASE + 0x0020, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators2_freq_en             , OSC_CONFIG_BASE + 0x0021, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators2_source              , OSC_CONFIG_BASE + 0x0022, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators2_mode                , OSC_CONFIG_BASE + 0x0023, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators2_harm                , OSC_CONFIG_BASE + 0x0024, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators2_phase_l             , OSC_CONFIG_BASE + 0x0025, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators2_phase_h             , OSC_CONFIG_BASE + 0x0026, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators2_freq_word_l         , OSC_CONFIG_BASE + 0x0027, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators2_freq_word_h         , OSC_CONFIG_BASE + 0x0028, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators2_osc_type            , OSC_CONFIG_BASE + 0x0029, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators2_osc_pluse_duty      , OSC_CONFIG_BASE + 0x002A, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators2_sinout1_scaling1    , OSC_CONFIG_BASE + 0x002B, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators2_sinout2_scaling1    , OSC_CONFIG_BASE + 0x002C, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators2_sinout1_en          , OSC_CONFIG_BASE + 0x002D, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators2_sinout2_en          , OSC_CONFIG_BASE + 0x002E, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators2_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x002F, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators2_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x0030, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators2_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x0031, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators2_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x0032, "NULL", "NULL");

    /* OSC2 config: base = 0x2200 + 0x20 * 2 */
    reg_table[i++] = REG(0 , oscillators3_en                  , OSC_CONFIG_BASE + 0x0040, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators3_freq_en             , OSC_CONFIG_BASE + 0x0041, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators3_source              , OSC_CONFIG_BASE + 0x0042, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators3_mode                , OSC_CONFIG_BASE + 0x0043, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators3_harm                , OSC_CONFIG_BASE + 0x0044, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators3_phase_l             , OSC_CONFIG_BASE + 0x0045, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators3_phase_h             , OSC_CONFIG_BASE + 0x0046, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators3_freq_word_l         , OSC_CONFIG_BASE + 0x0047, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators3_freq_word_h         , OSC_CONFIG_BASE + 0x0048, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators3_osc_type            , OSC_CONFIG_BASE + 0x0049, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators3_osc_pluse_duty      , OSC_CONFIG_BASE + 0x004A, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators3_sinout1_scaling1    , OSC_CONFIG_BASE + 0x004B, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators3_sinout2_scaling1    , OSC_CONFIG_BASE + 0x004C, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators3_sinout1_en          , OSC_CONFIG_BASE + 0x004D, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators3_sinout2_en          , OSC_CONFIG_BASE + 0x004E, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators3_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x004F, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators3_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x0050, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators3_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x0051, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators3_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x0052, "NULL", "NULL");

    /* OSC3 config: base = 0x2200 + 0x20 * 3 */
    reg_table[i++] = REG(0 , oscillators4_en                  , OSC_CONFIG_BASE + 0x0060, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators4_freq_en             , OSC_CONFIG_BASE + 0x0061, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators4_source              , OSC_CONFIG_BASE + 0x0062, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators4_mode                , OSC_CONFIG_BASE + 0x0063, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators4_harm                , OSC_CONFIG_BASE + 0x0064, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators4_phase_l             , OSC_CONFIG_BASE + 0x0065, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators4_phase_h             , OSC_CONFIG_BASE + 0x0066, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators4_freq_word_l         , OSC_CONFIG_BASE + 0x0067, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators4_freq_word_h         , OSC_CONFIG_BASE + 0x0068, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators4_osc_type            , OSC_CONFIG_BASE + 0x0069, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators4_osc_pluse_duty      , OSC_CONFIG_BASE + 0x006A, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators4_sinout1_scaling1    , OSC_CONFIG_BASE + 0x006B, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators4_sinout2_scaling1    , OSC_CONFIG_BASE + 0x006C, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators4_sinout1_en          , OSC_CONFIG_BASE + 0x006D, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators4_sinout2_en          , OSC_CONFIG_BASE + 0x006E, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators4_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x006F, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators4_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x0070, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators4_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x0071, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators4_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x0072, "NULL", "NULL");

    /* OSC4 config: base = 0x2200 + 0x20 * 4 */
    reg_table[i++] = REG(0 , oscillators5_en                  , OSC_CONFIG_BASE + 0x0080, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators5_freq_en             , OSC_CONFIG_BASE + 0x0081, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators5_source              , OSC_CONFIG_BASE + 0x0082, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators5_mode                , OSC_CONFIG_BASE + 0x0083, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators5_harm                , OSC_CONFIG_BASE + 0x0084, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators5_phase_l             , OSC_CONFIG_BASE + 0x0085, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators5_phase_h             , OSC_CONFIG_BASE + 0x0086, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators5_freq_word_l         , OSC_CONFIG_BASE + 0x0087, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators5_freq_word_h         , OSC_CONFIG_BASE + 0x0088, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators5_osc_type            , OSC_CONFIG_BASE + 0x0089, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators5_osc_pluse_duty      , OSC_CONFIG_BASE + 0x008A, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators5_sinout1_scaling1    , OSC_CONFIG_BASE + 0x008B, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators5_sinout2_scaling1    , OSC_CONFIG_BASE + 0x008C, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators5_sinout1_en          , OSC_CONFIG_BASE + 0x008D, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators5_sinout2_en          , OSC_CONFIG_BASE + 0x008E, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators5_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x008F, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators5_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x0090, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators5_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x0091, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators5_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x0092, "NULL", "NULL");

    /* OSC5 config: base = 0x2200 + 0x20 * 5 */
    reg_table[i++] = REG(0 , oscillators6_en                  , OSC_CONFIG_BASE + 0x00A0, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators6_freq_en             , OSC_CONFIG_BASE + 0x00A1, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators6_source              , OSC_CONFIG_BASE + 0x00A2, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators6_mode                , OSC_CONFIG_BASE + 0x00A3, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators6_harm                , OSC_CONFIG_BASE + 0x00A4, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators6_phase_l             , OSC_CONFIG_BASE + 0x00A5, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators6_phase_h             , OSC_CONFIG_BASE + 0x00A6, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators6_freq_word_l         , OSC_CONFIG_BASE + 0x00A7, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators6_freq_word_h         , OSC_CONFIG_BASE + 0x00A8, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators6_osc_type            , OSC_CONFIG_BASE + 0x00A9, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators6_osc_pluse_duty      , OSC_CONFIG_BASE + 0x00AA, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators6_sinout1_scaling1    , OSC_CONFIG_BASE + 0x00AB, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators6_sinout2_scaling1    , OSC_CONFIG_BASE + 0x00AC, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators6_sinout1_en          , OSC_CONFIG_BASE + 0x00AD, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators6_sinout2_en          , OSC_CONFIG_BASE + 0x00AE, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators6_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x00AF, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators6_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x00B0, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators6_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x00B1, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators6_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x00B2, "NULL", "NULL");

    /* OSC6 config: base = 0x2200 + 0x20 * 6 */
    reg_table[i++] = REG(0 , oscillators7_en                  , OSC_CONFIG_BASE + 0x00C0, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators7_freq_en             , OSC_CONFIG_BASE + 0x00C1, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators7_source              , OSC_CONFIG_BASE + 0x00C2, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators7_mode                , OSC_CONFIG_BASE + 0x00C3, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators7_harm                , OSC_CONFIG_BASE + 0x00C4, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators7_phase_l             , OSC_CONFIG_BASE + 0x00C5, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators7_phase_h             , OSC_CONFIG_BASE + 0x00C6, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators7_freq_word_l         , OSC_CONFIG_BASE + 0x00C7, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators7_freq_word_h         , OSC_CONFIG_BASE + 0x00C8, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators7_osc_type            , OSC_CONFIG_BASE + 0x00C9, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators7_osc_pluse_duty      , OSC_CONFIG_BASE + 0x00CA, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators7_sinout1_scaling1    , OSC_CONFIG_BASE + 0x00CB, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators7_sinout2_scaling1    , OSC_CONFIG_BASE + 0x00CC, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators7_sinout1_en          , OSC_CONFIG_BASE + 0x00CD, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators7_sinout2_en          , OSC_CONFIG_BASE + 0x00CE, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators7_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x00CF, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators7_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x00D0, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators7_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x00D1, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators7_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x00D2, "NULL", "NULL");

    /* OSC7 config: base = 0x2200 + 0x20 * 7 */
    reg_table[i++] = REG(0 , oscillators8_en                  , OSC_CONFIG_BASE + 0x00E0, "NULL", "NULL");
    reg_table[i++] = REG(1 , oscillators8_freq_en             , OSC_CONFIG_BASE + 0x00E1, "Pulse 1 then 0 after writing frequency and phase", "NULL");
    reg_table[i++] = REG(2 , oscillators8_source              , OSC_CONFIG_BASE + 0x00E2, "NULL", "NULL");
    reg_table[i++] = REG(3 , oscillators8_mode                , OSC_CONFIG_BASE + 0x00E3, "NULL", "NULL");
    reg_table[i++] = REG(4 , oscillators8_harm                , OSC_CONFIG_BASE + 0x00E4, "NULL", "NULL");
    reg_table[i++] = REG(5 , oscillators8_phase_l             , OSC_CONFIG_BASE + 0x00E5, "NULL", "NULL");
    reg_table[i++] = REG(6 , oscillators8_phase_h             , OSC_CONFIG_BASE + 0x00E6, "NULL", "NULL");
    reg_table[i++] = REG(7 , oscillators8_freq_word_l         , OSC_CONFIG_BASE + 0x00E7, "NULL", "NULL");
    reg_table[i++] = REG(8 , oscillators8_freq_word_h         , OSC_CONFIG_BASE + 0x00E8, "NULL", "NULL");
    reg_table[i++] = REG(9 , oscillators8_osc_type            , OSC_CONFIG_BASE + 0x00E9, "NULL", "NULL");
    reg_table[i++] = REG(10, oscillators8_osc_pluse_duty      , OSC_CONFIG_BASE + 0x00EA, "NULL", "NULL");
    reg_table[i++] = REG(11, oscillators8_sinout1_scaling1    , OSC_CONFIG_BASE + 0x00EB, "NULL", "NULL");
    reg_table[i++] = REG(12, oscillators8_sinout2_scaling1    , OSC_CONFIG_BASE + 0x00EC, "NULL", "NULL");
    reg_table[i++] = REG(13, oscillators8_sinout1_en          , OSC_CONFIG_BASE + 0x00ED, "NULL", "NULL");
    reg_table[i++] = REG(14, oscillators8_sinout2_en          , OSC_CONFIG_BASE + 0x00EE, "NULL", "NULL");
    reg_table[i++] = REG(15, oscillators8_pid_phase_ctrl_en   , OSC_CONFIG_BASE + 0x00EF, "NULL", "NULL");
    reg_table[i++] = REG(16, oscillators8_pid_phase_ctrl_source, OSC_CONFIG_BASE + 0x00F0, "NULL", "NULL");
    reg_table[i++] = REG(17, oscillators8_pid_freq_ctrl_en    , OSC_CONFIG_BASE + 0x00F1, "NULL", "NULL");
    reg_table[i++] = REG(18, oscillators8_pid_freq_ctrl_source, OSC_CONFIG_BASE + 0x00F2, "NULL", "NULL");

    /* AM/FM ch0 config */
    reg_table[i++] = REG(0 , am_fm_enable                     , AMFM_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , am_fm_mode                       , AMFM_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , modulation_multi_1               , AMFM_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , am_n1                            , AMFM_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , am_n2                            , AMFM_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , am_n3                            , AMFM_CONFIG_BASE + 0x0005, "NULL", "NULL");

    /* AM/FM ch1 config */
    reg_table[i++] = REG(0 , am_fm_enable_ch2                 , AMFM_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(1 , am_fm_mode_ch2                   , AMFM_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(2 , modulation_multi_1_ch2           , AMFM_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(3 , am_n1_ch2                        , AMFM_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(4 , am_n2_ch2                        , AMFM_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(5 , am_n3_ch2                        , AMFM_CONFIG_BASE + 0x0015, "NULL", "NULL");

    /* demod config: one demodulator */
    reg_table[i++] = REG(0 , demod1_sinc_dec_ratio_en         , DEMOD_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , demod1_sinc_dec_ratio_m1         , DEMOD_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , demod1_sinc_aver_ratio           , DEMOD_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , demod1_sinc_dec_ratio_m2         , DEMOD_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , demod1_enable                    , DEMOD_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , demod1_source                    , DEMOD_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , demod1_sinc_en                   , DEMOD_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , demod1_fir_order                 , DEMOD_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , demod1_alpha_l                   , DEMOD_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , demod1_alpha_m                   , DEMOD_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, demod1_alpha_h                   , DEMOD_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, demod1_constant_reg              , DEMOD_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, demod1_psd_scaling_l             , DEMOD_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, demod1_psd_scaling_h             , DEMOD_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, ref_source                       , DEMOD_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, ref_constant                     , DEMOD_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, demod_bigwidth_write_en          , DEMOD_CONFIG_BASE + 0x0010, "Pulse 1 then 0 after writing all words", "NULL");

    /* Up data ch1 config */
    reg_table[i++] = REG(0 , ch1_scpoe_up_en                  , UP_DATA_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , ch1_scpoe_average_en             , UP_DATA_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , ch1_scope_up_data_mux            , UP_DATA_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , ch1_scope_decimation_value       , UP_DATA_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , ch1_daq_up_en                    , UP_DATA_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , ch1_daq_up_data_mux_l            , UP_DATA_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , ch1_daq_up_data_mux_h            , UP_DATA_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , ch1_daq_decimation_value         , UP_DATA_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , ch1_up_en                        , UP_DATA_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , ch1_up_data_mode                 , UP_DATA_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, ch1_updata_soft_trig             , UP_DATA_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, ch1_storag_cnt                   , UP_DATA_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, ch1_block_size                   , UP_DATA_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, ch1_up_data_cache_length         , UP_DATA_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, ch1_up_data_write_length         , UP_DATA_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, ch1_read_block                   , UP_DATA_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, ch1_dma_up_en                    , UP_DATA_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, ch1_dma_up_data_clr              , UP_DATA_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, ch1_dma_rd_length                , UP_DATA_CONFIG_BASE + 0x0012, "NULL", "NULL");

    /* Up data ch2 config */
    reg_table[i++] = REG(0 , ch2_scpoe_up_en                  , UP_DATA_CONFIG_BASE + 0x0100, "NULL", "NULL");
    reg_table[i++] = REG(1 , ch2_scpoe_average_en             , UP_DATA_CONFIG_BASE + 0x0101, "NULL", "NULL");
    reg_table[i++] = REG(2 , ch2_scope_up_data_mux            , UP_DATA_CONFIG_BASE + 0x0102, "NULL", "NULL");
    reg_table[i++] = REG(3 , ch2_scope_decimation_value       , UP_DATA_CONFIG_BASE + 0x0103, "NULL", "NULL");
    reg_table[i++] = REG(4 , ch2_daq_up_en                    , UP_DATA_CONFIG_BASE + 0x0104, "NULL", "NULL");
    reg_table[i++] = REG(5 , ch2_daq_up_data_mux_l            , UP_DATA_CONFIG_BASE + 0x0105, "NULL", "NULL");
    reg_table[i++] = REG(6 , ch2_daq_up_data_mux_h            , UP_DATA_CONFIG_BASE + 0x0106, "NULL", "NULL");
    reg_table[i++] = REG(7 , ch2_daq_decimation_value         , UP_DATA_CONFIG_BASE + 0x0107, "NULL", "NULL");
    reg_table[i++] = REG(8 , ch2_up_en                        , UP_DATA_CONFIG_BASE + 0x0108, "NULL", "NULL");
    reg_table[i++] = REG(9 , ch2_up_data_mode                 , UP_DATA_CONFIG_BASE + 0x0109, "NULL", "NULL");
    reg_table[i++] = REG(10, ch2_updata_soft_trig             , UP_DATA_CONFIG_BASE + 0x010A, "NULL", "NULL");
    reg_table[i++] = REG(11, ch2_storag_cnt                   , UP_DATA_CONFIG_BASE + 0x010B, "NULL", "NULL");
    reg_table[i++] = REG(12, ch2_block_size                   , UP_DATA_CONFIG_BASE + 0x010C, "NULL", "NULL");
    reg_table[i++] = REG(13, ch2_up_data_cache_length         , UP_DATA_CONFIG_BASE + 0x010D, "NULL", "NULL");
    reg_table[i++] = REG(14, ch2_up_data_write_length         , UP_DATA_CONFIG_BASE + 0x010E, "NULL", "NULL");
    reg_table[i++] = REG(15, ch2_read_block                   , UP_DATA_CONFIG_BASE + 0x010F, "NULL", "NULL");
    reg_table[i++] = REG(16, ch2_dma_up_en                    , UP_DATA_CONFIG_BASE + 0x0110, "NULL", "NULL");
    reg_table[i++] = REG(17, ch2_dma_up_data_clr              , UP_DATA_CONFIG_BASE + 0x0111, "NULL", "NULL");
    reg_table[i++] = REG(18, ch2_dma_rd_length                , UP_DATA_CONFIG_BASE + 0x0112, "NULL", "NULL");

    /* IV card */
    reg_table[i++] = REG(0 , relays1_addr                     , I_CARD_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , range_relays1                    , I_CARD_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , range_relays1_en                 , I_CARD_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , relays2_addr                     , I_CARD_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , range_relays2                    , I_CARD_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , range_relays2_en                 , I_CARD_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , relays3_addr                     , I_CARD_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , range_relays3                    , I_CARD_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , range_relays3_en                 , I_CARD_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , relays4_addr                     , I_CARD_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, range_relays4                    , I_CARD_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, range_relays4_en                 , I_CARD_CONFIG_BASE + 0x000B, "NULL", "NULL");

    /* AWG1 config */
    reg_table[i++] = REG(0 , awg1_addr_en                     , AWG1_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , awg1_addr                        , AWG1_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , awg1_play_en                     , AWG1_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , awg1_play_mode                   , AWG1_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , awg1_play_seq_cnt                , AWG1_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , awg1_play_start_addr             , AWG1_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , awg1_play_length                 , AWG1_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , awg1_play_cnt                    , AWG1_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , awg1_play_waittime               , AWG1_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , awg1_play_amplitude              , AWG1_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, awg1_play_modulation             , AWG1_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, awg1_param_addr_en               , AWG1_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, awg1_param_addr                  , AWG1_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, awg1_seq_param_addr_en           , AWG1_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, awg1_seq_param_addr              , AWG1_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, awg1_seq_param                   , AWG1_CONFIG_BASE + 0x000F, "NULL", "NULL");

    /* AWG2 config */
    reg_table[i++] = REG(0 , awg2_addr_en                     , AWG2_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , awg2_addr                        , AWG2_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , awg2_play_en                     , AWG2_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , awg2_play_mode                   , AWG2_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , awg2_play_seq_cnt                , AWG2_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , awg2_play_start_addr             , AWG2_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , awg2_play_length                 , AWG2_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , awg2_play_cnt                    , AWG2_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , awg2_play_waittime               , AWG2_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , awg2_play_amplitude              , AWG2_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, awg2_play_modulation             , AWG2_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, awg2_param_addr_en               , AWG2_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, awg2_param_addr                  , AWG2_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, awg2_seq_param_addr_en           , AWG2_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, awg2_seq_param_addr              , AWG2_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, awg2_seq_param                   , AWG2_CONFIG_BASE + 0x000F, "NULL", "NULL");

    /* Boxcar / PWA config */
    reg_table[i++] = REG(0 , boxcar_trig_source               , BOXCAR_CONFIG_BASE + 0x0000, "NULL", "NULL");
    reg_table[i++] = REG(1 , gate_hysteresis_range_high_low   , BOXCAR_CONFIG_BASE + 0x0001, "NULL", "NULL");
    reg_table[i++] = REG(2 , gate_hysteresis_range_high_high  , BOXCAR_CONFIG_BASE + 0x0002, "NULL", "NULL");
    reg_table[i++] = REG(3 , gate_hysteresis_range_low_low    , BOXCAR_CONFIG_BASE + 0x0003, "NULL", "NULL");
    reg_table[i++] = REG(4 , gate_hysteresis_range_low_high   , BOXCAR_CONFIG_BASE + 0x0004, "NULL", "NULL");
    reg_table[i++] = REG(5 , ref_hysteresis_range_high_low    , BOXCAR_CONFIG_BASE + 0x0005, "NULL", "NULL");
    reg_table[i++] = REG(6 , ref_hysteresis_range_high_high   , BOXCAR_CONFIG_BASE + 0x0006, "NULL", "NULL");
    reg_table[i++] = REG(7 , ref_hysteresis_range_low_low     , BOXCAR_CONFIG_BASE + 0x0007, "NULL", "NULL");
    reg_table[i++] = REG(8 , ref_hysteresis_range_low_high    , BOXCAR_CONFIG_BASE + 0x0008, "NULL", "NULL");
    reg_table[i++] = REG(9 , pwa_sample_enable                , BOXCAR_CONFIG_BASE + 0x0009, "NULL", "NULL");
    reg_table[i++] = REG(10, pwa_sample_source                , BOXCAR_CONFIG_BASE + 0x000A, "NULL", "NULL");
    reg_table[i++] = REG(11, pwa_sample_length                , BOXCAR_CONFIG_BASE + 0x000B, "NULL", "NULL");
    reg_table[i++] = REG(12, pwa_sample_decimation            , BOXCAR_CONFIG_BASE + 0x000C, "NULL", "NULL");
    reg_table[i++] = REG(13, pwa_sample_average_en            , BOXCAR_CONFIG_BASE + 0x000D, "NULL", "NULL");
    reg_table[i++] = REG(14, pwa_sample_delay                 , BOXCAR_CONFIG_BASE + 0x000E, "NULL", "NULL");
    reg_table[i++] = REG(15, pwa_sample_num                   , BOXCAR_CONFIG_BASE + 0x000F, "NULL", "NULL");
    reg_table[i++] = REG(16, boxcar_sample_enable             , BOXCAR_CONFIG_BASE + 0x0010, "NULL", "NULL");
    reg_table[i++] = REG(17, boxcar_sample_source             , BOXCAR_CONFIG_BASE + 0x0011, "NULL", "NULL");
    reg_table[i++] = REG(18, boxcar_sample_length             , BOXCAR_CONFIG_BASE + 0x0012, "NULL", "NULL");
    reg_table[i++] = REG(19, boxcar_sample_delay              , BOXCAR_CONFIG_BASE + 0x0013, "NULL", "NULL");
    reg_table[i++] = REG(20, boxcar_sample_num                , BOXCAR_CONFIG_BASE + 0x0014, "NULL", "NULL");
    reg_table[i++] = REG(21, boxcar_sample_aver_ratio         , BOXCAR_CONFIG_BASE + 0x0015, "NULL", "NULL");
    reg_table[i++] = REG(22, boxcar_sample_aver_num           , BOXCAR_CONFIG_BASE + 0x0016, "NULL", "NULL");
    reg_table[i++] = REG(23, pwa_bigwidth_write_en            , BOXCAR_CONFIG_BASE + 0x0017, "Pulse 1 then 0 after writing all words", "NULL");

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
