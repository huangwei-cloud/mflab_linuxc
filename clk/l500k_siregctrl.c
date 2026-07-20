#include "l500k_siregctrl.h"

static uint8_t l500k_make_reg(uint8_t bits01, uint8_t bits23,
                              uint8_t bits45, uint8_t bits67)
{
    return (uint8_t)((bits01 & 0x03U) |
                     ((bits23 & 0x03U) << 2) |
                     ((bits45 & 0x03U) << 4) |
                     ((bits67 & 0x03U) << 6));
}

int l500k_singleinput_calc_regs(const l500k_singleinput_config_t *cfg,
                                l500k_singleinput_regs_t *out)
{
    uint8_t reg1_01;
    uint8_t reg1_23;
    uint8_t reg1_45;
    uint8_t reg1_67;
    uint8_t reg2_01;
    uint8_t reg2_23;
    uint8_t reg2_45;
    uint8_t reg2_67;
    uint8_t reg3_01;
    uint8_t reg3_23;
    uint8_t reg3_45;
    uint8_t reg3_67;
    uint8_t reg4_01;
    uint8_t reg4_23;
    uint8_t reg4_45;
    uint8_t reg4_67;

    if (cfg == 0 || out == 0) {
        return -1;
    }

    reg1_01 = cfg->v_single;
    reg1_23 = cfg->v_coupling;
    reg1_45 = cfg->v_gnd;
    reg1_67 = 0x02;

    reg2_01 = cfg->measure_mode;
    reg2_23 = cfg->iv_in_gnd;
    reg2_45 = 0x02;
    reg2_67 = 0x02;

    reg3_01 = cfg->v_impedance;
    reg3_23 = 0x02;
    reg3_45 = 0x02;
    reg3_67 = 0x02;

    reg4_01 = 0x02;
    reg4_23 = cfg->i_coupling;
    reg4_45 = 0x00;
    reg4_67 = 0x00;

    if (cfg->iv_fir == L500K_SINGLEINPUT_FILTER_50HZ) {
        reg2_45 = 0x01;
        reg2_67 = 0x02;
    } else if (cfg->iv_fir == L500K_SINGLEINPUT_FILTER_100HZ) {
        reg2_45 = 0x02;
        reg2_67 = 0x01;
    } else if (cfg->iv_fir == L500K_SINGLEINPUT_FILTER_BOTH) {
        reg2_45 = 0x01;
        reg2_67 = 0x01;
    }

    if (cfg->measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
        if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_1MV) {
            reg4_45 = 0x03;
            reg4_67 = 0x03;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_5MV) {
            reg4_45 = 0x03;
            reg4_67 = 0x01;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_10MV) {
            reg4_45 = 0x03;
            reg4_67 = 0x00;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_50MV) {
            reg4_45 = 0x01;
            reg4_67 = 0x01;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_0V1) {
            reg4_45 = 0x01;
            reg4_67 = 0x00;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_0V5) {
            reg4_45 = 0x02;
            reg4_67 = 0x01;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_1V) {
            reg4_45 = 0x02;
            reg4_67 = 0x00;
        } else if (cfg->v_range == L500K_SINGLEINPUT_VRANGE_5V) {
            reg4_45 = 0x00;
            reg4_67 = 0x00;
        }

        if (cfg->v_range >= L500K_SINGLEINPUT_VRANGE_1MV &&
            cfg->v_range <= L500K_SINGLEINPUT_VRANGE_0V1) {
            reg1_67 = 0x01;
        } else {
            reg1_67 = 0x02;
        }

        reg3_23 = 0x02;
        reg3_45 = 0x02;
        reg3_67 = 0x02;
        reg4_01 = 0x02;
    } else if (cfg->measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
        if (cfg->i_range == L500K_SINGLEINPUT_IRANGE_1NA ||
            cfg->i_range == L500K_SINGLEINPUT_IRANGE_100NA ||
            cfg->i_range == L500K_SINGLEINPUT_IRANGE_10UA ||
            cfg->i_range == L500K_SINGLEINPUT_IRANGE_1MA) {
            reg4_45 = 0x03;
            reg4_67 = 0x02;
        } else {
            reg4_45 = 0x01;
            reg4_67 = 0x02;
        }

        reg1_67 = 0x02;

        if (cfg->i_range == L500K_SINGLEINPUT_IRANGE_1NA ||
            cfg->i_range == L500K_SINGLEINPUT_IRANGE_10NA) {
            reg3_23 = 0x01;
            reg3_45 = 0x01;
            reg3_67 = 0x02;
            reg4_01 = 0x01;
        } else if (cfg->i_range == L500K_SINGLEINPUT_IRANGE_100NA ||
                   cfg->i_range == L500K_SINGLEINPUT_IRANGE_1UA) {
            reg3_23 = 0x01;
            reg3_45 = 0x02;
            reg3_67 = 0x02;
            reg4_01 = 0x01;
        } else if (cfg->i_range == L500K_SINGLEINPUT_IRANGE_10UA ||
                   cfg->i_range == L500K_SINGLEINPUT_IRANGE_100UA) {
            reg3_23 = 0x02;
            reg3_45 = 0x02;
            reg3_67 = 0x01;
            reg4_01 = 0x02;
        } else {
            reg3_23 = 0x02;
            reg3_45 = 0x02;
            reg3_67 = 0x02;
            reg4_01 = 0x02;
        }
    }

    out->reg1 = l500k_make_reg(reg1_01, reg1_23, reg1_45, reg1_67);
    out->reg2 = l500k_make_reg(reg2_01, reg2_23, reg2_45, reg2_67);
    out->reg3 = l500k_make_reg(reg3_01, reg3_23, reg3_45, reg3_67);
    out->reg4 = l500k_make_reg(reg4_01, reg4_23, reg4_45, reg4_67);

    usleep(100000);
    return 0;
}
