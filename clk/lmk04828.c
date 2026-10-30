#include "lmk04828.h"
#include "../axispi/spi.h"
#include "../axigpio/gpio.h"
#include "../log/log.h"
#include "../utils/scpi-def.h"
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>

/* GPIO mapping from the original bare-metal XGpioPs pin numbers:
 * MPSoC MIO is 0~77, so XGpioPs 78~83 are EMIO 0~5 in Linux sysfs.
 *
 * Bare-metal mapping:
 *   78: clk ready
 *   79: clk-sel, 0 = internal clock, 1 = external clock
 *   80: osc enable
 *   81: fira, external reference divider select
 *   82: firb, external reference divider select
 *   83: pca9538 init
 */
#define LMK_GPIO_CLK_READY      0
#define LMK_GPIO_CLK_SEL        1
#define LMK_GPIO_OSC_ENABLE     2
#define LMK_GPIO_FIRA           3
#define LMK_GPIO_FIRB           4
#define LMK_GPIO_PCA9538_INIT   5
#define DAC_SYNC                6

/* LMK register configuration:
 *   LMK04828_CLOCK_CONFIG_100M = 0: internal 100 MHz or external 100 MHz reference
 *   LMK04828_CLOCK_CONFIG_10M  = 1: external 10 MHz reference
 *
 * This is different from clk-sel. clk-sel only chooses internal/external path;
 * clock_config chooses the LMK04828 register table.
 */
#define LMK04828_ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))

/* The 100 MHz and 10 MHz LMK tables only differ at this register word:
 *   100 MHz table: 0x015A14
 *   10 MHz table : 0x015AC8
 * Keep one 100 MHz table below and patch this fixed index while writing
 * 10 MHz config. Do not replace by value, because duplicated register words
 * would make value matching unsafe.
 */
#define LMK04828_DIFF_INDEX     97
#define LMK04828_100M_DIFF_VALUE 0x015A14
#define LMK04828_10M_DIFF_VALUE 0x015AC8

typedef enum {
    LMK_REF_INTERNAL = 0,
    LMK_REF_EXTERNAL = 1,
} lmk_ref_source_t;

typedef enum {
    LMK04828_CLOCK_CONFIG_100M = 0,
    LMK04828_CLOCK_CONFIG_10M = 1,
} lmk04828_clock_config_t;

static const uint32_t lmk04828_awg_reset_bank[] = {
    0x000090,
    0x000010,
    0x014A33,
};

/* Register table copied from srcref/lmk04828/lmk04828.c.
 * The original bare-metal code writes every 24-bit word as:
 *   tx[0] = bits 23:16, tx[1] = bits 15:8, tx[2] = bits 7:0
 */
static const uint32_t lmk04828_awg_reg_100m[] = {
    0x000200,
    0x000306,
    0x0004D0,
    0x00055B,
    0x000600,
    0x000C51,
    0x000D04,
    0x010004,
    0x010155,
    0x010255,
    0x010300,
    0x010402,
    0x010500,
    0x0106F0,
    0x010711,
    0x010802,
    0x010955,
    0x010A55,
    0x010B00,
    0x010C02,
    0x010D00,
    0x010E70,
    0x010F55,
    0x011004,
    0x011155,
    0x011255,
    0x011300,
    0x011400,
    0x011500,
    0x011670,
    0x011755,
    0x011814,
    0x011955,
    0x011A55,
    0x011B00,
    0x011C02,
    0x011D00,
    0x011EF0,
    0x011F55,
    0x012014,
    0x012155,
    0x012255,
    0x012300,
    0x012402,
    0x012500,
    0x0126F0,
    0x012755,
    0x012810,
    0x012955,
    0x012A55,
    0x012B00,
    0x012C02,
    0x012D00,
    0x012EF0,
    0x012F11,
    0x013014,
    0x013155,
    0x013255,
    0x013300,
    0x013422,
    0x013500,
    0x0136F0,
    0x013701,
    0x013800,
    0x013903,
    0x013A00,
    0x013BC8,
    0x013C00,
    0x013D01,
    0x013E03,
    0x013F00,
    0x014001,
    0x014100,
    0x014200,
    0x01435F,
    0x014400,
    0x01457F,
    0x014608,
    0x014702,
    0x014802,
    0x014902,
    0x014A33,
    0x014B36,
    0x014C00,
    0x014D00,
    0x014EC0,
    0x014F7F,
    0x015003,
    0x015102,
    0x015200,
    0x015300,
    0x015414,
    0x015500,
    0x015632,
    0x015700,
    0x015814,
    0x015900,
    0x015A14,//diff
    0x015BD4,
    0x015C20,
    0x015D00,
    0x015E00,
    0x015F0B,
    0x016000,
    0x016101,
    0x016224,
    0x016300,
    0x016400,
    0x016503,
    0x0171AA,
    0x017202,
    0x017300,
    0x017400,
    0x017C15,
    0x017D33,
    0x016600,
    0x016700,
    0x01680A,
    0x016959,
    0x016A20,
    0x016B00,
    0x016C00,
    0x016D00,
    0x016E13,
    0x1FFD00,
    0x1FFE00,
    0x1FFF53,
};

