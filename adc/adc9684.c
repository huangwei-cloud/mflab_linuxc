#include "adc9684.h"
#include "../clk/hmc7043.h"
#include "../log/log.h"
#include <stdio.h>
#include <unistd.h>

#define AD9684_READ_CMD         0x00800000
#define AD9684_ID_LOW_REG       0x0003
#define AD9684_ID_LOW_VALUE     0x00800303
#define AD9684_CLK_STATUS_REG   0x011C
#define AD9684_SYSREF_CTRL_REG  0x0550
#define AD9684_READ_TIMEOUT     10000

int adc9684_write_reg(uint16_t addr, uint8_t data)
{
    AxiDevice *ad9684 = NULL;

    get_ad9684_intance(&ad9684);
    if (ad9684 == NULL || ad9684->reg_base == NULL) {
        P_LOG_ERROR("AD9684 device is not initialized");
        return -1;
    }

    ad9684->reg_base[0] = 0x00000000;
    ad9684->reg_base[1] = ((uint32_t)addr << 8) | data;
    ad9684->reg_base[0] = 0x00000002;
    usleep(1000);

    return 0;
}

uint32_t adc9684_read_reg(uint16_t addr)
{
    int i;
    AxiDevice *ad9684 = NULL;

    get_ad9684_intance(&ad9684);
    if (ad9684 == NULL || ad9684->reg_base == NULL) {
        P_LOG_ERROR("AD9684 device is not initialized");
        return 0xFFFFFFFF;
    }

    ad9684->reg_base[0] = 0x00000000;
    ad9684->reg_base[1] = AD9684_READ_CMD | ((uint32_t)addr << 8);
    ad9684->reg_base[0] = 0x00000002;

    for (i = 0; i < AD9684_READ_TIMEOUT; ++i) {
        if (ad9684->reg_base[2] == 0x00000001) {
            return ad9684->reg_base[1];
        }
        usleep(100);
    }

    P_LOG_ERROR("AD9684 read timeout, addr=0x%04X", addr);
    return 0xFFFFFFFF;
}

int init_adc9684(void)
{
    int i;
    uint32_t id_low;
    uint32_t clk_status;
    AxiDevice *ad9684 = NULL;

    get_ad9684_intance(&ad9684);
    if (ad9684 == NULL || ad9684->reg_base == NULL) {
        P_LOG_ERROR("AD9684 device is not initialized");
        return -1;
    }

    ad9684->reg_base[0] = 0x00000001;
    ad9684->reg_base[0] = 0x00000000;
    sleep(1);

    for (i = 0; i < 10; ++i) {
        id_low = adc9684_read_reg(AD9684_ID_LOW_REG);
        if (id_low == AD9684_ID_LOW_VALUE) {
            break;
        }

        P_LOG_ERROR("AD9684 ID error, id=0x%08X", id_low);
        sleep(3);
    }

    if (i >= 10) {
        return -1;
    }

    adc9684_write_reg(0x0000, 0x81);
    sleep(3);
    adc9684_write_reg(0x0000, 0x00);

    clk_status = adc9684_read_reg(AD9684_CLK_STATUS_REG);
    printf("ad9684 clk status: 0x%08X\r\n", clk_status);

    adc9684_write_reg(AD9684_SYSREF_CTRL_REG, 0x00);

    return 0;
}
