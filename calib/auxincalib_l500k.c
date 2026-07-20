#include "auxincalib_l500k.h"

#include <stdio.h>

#define AUXIN_L500K_CHANNEL_COUNT 4

typedef struct {
	double k1;
	double k2;
	double b;
} auxin_l500k_calib_t;

static auxin_l500k_calib_t g_auxin_l500k_calib[AUXIN_L500K_CHANNEL_COUNT];

static void auxin_l500k_set_default(void)
{
	int i;

	for(i = 0; i < AUXIN_L500K_CHANNEL_COUNT; ++i) {
		g_auxin_l500k_calib[i].k1 = 0.0;
		g_auxin_l500k_calib[i].k2 = 1.0;
		g_auxin_l500k_calib[i].b = 0.0;
	}
}

static void auxin_l500k_write_default_csv(const char *path)
{
	FILE *fp = fopen(path, "w");
	int i;

	if(!fp) {
		return;
	}

	fprintf(fp, "CH,K1,K2,B\n");
	for(i = 0; i < AUXIN_L500K_CHANNEL_COUNT; ++i) {
		fprintf(fp, "%d,0,1,0\n", i + 1);
	}
	fclose(fp);
}

int init_auxincalib_l500k_param(void)
{
	FILE *fp;
	char line[256];
	int ch;
	double k1, k2, b;

	auxin_l500k_set_default();

	fp = fopen(AUXIN_CALIB_L500K_PATH, "r");
	if(!fp) {
		auxin_l500k_write_default_csv(AUXIN_CALIB_L500K_PATH);
		return 0;
	}

	if(!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return 0;
	}

	while(fgets(line, sizeof(line), fp)) {
		if(sscanf(line, " %d , %lf , %lf , %lf", &ch, &k1, &k2, &b) == 4 &&
		   ch >= 1 && ch <= AUXIN_L500K_CHANNEL_COUNT) {
			g_auxin_l500k_calib[ch - 1].k1 = k1;
			g_auxin_l500k_calib[ch - 1].k2 = k2;
			g_auxin_l500k_calib[ch - 1].b = b;
		}
	}

	fclose(fp);
	return 0;
}

double auxin_l500k_calibrate_value(unsigned channel_index, double value)
{
	if(channel_index >= AUXIN_L500K_CHANNEL_COUNT) {
		return value;
	}

	return g_auxin_l500k_calib[channel_index].k1 * (value * value) +
		   g_auxin_l500k_calib[channel_index].k2 * value +
		   g_auxin_l500k_calib[channel_index].b;
}
