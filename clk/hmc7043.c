#include "hmc7043.h"
#include "lmk04828.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <assert.h>
#include "../axigpio/gpio.h"
#include "../utils/scpi-def.h"
#include "../pm/param.h"
#include "../awg/awg.h"


AxiDevice plreg;
AxiDevice bram1,bram2;
AxiDevice hmc7044_1,hmc7044_2,hmc7044_3;
AxiDevice ad4080_1,ad4080_2,ad9684;
AxiDevice awg_ddr_ch1,awg_ddr_ch2;
FrameFilter *g_frame_filter = NULL;
extern int g_device_model;

void get_plreg_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &plreg;
}

void get_bram1_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &bram1;
}

void get_bram2_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &bram2;
}

void get_ad4080_1_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &ad4080_1;
}

void get_ad4080_2_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &ad4080_2;
}

void get_ad9684_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &ad9684;
}


void get_awg_ddr_ch1_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &awg_ddr_ch1;
}

void get_awg_ddr_ch2_intance(AxiDevice **pdev)
{
	assert(pdev != NULL);

	*pdev = &awg_ddr_ch2;
}

void get_frame_filter_intance(FrameFilter **ff)
{
	assert(ff != NULL);

	*ff = g_frame_filter;
}


int axi_device_init(AxiDevice *dev, const char *name, uint32_t base_addr, uint32_t map_size)
{
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Failed to open /dev/mem");
        return -1;
    }

    strncpy(dev->name, name, sizeof(dev->name) - 1);
    dev->name[sizeof(dev->name) - 1] = '\0';

    dev->base_addr = base_addr;
    dev->map_size = map_size;

    dev->reg_base = (volatile uint32_t *)mmap(NULL, map_size,
                                              PROT_READ | PROT_WRITE,
                                              MAP_SHARED, fd, base_addr);
    close(fd);

    if (dev->reg_base == MAP_FAILED) {
        printf("mmap failed");
        return -1;
    }

    printf("%s initialized at base address: 0x%X (size: 0x%X)\n", dev->name, dev->base_addr, dev->map_size);
    return 0;
}


void hmc7044_write_reg(AxiDevice *dev, uint16_t addr, uint8_t data)
{
    if (!dev || !dev->reg_base) return;

    dev->reg_base[0] = 0x00000000;
    dev->reg_base[1] = (addr << 8) | data;
    dev->reg_base[0] = 0x00000002;
    usleep(1000);

    //printf("[%s] Write: Addr=0x%X, Data=0x%X\n", dev->name, addr, data);
}


uint32_t hmc7044_read_reg(AxiDevice *dev, uint16_t addr)
{
	uint32_t value;

    if (!dev || !dev->reg_base) return 0xFFFFFFFF;


    dev->reg_base[0] = 0x00000000;
    dev->reg_base[1] = 0x00800000 | (addr << 8);
    dev->reg_base[0] = 0x00000002;
    sleep(1);

    value = dev->reg_base[1];
    //printf("[%s] Read: Addr=0x%X, Data=0x%X\n", dev->name, addr, value);
    return value;
}

void axi_device_release(AxiDevice *dev) {
    if (dev->reg_base) {
        munmap((void *)dev->reg_base, dev->map_size);
        printf("%s released (size: 0x%X).\n", dev->name, dev->map_size);
    }
}

