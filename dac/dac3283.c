#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "../axispi/spi.h"
#include "../pm/param.h"
#include "dac3283.h"


#define DA3283_DEV_MAX	 (2)
#define DA3283_WRITE (0)
#define DA3283_READ  (1)

spi_t* daspidev[DA3283_DEV_MAX] = {NULL};

static const char *dac3283_get_spidev_path(int index)
{
	int model = get_device_model();

	if (model == DEVICE_MODEL_L500K) {
		return (index == 0) ? "/dev/spidev1.0" : "/dev/spidev2.0";
	}

	return (index == 0) ? "/dev/spidev8.0" : "/dev/spidev1.0";
}

uint8_t da3283_spi_transfer(spi_t *spidev, uint32_t direct, uint16_t addr, uint8_t value)
{
	uint8_t txbuf[2];
	uint8_t rxbuf[2];
	if(direct == DA3283_WRITE) {
		txbuf[0] = addr & 0xFF;
		txbuf[1] = value;
		if (spi_transfer(spidev, txbuf, rxbuf, sizeof(txbuf)) < 0) {
			fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spidev));
			return -1;
		}
	}else if(direct == DA3283_READ) {
		txbuf[0] = addr | 0x80;
		txbuf[1] = 0;
		if (spi_transfer(spidev, txbuf, rxbuf, sizeof(txbuf)) < 0) {
			fprintf(stderr, "spi_transfer(): %s\n", spi_errmsg(spidev));
			return -1;
		}
	}else {

	}
	
	return rxbuf[1];
}


int init_dac3283()
{
	int i;
	char buff[20] = {0};
	uint8_t id;

	for(i = 0; i < DA3283_DEV_MAX; i++) {
		daspidev[i] = spi_new();
		snprintf(buff, sizeof(buff), "%s", dac3283_get_spidev_path(i));
		
		if (spi_open(daspidev[i], buff, 0, 1000000) < 0) {
			fprintf(stderr, "spi_open(): %s\n", spi_errmsg(daspidev[i]));
			return -1;
		}
		
		da3283_spi_transfer(daspidev[i], DA3283_WRITE, 0x17, 0x04);

		id = da3283_spi_transfer(daspidev[i], DA3283_READ, 0x1f, 0x00);
		if( (id & 0x3F) != 0x12) {
			printf("[%s]-[%d]: spi dev id error\r\n", __FUNCTION__, __LINE__);
			return -1;
		}
		da3283_spi_transfer(daspidev[i], DA3283_WRITE, 0x00, 0x70);
		da3283_spi_transfer(daspidev[i], DA3283_WRITE, 0x01, 0x31);
		da3283_spi_transfer(daspidev[i], DA3283_WRITE, 0x13, 0x00);
		da3283_spi_transfer(daspidev[i], DA3283_WRITE, 0x12, 0x00);

		spi_free(daspidev[i]);
	}
	
	return 0;
}




