#ifndef _ADC9684_H_
#define _ADC9684_H_

#include <stdint.h>

int adc9684_write_reg(uint16_t addr, uint8_t data);
uint32_t adc9684_read_reg(uint16_t addr);
int init_adc9684(void);

#endif