void hmc7044_config_mm(RefDirect rd)
{
	uint32_t id;

	hmc7044_1.reg_base[0] = 1;
	hmc7044_1.reg_base[0] = 0;
	sleep(1);

	while(1) {
		id = hmc7044_read_reg(&hmc7044_1, 0x79);
		if(id == 0x00807952) {
			break;
		}else {
			printf("hmc7044-1 id error\r\n");
		}
		usleep(1000);
	}

	hmc7044_write_reg(&hmc7044_1, 0x0,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x0,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1,  0x00);
	hmc7044_write_reg(&hmc7044_1, 0x2,  0x4);
	hmc7044_write_reg(&hmc7044_1, 0x3,  0x2F);
	hmc7044_write_reg(&hmc7044_1, 0x4,  0x7F);

	hmc7044_write_reg(&hmc7044_1, 0x5,  0x42);
	hmc7044_write_reg(&hmc7044_1, 0x6,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x7,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0xA,  0x07);
	hmc7044_write_reg(&hmc7044_1, 0xB,  0x07);
	hmc7044_write_reg(&hmc7044_1, 0xC,  0x07);
	hmc7044_write_reg(&hmc7044_1, 0xD,  0x15);
	hmc7044_write_reg(&hmc7044_1, 0xE,  0x07);

	hmc7044_write_reg(&hmc7044_1, 0x14,  0x55);
	hmc7044_write_reg(&hmc7044_1, 0x15,  0x3);
	hmc7044_write_reg(&hmc7044_1, 0x16,  0xC);
	hmc7044_write_reg(&hmc7044_1, 0x17,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x18,  0x4);
	hmc7044_write_reg(&hmc7044_1, 0x19,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1A,  0x4);
	hmc7044_write_reg(&hmc7044_1, 0x1B,  0x18);
	hmc7044_write_reg(&hmc7044_1, 0x1C,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1D,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1E,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1F,  0x1);

	if(rd == INTERNAL){
		hmc7044_write_reg(&hmc7044_1, 0x20, 0x1);
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	} else {
		hmc7044_write_reg(&hmc7044_1, 0x20, 0xA);
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x1);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0xA);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	}

	hmc7044_write_reg(&hmc7044_1, 0x28,  0xF);
	hmc7044_write_reg(&hmc7044_1, 0x29,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x2A,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x31,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x32,  0x1);

	// NCO 3.2GHz
	hmc7044_write_reg(&hmc7044_1, 0x33,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x34,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x35,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0x36,  0x00);

	hmc7044_write_reg(&hmc7044_1, 0x37,  0xf);
	hmc7044_write_reg(&hmc7044_1, 0x38,  0x18);
	hmc7044_write_reg(&hmc7044_1, 0x39,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x3A,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x3B,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x46,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x47,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x48,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x49,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x50,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x51,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x52,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x53,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x54,  0x3);
	hmc7044_write_reg(&hmc7044_1, 0x5A,  0x1);
	hmc7044_write_reg(&hmc7044_1, 0x5B,  0x6);
	hmc7044_write_reg(&hmc7044_1, 0x5C,  0x00);
	hmc7044_write_reg(&hmc7044_1, 0x5D,  0xA);
	hmc7044_write_reg(&hmc7044_1, 0x64,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x65,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x70,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x71,  0x10);
	hmc7044_write_reg(&hmc7044_1, 0x78,  0x8);
	hmc7044_write_reg(&hmc7044_1, 0x79,  0x9);
	hmc7044_write_reg(&hmc7044_1, 0x7A,  0xA);
	hmc7044_write_reg(&hmc7044_1, 0x7B,  0xB);
	hmc7044_write_reg(&hmc7044_1, 0x7C,  0xC);
	hmc7044_write_reg(&hmc7044_1, 0x7D,  0xD);
	hmc7044_write_reg(&hmc7044_1, 0x7E,  0xE);
	hmc7044_write_reg(&hmc7044_1, 0x82,  0xF);
	hmc7044_write_reg(&hmc7044_1, 0x83,  0x10);
	hmc7044_write_reg(&hmc7044_1, 0x84,  0x11);
	hmc7044_write_reg(&hmc7044_1, 0x85,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x86,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x8C,  0x2);
	hmc7044_write_reg(&hmc7044_1, 0x8D,  0x3);
	hmc7044_write_reg(&hmc7044_1, 0x8E,  0x4);
	hmc7044_write_reg(&hmc7044_1, 0x8F,  0x5);
	hmc7044_write_reg(&hmc7044_1, 0x91,  0x6);
	hmc7044_write_reg(&hmc7044_1, 0x96,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x97,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x98,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x99,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9A,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9B,  0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9C,  0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9D,  0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9E,  0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9F,  0x4D);
	hmc7044_write_reg(&hmc7044_1, 0xA0,  0xDF);
	hmc7044_write_reg(&hmc7044_1, 0xA1,  0x97);
	hmc7044_write_reg(&hmc7044_1, 0xA2,  0x3);
	hmc7044_write_reg(&hmc7044_1, 0xA3,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA4,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA5,  0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA6,  0x1C);
	hmc7044_write_reg(&hmc7044_1, 0xA7,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA8,  0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA9,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAB,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAC,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0xAD,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAE,  0x8);
	hmc7044_write_reg(&hmc7044_1, 0xAF,  0x50);
	hmc7044_write_reg(&hmc7044_1, 0xB0,  0x4);
	hmc7044_write_reg(&hmc7044_1, 0xB1,  0xD);
	hmc7044_write_reg(&hmc7044_1, 0xB2,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB3,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB5,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB6,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB7,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB8,  0x0);
	//ch1 10MHz
	hmc7044_write_reg(&hmc7044_1, 0xC8,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xC9,  0x40);
	hmc7044_write_reg(&hmc7044_1, 0xCA,  0x01);
	hmc7044_write_reg(&hmc7044_1, 0xCB,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCC,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCD,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCE,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCF,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD0,  0x09);

	//ch2 100MHz
	hmc7044_write_reg(&hmc7044_1, 0xD2,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xD3,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0xD4,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD5,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD6,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD7,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD8,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD9,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDA,  0x09);

	//ch3 160
	hmc7044_write_reg(&hmc7044_1, 0xDC,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xDD,  0x14);
	hmc7044_write_reg(&hmc7044_1, 0xDE,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDF,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE0,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE1,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE2,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE3,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE4,  0x09);

	//ch4 5M
	hmc7044_write_reg(&hmc7044_1, 0xE6,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xE7,  0x80);
	hmc7044_write_reg(&hmc7044_1, 0xE8,  0x2);
	hmc7044_write_reg(&hmc7044_1, 0xE9,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEA,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEB,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEC,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xED,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEE,  0x09);

	//ch5 100
	hmc7044_write_reg(&hmc7044_1, 0xF0,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xF1,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0xF2,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF3,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF4,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF5,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF6,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF7,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF8,  0x9);

	//ch6 100
	hmc7044_write_reg(&hmc7044_1, 0xFA,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xFB,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0xFC,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFD,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFE,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFF,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x100,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x101,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x102,  0x9);

	//ch7 100
	hmc7044_write_reg(&hmc7044_1, 0x104,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x105,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0x106,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x107,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x108,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x109,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10A,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10B,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10C,  0x9);

	//ch8 100
	hmc7044_write_reg(&hmc7044_1, 0x10E,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x10F,  0x20);
	hmc7044_write_reg(&hmc7044_1, 0x110,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x111,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x112,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x113,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x114,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x115,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x116,  0x9);

	//ch9 160MHz
	hmc7044_write_reg(&hmc7044_1, 0x118,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x119,  0x14);
	hmc7044_write_reg(&hmc7044_1, 0x11A,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11B,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11C,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11D,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11E,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11F,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x120,  0x9);

	//ch10 5
	hmc7044_write_reg(&hmc7044_1, 0x122,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x123,  0x80);
	hmc7044_write_reg(&hmc7044_1, 0x124,  0x2);
	hmc7044_write_reg(&hmc7044_1, 0x125,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x126,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x127,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x128,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x129,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12A,  0x09);

	//ch11 200MHz
	hmc7044_write_reg(&hmc7044_1, 0x12C,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x12D,  0x10);
	hmc7044_write_reg(&hmc7044_1, 0x12E,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12F,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x130,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x131,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x132,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x133,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x134,  0x11);
	//ch12 40Mhz
	hmc7044_write_reg(&hmc7044_1, 0x136,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x137,  0x50);
	hmc7044_write_reg(&hmc7044_1, 0x138,  0x00);
	hmc7044_write_reg(&hmc7044_1, 0x139,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13A,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13B,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13C,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13D,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13E,  0x11);
	//ch13 40M
	hmc7044_write_reg(&hmc7044_1, 0x140,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x141,  0x50);
	hmc7044_write_reg(&hmc7044_1, 0x142,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x143,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x144,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x145,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x146,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x147,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x148,  0x11);
	//ch14 2M
	hmc7044_write_reg(&hmc7044_1, 0x14A,  0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x14B,  0x80);
	hmc7044_write_reg(&hmc7044_1, 0x14C,  0x2);
	hmc7044_write_reg(&hmc7044_1, 0x14D,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14E,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14F,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x150,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x151,  0x0);
	hmc7044_write_reg(&hmc7044_1, 0x152,  0x11);

	//send sync
	usleep(100000);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x42);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	usleep(100000);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0x44);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	usleep(100000);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0xC0);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
}


