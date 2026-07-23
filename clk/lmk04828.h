#ifndef _LMK04828_H_
#define _LMK04828_H_

#include "hmc7043.h"
#include <stdint.h>

#define LMK04828_SPI_DEV        "/dev/spidev1.0"
#define LMK04828_SPI_MODE       0
#define LMK04828_SPI_SPEED_HZ   1000000

int lmk04828_init(RefClk ref_clk);
int lmk04828_write_reg(uint16_t addr, uint8_t value);

#endif
