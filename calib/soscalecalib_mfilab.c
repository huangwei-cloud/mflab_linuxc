#include "soscalecalib_mfilab.h"

#include <math.h>
#include <stdio.h>

/* OUT1/OUT2: fixed +/-2 V peak range, one gain coefficient per channel. */
static double g_soscale_mfilab_calib[2] = {1.0, 1.0};

int init_soscalecalib_mfilab_param(void)
{
	FILE *fp;
	char line[256];
	int channel;
	double coefficient;

	g_soscale_mfilab_calib[0] = 1.0;
	g_soscale_mfilab_calib[1] = 1.0;
	fp = fopen(SOSCALE_CALIB_MFILAB_PATH, "r");
	if(!fp) {
		fp = fopen(SOSCALE_CALIB_MFILAB_PATH, "w");
		if(!fp) {
			return -1;
		}
		fprintf(fp, "channel,coefficient\n1,1\n2,1\n");
		fclose(fp);
		return 0;
	}

	/* coefficient = expected amplitude / measured amplitude at Scale = 1. */
	if(fgets(line, sizeof(line), fp)) {
		while(fgets(line, sizeof(line), fp)) {
			if(sscanf(line, " %d , %lf", &channel, &coefficient) == 2 &&
			   channel >= 1 && channel <= 2 && isfinite(coefficient) && coefficient > 0.0) {
				g_soscale_mfilab_calib[channel - 1] = coefficient;
			}
		}
	}
	fclose(fp);
	return 0;
}

double soscale_mfilab_calibrate_value(int channel, double value)
{
	if(channel != 1 && channel != 2) {
		return value;
	}
	return value * g_soscale_mfilab_calib[channel - 1];
}