void hmc7044_config1(RefDirect rd)
{
	uint32_t id;

	hmc7044_1.reg_base[0] = 1;
	hmc7044_1.reg_base[0] = 0;
	sleep(1);

	while(1) {
		id = hmc7044_read_reg(&hmc7044_1, 0x79);
		if(id == 0x00807952) {
			break;
		}else {
			printf("hmc7044-1 id error\r\n");
		}
		usleep(1000);
	}

	hmc7044_write_reg(&hmc7044_1, 0x0, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x0, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x2, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x3, 0x2F);
	hmc7044_write_reg(&hmc7044_1, 0x4, 0x7F);
	hmc7044_write_reg(&hmc7044_1, 0x5, 0x42);

	hmc7044_write_reg(&hmc7044_1, 0x6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0xA, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xB, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xC, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xD, 0x15);
	hmc7044_write_reg(&hmc7044_1, 0xE, 0x07);

	hmc7044_write_reg(&hmc7044_1, 0x14, 0x55);
	hmc7044_write_reg(&hmc7044_1, 0x15, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x16, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0x17, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x18, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x19, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1A, 0x8);
	hmc7044_write_reg(&hmc7044_1, 0x1B, 0x18);
	hmc7044_write_reg(&hmc7044_1, 0x1C, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1D, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1E, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1F, 0x1);

	if(rd == INTERNAL){
		hmc7044_write_reg(&hmc7044_1, 0x20, 0x1);
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	} else {
		hmc7044_write_reg(&hmc7044_1, 0x20, 0xA);
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x1);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0xA);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	}

	hmc7044_write_reg(&hmc7044_1, 0x28, 0xF);
	hmc7044_write_reg(&hmc7044_1, 0x29, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x2A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x31, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x32, 0x1);

	// NCO 3.2GHz
	hmc7044_write_reg(&hmc7044_1, 0x33, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x34, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x35, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0x36, 0x00);

	hmc7044_write_reg(&hmc7044_1, 0x37, 0xf);
	hmc7044_write_reg(&hmc7044_1, 0x38, 0x18);
	hmc7044_write_reg(&hmc7044_1, 0x39, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x3A, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x3B, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x46, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x47, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x48, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x49, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x50, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x51, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x52, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x53, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x54, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x5A, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0x5B, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x5C, 0x08);
	hmc7044_write_reg(&hmc7044_1, 0x5D, 0x7);
	hmc7044_write_reg(&hmc7044_1, 0x64, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x65, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x70, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x71, 0x10);
	hmc7044_write_reg(&hmc7044_1, 0x78, 0x8);
	hmc7044_write_reg(&hmc7044_1, 0x79, 0x9);
	hmc7044_write_reg(&hmc7044_1, 0x7A, 0xA);
	hmc7044_write_reg(&hmc7044_1, 0x7B, 0xB);
	hmc7044_write_reg(&hmc7044_1, 0x7C, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0x7D, 0xD);
	hmc7044_write_reg(&hmc7044_1, 0x7E, 0xE);
	hmc7044_write_reg(&hmc7044_1, 0x82, 0xF);
	hmc7044_write_reg(&hmc7044_1, 0x83, 0x10);
	hmc7044_write_reg(&hmc7044_1, 0x84, 0x11);
	hmc7044_write_reg(&hmc7044_1, 0x85, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x86, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x8C, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0x8D, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x8E, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x8F, 0x5);
	hmc7044_write_reg(&hmc7044_1, 0x91, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x96, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x97, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x98, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x99, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9B, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9C, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9D, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9E, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9F, 0x4D);
	hmc7044_write_reg(&hmc7044_1, 0xA0, 0xDF);
	hmc7044_write_reg(&hmc7044_1, 0xA1, 0x97);
	hmc7044_write_reg(&hmc7044_1, 0xA2, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0xA3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA5, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA6, 0x1C);
	hmc7044_write_reg(&hmc7044_1, 0xA7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA8, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAC, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xAD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAE, 0x8);
	hmc7044_write_reg(&hmc7044_1, 0xAF, 0x50);
	hmc7044_write_reg(&hmc7044_1, 0xB0, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0xB1, 0xD);
	hmc7044_write_reg(&hmc7044_1, 0xB2, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB8, 0x0);
	//ch1 10MHz
	hmc7044_write_reg(&hmc7044_1, 0xC8, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xC9, 0x40);
	hmc7044_write_reg(&hmc7044_1, 0xCA, 0x01);
	hmc7044_write_reg(&hmc7044_1, 0xCB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCC, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD0, 0x09);

	//ch2 100MHz
	hmc7044_write_reg(&hmc7044_1, 0xD2, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xD3, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xD4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD8, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDA, 0x09);

	//ch3 close
	hmc7044_write_reg(&hmc7044_1, 0xDC, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0xDD, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0xDE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE0, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE1, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE2, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE4, 0x09);

	//ch4 close
	hmc7044_write_reg(&hmc7044_1, 0xE6, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0xE7, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0xE8, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEA, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEC, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xED, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEE, 0x09);

	//ch5 close
	hmc7044_write_reg(&hmc7044_1, 0xF0, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0xF1, 0x0C);
	hmc7044_write_reg(&hmc7044_1, 0xF2, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF8, 0x9);

	//ch6 close
	hmc7044_write_reg(&hmc7044_1, 0xFA, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0xFB, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0xFC, 0x9);
	hmc7044_write_reg(&hmc7044_1, 0xFD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x100, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x101, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x102, 0x9);

	//ch7 close
	hmc7044_write_reg(&hmc7044_1, 0x104, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0x105, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0x106, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x107, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x108, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x109, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10C, 0x9);

	//ch8 close
	hmc7044_write_reg(&hmc7044_1, 0x10E, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0x10F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x110, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x111, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x112, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x113, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x114, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x115, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x116, 0x9);

	//ch9 800MHz
	hmc7044_write_reg(&hmc7044_1, 0x118, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x119, 0x04);
	hmc7044_write_reg(&hmc7044_1, 0x11A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11C, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x120, 0x9);

	//ch10 6.25Mhz
	hmc7044_write_reg(&hmc7044_1, 0x122, 0xDD);
	hmc7044_write_reg(&hmc7044_1, 0x123, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0x124, 0xA);
	hmc7044_write_reg(&hmc7044_1, 0x125, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x126, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x127, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x128, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x129, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12A, 0x89);

	//ch11 800MHz
	hmc7044_write_reg(&hmc7044_1, 0x12C, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x12D, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x12E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x130, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x131, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x132, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x133, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x134, 0x9);
	//ch12 6.25Mhz
	hmc7044_write_reg(&hmc7044_1, 0x136, 0xDD);
	hmc7044_write_reg(&hmc7044_1, 0x137, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x138, 0x0A);
	hmc7044_write_reg(&hmc7044_1, 0x139, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13C, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13E, 0x89);
	//ch13 close
	hmc7044_write_reg(&hmc7044_1, 0x140, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0x141, 0x12);
	hmc7044_write_reg(&hmc7044_1, 0x142, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x143, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x144, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x145, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x146, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x147, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x148, 0x11);
	//ch14 close
	hmc7044_write_reg(&hmc7044_1, 0x14A, 0xD0);
	hmc7044_write_reg(&hmc7044_1, 0x14B, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0x14C, 0x9);
	hmc7044_write_reg(&hmc7044_1, 0x14D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x150, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x151, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x152, 0x11);

	//send sync
	usleep(100000);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x42);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	usleep(100000);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0x44);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	usleep(100000);
	//hmc7044_write_reg(&hmc7044_1, 0x1, 0xC0);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
}

