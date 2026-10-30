#include "sooffsetcalib_mfllab.h"

#include <stdio.h>

typedef struct {
	double k1;
	double k2;
	double b;
} sooffset_mfilab_calib_t;

/* OUT1/OUT2: fixed +/-2 V peak range, one coefficient set per channel. */
static sooffset_mfilab_calib_t g_sooffset_mfilab_calib[2] = {
	{0.0, 1.0, 0.0},
	{0.0, 1.0, 0.0}
};

int init_sooffsetcalib_mfilab_param(void)
{
	FILE *fp;
	char line[256];
	int channel;
	double k1, k2, b;

	for(channel = 0; channel < 2; ++channel) {
		g_sooffset_mfilab_calib[channel] = (sooffset_mfilab_calib_t){0.0, 1.0, 0.0};
	}

	fp = fopen(SOOFFSET_CALIB_MFILAB_PATH, "r");
	if(!fp) {
		fp = fopen(SOOFFSET_CALIB_MFILAB_PATH, "w");
		if(!fp) {
			return -1;
		}
		fprintf(fp, "channel,K1,K2,B\n1,0,1,0\n2,0,1,0\n");
		fclose(fp);
		return 0;
	}

	/* CSV: channel,K1,K2,B; channel is 1 or 2. Calibration input/output are volts. */
	if(fgets(line, sizeof(line), fp)) {
		while(fgets(line, sizeof(line), fp)) {
			if(sscanf(line, " %d , %lf , %lf , %lf", &channel, &k1, &k2, &b) == 4 &&
			   channel >= 1 && channel <= 2) {
				g_sooffset_mfilab_calib[channel - 1] = (sooffset_mfilab_calib_t){k1, k2, b};
			}
		}
	}
	fclose(fp);
	return 0;
}

double sooffset_mfilab_calibrate_value(int channel, double value)
{
	const sooffset_mfilab_calib_t *calib;

	if(channel != 1 && channel != 2) {
		return value;
	}
	calib = &g_sooffset_mfilab_calib[channel - 1];
	return calib->k1 * value * value + calib->k2 * value + calib->b;
}
