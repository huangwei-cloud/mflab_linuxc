#include "sicalib.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "../utils/scpi-def.h"
#include "../pm/param.h"
#include "../ring/ringbuffer.h"
#include "sicalib_l500k.h"
#include "sicalib_m005m.h"



caldb_t g_dbch1, g_dbch2, g_dbcurch2;

/* --------- ���ߺ�����ȥ����β������հ� --------- */
static void rstrip(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n-1] == '\n' || s[n-1] == '\r' || isspace((unsigned char)s[n-1]))) {
        s[--n] = '\0';
    }
}

/* --------- ���� double --------- */
static int parse_double(const char *s, double *out)
{
    if (!s || !*s) return -1;
    char *end = NULL;
    double v = strtod(s, &end);
    if (s == end) return -1;  /* û�н����ɹ� */
    *out = v;
    return 0;
}

int caldb_load_csv(caldb_t *db, const char *csv_path)
{

    if (!db || !csv_path) return -1;

    /* ��ʼ�� */
    for (int i = 0; i < CALIB_MAX_IDS; ++i) {
        db->table[i].valid = 0;
        db->table[i].vouta = db->table[i].voutb = db->table[i].voutc = db->table[i].voutd = 0.0;
    }

    FILE *fp = fopen(csv_path, "r");
    if (!fp) return -2;


    char line[512];
    int line_no = 0;

    while (fgets(line, sizeof(line), fp)) {
        ++line_no;
        rstrip(line);
        if (line[0] == '\0') continue;           /* ���� */
        if (line[0] == '#')  continue;           /* ע���� */

        /* ������ͷ����һ�а��� id,vouta...�� */
        if (line_no == 1) {
            if (strstr(line, "id") || strstr(line, "ID")) continue;
        }

        /* ��� 5 ���ֶΣ�id,vouta,voutb,voutc,voutd */
        char *save = NULL;
        char *tok  = strtok_r(line, ",", &save);
        if (!tok) continue;

        /* ���� id */
        char *end = NULL;
        unsigned long id_ul = strtoul(tok, &end, 0);
        if (tok == end || id_ul >= CALIB_MAX_IDS) continue;
        unsigned id = (unsigned)id_ul;

        /* ���� vouta..voutd (double) */
        double vals[4] = {0.0};
        int ok = 1;
        for (int i = 0; i < 4; ++i) {
            tok = strtok_r(NULL, ",", &save);
            if (!tok || parse_double(tok, &vals[i]) != 0) { ok = 0; break; }
        }
        if (!ok) continue;

        db->table[id].vouta   = vals[0];
        db->table[id].voutb   = vals[1];
        db->table[id].voutc   = vals[2];
        db->table[id].voutd   = vals[3];
        db->table[id].valid = 1;
    }

    fclose(fp);
    return 0;
}

int caldb_get_by_id(const caldb_t *db, unsigned id, calib_params_t *out)
{
    if (!db || !out) return -1;
    if (id >= CALIB_MAX_IDS) return -1;
    if (!db->table[id].valid) return -1;
    *out = db->table[id];
    return 0;
}

