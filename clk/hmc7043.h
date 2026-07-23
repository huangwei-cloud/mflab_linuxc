#ifndef _HMC7043_H_
#define _HMC7043_H_

#include "../utils/zwtypes.h"
#include "../ring/ringbuffer.h"


typedef struct {
    char name[32];
    uint32_t base_addr;
    uint32_t map_size;
    volatile uint32_t *reg_base;
} AxiDevice;

typedef enum _DIRECT_REF 
{ 
	INTERNAL,	
	EXTERN,
}RefDirect;

typedef enum _CLK_REF 
{ 
	INTERNAL_100M,	
	EXTERN_100M,
	EXTERN_10M,
}RefClk;


int axi_device_init(AxiDevice *dev, const char *name, uint32_t base_addr, uint32_t map_size);

uint32_t hmc7044_read_reg(AxiDevice *dev, uint16_t addr);

void hmc7044_write_reg(AxiDevice *dev, uint16_t addr, uint8_t data);

void init_mem();

int init_clk(RefClk rc);

void get_plreg_intance(AxiDevice **pdev);

void get_bram1_intance(AxiDevice **pdev);

void get_bram2_intance(AxiDevice **pdev);

void get_ad4080_1_intance(AxiDevice **pdev);

void get_ad4080_2_intance(AxiDevice **pdev);

void get_ad9684_intance(AxiDevice **pdev);

void get_awg_ddr_ch1_intance(AxiDevice **pdev);

void get_awg_ddr_ch2_intance(AxiDevice **pdev);

void get_frame_filter_intance(FrameFilter **ff);

#endif

