#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "adc4080.h"
#include "../axispi/spi.h"
#include "../clk/hmc7043.h"
#include "../pm/param.h"


#define AD4080_DEV_MAX	 (6)
#define AD4080_WRITE (0)
#define AD4080_READ  (1)


spi_t* adspidev[AD4080_DEV_MAX] = {NULL};

static int ad4080_axi_write_reg(AxiDevice *dev, uint16_t addr, uint8_t data)
{
	if (!dev || !dev->reg_base) return -1;

	dev->reg_base[0] = 0x00000001;
	dev->reg_base[1] = ((uint32_t)addr << 8) | data;
	dev->reg_base[0] = 0x00000003;
	usleep(1000);

	return 0;
}

static uint32_t ad4080_axi_read_reg(AxiDevice *dev, uint16_t addr)
{
	uint32_t data;

	if (!dev || !dev->reg_base) return 0xFFFFFFFF;

	dev->reg_base[0] = 0x00000001;
	dev->reg_base[1] = 0x00800000 | ((uint32_t)addr << 8);
	dev->reg_base[0] = 0x00000003;
	usleep(100);

	data = dev->reg_base[2];
	printf("[%s] read reg addr=0x%04x data=0x%04x\r\n", dev->name, addr, data);

	return data;
}

static int ad4080_axi_init_one(AxiDevice *dev)
{
	if (!dev || !dev->reg_base) return -1;

	/* Clear readback/status register before the ADC soft reset sequence. */
	dev->reg_base[2] = 0x00000000;

	/* Reset pulse: hold reset, release it, then wait for the ADC to be ready. */
	dev->reg_base[0] = 0x00000001;
	usleep(5000);
	dev->reg_base[0] = 0x00000000;
	usleep(100);
	dev->reg_base[0] = 0x00000001;
	usleep(5000);

	/* Enter configuration mode and read the chip ID/status for debug visibility. */
	ad4080_axi_read_reg(dev, 0x3FFF);
	ad4080_axi_read_reg(dev, 0x000C);

	/* Apply the AD4080/AD4630-family configuration, then leave configuration mode. */
	ad4080_axi_write_reg(dev, 0x0015, 0x00);
	ad4080_axi_write_reg(dev, 0x0020, 0x93);
	ad4080_axi_read_reg(dev, 0x0020);
	ad4080_axi_write_reg(dev, 0x0014, 0x01);

	return 0;
}

static int init_adc4080_axi(void)
{
	AxiDevice *ad4080_1 = NULL;
	AxiDevice *ad4080_2 = NULL;

	get_ad4080_1_intance(&ad4080_1);
	get_ad4080_2_intance(&ad4080_2);

	if (ad4080_axi_init_one(ad4080_1) != 0) {
		return -1;
	}
	if (ad4080_axi_init_one(ad4080_2) != 0) {
		return -1;
	}

	return 0;
}

uint8_t ad4080_spi_transfer(spi_t *spidev, uint32_t direct, uint16_t addr, uint8_t value)
{
	uint8_t txbuf[3];
	uint8_t rxbuf[3];
	if(direct == AD4080_WRITE) {
		txbuf[0] = addr >> 8;
		txbuf[1] = addr & 0xFF;
		txbuf[2] = value;
		if (spi_transfer(spidev, txbuf, rxbuf, sizeof(txbuf)) < 0) {
			fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spidev));
			return -1;
		}
	}else if(direct == AD4080_READ) {
		txbuf[0] = (addr | 0x8000) >> 8;
		txbuf[1] = addr & 0xFF;
		txbuf[2] = value;
		if (spi_transfer(spidev, txbuf, rxbuf, sizeof(txbuf)) < 0) {
			fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spidev));
			return -1;
		}
	}else {

	}
	
	return rxbuf[2];
}


static int init_adc4080_spi(void)
{
	int i;
	char buff[20] = {0};
	uint8_t id;
	

	for(i = 0; i < AD4080_DEV_MAX; i++) {
		adspidev[i] = spi_new();

		sprintf(buff, "/dev/spidev%d.0", i+2);//2->7
		if (spi_open(adspidev[i], buff, 0, 1000000) < 0) {
			fprintf(stderr, "spi_open(): %s\n", spi_errmsg(adspidev[i]));
			return -1;
		}
		
		//reg0 = ad4080_spi_transfer(adspidev[i], AD4080_READ, 0x00, 0x00);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0, 0x81);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0, 0x10);

		//sleep(2);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0x0A, 0x55);

		id = ad4080_spi_transfer(adspidev[i], AD4080_READ, 0x04, 0x00);
		if(id != 0x50) {
			printf("[%s]-[%d]: spi dev id error\r\n", __FUNCTION__, __LINE__);
			return -1;
		}
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0x02, 0x00);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0x15, 0x44);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0x16, 0x21);
		ad4080_spi_transfer(adspidev[i], AD4080_WRITE, 0x17, 0x20);

		spi_free(adspidev[i]);
	}

	return 0;
}

int init_adc4080()
{
	if (get_device_model() == DEVICE_MODEL_L500K) {
		return init_adc4080_axi();
	}

	return init_adc4080_spi();
}