/*****************************************************************************
 �� �� ��  : set_si_calibration
 ��������  : SI��ѹͨ��У׼���������õĶ���ֵ������ID������ID����У׼ֵ����
 �������  : int ch  
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2025��10��24��
    ��    ��   : huangwei
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_si_calibration(int ch)
{
	if (get_device_model() == DEVICE_MODEL_M005M) {
		set_si_m005m_calibration(ch);
		return;
	}
	if (get_device_model() == DEVICE_MODEL_L500K) {
		set_si_l500k_calibration(ch);
		return;
	}

	ParamValue getpv;
	unsigned in_mode, coupling,impedance,filter,range;
	unsigned id, v1_value, v2_value;
	calib_params_t cpt;
	
	if(ch == CH1) {
		get_param_by_name(PARAM_CURRENT,"si_volch1_mode", 1, &getpv);
		in_mode = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch1_couping", 1, &getpv);
		coupling = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch1_impedance", 1, &getpv);
		impedance = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch1_filter", 1, &getpv);
		filter = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch1_range", 1, &getpv);
		range = getpv.value.i32;

		printf("%d, %d, %d, %d, %d\r\n", in_mode, coupling,impedance,filter,range);
     	id = encode_id(in_mode, coupling,impedance,filter,range);
		printf("csv id: %d\r\n", id);
		caldb_get_by_id(&g_dbch1, id, &cpt);

		if(cpt.valid) {
			//���ﲢ��������DACͨ������Ҫ���ã����ݷ�Χ�ж����õ�ͨ������4����λֻ��Ҫ����C/D, ��4����λ�̶�����AB + C/D
			if((range == VOL_RANGE_1MV) || (range == VOL_RANGE_10MV)) {//����AB+C
				v1_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 0);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);
		
				v1_value = round(cpt.voutb / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 1);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);
		
				v1_value = round(cpt.voutc / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 2);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);
			} else if((range == VOL_RANGE_3MV) || (range == VOL_RANGE_30MV)) { //����AB+D
				v1_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 0);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);
		
				v1_value = round(cpt.voutb / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 1);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);

				v1_value = round(cpt.voutd / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 3);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);	
			} else if((range == VOL_RANGE_100MV) || (range == VOL_RANGE_1V)) { //����C
				v1_value = round(cpt.voutc / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 2);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);
			} else if((range == VOL_RANGE_300MV) || (range == VOL_RANGE_3V)) { //����D
				v1_value = round(cpt.voutd / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("v_trim_ch", 3);
				reg_write_by_name("v_trim", v1_value);
				reg_write_by_name("v_trim_en", 0);
				reg_write_by_name("v_trim_en", 1);
				reg_write_by_name("v_trim_en", 0);	
			}
		}
		
	}else if(ch == CH2) {
		get_param_by_name(PARAM_CURRENT,"si_volch2_mode", 1, &getpv);
		in_mode = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch2_couping", 1, &getpv);
		coupling = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch2_impedance", 1, &getpv);
		impedance = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch2_filter", 1, &getpv);
		filter = getpv.value.i32;
		get_param_by_name(PARAM_CURRENT,"si_volch2_range", 1, &getpv);
		range = getpv.value.i32;

     	id = encode_id(in_mode, coupling, impedance, filter,range);
		caldb_get_by_id(&g_dbch2, id, &cpt);

		if(cpt.valid) {

			if((range == VOL_RANGE_1MV) || (range == VOL_RANGE_10MV)) {//����AB+C
				v2_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 0);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);
		
				v2_value = round(cpt.voutb / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 1);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);
		
				v2_value = round(cpt.voutc / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 2);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);
			} else if((range == VOL_RANGE_3MV) || (range == VOL_RANGE_30MV)) { //����AB+D
				v2_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 0);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);
		
				v2_value = round(cpt.voutb / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 1);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);
		
				v2_value = round(cpt.voutd / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 3);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);

			} else if((range == VOL_RANGE_100MV) || (range == VOL_RANGE_1V)) { //����C
				v2_value = round(cpt.voutc / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 2);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);

			} else if((range == VOL_RANGE_300MV) || (range == VOL_RANGE_3V)) { //����D
				v2_value = round(cpt.voutd / 2.5 * (pow(2, 16) - 1));
				reg_write_by_name("i_trim_ch", 3);
				reg_write_by_name("i_trim", v2_value);
				reg_write_by_name("i_trim_en", 0);
				reg_write_by_name("i_trim_en", 1);
				reg_write_by_name("i_trim_en", 0);

			}	
		}
	} else {

	}
}

/*****************************************************************************
 �� �� ��  : set_si_cur_calibration
 ��������  : SI����ͨ��У׼������ͨ��Ĭ��ֻ��һ��ͨ��(CH2)
 �������  : id (0->7)�ܹ�8�����
 �������  : ��
 �� �� ֵ  : void
 ���ú���  : 
 ��������  : 
 
 �޸���ʷ      :
  1.��    ��   : 2025��10��24��
    ��    ��   : huangwei
    �޸�����   : �����ɺ���

*****************************************************************************/
void set_si_cur_calibration(int id)
{
	calib_params_t cpt;
	ParamValue getpv;
	unsigned int range, i2_value;

	if (id > VOL_RANGE_10MA) {
		printf("error: cur id is over range\r\n");
		return;
	}
	
	caldb_get_by_id(&g_dbcurch2, id, &cpt);

	get_param_by_name(PARAM_CURRENT,"si_curch_range", 0, &getpv);
	range = getpv.value.i32;

	if(cpt.valid) {

		if( (range == VOL_RANGE_1NA) || (range == VOL_RANGE_100NA) || (range == VOL_RANGE_10UA)
			|| (range == VOL_RANGE_1MA)) {
			i2_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("i_trim_ch", 0);
			reg_write_by_name("i_trim", i2_value);
			reg_write_by_name("i_trim_en", 0);
			reg_write_by_name("i_trim_en", 1);
			reg_write_by_name("i_trim_en", 0);
	
			i2_value = round(cpt.voutb / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("i_trim_ch", 1);
			reg_write_by_name("i_trim", i2_value);
			reg_write_by_name("i_trim_en", 0);
			reg_write_by_name("i_trim_en", 1);
			reg_write_by_name("i_trim_en", 0);
		} else { 
			i2_value = round(cpt.vouta / 2.5 * (pow(2, 16) - 1));
			reg_write_by_name("i_trim_ch", 0);
			reg_write_by_name("i_trim", i2_value);
			reg_write_by_name("i_trim_en", 0);
			reg_write_by_name("i_trim_en", 1);
			reg_write_by_name("i_trim_en", 0);
		}
	}
}

int init_sicalib_param()
{
	if (get_device_model() == DEVICE_MODEL_M005M) {
		return init_sicalib_m005m_param();
	}
	if (get_device_model() == DEVICE_MODEL_L500K) {
		return init_sicalib_l500k_param();
	}

	if (caldb_load_csv(&g_dbch1, SI_CALIB_VVVI_VOLCH1_PATH) != 0) {
        printf("load %s failed\n", SI_CALIB_VVVI_VOLCH1_PATH);
        return -1;
    }

	if (caldb_load_csv(&g_dbch2, SI_CALIB_VVVI_VOLCH2_PATH) != 0) {
	printf("load %s failed\n", SI_CALIB_VVVI_VOLCH2_PATH);
    	return -1;
	}

	if (caldb_load_csv(&g_dbcurch2, SI_CALIB_VVVI_CURCH2_PATH) != 0) {
	printf("load %s failed\n", SI_CALIB_VVVI_CURCH2_PATH);
    	return -1;
	}
	
	return 0;
}


