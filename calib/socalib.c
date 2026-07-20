#include "socalib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "../utils/scpi-def.h"
#include "../pm/param.h"

typedef enum _SOVOL_RANGE {
	RANGE_10MV = 0,
	RANGE_100MV,
	RANGE_1V,
	RANGE_2V,
	RANGE_10V,
}SoVolRange;

socaldb_t g_sodbch1, g_sodbch2;

void get_socaldb1_instance(socaldb_t **sdb)
{
	assert(sdb != NULL);

	*sdb = &g_sodbch1;
}

void get_socaldb2_instance(socaldb_t **sdb)
{
	assert(sdb != NULL);

	*sdb = &g_sodbch2;
}

static void rstrip(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n-1]=='\n' || s[n-1]=='\r' || isspace((unsigned char)s[n-1]))) s[--n] = '\0';
}
static int parse_double(const char *s, double *out)
{
    if (!s || !*s) return -1;
    char *end = NULL;
    double v = strtod(s, &end);
    if (s == end) return -1;
    *out = v; return 0;
}

int socaldb_load_csv(socaldb_t *db, const char *csv_path)
{
    if (!db || !csv_path) return -1;
    for (int i=0;i<CH_TABLE_SIZE;i++) {
		db->table[i].valid = 0;
		db->table[i].vouta = db->table[i].voutb = db->table[i].voutc = db->table[i].voutd = 0.0;
    }

    FILE *fp = fopen(csv_path, "r");
    if (!fp) return -2;

    char line[512]; int line_no=0;
    while (fgets(line, sizeof(line), fp)) {
        ++line_no; rstrip(line);
        if (!line[0] || line[0]=='#') continue;
        if (line_no==1) { if (strstr(line,"id")) continue; }

        char *save=NULL;
        char *tok = strtok_r(line, ",", &save);
        if (!tok) continue;

        char *end=NULL;
        unsigned long id_ul = strtoul(tok, &end, 0);
        if (tok==end || id_ul >= CH_TABLE_SIZE) continue;
        unsigned id = (unsigned)id_ul;

        double vals[4] = {0.0};
        int ok = 1;
        for (int i = 0; i < 4; ++i) {
            tok = strtok_r(NULL, ",", &save);
            if (!tok || parse_double(tok, &vals[i]) != 0) { ok = 0; break; }
        }
        if (!ok) continue;

        db->table[id].vouta = vals[0];
        db->table[id].voutb = vals[1];
        db->table[id].voutc = vals[2];
        db->table[id].voutd = vals[3];
        db->table[id].valid = 1;

    }

    fclose(fp);
    return 0;
}

int socaldb_get_by_id(const socaldb_t *db, unsigned id, ch_row_t *out)
{
    if (!db || !out) return -1;
    if (id >= CH_TABLE_SIZE) return -1;
    if (!db->table[id].valid) return -1;
    *out = db->table[id];
    return 0;
}

void set_so_calibration(int ch, int id)
{
	unsigned so_value;
	ch_row_t row_t;

	if(ch == CH1) {
		socaldb_get_by_id(&g_sodbch1, id, &row_t);
		printf("%d,%f,%f,%f,%f\r\n",id,row_t.vouta,row_t.voutb,row_t.voutc,row_t.voutd);
		if(id <= ID_FASTX_TTL) {
			so_value = round( row_t.voutb / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("signalout_trim_ch", 1);
			reg_write_by_name("signalout_trim", so_value);
			reg_write_by_name("signalout_trim_en", 0);
			reg_write_by_name("signalout_trim_en", 1);
			reg_write_by_name("signalout_trim_en", 0);
		} else {
			so_value = round( row_t.vouta / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("signalout_trim_ch", 0);
			reg_write_by_name("signalout_trim", so_value);
			reg_write_by_name("signalout_trim_en", 0);
			reg_write_by_name("signalout_trim_en", 1);
			reg_write_by_name("signalout_trim_en", 0);
		}
	}else if(ch == CH2) {
		socaldb_get_by_id(&g_sodbch2, id, &row_t);
		printf("%d,%f,%f,%f,%f\r\n",id,row_t.vouta,row_t.voutb,row_t.voutc,row_t.voutd);
		if(id <= ID_FASTX_TTL) {
			so_value = round( row_t.voutd / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("signalout_trim_ch", 3);
			reg_write_by_name("signalout_trim", so_value);
			reg_write_by_name("signalout_trim_en", 0);
			reg_write_by_name("signalout_trim_en", 1);
			reg_write_by_name("signalout_trim_en", 0);
		} else {
			so_value = round( row_t.voutc / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("signalout_trim_ch", 2);
			reg_write_by_name("signalout_trim", so_value);
			reg_write_by_name("signalout_trim_en", 0);
			reg_write_by_name("signalout_trim_en", 1);
			reg_write_by_name("signalout_trim_en", 0);
		}
	} else {

	}
 }

static int load_socalib_pair(const char *ch1_path, const char *ch2_path)
{
	if (socaldb_load_csv(&g_sodbch1, ch1_path) != 0) {
        printf("load %s failed\n", ch1_path);
        return -1;
    }

	if (socaldb_load_csv(&g_sodbch2, ch2_path) != 0) {
        printf("load %s failed\n", ch2_path);
        return -1;
    }

	return 0;
}

int init_socalib_param()
{
	int model = get_device_model();

	if ((model == DEVICE_MODEL_M005M_VV) || (model == DEVICE_MODEL_M005M_VI)) {
		return load_socalib_pair(SO_CALIB_VVVI_CH1_PATH, SO_CALIB_VVVI_CH2_PATH);
	} else if (model == DEVICE_MODEL_M005M) {
		return load_socalib_pair(SO_CALIB_M005M_CH1_PATH, SO_CALIB_M005M_CH2_PATH);
	} else if (model == DEVICE_MODEL_L500K) {
		return load_socalib_pair(SO_CALIB_L500K_CH1_PATH, SO_CALIB_L500K_CH2_PATH);
	}

	return load_socalib_pair(SO_CALIB_VVVI_CH1_PATH, SO_CALIB_VVVI_CH2_PATH);
}