void hmc7044_config2(RefDirect rd)
{
	(void)rd;

	uint32_t id;

	hmc7044_2.reg_base[0] = 1;
	hmc7044_2.reg_base[0] = 0;
	sleep(1);

	while(1) {
		id = hmc7044_read_reg(&hmc7044_2, 0x78);
		if(id == 0x008078F1) {
			break;
		}else {
			printf("hmc7044-2 id error\r\n");
		}
		usleep(1000);
	}

	hmc7044_write_reg(&hmc7044_2, 0x0, 0x1);
	hmc7044_write_reg(&hmc7044_2, 0x0, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x1, 0x40);
	hmc7044_write_reg(&hmc7044_2, 0x2, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x3, 0x34);
	hmc7044_write_reg(&hmc7044_2, 0x4, 0x7F);

	hmc7044_write_reg(&hmc7044_2, 0x5, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x6, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x9, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xA, 0x03);
	hmc7044_write_reg(&hmc7044_2, 0xB, 0x03);

	hmc7044_write_reg(&hmc7044_2, 0x46, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x50, 0x7);
	hmc7044_write_reg(&hmc7044_2, 0x54, 0x3);

	hmc7044_write_reg(&hmc7044_2, 0x5A, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0x5B, 0x00);

	hmc7044_write_reg(&hmc7044_2, 0x5C, 0xA0);
	hmc7044_write_reg(&hmc7044_2, 0x5D, 0x0);

	hmc7044_write_reg(&hmc7044_2, 0x64, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x65, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x71, 0x16);
	hmc7044_write_reg(&hmc7044_2, 0x78, 0x8);
	hmc7044_write_reg(&hmc7044_2, 0x79, 0x9);
	hmc7044_write_reg(&hmc7044_2, 0x7A, 0xA);
	hmc7044_write_reg(&hmc7044_2, 0x7B, 0xB);
	hmc7044_write_reg(&hmc7044_2, 0x7D, 0xD);
	hmc7044_write_reg(&hmc7044_2, 0x7F, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x91, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x98, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x99, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x9D, 0xAA);
	hmc7044_write_reg(&hmc7044_2, 0x9E, 0xAA);
	hmc7044_write_reg(&hmc7044_2, 0x9F, 0x4D);
	hmc7044_write_reg(&hmc7044_2, 0xA0, 0xDF);
	hmc7044_write_reg(&hmc7044_2, 0xA2, 0x3);
	hmc7044_write_reg(&hmc7044_2, 0xA3, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xA4, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xAD, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xB5, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xB6, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xB7, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xB8, 0x0);
	//ch1 40MHz
	hmc7044_write_reg(&hmc7044_2, 0xC8, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xC9, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0xCA, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0xCB, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xCC, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xCD, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xCE, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xCF, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xD0, 0x11);


	//ch2 400MHz
	hmc7044_write_reg(&hmc7044_2, 0xD2, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xD3, 0x4);
	hmc7044_write_reg(&hmc7044_2, 0xD4, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0xD5, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xD6, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xD7, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xD8, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xD9, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xDA, 0x11);

	//ch3 close
	hmc7044_write_reg(&hmc7044_2, 0xDC, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xDD, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0xDE, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xDF, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE0, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE1, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE2, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE3, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE4, 0x11);

	//ch4 close
	hmc7044_write_reg(&hmc7044_2, 0xE6, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xE7, 0x4);
	hmc7044_write_reg(&hmc7044_2, 0xE8, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xE9, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xEA, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xEB, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xEC, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xED, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xEE, 0x11);

	//ch5 close
	hmc7044_write_reg(&hmc7044_2, 0xF0, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xF1, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0xF2, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF3, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF4, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF5, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF6, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF7, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xF8, 0x11);

	//ch6 close
	hmc7044_write_reg(&hmc7044_2, 0xFA, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0xFB, 0x4);
	hmc7044_write_reg(&hmc7044_2, 0xFC, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0xFD, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0xFE, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0xFF, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x100, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x101, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x102, 0x11);

	//ch7 close
	hmc7044_write_reg(&hmc7044_2, 0x104, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x105, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0x106, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x107, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x108, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x109, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x10A, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x10B, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x10C, 0x11);

	//ch8 close
	hmc7044_write_reg(&hmc7044_2, 0x10E, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x10F, 0x04);
	hmc7044_write_reg(&hmc7044_2, 0x110, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x111, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x112, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x113, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x114, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x115, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x116, 0x11);

	//ch9 800MHz
	hmc7044_write_reg(&hmc7044_2, 0x118, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x119, 0x08);
	hmc7044_write_reg(&hmc7044_2, 0x11A, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0x11B, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x11C, 0x04);
	hmc7044_write_reg(&hmc7044_2, 0x11D, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x11E, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x11F, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x120, 0x11);

	//ch10 6.25Mhz
	hmc7044_write_reg(&hmc7044_2, 0x122, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x123, 0x08);
	hmc7044_write_reg(&hmc7044_2, 0x124, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0x125, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x126, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x127, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x128, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x129, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x12A, 0x11);

	//ch11 800MHz
	hmc7044_write_reg(&hmc7044_2, 0x12C, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x12D, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0x12E, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0x12F, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x130, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x131, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x132, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x133, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x134, 0x11);
	//ch12 6.25Mhz
	hmc7044_write_reg(&hmc7044_2, 0x136, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x137, 0x4);
	hmc7044_write_reg(&hmc7044_2, 0x138, 0x00);
	hmc7044_write_reg(&hmc7044_2, 0x139, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x13A, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x13B, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x13C, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x13D, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x13E, 0x11);
	//ch13 close
	hmc7044_write_reg(&hmc7044_2, 0x140, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x141, 0x14);
	hmc7044_write_reg(&hmc7044_2, 0x142, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x143, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x144, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x145, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x146, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x147, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x148, 0x11);
	//ch14 close
	hmc7044_write_reg(&hmc7044_2, 0x14A, 0xD1);
	hmc7044_write_reg(&hmc7044_2, 0x14B, 0x4);
	hmc7044_write_reg(&hmc7044_2, 0x14C, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x14D, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x14E, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x14F, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x150, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x151, 0x0);
	hmc7044_write_reg(&hmc7044_2, 0x152, 0x11);

	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0x42);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0x40);
	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0x44);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0x40);
	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0xc0);
	//hmc7044_write_reg(&hmc7044_2, 0x1, 0x40);

}

