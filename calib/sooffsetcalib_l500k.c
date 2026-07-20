#include "sooffsetcalib_l500k.h"

#include <stdio.h>

typedef struct {
	double k1;
	double k2;
	double b;
} sooffset_l500k_calib_t;

static sooffset_l500k_calib_t g_sooffset_l500k_ch1_calib[SOOFFSET_CALIB_L500K_RANGE_COUNT];
static sooffset_l500k_calib_t g_sooffset_l500k_ch2_calib[SOOFFSET_CALIB_L500K_RANGE_COUNT];

static void sooffset_l500k_set_default(sooffset_l500k_calib_t *table)
{
	int i;

	for(i = 0; i < SOOFFSET_CALIB_L500K_RANGE_COUNT; ++i) {
		table[i].k1 = 0.0;
		table[i].k2 = 1.0;
		table[i].b = 0.0;
	}
}

static void sooffset_l500k_write_default_csv(const char *path)
{
	FILE *fp = fopen(path, "w");
	int i;

	if(!fp) {
		return;
	}

	fprintf(fp, "range,K1,K2,B\n");
	for(i = 0; i < SOOFFSET_CALIB_L500K_RANGE_COUNT; ++i) {
		fprintf(fp, "%d,0,1,0\n", i);
	}
	fclose(fp);
}

static int sooffset_l500k_load_csv(const char *path, sooffset_l500k_calib_t *table)
{
	FILE *fp;
	char line[256];
	int range;
	double k1, k2, b;

	sooffset_l500k_set_default(table);

	fp = fopen(path, "r");
	if(!fp) {
		sooffset_l500k_write_default_csv(path);
		return 0;
	}

	if(!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return 0;
	}

	while(fgets(line, sizeof(line), fp)) {
		if(sscanf(line, " %d , %lf , %lf , %lf", &range, &k1, &k2, &b) == 4 &&
		   range >= 0 && range < SOOFFSET_CALIB_L500K_RANGE_COUNT) {
			table[range].k1 = k1;
			table[range].k2 = k2;
			table[range].b = b;
		}
	}

	fclose(fp);
	return 0;
}

int init_sooffsetcalib_l500k_param(void)
{
	int ret = 0;

	if(sooffset_l500k_load_csv(SOOFFSET_CALIB_L500K_CH1_PATH, g_sooffset_l500k_ch1_calib) != 0) {
		ret = -1;
	}
	if(sooffset_l500k_load_csv(SOOFFSET_CALIB_L500K_CH2_PATH, g_sooffset_l500k_ch2_calib) != 0) {
		ret = -1;
	}

	return ret;
}

double sooffset_l500k_calibrate_value(int channel, int range, double value)
{
	sooffset_l500k_calib_t *table = NULL;

	if(range < 0 || range >= SOOFFSET_CALIB_L500K_RANGE_COUNT) {
		return value;
	}
	if(range == SOOFFSET_CALIB_L500K_RANGE_COUNT - 1) {
		value /= 3.222;
	}
	if(channel == 1) {
		table = g_sooffset_l500k_ch1_calib;
	} else if(channel == 2) {
		table = g_sooffset_l500k_ch2_calib;
	} else {
		return value;
	}

	return table[range].k1 * (value * value) +
		   table[range].k2 * value +
		   table[range].b;
}
