#include "sooutputquant_l500k.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static const double g_sooutput_l500k_default_quant[SOOUTPUT_QUANT_L500K_RANGE_COUNT] = {
	0.624998536,
	0.816338008,
	0.847855873,
	0.866632121,
	0.996889183
};

static const double g_sooutput_l500k_default_fast_quant = 0.934434798;

static double g_sooutput_l500k_ch1_quant[SOOUTPUT_QUANT_L500K_RANGE_COUNT];
static double g_sooutput_l500k_ch2_quant[SOOUTPUT_QUANT_L500K_RANGE_COUNT];
static double g_sooutput_l500k_ch1_fast_quant;
static double g_sooutput_l500k_ch2_fast_quant;

static void sooutput_l500k_set_default(double *table, double *fast_quant)
{
	int i;

	for(i = 0; i < SOOUTPUT_QUANT_L500K_RANGE_COUNT; ++i) {
		table[i] = g_sooutput_l500k_default_quant[i];
	}
	*fast_quant = g_sooutput_l500k_default_fast_quant;
}

static void sooutput_l500k_write_default_csv(const char *path)
{
	FILE *fp = fopen(path, "w");
	int i;

	if(!fp) {
		return;
	}

	fprintf(fp, "item,value\n");
	for(i = 0; i < SOOUTPUT_QUANT_L500K_RANGE_COUNT; ++i) {
		fprintf(fp, "SINE%d,%.9f\n", i, g_sooutput_l500k_default_quant[i]);
	}
	fprintf(fp, "FAST,%.9f\n", g_sooutput_l500k_default_fast_quant);
	fclose(fp);
}

static void trim_token(char *s)
{
	char *p = s;
	size_t len;

	while(*p && isspace((unsigned char)*p)) {
		++p;
	}
	if(p != s) {
		memmove(s, p, strlen(p) + 1);
	}

	len = strlen(s);
	while(len > 0 && isspace((unsigned char)s[len - 1])) {
		s[--len] = '\0';
	}
}

static int sooutput_l500k_parse_item(const char *item)
{
	int range;

	if(!item) {
		return -1;
	}

	if(sscanf(item, "%d", &range) == 1) {
		return range;
	}
	if(strncasecmp(item, "SINE", 4) == 0 && sscanf(item + 4, "%d", &range) == 1) {
		return range;
	}
	if(strcasecmp(item, "FAST") == 0) {
		return SOOUTPUT_QUANT_L500K_RANGE_COUNT;
	}

	return -1;
}

static int sooutput_l500k_load_csv(const char *path, double *table, double *fast_quant)
{
	FILE *fp;
	char line[256];
	char item[64];
	int index;
	double value;

	sooutput_l500k_set_default(table, fast_quant);

	fp = fopen(path, "r");
	if(!fp) {
		sooutput_l500k_write_default_csv(path);
		return 0;
	}

	if(!fgets(line, sizeof(line), fp)) {
		fclose(fp);
		return 0;
	}

	while(fgets(line, sizeof(line), fp)) {
		if(sscanf(line, " %63[^,] , %lf", item, &value) == 2) {
			trim_token(item);
			index = sooutput_l500k_parse_item(item);
			if(index >= 0 && index < SOOUTPUT_QUANT_L500K_RANGE_COUNT) {
				table[index] = value;
			} else if(index == SOOUTPUT_QUANT_L500K_RANGE_COUNT) {
				*fast_quant = value;
			}
		}
	}

	fclose(fp);
	return 0;
}

int init_sooutputquant_l500k_param(void)
{
	int ret = 0;

	if(sooutput_l500k_load_csv(SOOUTPUT_QUANT_L500K_CH1_PATH,
							   g_sooutput_l500k_ch1_quant,
							   &g_sooutput_l500k_ch1_fast_quant) != 0) {
		ret = -1;
	}
	if(sooutput_l500k_load_csv(SOOUTPUT_QUANT_L500K_CH2_PATH,
							   g_sooutput_l500k_ch2_quant,
							   &g_sooutput_l500k_ch2_fast_quant) != 0) {
		ret = -1;
	}

	return ret;
}

double sooutput_l500k_get_sine_quant(int channel, int range)
{
	if(range < 0 || range >= SOOUTPUT_QUANT_L500K_RANGE_COUNT) {
		return 1.0;
	}

	if(channel == 1) {
		return g_sooutput_l500k_ch1_quant[range];
	}
	if(channel == 2) {
		return g_sooutput_l500k_ch2_quant[range];
	}

	return 1.0;
}

double sooutput_l500k_get_fast_quant(int channel)
{
	if(channel == 1) {
		return g_sooutput_l500k_ch1_fast_quant;
	}
	if(channel == 2) {
		return g_sooutput_l500k_ch2_fast_quant;
	}

	return 1.0;
}
