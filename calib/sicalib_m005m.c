#include "sicalib_m005m.h"
#include "sicalib.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "../clk/reg.h"
#include "../clk/siregctrl.h"
#include "../utils/scpi-def.h"

extern gLobalParam g_param;

static caldb_t g_m005m_vol_db[2];
static caldb_t g_m005m_cur_db[2];

static unsigned cfg_to_index(uint8_t value)
{
    return value > 0U ? (unsigned)(value - 1U) : 0U;
}

static unsigned encode_cur_id(unsigned coupling, unsigned filter, unsigned range)
{
    return ((range & 7U) << 3) |
           ((coupling & 1U) << 2) |
           ((filter & 3U) << 0);
}

static uint32_t dac_code_from_voltage(double value)
{
    return (uint32_t)round(value / 2.5 * (pow(2.0, 16.0) - 1.0));
}

static void write_trim_dac(int ch, unsigned trim_ch, double value)
{
    const char *trim_ch_name = (ch == CH1) ? "v_trim_ch" : "i_trim_ch";
    const char *trim_name = (ch == CH1) ? "v_trim" : "i_trim";
    const char *trim_en_name = (ch == CH1) ? "v_trim_en" : "i_trim_en";
    uint32_t code = dac_code_from_voltage(value);

    reg_write_by_name(trim_ch_name, trim_ch);
    reg_write_by_name(trim_name, code);
    reg_write_by_name(trim_en_name, 0);
    reg_write_by_name(trim_en_name, 1);
    reg_write_by_name(trim_en_name, 0);
}

static void apply_voltage_calibration(int ch, const singleinput_config_t *cfg)
{
    unsigned range = cfg_to_index(cfg->range);
    unsigned id = encode_id(cfg_to_index(cfg->single_mode),
                            cfg_to_index(cfg->coupling_voltage),
                            cfg_to_index(cfg->impedance),
                            cfg_to_index(cfg->filter),
                            range);
    calib_params_t cpt;

    if (caldb_get_by_id(&g_m005m_vol_db[ch - 1], id, &cpt) != 0) {
        return;
    }

    if (range < 4U) {
        write_trim_dac(ch, 0U, cpt.vouta);
        write_trim_dac(ch, 1U, cpt.voutb);
    }

    if ((range == 0U) || (range == 2U) || (range == 4U) || (range == 6U)) {
        write_trim_dac(ch, 2U, cpt.voutc);
    } else {
        write_trim_dac(ch, 3U, cpt.voutd);
    }
}

static void apply_current_calibration(int ch, const singleinput_config_t *cfg)
{
    unsigned range = cfg_to_index(cfg->range);
    unsigned id = encode_cur_id(cfg_to_index(cfg->coupling_current),
                                cfg_to_index(cfg->filter),
                                range);
    calib_params_t cpt;
    double voutd;

    if (caldb_get_by_id(&g_m005m_cur_db[ch - 1], id, &cpt) != 0) {
        return;
    }

    voutd = (cpt.voutd != 0.0 || cpt.vouta == 0.0) ? cpt.voutd : cpt.vouta;
    write_trim_dac(ch, 3U, voutd);
}

void set_si_m005m_calibration(int ch)
{
    const singleinput_config_t *cfg;

    if ((ch != CH1) && (ch != CH2)) {
        return;
    }

    cfg = &g_param.g_si_config[ch - 1];
    if (cfg->measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
        apply_current_calibration(ch, cfg);
    } else {
        apply_voltage_calibration(ch, cfg);
    }
}

int init_sicalib_m005m_param(void)
{
    if (caldb_load_csv(&g_m005m_vol_db[0], SI_CALIB_M005M_VOLCH1_PATH) != 0) {
        printf("load %s failed\n", SI_CALIB_M005M_VOLCH1_PATH);
        return -1;
    }

    if (caldb_load_csv(&g_m005m_vol_db[1], SI_CALIB_M005M_VOLCH2_PATH) != 0) {
        printf("load %s failed\n", SI_CALIB_M005M_VOLCH2_PATH);
        return -1;
    }

    if (caldb_load_csv(&g_m005m_cur_db[0], SI_CALIB_M005M_CURCH1_PATH) != 0) {
        printf("load %s failed\n", SI_CALIB_M005M_CURCH1_PATH);
        return -1;
    }

    if (caldb_load_csv(&g_m005m_cur_db[1], SI_CALIB_M005M_CURCH2_PATH) != 0) {
        printf("load %s failed\n", SI_CALIB_M005M_CURCH2_PATH);
        return -1;
    }

    return 0;
}