static const uint32_t lmk04828_awg_sync[] = {
    0x01439F,
    0x01431F,
    0x01433F,
    0x01431F,
    0x0144FF,
};

/* clk-sel GPIO value:
 *   INTERNAL_100M -> 0
 *   EXTERN_100M / EXTERN_10M -> 1
 */
static lmk_ref_source_t lmk04828_clk_sel(RefClk ref_clk)
{
    return (ref_clk == INTERNAL_100M) ? LMK_REF_INTERNAL : LMK_REF_EXTERNAL;
}

/* LMK register configuration:
 *   INTERNAL_100M / EXTERN_100M share the same 100 MHz LMK table.
 *   EXTERN_10M uses the 10 MHz LMK variant.
 */
static lmk04828_clock_config_t lmk04828_clock_config(RefClk ref_clk)
{
    return (ref_clk == EXTERN_10M) ? LMK04828_CLOCK_CONFIG_10M : LMK04828_CLOCK_CONFIG_100M;
}

/* Select internal/external reference clock path before SPI config.
 *
 * Bare-metal reference:
 *   XGpioPs_WritePin(79, clk_source);
 *   if external:
 *     XGpioPs_WritePin(80, 0);
 *     XGpioPs_WritePin(81, 1);
 *     XGpioPs_WritePin(82, 1);
 */
static int lmk04828_select_ref_clock(RefClk ref_clk)
{
    lmk_ref_source_t clk_sel = lmk04828_clk_sel(ref_clk);

    gpio_set_value(LMK_GPIO_CLK_SEL, clk_sel);

    if (clk_sel == LMK_REF_EXTERNAL) {
        gpio_set_value(LMK_GPIO_OSC_ENABLE, 0);
        gpio_set_value(LMK_GPIO_FIRA, 1);
        gpio_set_value(LMK_GPIO_FIRB, 1);
    }

    return 0;
}

int lmk04828_write_reg(uint16_t addr, uint8_t value)
{
    int ret;
    uint8_t txbuf[3];
    uint8_t rxbuf[3] = {0};
    spi_t *spi = spi_new();

    if (spi == NULL) {
        P_LOG_ERROR("LMK04828 spi_new failed");
        return -1;
    }

    if (spi_open(spi, LMK04828_SPI_DEV, LMK04828_SPI_MODE, LMK04828_SPI_SPEED_HZ) < 0) {
        P_LOG_ERROR("LMK04828 spi_open(%s) failed: %s", LMK04828_SPI_DEV, spi_errmsg(spi));
        spi_free(spi);
        return -1;
    }

    txbuf[0] = (uint8_t)((addr >> 8) & 0x7F);
    txbuf[1] = (uint8_t)(addr & 0xFF);
    txbuf[2] = value;

    ret = spi_transfer(spi, txbuf, rxbuf, sizeof(txbuf));
    if (ret < 0) {
        P_LOG_ERROR("LMK04828 write reg failed, addr=0x%04X, value=0x%02X: %s",
                    addr, value, spi_errmsg(spi));
    }

    spi_close(spi);
    spi_free(spi);
    return (ret < 0) ? -1 : 0;
}