void hmc7044_config3(RefDirect rd)
{
	(void)rd;

	uint32_t id;

	hmc7044_3.reg_base[0] = 1;
	hmc7044_3.reg_base[0] = 0;
	sleep(1);

	while(1) {
		id = hmc7044_read_reg(&hmc7044_3, 0x78);
		if(id == 0x008078F1) {
			break;
		}else {
			printf("hmc7044-3 id error\r\n");
		}
		usleep(1000);
	}

	hmc7044_write_reg(&hmc7044_3, 0x0, 0x1);
	hmc7044_write_reg(&hmc7044_3, 0x0, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x1, 0x40);
	hmc7044_write_reg(&hmc7044_3, 0x2, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x3, 0x34);
	hmc7044_write_reg(&hmc7044_3, 0x4, 0x7F);

	hmc7044_write_reg(&hmc7044_3, 0x5, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x6, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x9, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xA, 0x03);
	hmc7044_write_reg(&hmc7044_3, 0xB, 0x03);

	hmc7044_write_reg(&hmc7044_3, 0x46, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x50, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x54, 0x3);

	hmc7044_write_reg(&hmc7044_3, 0x5A, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0x5B, 0x00);

	hmc7044_write_reg(&hmc7044_3, 0x5C, 0xA0);
	hmc7044_write_reg(&hmc7044_3, 0x5D, 0x0);

	hmc7044_write_reg(&hmc7044_3, 0x64, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x65, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x71, 0x16);
	hmc7044_write_reg(&hmc7044_3, 0x78, 0x8);
	hmc7044_write_reg(&hmc7044_3, 0x79, 0x9);
	hmc7044_write_reg(&hmc7044_3, 0x7A, 0xA);
	hmc7044_write_reg(&hmc7044_3, 0x7B, 0xB);
	hmc7044_write_reg(&hmc7044_3, 0x7D, 0xD);
	hmc7044_write_reg(&hmc7044_3, 0x7F, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x91, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x98, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x99, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x9D, 0xAA);
	hmc7044_write_reg(&hmc7044_3, 0x9E, 0xAA);
	hmc7044_write_reg(&hmc7044_3, 0x9F, 0x4D);
	hmc7044_write_reg(&hmc7044_3, 0xA0, 0xDF);
	hmc7044_write_reg(&hmc7044_3, 0xA2, 0x3);
	hmc7044_write_reg(&hmc7044_3, 0xA3, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xA4, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xAD, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xB5, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xB6, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xB7, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xB8, 0x0);
	//ch1 40MHz
	hmc7044_write_reg(&hmc7044_3, 0xC8, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0xC9, 0x05);
	hmc7044_write_reg(&hmc7044_3, 0xCA, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0xCB, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xCC, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xCD, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xCE, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xCF, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xD0, 0x09);

	//ch2 400MHz
	hmc7044_write_reg(&hmc7044_3, 0xD2, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0xD3, 0xA0);
	hmc7044_write_reg(&hmc7044_3, 0xD4, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0xD5, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xD6, 0x5);
	hmc7044_write_reg(&hmc7044_3, 0xD7, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xD8, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xD9, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xDA, 0x09);

	//ch3 close
	hmc7044_write_reg(&hmc7044_3, 0xDC, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0xDD, 0x05);
	hmc7044_write_reg(&hmc7044_3, 0xDE, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xDF, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE0, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE1, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE2, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE3, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE4, 0x09);

	//ch4 close
	hmc7044_write_reg(&hmc7044_3, 0xE6, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0xE7, 0xA0);
	hmc7044_write_reg(&hmc7044_3, 0xE8, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xE9, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xEA, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xEB, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xEC, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xED, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xEE, 0x09);

	//ch5 close
	hmc7044_write_reg(&hmc7044_3, 0xF0, 0xD0);
	hmc7044_write_reg(&hmc7044_3, 0xF1, 0xA0);
	hmc7044_write_reg(&hmc7044_3, 0xF2, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF3, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF4, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF5, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF6, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF7, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xF8, 0x11);

	//ch6 close
	hmc7044_write_reg(&hmc7044_3, 0xFA, 0xD0);
	hmc7044_write_reg(&hmc7044_3, 0xFB, 0x10);
	hmc7044_write_reg(&hmc7044_3, 0xFC, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0xFD, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xFE, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0xFF, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x100, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x101, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x102, 0x11);

	//ch7 close
	hmc7044_write_reg(&hmc7044_3, 0x104, 0xD0);
	hmc7044_write_reg(&hmc7044_3, 0x105, 0x50);
	hmc7044_write_reg(&hmc7044_3, 0x106, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x107, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x108, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x109, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x10A, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x10B, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x10C, 0x11);

	//ch8 close
	hmc7044_write_reg(&hmc7044_3, 0x10E, 0xD0);
	hmc7044_write_reg(&hmc7044_3, 0x10F, 0x10);
	hmc7044_write_reg(&hmc7044_3, 0x110, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x111, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x112, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x113, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x114, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x115, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x116, 0x11);

	//ch9 800MHz
	hmc7044_write_reg(&hmc7044_3, 0x118, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x119, 0x28);
	hmc7044_write_reg(&hmc7044_3, 0x11A, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0x11B, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x11C, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x11D, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x11E, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x11F, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x120, 0x11);

	//ch10 6.25Mhz
	hmc7044_write_reg(&hmc7044_3, 0x122, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x123, 0x08);
	hmc7044_write_reg(&hmc7044_3, 0x124, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0x125, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x126, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x127, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x128, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x129, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x12A, 0x10);

	//ch11 800MHz
	hmc7044_write_reg(&hmc7044_3, 0x12C, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x12D, 0x14);
	hmc7044_write_reg(&hmc7044_3, 0x12E, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0x12F, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x130, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x131, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x132, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x133, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x134, 0x11);
	//ch12 6.25Mhz
	hmc7044_write_reg(&hmc7044_3, 0x136, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x137, 0xA0);
	hmc7044_write_reg(&hmc7044_3, 0x138, 0x00);
	hmc7044_write_reg(&hmc7044_3, 0x139, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x13A, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x13B, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x13C, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x13D, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x13E, 0x11);
	//ch13 close
	hmc7044_write_reg(&hmc7044_3, 0x140, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x141, 0x08);
	hmc7044_write_reg(&hmc7044_3, 0x142, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x143, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x144, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x145, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x146, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x147, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x148, 0x11);
	//ch14 close
	hmc7044_write_reg(&hmc7044_3, 0x14A, 0xD1);
	hmc7044_write_reg(&hmc7044_3, 0x14B, 0x14);
	hmc7044_write_reg(&hmc7044_3, 0x14C, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x14D, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x14E, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x14F, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x150, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x151, 0x0);
	hmc7044_write_reg(&hmc7044_3, 0x152, 0x11);

	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0x42);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0x40);
	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0x44);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0x40);
	//usleep(100000);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0xc0);
	//hmc7044_write_reg(&hmc7044_3, (0x1, 0x40);

}

