#include "siregctrl.h"
#include "../log/log.h"

static int singleinput_calc_regs_internal(const singleinput_config_t *cfg,
                                          singleinput_regs_t *out,
                                          int standard_ground)
{
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;
    uint8_t reg5;
    uint8_t reg6;
    uint8_t reg1_couple;
    uint8_t reg1_single;
    uint8_t reg1_end;
    uint8_t reg1_v;
    uint8_t reg2_rin;
    uint8_t reg2_v;
    uint8_t reg3_fir;
    uint8_t reg3_v;
    uint8_t reg4_v;
    uint8_t reg5_v;
    uint8_t reg5_couple;
    uint64_t reg;

    if (cfg == 0 || out == 0) {
        return -1;
    }

    if (cfg->coupling_voltage == SINGLEINPUT_COUPLING_AC) {
        reg1_couple = 0x20;
    } else {
        reg1_couple = 0x10;
    }

    if (cfg->single_mode == SINGLEINPUT_MODE_SINGLE) {
        reg1_single = 0x02;
    } else {
        reg1_single = 0x01;
    }

    if (cfg->grounding_mode == SINGLEINPUT_GROUNDING_GND) {
        reg1_end = 0x80;
    } else {
        reg1_end = 0x40;
    }

    if (cfg->range >= SINGLEINPUT_RANGE_1MV_1NA &&
        cfg->range <= SINGLEINPUT_RANGE_30MV_1UA) {
        reg1_v = 0x04;
    } else {
        reg1_v = 0x08;
    }

    reg1 = reg1_couple | reg1_single | reg1_end | reg1_v;

    if (cfg->impedance == SINGLEINPUT_IMPEDANCE_50R) {
        if (cfg->range >= SINGLEINPUT_RANGE_1MV_1NA &&
            cfg->range <= SINGLEINPUT_RANGE_30MV_1UA) {
            reg2_rin = 0x09;
        } else {
            reg2_rin = 0x06;
        }
    } else {
        reg2_rin = 0x0A;
    }

    if (cfg->measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
        reg2_v = 0x90;
    } else {
        if (cfg->range >= SINGLEINPUT_RANGE_1MV_1NA &&
            cfg->range <= SINGLEINPUT_RANGE_30MV_1UA) {
            reg2_v = standard_ground ? 0xA0 : 0x50;
        } else {
            reg2_v = 0x60;
        }
    }

    reg2 = reg2_rin | reg2_v;

    if (cfg->filter == SINGLEINPUT_FILTER_BYPASS) {
        reg3_fir = 0x0A;
    } else if (cfg->filter == SINGLEINPUT_FILTER_50HZ) {
        reg3_fir = 0x09;
    } else if (cfg->filter == SINGLEINPUT_FILTER_100HZ) {
        reg3_fir = 0x06;
    } else {
        reg3_fir = 0x05;
    }

    if (cfg->measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
        if (cfg->range == SINGLEINPUT_RANGE_1MV_1NA ||
            cfg->range == SINGLEINPUT_RANGE_10MV_100NA ||
            cfg->range == SINGLEINPUT_RANGE_0V1_10UA ||
            cfg->range == SINGLEINPUT_RANGE_1V_1MA) {
            reg3_v = 0x90;
        } else {
            reg3_v = 0xA0;
        }
    } else {
        if (cfg->range == SINGLEINPUT_RANGE_1MV_1NA ||
            cfg->range == SINGLEINPUT_RANGE_3MV_10NA) {
            reg3_v = 0x50;
        } else if (cfg->range >= SINGLEINPUT_RANGE_10MV_100NA &&
                   cfg->range <= SINGLEINPUT_RANGE_0V3_100UA) {
            reg3_v = 0x90;
        } else {
            reg3_v = 0xA0;
        }
    }

    reg3 = reg3_fir | reg3_v;

    if (cfg->measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
        if (cfg->range == SINGLEINPUT_RANGE_1MV_1NA ||
            cfg->range == SINGLEINPUT_RANGE_3MV_10NA) {
            reg4_v = 0x96;
        } else if (cfg->range == SINGLEINPUT_RANGE_10MV_100NA ||
                   cfg->range == SINGLEINPUT_RANGE_30MV_1UA) {
            reg4_v = 0xA6;
        } else if (cfg->range == SINGLEINPUT_RANGE_0V1_10UA ||
                   cfg->range == SINGLEINPUT_RANGE_0V3_100UA) {
            reg4_v = 0x6A;
        } else {
            reg4_v = 0xAA;
        }
    } else {
        if (cfg->range == SINGLEINPUT_RANGE_1MV_1NA ||
            cfg->range == SINGLEINPUT_RANGE_10MV_100NA ||
            cfg->range == SINGLEINPUT_RANGE_0V1_10UA ||
            cfg->range == SINGLEINPUT_RANGE_1V_1MA) {
            reg4_v = 0xA9;
        } else {
            reg4_v = 0xAA;
        }
    }

    reg4 = reg4_v;

    if (cfg->measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
        if (cfg->range >= SINGLEINPUT_RANGE_1MV_1NA &&
            cfg->range <= SINGLEINPUT_RANGE_30MV_1UA) {
            reg5_v = 0xA1;
        } else {
            reg5_v = 0xA2;
        }

        if (cfg->coupling_current == SINGLEINPUT_COUPLING_AC) {
            reg5_couple = 0x04;
        } else {
            reg5_couple = 0x08;
        }
    } else {
        reg5_v = 0xAA;
        reg5_couple = 0x00;
    }

    reg5 = reg5_v | reg5_couple;
    reg6 = 0xFF;

    reg = ((uint64_t)reg1) |
          ((uint64_t)reg2 << 8) |
          ((uint64_t)reg3 << 16) |
          ((uint64_t)reg4 << 24) |
          ((uint64_t)reg5 << 32) |
          ((uint64_t)reg6 << 40);

    out->reg_low = (uint32_t)(reg & 0xFFFFFFFFU);
    out->reg_high = (uint32_t)(reg >> 32);

    P_LOG_DEBUG("singleinput_calc_regs: reg1=0x%02X, reg2=0x%02X, reg3=0x%02X, reg4=0x%02X, reg5=0x%02X, reg6=0x%02X",
                reg1, reg2, reg3, reg4, reg5, reg6);
    P_LOG_DEBUG("singleinput_calc_regs: reg_low=0x%08X, reg_high=0x%08X", out->reg_low, out->reg_high);            
    return 0;
}

int singleinput_calc_regs(const singleinput_config_t *cfg, singleinput_regs_t *out)
{
    return singleinput_calc_regs_internal(cfg, out, 0);
}

int singleinput_calc_regs_with_standard_ground(const singleinput_config_t *cfg,
                                               singleinput_regs_t *out,
                                               int standard_ground)
{
    return singleinput_calc_regs_internal(cfg, out, standard_ground ? 1 : 0);
}