static int lmk04828_read_reg(uint16_t addr, uint8_t *value)
{
    int ret;
    uint8_t txbuf[3];
    uint8_t rxbuf[3] = {0};
    spi_t *spi = spi_new();

    if (value == NULL) {
        return -1;
    }
    if (spi == NULL) {
        P_LOG_ERROR("LMK04828 spi_new failed");
        return -1;
    }

    if (spi_open(spi, LMK04828_SPI_DEV, LMK04828_SPI_MODE, LMK04828_SPI_SPEED_HZ) < 0) {
        P_LOG_ERROR("LMK04828 spi_open(%s) failed: %s", LMK04828_SPI_DEV, spi_errmsg(spi));
        spi_free(spi);
        return -1;
    }

    txbuf[0] = (uint8_t)(0x80 | ((addr >> 8) & 0x7F));
    txbuf[1] = (uint8_t)(addr & 0xFF);
    txbuf[2] = 0x00;

    ret = spi_transfer(spi, txbuf, rxbuf, sizeof(txbuf));
    if (ret < 0) {
        P_LOG_ERROR("LMK04828 read reg failed, addr=0x%04X: %s",
                    addr, spi_errmsg(spi));
    } else {
        *value = rxbuf[2];
    }

    spi_close(spi);
    spi_free(spi);

    return (ret < 0) ? -1 : 0;
}

static int lmk04828_is_locked(void)
{
    uint8_t value_182 = 0;
    uint8_t value_183 = 0;
    int i;
    gLobalParam *glp = NULL;

    get_global_param(&glp);
    if (glp != NULL) {
        glp->g_dev_lock_state = 0;
    }

    for (i = 0; i < 20; i++) {
        value_182 = 0;
        value_183 = 0;
        if (lmk04828_read_reg(0x0182, &value_182) != 0) {
            P_LOG_ERROR("LMK04828 read lock register 0x0182 failed");
        } else {
            P_LOG_DEBUG("LMK04828 lock register 0x0182 = 0x%02X", value_182);
        }

        if (lmk04828_read_reg(0x0183, &value_183) != 0) {
            P_LOG_ERROR("LMK04828 read lock register 0x0183 failed");
        } else {
            P_LOG_DEBUG("LMK04828 lock register 0x0183 = 0x%02X", value_183);
        }

        if (glp != NULL) {
            /* Raw diagnostics: bits 7:0 = reg 0x182, bits 15:8 = reg 0x183.
             * Register bit1 is current lock; bit2 is loss-of-lock history.
             * Replace diagnostics with 1 when both PLLs are currently locked.
             */
            glp->g_dev_lock_state = (uint32_t)value_182 | ((uint32_t)value_183 << 8);
        }

        if ((value_182 & 0x02) && (value_183 & 0x02)) {
            if (glp != NULL) {
                glp->g_dev_lock_state = 1;
            }
            P_LOG_DEBUG("LMK04828 clock locked");
            break;
        }

        usleep(500000);
    }

    if (!(value_182 & 0x02) || !(value_183 & 0x02)) {
        P_LOG_ERROR("LMK04828 clock lock timeout, 0x0182=0x%02X, 0x0183=0x%02X",
                    value_182, value_183);
    }

    return 1;
}

static int lmk04828_write_addr_data(spi_t *spi, uint32_t addr_data)
{
    uint8_t txbuf[3];
    uint8_t rxbuf[3] = {0};

    txbuf[0] = (uint8_t)((addr_data >> 16) & 0xFF);
    txbuf[1] = (uint8_t)((addr_data >> 8) & 0xFF);
    txbuf[2] = (uint8_t)(addr_data & 0xFF);

    if (spi_transfer(spi, txbuf, rxbuf, sizeof(txbuf)) < 0) {
        P_LOG_ERROR("LMK04828 write word failed, addr_data=0x%06X: %s",
                    addr_data, spi_errmsg(spi));
        return -1;
    }

    usleep(100);
    return 0;
}