void hmc7044_config_lk(RefDirect rd)
{
	uint32_t id;

	hmc7044_1.reg_base[0] = 1;
	hmc7044_1.reg_base[0] = 0;
	sleep(1);

	while(1) {
		id = hmc7044_read_reg(&hmc7044_1, 0x79);
		if(id == 0x00807952) {
			break;
		}else {
			printf("hmc7044-1 id error: 0x%08x\r\n", id);
		}
		usleep(1000);
	}

	hmc7044_write_reg(&hmc7044_1, 0x0, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x0, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0x2, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x3, 0x2F);
	hmc7044_write_reg(&hmc7044_1, 0x4, 0x7F);
	hmc7044_write_reg(&hmc7044_1, 0x5, 0x42);
	hmc7044_write_reg(&hmc7044_1, 0x6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0xA, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xB, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xC, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0xD, 0x15);
	hmc7044_write_reg(&hmc7044_1, 0xE, 0x07);
	hmc7044_write_reg(&hmc7044_1, 0x14, 0x55);
	hmc7044_write_reg(&hmc7044_1, 0x15, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x16, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0x17, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x18, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x19, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x1A, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x1B, 0x18);
	hmc7044_write_reg(&hmc7044_1, 0x1C, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1D, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1E, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x1F, 0x1);
	if(rd == INTERNAL) {
		hmc7044_write_reg(&hmc7044_1, 0x20, 0x1);//100M
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0x4);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	}else {
		hmc7044_write_reg(&hmc7044_1, 0x20, 0xA);//10M
		hmc7044_write_reg(&hmc7044_1, 0x21, 0x1);
		hmc7044_write_reg(&hmc7044_1, 0x22, 0x0);
		hmc7044_write_reg(&hmc7044_1, 0x26, 0xA);
		hmc7044_write_reg(&hmc7044_1, 0x27, 0x0);
	}
	hmc7044_write_reg(&hmc7044_1, 0x28, 0xF);
	hmc7044_write_reg(&hmc7044_1, 0x29, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x2A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x31, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x32, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x33, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x34, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x35, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0x36, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0x37, 0xf);
	hmc7044_write_reg(&hmc7044_1, 0x38, 0x18);
	hmc7044_write_reg(&hmc7044_1, 0x39, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x3A, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x3B, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x46, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x47, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x48, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x49, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x50, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x51, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x52, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x53, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x54, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x5A, 0x1);
	hmc7044_write_reg(&hmc7044_1, 0x5B, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x5C, 0x00);
	hmc7044_write_reg(&hmc7044_1, 0x5D, 0xA);
	hmc7044_write_reg(&hmc7044_1, 0x64, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x65, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x70, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x71, 0x10);
	hmc7044_write_reg(&hmc7044_1, 0x78, 0x8);
	hmc7044_write_reg(&hmc7044_1, 0x79, 0x9);
	hmc7044_write_reg(&hmc7044_1, 0x7A, 0xA);
	hmc7044_write_reg(&hmc7044_1, 0x7B, 0xB);
	hmc7044_write_reg(&hmc7044_1, 0x7C, 0xC);
	hmc7044_write_reg(&hmc7044_1, 0x7D, 0xD);
	hmc7044_write_reg(&hmc7044_1, 0x7E, 0xE);
	hmc7044_write_reg(&hmc7044_1, 0x82, 0xF);
	hmc7044_write_reg(&hmc7044_1, 0x83, 0x10);
	hmc7044_write_reg(&hmc7044_1, 0x84, 0x11);
	hmc7044_write_reg(&hmc7044_1, 0x85, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x86, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x8C, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0x8D, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0x8E, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0x8F, 0x5);
	hmc7044_write_reg(&hmc7044_1, 0x91, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x96, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x97, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x98, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x99, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x9B, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9C, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9D, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9E, 0xAA);
	hmc7044_write_reg(&hmc7044_1, 0x9F, 0x4D);
	hmc7044_write_reg(&hmc7044_1, 0xA0, 0xDF);
	hmc7044_write_reg(&hmc7044_1, 0xA1, 0x97);
	hmc7044_write_reg(&hmc7044_1, 0xA2, 0x3);
	hmc7044_write_reg(&hmc7044_1, 0xA3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA5, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA6, 0x1C);
	hmc7044_write_reg(&hmc7044_1, 0xA7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xA8, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0xA9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAC, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xAD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xAE, 0x8);
	hmc7044_write_reg(&hmc7044_1, 0xAF, 0x50);
	hmc7044_write_reg(&hmc7044_1, 0xB0, 0x4);
	hmc7044_write_reg(&hmc7044_1, 0xB1, 0xD);
	hmc7044_write_reg(&hmc7044_1, 0xB2, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xB8, 0x0);
	//ch1 10MHz
	hmc7044_write_reg(&hmc7044_1, 0xC8, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xC9, 0x40);
	hmc7044_write_reg(&hmc7044_1, 0xCA, 0x01);
	hmc7044_write_reg(&hmc7044_1, 0xCB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCC, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xCF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD0, 0x09);
	//ch2 100MHz
	hmc7044_write_reg(&hmc7044_1, 0xD2, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xD3, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xD4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD8, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xD9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDA, 0x11);
	//ch3 100MHz
	hmc7044_write_reg(&hmc7044_1, 0xDC, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xDD, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xDE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xDF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE0, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE1, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE2, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE4, 0x11);
	//ch4 100MHz
	hmc7044_write_reg(&hmc7044_1, 0xE6, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xE7, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0xE8, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xE9, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEA, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEB, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEC, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xED, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xEE, 0x11);
	//ch5 10MHz
	hmc7044_write_reg(&hmc7044_1, 0xF0, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xF1, 0x40);
	hmc7044_write_reg(&hmc7044_1, 0xF2, 0x01);
	hmc7044_write_reg(&hmc7044_1, 0xF3, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF4, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF5, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF6, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF7, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xF8, 0x11);
	//ch6 5MHz
	hmc7044_write_reg(&hmc7044_1, 0xFA, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0xFB, 0x80);
	hmc7044_write_reg(&hmc7044_1, 0xFC, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0xFD, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFE, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0xFF, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x100, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x101, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x102, 0x11);
	//ch7 40M
	hmc7044_write_reg(&hmc7044_1, 0x104, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x105, 0x50);
	hmc7044_write_reg(&hmc7044_1, 0x106, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x107, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x108, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x109, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x10C, 0x11);
	//ch8 100MHz
	hmc7044_write_reg(&hmc7044_1, 0x10E, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x10F, 0x20);
	hmc7044_write_reg(&hmc7044_1, 0x110, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x111, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x112, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x113, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x114, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x115, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x116, 0x11);
	//ch9 160M
	hmc7044_write_reg(&hmc7044_1, 0x118, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x119, 0x14);
	hmc7044_write_reg(&hmc7044_1, 0x11A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11C, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x11F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x120, 0x9);
	//ch10 5Mhz
	hmc7044_write_reg(&hmc7044_1, 0x122, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x123, 0x80);
	hmc7044_write_reg(&hmc7044_1, 0x124, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0x125, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x126, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x127, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x128, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x129, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12A, 0x9);
	//ch11 160
	hmc7044_write_reg(&hmc7044_1, 0x12C, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x12D, 0x14);
	hmc7044_write_reg(&hmc7044_1, 0x12E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x12F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x130, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x131, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x132, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x133, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x134, 0x9);
	//ch12 2MHz
	hmc7044_write_reg(&hmc7044_1, 0x136, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x137, 0x80);
	hmc7044_write_reg(&hmc7044_1, 0x138, 0x2);
	hmc7044_write_reg(&hmc7044_1, 0x139, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13A, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13B, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13C, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x13E, 0x9);
	//ch13 close
	hmc7044_write_reg(&hmc7044_1, 0x140, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x141, 0x40);
	hmc7044_write_reg(&hmc7044_1, 0x142, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x143, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x144, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x145, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x146, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x147, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x148, 0x11);
	//ch14 2MHz
	hmc7044_write_reg(&hmc7044_1, 0x14A, 0xD1);
	hmc7044_write_reg(&hmc7044_1, 0x14B, 0x40);
	hmc7044_write_reg(&hmc7044_1, 0x14C, 0x6);
	hmc7044_write_reg(&hmc7044_1, 0x14D, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14E, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x14F, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x150, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x151, 0x0);
	hmc7044_write_reg(&hmc7044_1, 0x152, 0x11);
	usleep(100000);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x42);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	usleep(100000);
	// hmc7044_write_reg(&hmc7044_1, 0x1, 0x44);
	// hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);
	// usleep(100000);
	// hmc7044_write_reg(&hmc7044_1, 0x1, 0xc0);
	hmc7044_write_reg(&hmc7044_1, 0x1, 0x40);

}

void init_mem()
{
	axi_device_init(&ad9684, "AD9684", 0x80010000, 0x10000);
	axi_device_init(&plreg, "PL-REG", 0x80030000, 0x10000);

	g_frame_filter = frame_filter_create(30000000);
}

int init_clk(RefClk rc)
{
	return lmk04828_init(rc);
}




