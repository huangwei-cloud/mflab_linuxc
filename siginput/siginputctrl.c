#include "siginputctrl.h"
#include "../clk/reg.h"
#include "../log/log.h"

static uint8_t siginput_make_reg(uint8_t bits01, uint8_t bits23,
                                 uint8_t bits45, uint8_t bits67)
{
    return (uint8_t)((bits01 & 0x03U) |
                     ((bits23 & 0x03U) << 2) |
                     ((bits45 & 0x03U) << 4) |
                     ((bits67 & 0x03U) << 6));
}

static int siginput_write_pulse(const char *name)
{
    if (reg_write_by_name(name, 0) != 0) return -1;
    if (reg_write_by_name(name, 1) != 0) return -1;
    if (reg_write_by_name(name, 0) != 0) return -1;
    return 0;
}

int siginput_config_to_regs(const siginput_config_t *cfg)
{
    uint8_t reg1_01;
    uint8_t reg1_23;
    uint8_t reg1_45;
    uint8_t reg1_67;
    uint8_t reg2_01;
    uint8_t reg2_23;
    uint8_t reg1;
    uint8_t reg2;

    if (cfg == 0) {
        return -1;
    }

    if (cfg->ch != SIGINPUT_CH0 && cfg->ch != SIGINPUT_CH1) {
        P_LOG_ERROR("siginput invalid channel: %u", cfg->ch);
        return -1;
    }

    if (cfg->impedance == SIGINPUT_V_IMPEDANCE_50R) {
        reg1_01 = 0x02;
        reg2_23 = 0x02;
        reg2_01 = 0x00;
        reg1_67 = 0x00;

        reg1_45 = (cfg->range == SIGINPUT_V_RANGE_1V) ? 0x01 : 0x02;
        reg1_23 = (cfg->coupling == SIGINPUT_V_COUPLING_DC) ? 0x02 : 0x01;
    } else {
        reg1_01 = 0x01;
        reg2_23 = 0x01;
        reg1_45 = 0x00;
        reg1_23 = 0x00;

        reg2_01 = (cfg->range == SIGINPUT_V_RANGE_1V) ? 0x01 : 0x02;
        reg1_67 = (cfg->coupling == SIGINPUT_V_COUPLING_DC) ? 0x02 : 0x01;
    }

    reg1 = siginput_make_reg(reg1_01, reg1_23, reg1_45, reg1_67);
    reg2 = (uint8_t)((reg2_01 & 0x03U) | ((reg2_23 & 0x03U) << 2));

    P_LOG_DEBUG("siginput config: ch=%u, reg1=0x%02X, reg2=0x%02X",
                cfg->ch, reg1, reg2);

    if (cfg->ch == SIGINPUT_CH0) {
        if (reg_write_by_name("relays1_addr", 0x00000000) != 0) return -1;
        if (reg_write_by_name("range_relays1", reg1) != 0) return -1;
        if (siginput_write_pulse("range_relays1_en") != 0) return -1;

        if (reg_write_by_name("relays2_addr", 0x00000001) != 0) return -1;
        if (reg_write_by_name("range_relays2", reg2) != 0) return -1;
        if (siginput_write_pulse("range_relays2_en") != 0) return -1;
    } else {
        if (reg_write_by_name("relays3_addr", 0x00000002) != 0) return -1;
        if (reg_write_by_name("range_relays3", reg1) != 0) return -1;
        if (siginput_write_pulse("range_relays3_en") != 0) return -1;

        if (reg_write_by_name("relays4_addr", 0x00000003) != 0) return -1;
        if (reg_write_by_name("range_relays4", reg2) != 0) return -1;
        if (siginput_write_pulse("range_relays4_en") != 0) return -1;
    }

    return 0;
}