static int lmk04828_write_table(spi_t *spi, const uint32_t *table, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i) {
        if (lmk04828_write_addr_data(spi, table[i]) != 0) {
            return -1;
        }
    }

    return 0;
}

static int lmk04828_config_clock(lmk04828_clock_config_t clock_config)
{
    size_t i;
    spi_t *spi;

    if (clock_config != LMK04828_CLOCK_CONFIG_100M && clock_config != LMK04828_CLOCK_CONFIG_10M) {
        P_LOG_ERROR("LMK04828 invalid clock config: %d", clock_config);
        return -1;
    }

    spi = spi_new();
    if (spi == NULL) {
        P_LOG_ERROR("LMK04828 spi_new failed");
        return -1;
    }

    if (spi_open(spi, LMK04828_SPI_DEV, LMK04828_SPI_MODE, LMK04828_SPI_SPEED_HZ) < 0) {
        P_LOG_ERROR("LMK04828 spi_open(%s) failed: %s", LMK04828_SPI_DEV, spi_errmsg(spi));
        spi_free(spi);
        return -1;
    }

    /* Same write sequence as bare-metal lmk04828_init():
     * reset bank -> selected clock table -> sync table.
     */
    if (lmk04828_write_table(spi, lmk04828_awg_reset_bank,
                             LMK04828_ARRAY_SIZE(lmk04828_awg_reset_bank)) != 0) {
        spi_close(spi);
        spi_free(spi);
        return -1;
    }

    if (lmk04828_awg_reg_100m[LMK04828_DIFF_INDEX] != LMK04828_100M_DIFF_VALUE) {
        P_LOG_ERROR("LMK04828 diff index mismatch, index=%d, value=0x%06X",
                    LMK04828_DIFF_INDEX, lmk04828_awg_reg_100m[LMK04828_DIFF_INDEX]);
        spi_close(spi);
        spi_free(spi);
        return -1;
    }

    for (i = 0; i < LMK04828_ARRAY_SIZE(lmk04828_awg_reg_100m); ++i) {
        uint32_t addr_data = lmk04828_awg_reg_100m[i];

        /* External 10 MHz uses the same table except this one fixed index. */
        if (clock_config == LMK04828_CLOCK_CONFIG_10M && i == LMK04828_DIFF_INDEX) {
            addr_data = LMK04828_10M_DIFF_VALUE;
        }

        if (lmk04828_write_addr_data(spi, addr_data) != 0) {
            spi_close(spi);
            spi_free(spi);
            return -1;
        }
    }

    if (lmk04828_write_table(spi, lmk04828_awg_sync,
                             LMK04828_ARRAY_SIZE(lmk04828_awg_sync)) != 0) {
        spi_close(spi);
        spi_free(spi);
        return -1;
    }

    spi_close(spi);
    spi_free(spi);
    return 0;
}

int lmk04828_init(RefClk ref_clk)
{
    lmk04828_clock_config_t clock_config = lmk04828_clock_config(ref_clk);

    /* Keep this sequence aligned with the bare-metal startup flow:
     * 1. choose internal/external clock path by GPIO
     * 2. wait for hardware path to settle
     * 3. configure LMK04828 through /dev/spidev1.0
     * 4. notify PL that clock and PCA9538 init can continue
     */
    lmk04828_select_ref_clock(ref_clk);

    sleep(2);

    if (lmk04828_config_clock(clock_config) != 0) {
        P_LOG_ERROR("LMK04828 register config failed, ref_clk=%d, clock_config=%d",
                    ref_clk, clock_config);
        return -1;
    }

    lmk04828_is_locked();

    sleep(3);

    /* Notify PL that the clock tree is ready. */
    gpio_set_value(LMK_GPIO_CLK_READY, 1);

    sleep(1);

    /* Notify PL that PCA9538 related initialization can continue. */
    gpio_set_value(LMK_GPIO_PCA9538_INIT, 1);

    gpio_set_value(DAC_SYNC, 1);

    return 0;
}
