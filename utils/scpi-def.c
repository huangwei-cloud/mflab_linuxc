/*-
 * BSD 2-Clause License
 *
 * Copyright (c) 2012-2018, Jan Breuer
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file   scpi-def.c
 * @date   Thu Nov 15 10:58:45 UTC 2012
 *
 * @brief  SCPI parser test
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <inttypes.h>
#include "scpi.h"
#include "scpi-def.h"
#include "utils.h"
#include "../clk/hmc7043.h"
#include "../utils/utils.h"
#include "../pm/param.h"
#include "../clk/reg.h"
#include "../axigpio/gpio.h"
#include "../calib/sicalib.h"
#include "../calib/sicalib_l500k.h"
#include "../calib/sicalib_m005m.h"
#include "../calib/socalib.h"
#include "../calib/auxoutcalib_l500k.h"
#include "../calib/choutcalib_l500k.h"
#include "../calib/sooutputquant_l500k.h"
#include "../calib/sooffsetcalib_l500k.h"
#include "../sweep/sweep.h"
#include "../awg/awg.h"
#include "../clk/regctrl.h"
#include "../task/datatask.h"
#include "../log/log.h"

gLobalParam g_param;
int g_restart_config = 0;/*default param config*/
extern g_running;
extern int g_device_model;
extern awg_ctx_t g_awg_ctx;

double zwdx_oscillator_ref_clock_hz(void)
{
	//M005M REF OSC 40MHz, L500K REF OSC 2MHz
	return (get_device_model() == DEVICE_MODEL_L500K) ? 2000000.0 : 40000000.0;
}

/*Signal Output - Sineout Verify*/
double g_so_sineout_verify[5] = {0.624998536, 0.816338008, 0.847855873, 0.866632121, 0.996889183};//5����Χ5��ֵ
//double g_so_sineout_verify[5] = {1, 1, 1, 1, 1};//5����Χ5��ֵ


/*Signal Output - FastX Verify*/
double g_so_fastx_verify = 0.934434798;

/*Aux Output - Verify*/
double g_ao_verify = 0.952380952;

/*test*/
#define TEST
int g_daq_start = 0;//3:start 2:stop-->display 5:start 4:stop -->DAQ, 7:start 6:stop -->SCOPE

/*****************************Diff[0]***GND[1]***AC[2]***100[3]**50[4]*Scale1[5]*Scale2[6]*Scale3[7]**Range[8]**/
static char si_default_config[] = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x05};

void get_global_param(gLobalParam ** glp)
{
	assert(glp != NULL);

	*glp = &g_param;
}

void pack_registers(SoRangeWaveFormat *fmt, uint8_t regs[3])
{
    // ��1���Ĵ���: CH1 ����[��4λ] + ��Χ[��4λ]
    regs[0] = ((fmt->ch1_wave_format & 0x0F) << 4) |
              (fmt->ch1_range & 0x0F);

    // ��2���Ĵ���: CH2 ����[��4λ] + ��Χ[��4λ]
    regs[1] = ((fmt->ch2_wave_format & 0x0F) << 4) |
              (fmt->ch2_range & 0x0F);

    // ��3���Ĵ���: bit[0:1]=CH1_RANGE2, bit[2:3]=CH2_RANGE2
    regs[2] = ((fmt->ch2_range2 & 0x03) << 2) |
              (fmt->ch1_range2 & 0x03);

	printf("Reg1 = 0x%02X\n", regs[0]);
    printf("Reg2 = 0x%02X\n", regs[1]);
    printf("Reg3 = 0x%02X\n", regs[2]);
}


static scpi_result_t DMM_MeasureVoltageDcQ(scpi_t * context) {
    scpi_number_t param1, param2;
    char bf[15];
    fprintf(stderr, "meas:volt:dc\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param1, FALSE)) {
        /* do something, if parameter not present */
    }

    /* read second paraeter if present */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param2, FALSE)) {
        /* do something, if parameter not present */
    }


    SCPI_NumberToStr(context, scpi_special_numbers_def, &param1, bf, 15);
    fprintf(stderr, "\tP1=%s\r\n", bf);


    SCPI_NumberToStr(context, scpi_special_numbers_def, &param2, bf, 15);
    fprintf(stderr, "\tP2=%s\r\n", bf);

    SCPI_ResultDouble(context, 0);

    return SCPI_RES_OK;
}

static scpi_result_t DMM_MeasureVoltageAcQ(scpi_t * context) {
    scpi_number_t param1, param2;
    char bf[15];
    fprintf(stderr, "meas:volt:ac\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param1, FALSE)) {
        /* do something, if parameter not present */
    }

    /* read second paraeter if present */
    if (!SCPI_ParamNumber(context, scpi_special_numbers_def, &param2, FALSE)) {
        /* do something, if parameter not present */
    }


    SCPI_NumberToStr(context, scpi_special_numbers_def, &param1, bf, 15);
    fprintf(stderr, "\tP1=%s\r\n", bf);


    SCPI_NumberToStr(context, scpi_special_numbers_def, &param2, bf, 15);
    fprintf(stderr, "\tP2=%s\r\n", bf);

    SCPI_ResultDouble(context, 0);

    return SCPI_RES_OK;
}

static scpi_result_t DMM_ConfigureVoltageDc(scpi_t * context) {
    double param1, param2;
    fprintf(stderr, "conf:volt:dc\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamDouble(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    /* read second paraeter if present */
    if (!SCPI_ParamDouble(context, &param2, FALSE)) {
        /* do something, if parameter not present */
    }

    fprintf(stderr, "\tP1=%lf\r\n", param1);
    fprintf(stderr, "\tP2=%lf\r\n", param2);

    return SCPI_RES_OK;
}

static scpi_result_t TEST_Bool(scpi_t * context) {
    scpi_bool_t param1;
    fprintf(stderr, "TEST:BOOL\r\n"); /* debug command name */

    /* read first parameter if present */
    if (!SCPI_ParamBool(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    fprintf(stderr, "\tP1=%d\r\n", param1);

    return SCPI_RES_OK;
}

scpi_choice_def_t trigger_source[] = {
    {"BUS", 5},
    {"IMMediate", 6},
    {"EXTernal", 7},
    SCPI_CHOICE_LIST_END /* termination of option list */
};

static scpi_result_t TEST_ChoiceQ(scpi_t * context) {

    int32_t param;
    const char * name;

    if (!SCPI_ParamChoice(context, trigger_source, &param, TRUE)) {
        return SCPI_RES_ERR;
    }

    SCPI_ChoiceToName(trigger_source, param, &name);
    fprintf(stderr, "\tP1=%s (%ld)\r\n", name, (long int) param);

    SCPI_ResultInt32(context, param);

    return SCPI_RES_OK;
}

static scpi_result_t TEST_Numbers(scpi_t * context) {
    int32_t numbers[2];

    SCPI_CommandNumbers(context, numbers, 2, 1);

    fprintf(stderr, "TEST numbers %d %d\r\n", numbers[0], numbers[1]);

    return SCPI_RES_OK;
}

static scpi_result_t TEST_Text(scpi_t * context) {
    char buffer[100];
    size_t copy_len;

    if (!SCPI_ParamCopyText(context, buffer, sizeof (buffer), &copy_len, FALSE)) {
        buffer[0] = '\0';
    }

    fprintf(stderr, "TEXT: ***%s***\r\n", buffer);

    return SCPI_RES_OK;
}

static scpi_result_t TEST_ArbQ(scpi_t * context) {
    const char * data;
    size_t len;

    if (SCPI_ParamArbitraryBlock(context, &data, &len, FALSE)) {
        SCPI_ResultArbitraryBlock(context, data, len);
    }

    return SCPI_RES_OK;
}

struct _scpi_channel_value_t {
    int32_t row;
    int32_t col;
};
typedef struct _scpi_channel_value_t scpi_channel_value_t;

/**
 * @brief
 * parses lists
 * channel numbers > 0.
 * no checks yet.
 * valid: (@1), (@3!1:1!3), ...
 * (@1!1:3!2) would be 1!1, 1!2, 2!1, 2!2, 3!1, 3!2.
 * (@3!1:1!3) would be 3!1, 3!2, 3!3, 2!1, 2!2, 2!3, ... 1!3.
 *
 * @param channel_list channel list, compare to SCPI99 Vol 1 Ch. 8.3.2
 */
static scpi_result_t TEST_Chanlst(scpi_t *context) {
    scpi_parameter_t channel_list_param;
#define MAXROW 2    /* maximum number of rows */
#define MAXCOL 6    /* maximum number of columns */
#define MAXDIM 2    /* maximum number of dimensions */
    scpi_channel_value_t array[MAXROW * MAXCOL]; /* array which holds values in order (2D) */
    size_t chanlst_idx; /* index for channel list */
    size_t arr_idx = 0; /* index for array */
    size_t n, m = 1; /* counters for row (n) and columns (m) */

    /* get channel list */
    if (SCPI_Parameter(context, &channel_list_param, TRUE)) {
        scpi_expr_result_t res;
        scpi_bool_t is_range;
        int32_t values_from[MAXDIM];
        int32_t values_to[MAXDIM];
        size_t dimensions;

        bool for_stop_row = FALSE; /* true if iteration for rows has to stop */
        bool for_stop_col = FALSE; /* true if iteration for columns has to stop */
        int32_t dir_row = 1; /* direction of counter for rows, +/-1 */
        int32_t dir_col = 1; /* direction of counter for columns, +/-1 */

        /* the next statement is valid usage and it gets only real number of dimensions for the first item (index 0) */
        if (!SCPI_ExprChannelListEntry(context, &channel_list_param, 0, &is_range, NULL, NULL, 0, &dimensions)) {
            chanlst_idx = 0; /* call first index */
            arr_idx = 0; /* set arr_idx to 0 */
            do { /* if valid, iterate over channel_list_param index while res == valid (do-while cause we have to do it once) */
                res = SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions);
                if (is_range == FALSE) { /* still can have multiple dimensions */
                    if (dimensions == 1) {
                        /* here we have our values
                         * row == values_from[0]
                         * col == 0 (fixed number)
                         * call a function or something */
                        array[arr_idx].row = values_from[0];
                        array[arr_idx].col = 0;
                    } else if (dimensions == 2) {
                        /* here we have our values
                         * row == values_fom[0]
                         * col == values_from[1]
                         * call a function or something */
                        array[arr_idx].row = values_from[0];
                        array[arr_idx].col = values_from[1];
                    } else {
                        return SCPI_RES_ERR;
                    }
                    arr_idx++; /* inkrement array where we want to save our values to, not neccessary otherwise */
                    if (arr_idx >= MAXROW * MAXCOL) {
                        return SCPI_RES_ERR;
                    }
                } else if (is_range == TRUE) {
                    if (values_from[0] > values_to[0]) {
                        dir_row = -1; /* we have to decrement from values_from */
                    } else { /* if (values_from[0] < values_to[0]) */
                        dir_row = +1; /* default, we increment from values_from */
                    }

                    /* iterating over rows, do it once -> set for_stop_row = false
                     * needed if there is channel list index isn't at end yet */
                    for_stop_row = FALSE;
                    for (n = values_from[0]; for_stop_row == FALSE; n += dir_row) {
                        /* usual case for ranges, 2 dimensions */
                        if (dimensions == 2) {
                            if (values_from[1] > values_to[1]) {
                                dir_col = -1;
                            } else if (values_from[1] < values_to[1]) {
                                dir_col = +1;
                            }
                            /* iterating over columns, do it at least once -> set for_stop_col = false
                             * needed if there is channel list index isn't at end yet */
                            for_stop_col = FALSE;
                            for (m = values_from[1]; for_stop_col == FALSE; m += dir_col) {
                                /* here we have our values
                                 * row == n
                                 * col == m
                                 * call a function or something */
                                array[arr_idx].row = n;
                                array[arr_idx].col = m;
                                arr_idx++;
                                if (arr_idx >= MAXROW * MAXCOL) {
                                    return SCPI_RES_ERR;
                                }
                                if (m == (size_t)values_to[1]) {
                                    /* endpoint reached, stop column for-loop */
                                    for_stop_col = TRUE;
                                }
                            }
                            /* special case for range, example: (@2!1) */
                        } else if (dimensions == 1) {
                            /* here we have values
                             * row == n
                             * col == 0 (fixed number)
                             * call function or sth. */
                            array[arr_idx].row = n;
                            array[arr_idx].col = 0;
                            arr_idx++;
                            if (arr_idx >= MAXROW * MAXCOL) {
                                return SCPI_RES_ERR;
                            }
                        }
                        if (n == (size_t)values_to[0]) {
                            /* endpoint reached, stop row for-loop */
                            for_stop_row = TRUE;
                        }
                    }


                } else {
                    return SCPI_RES_ERR;
                }
                /* increase index */
                chanlst_idx++;
            } while (SCPI_EXPR_OK == SCPI_ExprChannelListEntry(context, &channel_list_param, chanlst_idx, &is_range, values_from, values_to, 4, &dimensions));
            /* while checks, whether incremented index is valid */
        }
        /* do something at the end if needed */
        /* array[arr_idx].row = 0; */
        /* array[arr_idx].col = 0; */
    }

    {
        size_t i;
        fprintf(stderr, "TEST_Chanlst: ");
        for (i = 0; i< arr_idx; i++) {
            fprintf(stderr, "%d!%d, ", array[i].row, array[i].col);
        }
        fprintf(stderr, "\r\n");
    }
    return SCPI_RES_OK;
}


static scpi_result_t zwdx_trca_query(scpi_t *context)
{
	double test[100] = {0};
	int32_t intbuff[120];
	int32_t i;


	for(i = 0; i < 100; i++) {
		test[i] = 1.522;
		intbuff[i] = 512;
	}

	//SCPI_ResultInt32(context, 20);
	//SCPI_ResultArrayDouble(context, test, 70, SCPI_FORMAT_ASCII);
	SCPI_ResultArrayInt32(context, intbuff, 120, SCPI_FORMAT_ASCII);
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_trca_write(scpi_t *context)
{
	int32_t param1,param2, i;

    if (!SCPI_ParamInt32(context, &param1, FALSE)) {
        /* do something, if parameter not present */
    }

    /* read second paraeter if present */
    if (!SCPI_ParamInt32(context, &param2, FALSE)) {
        /* do something, if parameter not present */
    }
	printf("param: %d %d\n", param1, param2);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_test_string(scpi_t *context)
{

	int32_t param1,param2, i;
	const char * param;
	size_t param_len;
	int state = -1;

    SCPI_ParamCharacters(context, &param, &param_len, TRUE);

	printf("P: %.*s\n", param_len, param);

	 // ������Сд�����У���һƥ��֧�ֵ�ֵ
    if (strncasecmp(param, "ON", param_len) == 0 ||
        strncasecmp(param, "TRUE", param_len) == 0 ||
        strncasecmp(param, "1", param_len) == 0) {
        state = 1;
    } else if (strncasecmp(param, "OFF", param_len) == 0 ||
               strncasecmp(param, "FALSE", param_len) == 0 ||
               strncasecmp(param, "0", param_len) == 0) {
        state = 0;
    } else {
        return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE); // ���ش���
    }

	if (state == 1)
	{
		printf("State set to ON\n");
	}
	else
	{
		printf("State set to OFF\n");
	}

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_test_chardata(scpi_t *context)
{
	const char *data;
	size_t len;	// ��ȡ���������ݿ�
	//unsigned char buffer[512] = {0};

	//printf("hello world\n");
	if (!SCPI_ParamArbitraryBlock(context, &data, &len, TRUE))
	{
		return SCPI_RES_ERR;  // ������ȡʧ��
	}
	// ��������
	//printf("Received binary data of length: %zu\n", len);

	// ����洢���豸�������?
	//memcpy(buffer, data, len);

	//printhex(buffer, len, 20);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_test_param_unit(scpi_t *context)
{
	const char *data;
	double value;
	const char *suffix = NULL;

	printf("hello world\n");
	if (!SCPI_ParamWithSuffix(context, &value, &suffix, FALSE))
	{
		return SCPI_RES_ERR;  // ������ȡʧ��
	}
	printf("P1:%.2f,%s\n", value, suffix);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_pl_reg(scpi_t *context)
{
	uint32_t addr;
	size_t len;
	char buf[32];
	const char * param_str;
	AxiDevice *pdev = NULL;
	uint32_t offset, value;

	get_plreg_intance(&pdev);

	SCPI_ParamCharacters(context, &param_str, &len, TRUE);

	memcpy(buf, param_str, len);
	buf[len] = '\0';

	if (buf[0] == 'H' || buf[0] == 'h') {
	    sscanf(buf + 1, "%x", &offset);
	} else {
	    offset = atoi(buf);
	}

	SCPI_ParamCharacters(context, &param_str, &len, TRUE);

	memcpy(buf, param_str, len);
	buf[len] = '\0';

	if (buf[0] == 'H' || buf[0] == 'h') {
	    sscanf(buf + 1, "%x", &value);
	} else {
	    value = atoi(buf);
	}

	pdev->reg_base[offset] = value;

	addr = 0x43C30000 + (offset*4);

	printf("addr: 0x%08x data: 0x%08x\n", addr, value);

	return SCPI_RES_OK;

}

static scpi_result_t zwdx_get_pl_reg(scpi_t *context)
{
	size_t len;
	char buf[32];
	const char * param_str;
	AxiDevice *pdev = NULL;
	uint32_t offset, value;

	get_plreg_intance(&pdev);

	SCPI_ParamCharacters(context, &param_str, &len, TRUE);

	memcpy(buf, param_str, len);
	buf[len] = '\0';

	if (buf[0] == 'H' || buf[0] == 'h') {
	    sscanf(buf + 1, "%x", &offset);
	} else {
	    offset = atoi(buf);
	}

	value = pdev->reg_base[offset];

	printf("offset: 0x%08x data: 0x%08x\n", offset, value);

	SCPI_ResultUInt32Base(context, value, 10);

	return SCPI_RES_OK;

}


static scpi_result_t zwdx_set_activepre(scpi_t *context)
{
	int32_t param1;
	int count,savestate;
	const char *path = NULL;
	ParamItem * items = NULL;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	items = get_param_items();
	count = get_param_count();

    switch (param1) {
        case 0: path = FLASH1_PATH; break;
        case 1: path = FLASH2_PATH; break;
        case 2: path = FLASH3_PATH; break;
        case 3: path = FLASH4_PATH; break;
        default: return SCPI_RES_ERR;
    }

	savestate = save_params_to_json(path, items, count);
	if(savestate < 0) {
		printf("save file: %s failed\r\n", path);
	}else {
		printf("save file: %s success\r\n", path);
	}

	return SCPI_RES_OK;

}
static scpi_result_t zwdx_set_defaultpre(scpi_t *context)
{

	int32_t param1;
	SystemConfig *psc = NULL;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	get_system_param(&psc);
	psc->default_preset = param1;

	save_system_config(SYSTEM_PATH, psc, 1);

	/*go main thread config default*/
	//g_restart_config = 1;������BUG, �����豸���²���

	return SCPI_RES_OK;
}

void start_static_ip(const char *ip, const char *gateway, const char *mac)
{
    const char *config_file = "/etc/network/interfaces";
    FILE *fp = fopen(config_file, "r");
    char line[256];
    int need_update = 1;

    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            // ���� "address" ��ͷ����
            if (strncmp(line, "    address", 11) == 0) {
                char cur_ip[64];
                if (sscanf(line, "    address %63s", cur_ip) == 1) {
                    if (strcmp(cur_ip, ip) == 0) {
                        need_update = 0;   // �Ѿ���Ŀ�� IP
                    }
                }
                break;
            }
        }
        fclose(fp);
    }

    // д���µ������ļ�
    fp = fopen(config_file, "w");
    if (!fp) {
        perror("fopen");
        return;
    }

    // д�뾲̬ IP ����
    fprintf(fp, "auto eth0\n");
    fprintf(fp, "iface eth0 inet static\n");
    fprintf(fp, "    address %s\n", ip);
    fprintf(fp, "    netmask 255.255.255.0\n");
    fprintf(fp, "    gateway %s\n", gateway);
	fprintf(fp, "    hwaddress ether %s\n", mac);
    fprintf(fp, "    dns-nameservers 8.8.8.8\n");

    fclose(fp);

	if (!need_update) {
        printf("Already have IP=%s, no need update\n", ip);
        return;
    }

    // ����Ӧ���� IP ����
    system("ifdown eth0 && ifup eth0");

    printf("static ip set success��\n");
    printf("ip address: %s\n", ip);
    printf("gw: %s\n", gateway);
}

int start_dhcp_ip(const char *mac)
{
	const char *ifname = "eth0";
	const char *config_file = "/etc/network/interfaces";
	FILE *fp;
	char line[256];
	int dhcp_in_file = 0;

	// 1) ��������ļ��Ƿ����?DHCP
	fp = fopen(config_file, "r");
	if (fp) {
		while (fgets(line, sizeof(line), fp)) {
			line[strcspn(line, "\r\n")] = '\0';
			if (strstr(line, "iface eth0 inet dhcp")) {
				dhcp_in_file = 1;
				break;
			}
		}
		fclose(fp);
	}

	// 2) ���� DHCP ����д����
	if (!dhcp_in_file) {
		fp = fopen(config_file, "w");
		if (!fp) { perror("open interfaces"); return -1; }
		fprintf(fp, "auto %s\n", ifname);
		fprintf(fp, "iface %s inet dhcp\n", ifname);
		fprintf(fp, "	hwaddress ether %s\n", mac);
		fclose(fp);
	} else {
		printf("Already is dhcp, no need reconfig\r\n");
		return -1;
	}

	// 3) ͣ�ɵ� DHCP �ͻ��ˣ�����������
	system("killall udhcpc 2>/dev/null");
	system("ifdown eth0 2>/dev/null");	 // ���Դ���
	system("ifup eth0 2>/dev/null");

	// 4) �ԡ��������ԡ���Ĭģʽ������ udhcpc�����ⳤʱ��������
	//	  -n: ʧ�������˳�	 -q: ��Ĭ	-T 2: ��ʱ 2 ��   -t 3: ���� 3 ��
	int ret = system("udhcpc -i eth0 -n -q -T 2 -t 3");
	if (ret != 0) {
		fprintf(stderr, "udhcpc failed (ret=%d)\n", ret);
	}

	printf("DHCP Start Success\r\n");
}

/*qt applay button*/
static scpi_result_t zwdx_set_static_ip(scpi_t *context)
{
	const char * param1;
	const char * param2;
	const char * param3;
	size_t param1_len, param2_len, param3_len;
	SystemConfig *psc = NULL;

	get_system_param(&psc);

    SCPI_ParamCharacters(context, &param1, &param1_len, TRUE);
	SCPI_ParamCharacters(context, &param2, &param2_len, TRUE);
	SCPI_ParamCharacters(context, &param3, &param3_len, TRUE);


	strncpy(psc->ip, param1, param1_len);
	strncpy(psc->mask, param2, param2_len);
	strncpy(psc->gw, param3, param3_len);
	save_system_config(SYSTEM_PATH, psc, 1);
	//printf("IP: %.*s\n", param1_len, param1);
	//printf("IP: %.*s\n", param2_len, param2);
	//printf("IP: %.*s\n", param3_len, param3);

	start_static_ip(psc->ip, psc->gw, psc->mac);


	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_dhcp_en(scpi_t *context)
{
	int32_t param1;
	SystemConfig *psc = NULL;
	static int isfirst_start = 1;
	get_system_param(&psc);

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(isfirst_start) {
		/*��һ�����У�������Դ��CMDPARAM��ʹ��,ʹ��SYSPARAM�ж�,��ִ���κβ���*/
	}else {
		psc->dhcpen = param1; //1:enable 0:disable ��λ������
	}

	save_system_config(SYSTEM_PATH, psc, 1);

	if(psc->dhcpen) {
		if(isfirst_start) {
			start_dhcp_ip(psc->mac); //���ϵ��һ��ִ���? ��λ�����ò�ִ�У���Ҫ�豸�л����ߣ������豸
		}
	} else {
		start_static_ip(psc->ip, psc->gw, psc->mac);
	}

	isfirst_start = 0;

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_dev_clk_source(scpi_t *context)
{
	static bool isfirst = true;
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if(isfirst) {
		isfirst = false;
	} else {
		pv.type = PARAM_INT32;
		pv.value.i32 = param1;

		g_param.g_dev_clk_source = param1;
		set_param_by_name(PARAM_CURRENT,"dev_clk_source", 0, pv);

		printf("CLK SOURCE : %d\r\n", param1);

	}

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_dev_clk_extfreq(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"dev_extclk_freq", 0, pv);

	if(param1 == EXTERN_100M) {
		dev_clk_switch(EXTERN_100M);
	}else if(param1 == EXTERN_10M ) {
		dev_clk_switch(EXTERN_10M);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_dev_trig_source(scpi_t *context)
{
	uint32_t trig_source;
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	trig_source = param1;

	g_param.g_dev_trig_source = trig_source;

	set_param_by_name(PARAM_CURRENT,"dev_trig_source", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_get_dev_clk_state(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	SCPI_ResultInt32(context, g_param.g_dev_lock_state);

	return SCPI_RES_OK;
}



static scpi_result_t zwdx_get_ref_freq(scpi_t *context)
{
	int32_t param1;
	uint32_t freq_l, freq_h;
	double freq;
	uint64_t freq_code;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_read_by_name("pll_freq_l", &freq_l);
	reg_read_by_name("pll_freq_h", &freq_h);

	freq_code = ((uint64_t)freq_h << 32) | ((uint64_t)freq_l);

	freq = freq_code / pow(2, 48) * zwdx_oscillator_ref_clock_hz();

	SCPI_ResultArrayDouble(context, &freq, 1, SCPI_FORMAT_ASCII);

	return SCPI_RES_OK;
}


static int zwdx_valid_demo_channel(int32_t channel)
{
	return (channel >= CH1) && (channel <= DEVICE_DEMOD_NUM_L500K);
}

static int zwdx_write_demo_reg(int32_t channel, const char *name, uint32_t value)
{
	char reg_name[64];

	if (!zwdx_valid_demo_channel(channel)) {
		return -1;
	}

	snprintf(reg_name, sizeof(reg_name), name, channel);
	return reg_write_by_name(reg_name, value);
}

static int zwdx_save_demo_param(int32_t channel, const char *name, ParamValue pv)
{
	char param_name[64];

	if (!zwdx_valid_demo_channel(channel)) {
		return -1;
	}

	snprintf(param_name, sizeof(param_name), name, channel);
	return set_param_by_name(PARAM_CURRENT, param_name, 1, pv);
}

static scpi_result_t zwdx_set_demo_ref_mode(scpi_t *context)
{
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

    if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!zwdx_valid_demo_channel(param1)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	g_param.g_demod_param[param1 - 1].osc_source = param2;
	zwdx_save_demo_param(param1, "demo_ref_mode_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_source(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (param1 < 0 || param1 > 1) {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
	}

	reg_write_by_name("pll_source", (uint32_t)param1);

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	set_param_by_name(PARAM_CURRENT,"ref_source", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_sys_high(scpi_t *context)
{
	double param1;
	int32_t srccode;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	if(get_device_model() == DEVICE_MODEL_L500K) {
		srccode = (int32_t)(param1 * (double)((1ULL << 15) - 1) / 2.5);
	} else {
		srccode = (int32_t)(param1 * (pow(2, 19) - 1) / 10.0);
	}


	set_param_by_name(PARAM_CURRENT,"ref_hys_high", 0, pv);
	reg_write_by_name("pll_ref_hysteresis_high", srccode);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_sys_low(scpi_t *context)
{
	double param1;
	int32_t srccode;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}


	reg_write_by_name("pll_ref_hysteresis_low", param1);

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	if(get_device_model() == DEVICE_MODEL_L500K) {
		srccode = (int32_t)(param1 * (double)((1ULL << 15) - 1) / 2.5);
	} else {
		srccode = (int32_t)(param1 * (pow(2, 19) - 1) / 10.0);
	}

	set_param_by_name(PARAM_CURRENT,"ref_hys_low", 0, pv);
	reg_write_by_name("pll_ref_hysteresis_low", srccode);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_filter_win(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("pll_mov_fir_ratio", param1);

	pv.type = PARAM_INT32;
	pv.value.i32= param1;
	set_param_by_name(PARAM_CURRENT,"ref_filter_win", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_calc_time(scpi_t *context)
{
	double param1;
	uint64_t calc_time;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if(g_device_model == DEVICE_MODEL_L500K) {
		calc_time = (uint64_t)(param1 * pow(10, 9) / 500.0); //2MB
	} else if(g_device_model == DEVICE_MODEL_M005M_VV || g_device_model == DEVICE_MODEL_M005M_VI ||
		g_device_model == DEVICE_MODEL_M005M) {
		calc_time = (uint64_t)(param1 * pow(10, 9) / 25.0);	//40M
	}

	reg_write_by_name("pll_calc_time_l", (uint32_t)(calc_time & 0xFFFFFFFF));
	reg_write_by_name("pll_calc_time_h", (uint32_t)((calc_time >> 32) & 0xFFFFFFFF));

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;
	set_param_by_name(PARAM_CURRENT,"ref_calc_time", 0, pv);

	return SCPI_RES_OK;

}

static scpi_result_t zwdx_set_ref_intfilter_win(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	reg_write_by_name("ref_mov_fir_ratio", param1);

	set_param_by_name(PARAM_CURRENT,"ref_intfilter_win", 0, pv);

	return SCPI_RES_OK;
}


void config_si_vv_regs(SiVolCtrolRegs regs, int index)
{
	int rtn = 0;

	if(index == 1) {
		#if 0
		while(1) {
			rtn = gpio_get_value(39);
			if(rtn)
				break;
			usleep(10);
		}
		#endif

		reg_write_by_name("v_relays_addr", 0);
		reg_write_by_name("v_range_relays", regs.reg1);
		reg_write_by_name("v_range_relays_en", 0);
		reg_write_by_name("v_range_relays_en", 1);
		reg_write_by_name("v_range_relays_en", 0);
		usleep(100000);

		reg_write_by_name("v_relays_addr", 1);
		reg_write_by_name("v_range_relays", regs.reg2);
		reg_write_by_name("v_range_relays_en", 0);
		reg_write_by_name("v_range_relays_en", 1);
		reg_write_by_name("v_range_relays_en", 0);
		usleep(100000);

		reg_write_by_name("v_relays_addr", 2);
		reg_write_by_name("v_range_relays", regs.reg3);
		reg_write_by_name("v_range_relays_en", 0);
		reg_write_by_name("v_range_relays_en", 1);
		reg_write_by_name("v_range_relays_en", 0);

	}else if(index == 2) {
		if (g_device_model == DEVICE_MODEL_M005M_VV) {

			reg_write_by_name("i_relays_addr", 0);
			reg_write_by_name("i_range_relays", regs.reg1);
			reg_write_by_name("i_range_relays_en", 0);
			reg_write_by_name("i_range_relays_en", 1);
			reg_write_by_name("i_range_relays_en", 0);
			usleep(100000);

			reg_write_by_name("i_relays_addr", 1);
			reg_write_by_name("i_range_relays", regs.reg2);
			reg_write_by_name("i_range_relays_en", 0);
			reg_write_by_name("i_range_relays_en", 1);
			reg_write_by_name("i_range_relays_en", 0);
			usleep(100000);

			reg_write_by_name("i_relays_addr", 2);
			reg_write_by_name("i_range_relays", regs.reg3);
			reg_write_by_name("i_range_relays_en", 0);
			reg_write_by_name("i_range_relays_en", 1);
			reg_write_by_name("i_range_relays_en", 0);

		}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
			//VI�汾2ͨ������Ҫ���ã��ɵ���ͨ���Լ�ȥ����
		}
	}
}

static scpi_result_t zwdx_set_si_volch_mode(scpi_t *context)
{
	int32_t mode;
	int32_t param1, param2;
	uint32_t temp;
	ParamValue pv;
	SiVolCtrolRegs regs = {0};

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	mode = param2;

	if(param1 == CH1) {
		if(mode == 0) {/*single*/
			g_param.g_si_vol_ch1_ctrol.in_mode = IN_SINGLE;
		}else if(mode == 1) {/*diff*/
			g_param.g_si_vol_ch1_ctrol.in_mode = IN_DIFF;
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_mode", 1, pv);

	}else if(param1 == CH2) {
		if(mode == 0) {/*single*/
			g_param.g_si_vol_ch2_ctrol.in_mode = IN_SINGLE;
		}else if(mode == 1) {/*diff*/
			g_param.g_si_vol_ch2_ctrol.in_mode = IN_DIFF;
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		set_param_by_name(PARAM_CURRENT,"si_volch2_mode", 1, pv);
	}
	set_si_calibration(param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_volch_gnd(scpi_t *context)
{
	int32_t gnd;
	int32_t param1, param2;
	uint32_t temp;
	ParamValue pv;
	SiVolCtrolRegs regs = {0};

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	gnd = param2;

	if(param1 == CH1) {

		if(gnd == 0) {		/*gnd*/
			g_param.g_si_vol_ch1_ctrol.gnd_sel = GND_EARTH;
		}else if(gnd == 1) {/*float*/
			g_param.g_si_vol_ch1_ctrol.gnd_sel = GND_FLOAT;
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_gnd", 1, pv);
	} else if(param1 == CH2) {
		if(gnd == 0) {		/*gnd*/
			g_param.g_si_vol_ch2_ctrol.gnd_sel = GND_EARTH;
		}else if(gnd == 1) {/*float*/
			g_param.g_si_vol_ch2_ctrol.gnd_sel = GND_FLOAT;
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		reg_write_by_name("i_range_relays_en", 0);
		set_param_by_name(PARAM_CURRENT,"si_volch2_gnd", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_volch_couping(scpi_t *context)
{
	int32_t couping;
	int32_t param1, param2;
	uint32_t temp;
	ParamValue pv;
	SiVolCtrolRegs regs = {0};

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	couping = param2;

	if(param1 == CH1) {

		if(couping == 0) {		/*AC*/
			g_param.g_si_vol_ch1_ctrol.coup_sel = COUP_AC;
		}else if(couping == 1) {/*DC*/
			g_param.g_si_vol_ch1_ctrol.coup_sel = COUP_DC;
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_couping", 1, pv);
	} else if(param1 == CH2) {
		if(couping == 0) {		/*AC*/
			g_param.g_si_vol_ch2_ctrol.coup_sel = COUP_AC;
		}else if(couping == 1) {/*DC*/
			g_param.g_si_vol_ch2_ctrol.coup_sel = COUP_DC;
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		set_param_by_name(PARAM_CURRENT,"si_volch2_couping", 1, pv);
	}

	set_si_calibration(param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_volch_impedance(scpi_t *context)
{
	int32_t impedance;
	int32_t param1, param2;
	uint32_t temp;
	ParamValue pv;
	SiVolCtrolRegs regs = {0};

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	impedance = param2;

	if(param1 == CH1) {

		g_param.g_si_vol_ch1_ctrol.z_index = impedance;

		if(g_param.g_si_vol_ch1_ctrol.range_idx < 4) {
			g_param.g_si_vol_ch1_ctrol.z_for_high4 = 0x02;
			if(impedance == 0) {	  /*10M��*/
				g_param.g_si_vol_ch1_ctrol.z_for_low4 = Z_10M;
			}else if(impedance == 1) {/*50��*/
				g_param.g_si_vol_ch1_ctrol.z_for_low4 = Z_50M;
			}
		}else {
			g_param.g_si_vol_ch1_ctrol.z_for_low4 = 0x02;
			if(impedance == 0) {	  /*10M��*/
				g_param.g_si_vol_ch1_ctrol.z_for_high4 = Z_10M;
			}else if(impedance == 1) {/*50��*/
				g_param.g_si_vol_ch1_ctrol.z_for_high4 = Z_50M;
			}
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_impedance", 1, pv);
	} else if(param1 == CH2) {
		g_param.g_si_vol_ch2_ctrol.z_index = impedance;

		if(g_param.g_si_vol_ch2_ctrol.range_idx < 4) {
			g_param.g_si_vol_ch2_ctrol.z_for_high4 = 0x02;
			if(impedance == 0) {	  /*10M��*/
				g_param.g_si_vol_ch2_ctrol.z_for_low4 = Z_10M;
			}else if(impedance == 1) {/*50��*/
				g_param.g_si_vol_ch2_ctrol.z_for_low4 = Z_50M;
			}
		}else {
			g_param.g_si_vol_ch2_ctrol.z_for_low4 = 0x02;
			if(impedance == 0) {	  /*10M��*/
				g_param.g_si_vol_ch2_ctrol.z_for_high4 = Z_10M;
			}else if(impedance == 1) {/*50��*/
				g_param.g_si_vol_ch2_ctrol.z_for_high4 = Z_50M;
			}
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		set_param_by_name(PARAM_CURRENT,"si_volch2_impedance", 1, pv);
	}
	set_si_calibration(param1);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_si_volch_filter(scpi_t *context)
{
	int32_t filter;
	int32_t param1, param2;
	uint32_t temp;
	ParamValue pv;
	SiVolCtrolRegs regs = {0};

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	filter = param2;

	if(param1 == CH1) {

		if(filter == 0) {		/*None*/
			g_param.g_si_vol_ch1_ctrol.filt50 = FILT_OFF;
			g_param.g_si_vol_ch1_ctrol.filt100 = FILT_OFF;
		}else if(filter == 1) {/*50Hz*/
			g_param.g_si_vol_ch1_ctrol.filt50 = FILT_ON;
			g_param.g_si_vol_ch1_ctrol.filt100 = FILT_OFF;
		}else if(filter == 2) {/*100Hz*/
			g_param.g_si_vol_ch1_ctrol.filt50 = FILT_OFF;
			g_param.g_si_vol_ch1_ctrol.filt100 = FILT_ON;
		}else {                /*Both*/
			g_param.g_si_vol_ch1_ctrol.filt50 = FILT_ON;
			g_param.g_si_vol_ch1_ctrol.filt100 = FILT_ON;
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_filter", 1, pv);

	} else if(param1 == CH2) {
		if(filter == 0) {		/*None*/
			g_param.g_si_vol_ch2_ctrol.filt50 = FILT_OFF;
			g_param.g_si_vol_ch2_ctrol.filt100 = FILT_OFF;
		}else if(filter == 1) {/*50Hz*/
			g_param.g_si_vol_ch2_ctrol.filt50 = FILT_ON;
			g_param.g_si_vol_ch2_ctrol.filt100 = FILT_OFF;
		}else if(filter == 2){ /*100Hz*/
			g_param.g_si_vol_ch2_ctrol.filt50 = FILT_OFF;
			g_param.g_si_vol_ch2_ctrol.filt100 = FILT_ON;
		}else {
			g_param.g_si_vol_ch2_ctrol.filt50 = FILT_ON;
			g_param.g_si_vol_ch2_ctrol.filt100 = FILT_ON;
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		set_param_by_name(PARAM_CURRENT,"si_volch2_filter", 1, pv);
	}
	set_si_calibration(param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_volch_range(scpi_t *context)
{
	int32_t param1,param2;
	SiVolCtrolRegs regs = {0};

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	if(param1 == CH1) {
		g_param.g_si_vol_ch1_ctrol.range_idx = param2;
		if(g_param.g_si_vol_ch1_ctrol.range_idx < 4) {  /*�л���Χ����Ҫ���迹���ݷ�Χѡ��λ��*/
			g_param.g_si_vol_ch1_ctrol.z_for_high4 = 0x02;
			if(g_param.g_si_vol_ch1_ctrol.z_index == 0) { /*10M��*/
				g_param.g_si_vol_ch1_ctrol.z_for_low4 = Z_10M;
			}else { /* 50 ��*/
				g_param.g_si_vol_ch1_ctrol.z_for_low4 = Z_50M;
			}
		}else {
			g_param.g_si_vol_ch1_ctrol.z_for_low4 = 0x02;
			if(g_param.g_si_vol_ch1_ctrol.z_index == 0) { /*10M��*/
				g_param.g_si_vol_ch1_ctrol.z_for_high4 = Z_10M;
			}else { /* 50 ��*/
				g_param.g_si_vol_ch1_ctrol.z_for_high4 = Z_50M;
			}
		}

		si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

		config_si_vv_regs(regs, 1);

		set_param_by_name(PARAM_CURRENT,"si_volch1_range", 1, pv);
		g_param.g_vol_range1 = param2; //����Ҫ��COMBOX��Ӧ
	} else if(param1 == CH2) {
		g_param.g_si_vol_ch2_ctrol.range_idx = param2;
		if(g_param.g_si_vol_ch2_ctrol.range_idx < 4) {  /*�л���Χ����Ҫ���迹���ݷ�Χѡ��λ��*/
			g_param.g_si_vol_ch2_ctrol.z_for_high4 = 0x02;
			if(g_param.g_si_vol_ch2_ctrol.z_index == 0) { /*10M��*/
				g_param.g_si_vol_ch2_ctrol.z_for_low4 = Z_10M;
			}else { /* 50 ��*/
				g_param.g_si_vol_ch2_ctrol.z_for_low4 = Z_50M;
			}
		}else {
			g_param.g_si_vol_ch2_ctrol.z_for_low4 = 0x02;
			if(g_param.g_si_vol_ch2_ctrol.z_index == 0) { /*10M��*/
				g_param.g_si_vol_ch2_ctrol.z_for_high4 = Z_10M;
			}else { /* 50 ��*/
				g_param.g_si_vol_ch2_ctrol.z_for_high4 = Z_50M;
			}
		}

		si_pack_regs(&g_param.g_si_vol_ch2_ctrol, &regs);

		config_si_vv_regs(regs, 2);

		set_param_by_name(PARAM_CURRENT,"si_volch2_range", 1, pv);
		g_param.g_vol_range2 = param2; //����Ҫ��COMBOX��Ӧ
	}
	set_si_calibration(param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_curch_gnd(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"si_curch_gnd", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_curch_range(scpi_t *context)
{
	int32_t temp1,temp2;
	int32_t param1;
	SiCurCtrolRegs regs = {0};

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	g_param.g_si_cur_ctrol.range_idx = param1;

	si_pack_cur_regs(&g_param.g_si_cur_ctrol, &regs);

	reg_write_by_name("i_relays_addr", 0);
	reg_write_by_name("i_range_relays", regs.reg1);
	reg_write_by_name("i_range_relays_en", 0);
	reg_write_by_name("i_range_relays_en", 1);
	reg_write_by_name("i_range_relays_en", 0);

	usleep(100000);

	reg_write_by_name("i_relays_addr", 1);
	reg_write_by_name("i_range_relays", regs.reg2);
	reg_write_by_name("i_range_relays_en", 0);
	reg_write_by_name("i_range_relays_en", 1);
	reg_write_by_name("i_range_relays_en", 0);

	set_param_by_name(PARAM_CURRENT,"si_curch_range", 0, pv);

	g_param.g_cur_range = param1; //����Ҫ��COMBOX��Ӧ

	set_si_cur_calibration(param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_si_curch_couping(scpi_t *context)
{
	int32_t temp1,temp2;
	int32_t param1;
	SiCurCtrolRegs regs = {0};

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	g_param.g_si_cur_ctrol.coup_sel = param1 ? 0x02 : 0x01; //DC:0x02 AC:0x01

	si_pack_cur_regs(&g_param.g_si_cur_ctrol, &regs);

	reg_write_by_name("i_relays_addr", 0);
	reg_write_by_name("i_range_relays", regs.reg1);
	reg_write_by_name("i_range_relays_en", 0);
	reg_write_by_name("i_range_relays_en", 1);
	reg_write_by_name("i_range_relays_en", 0);

	usleep(100000);

	reg_write_by_name("i_relays_addr", 1);
	reg_write_by_name("i_range_relays", regs.reg2);
	reg_write_by_name("i_range_relays_en", 0);
	reg_write_by_name("i_range_relays_en", 1);
	reg_write_by_name("i_range_relays_en", 0);

	set_param_by_name(PARAM_CURRENT,"si_curch_couping", 0, pv);

	return SCPI_RES_OK;
}

/*****************************************************************************
 �� �� ��  : zwdx_set_si_work_test
 ��������  : ��λ��H-L���԰�ť��,�����λ���������ˣ���DEMO-ON/OFF SO-ON/OFF��
 ???????  : scpi_t *context
 ???????  : ??
 ?? ?? ?  : static scpi_result_t
 ???��???  :
 ????????  :

 ??????      :
  1.��    ��   : 2025��8��20��
    ??    ??   : huangwei
    ???????   : ?????????

*****************************************************************************/
static scpi_result_t zwdx_set_si_work_test(scpi_t *context)
{
	uint32_t param1;

	if (!SCPI_ParamUInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("osc_work_all", 0);
	reg_write_by_name("osc_work_all", 1);

	if(g_param.g_dev_trig_source == INT) {
		reg_write_by_name("soft_trig", 0);
		usleep(100);
		reg_write_by_name("soft_trig", 1);
		usleep(100);
	    reg_write_by_name("soft_trig", 0);
	}else if(g_param.g_dev_trig_source == EXT) {
		/*�ⲿ�����̴߳���*/
	}

	return SCPI_RES_OK;
}

#if 0
static scpi_result_t zwdx_set_refsource(scpi_t *context)
{
	size_t param_len;
	const char *param = NULL;
	ParamValue pv;

    // ��ȡ�ַ������Ͳ��������� "AUXIN1" �� "AUXIN2"��
    if (!SCPI_ParamCharacters(context, &param, &param_len, false)) {
        return SCPI_RES_ERR; // ������ȡʧ��
    }

    // ��������
    if (strncasecmp(param, "AUXIN1", param_len) == 0) {
		reg_write_by_name("pll_source", 0);

		pv.type = PARAM_STRING;
		pv.value.str = "AXUIN1";
		set_param_by_name(PARAM_CURRENT,"ref_pll_source", 0, pv);
    } else if (strncasecmp(param, "AUXIN2", param_len) == 0) {
		reg_write_by_name("pll_source", 1);

		pv.type = PARAM_STRING;
		pv.value.str = "AUXIN2";
		set_param_by_name(PARAM_CURRENT,"ref_pll_source", 0, pv);
    } else {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_oscsource(scpi_t *context)
{

	size_t param_len;
	AxiDevice *pdev = NULL;
	const char *param = NULL;

	get_plreg_intance(&pdev);

    if (!SCPI_ParamCharacters(context, &param, &param_len, TRUE)) {
        return SCPI_RES_ERR; // ������ȡʧ��
    }

    // ��������
    if (strncasecmp(param, "INT", param_len) == 0) {


		reg_write_by_name("pll_enable", 0);

		g_param.g_osc_source = 0;
    } else if (strncasecmp(param, "EXT", param_len) == 0) {


		reg_write_by_name("pll_enable", 1);

		g_param.g_osc_source = 1;
    } else {
        SCPI_ErrorPush(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
        return SCPI_RES_ERR;
    }

	pdev->reg_base[0x0702] = 2000;
	pdev->reg_base[0x0703] = -2000;
	pdev->reg_base[0x0704] = 8;
	pdev->reg_base[0x0705] = 0x00186A00;
	pdev->reg_base[0x0706] = 0;

	//set_param_by_name(PARAM_CURRENT,"ref_pll_source", 0, pv);
	//addr = 0x43C30000 + (offset*4);
	//printf("addr: 0x%08x data: 0x%08x\n", addr, 1);

	return SCPI_RES_OK;
}

#endif

static scpi_result_t zwdx_set_so_oscfreq(scpi_t *context)
{
	int32_t param1;
	double param2;
	double temp;
	uint64_t freq;
	ParamValue pv;

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	temp = param2 * pow(2, 48) / 40000000.0; //M005M and L500K FPGA DAC is 40MHz ,app all is Hz
	freq = (uint64_t)temp;

	if(param1 == 1) {
		reg_write_by_name("signalout1_osc_freq_word_l", (uint32_t)freq & 0xFFFFFFFF);
		reg_write_by_name("signalout1_osc_freq_word_h", (uint32_t)((freq >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_osc_freq_ch1", 1, pv);
	}else if(param1 == 2) {
		reg_write_by_name("signalout2_osc_freq_word_l", (uint32_t)freq & 0xFFFFFFFF);
		reg_write_by_name("signalout2_osc_freq_word_h", (uint32_t)((freq >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_osc_freq_ch2", 1, pv);

	}

	printf("FREQ: %lld, 0x07->0x%08x, 0x08->0x%08x\r\n", freq, (uint32_t)freq & 0xFFFFFFFF, (uint32_t)((freq >> 32) & 0xFFFFFFFF));


	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_oscphase(scpi_t *context)
{
	int32_t param1;
	double  param2, phase_deg;
	uint64_t phase_code;
	ParamValue pv;

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	if(param2 < 0) {
		phase_deg = param2 + 360;
	} else {
		phase_deg = param2;
	}
	phase_code = (uint64_t)(phase_deg * pow(2, 48) / 360.0);//FPGA is 0-360 ,app all is [-180,180]

	if(param1 == 1) {
		reg_write_by_name("signalout1_osc_phase_l", (uint32_t)(phase_code & 0xFFFFFFFF));
		reg_write_by_name("signalout1_osc_phase_h", (uint32_t)((phase_code >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_osc_phase_ch1", 1, pv);
	}else if(param1 == 2) {
		reg_write_by_name("signalout2_osc_phase_l", (uint32_t)(phase_code & 0xFFFFFFFF));
		reg_write_by_name("signalout2_osc_phase_h", (uint32_t)((phase_code >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_osc_phase_ch2", 1, pv);
	}

	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_oscharm(scpi_t *context)
{
	int32_t param1;
	AxiDevice *pdev = NULL;

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }
	get_plreg_intance(&pdev);

	pdev->reg_base[0x1509] = param1;

	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_range(scpi_t *context)
{
	uint8_t regs[3];
	double scale, temp, srcoffset, actual_value;
	uint64_t codevalue;
	int32_t param1, param2;
	int model = get_device_model();
	ParamValue pv, getpv;

    if (!SCPI_ParamInt32(context, &param1, TRUE)) {
        return SCPI_RES_ERR;
    }
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	if(param1 == CH1) {
		/*����ʱ�����ˣ���Ϊ������Կ緶Χɨ�����BUG(����һ�ε�Scale����,��ѹ����)*/
		#if 0
		//[1]���÷�Χ֮ǰ������һ��SCALE������Ⱥ�˳�������?
		get_param_by_name(PARAM_CURRENT,"so_signal_scale_ch1", 1, &getpv);
		scale = getpv.value.d;
		temp = scale * g_so_sineout_verify[param2];
		codevalue = temp * (1ULL << 32); //�����ǰѸ����� * ����һ��ϵ���仯��һ������
		reg_write_by_name("signalout1_scaling2_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout1_scaling2_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		#endif
		//[2]���÷�Χǰ��������OFFSETֵ������Ⱥ�˳�������?
		get_param_by_name(PARAM_CURRENT,"so_offset_ch1", 1, &getpv);
		srcoffset = getpv.value.d;
		actual_value = srcoffset;
		if (model == DEVICE_MODEL_L500K) {
			actual_value = sooffset_l500k_calibrate_value(CH1, param2, srcoffset);
			temp = (uint32_t)((actual_value + 5.0) / 10.0 * (double)((1 << 20) - 1));
		} else {
			temp = (uint32_t)((actual_value + 10.0) / 20.0 * (double)((1 << 20) - 1));
		}
		reg_write_by_name("singalout1_offset", temp);
		reg_write_by_name("singalout1_offset_en", 1);
		reg_write_by_name("singalout1_offset_en", 0);

		switch(param2)
		{
			case 0:
				g_param.g_so_range_wave_format.ch1_range = 0x0A;
				g_param.g_so_range_wave_format.ch1_range2 = 0x00;
				break;
			case 1:
				g_param.g_so_range_wave_format.ch1_range = 0x0A;
				g_param.g_so_range_wave_format.ch1_range2 = 0x01;
				break;
			case 2:
				g_param.g_so_range_wave_format.ch1_range = 0x0A;
				g_param.g_so_range_wave_format.ch1_range2 = 0x02;
				break;
			case 3:
				g_param.g_so_range_wave_format.ch1_range = 0x0A;
				g_param.g_so_range_wave_format.ch1_range2 = 0x03;
				break;
			case 4:
				g_param.g_so_range_wave_format.ch1_range = 0x05;
				g_param.g_so_range_wave_format.ch1_range2 = 0x03;
				break;
			default:
				break;
		}
		set_param_by_name(PARAM_CURRENT,"so_range_ch1", 1, pv);
	}else if(param1 == CH2) {
		#if 0
		//[1]���÷�Χ֮ǰ������һ��SCALE������Ⱥ�˳�������?
		get_param_by_name(PARAM_CURRENT,"so_signal_scale_ch2", 1, &getpv);
		scale = getpv.value.d;
		temp = scale * g_so_sineout_verify[param2];
		codevalue = temp * (1ULL << 32); //�����ǰѸ����� * ����һ��ϵ���仯��һ������
		reg_write_by_name("signalout2_scaling2_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout2_scaling2_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		#endif

		//[2]���÷�Χǰ��������OFFSETֵ������Ⱥ�˳�������?
		get_param_by_name(PARAM_CURRENT,"so_offset_ch2", 1, &getpv);
		srcoffset = getpv.value.d;
		actual_value = srcoffset;
		if (model == DEVICE_MODEL_L500K) {
			actual_value = sooffset_l500k_calibrate_value(CH2, param2, srcoffset);
			temp = (uint32_t)((actual_value + 5.0) / 10.0 * (double)((1 << 20) - 1));
		} else {
			temp = (uint32_t)((actual_value + 10.0) / 20.0 * (double)((1 << 20) - 1));
		}
		reg_write_by_name("singalout2_offset", temp);
		reg_write_by_name("singalout2_offset_en", 1);
		reg_write_by_name("singalout2_offset_en", 0);

		switch(param2)
		{
			case 0:
				g_param.g_so_range_wave_format.ch2_range = 0x0A;
				g_param.g_so_range_wave_format.ch2_range2 = 0x00;
				break;
			case 1:
				g_param.g_so_range_wave_format.ch2_range = 0x0A;
				g_param.g_so_range_wave_format.ch2_range2 = 0x01;
				break;
			case 2:
				g_param.g_so_range_wave_format.ch2_range = 0x0A;
				g_param.g_so_range_wave_format.ch2_range2 = 0x02;
				break;
			case 3:
				g_param.g_so_range_wave_format.ch2_range = 0x0A;
				g_param.g_so_range_wave_format.ch2_range2 = 0x03;
				break;
			case 4:
				g_param.g_so_range_wave_format.ch2_range = 0x05;
				g_param.g_so_range_wave_format.ch2_range2 = 0x03;
				break;
			default:
				break;
		}
		set_param_by_name(PARAM_CURRENT,"so_range_ch2", 1, pv);
	}

	pack_registers(&g_param.g_so_range_wave_format, regs);

	reg_write_by_name("sine_relays_addr", 0);
	reg_write_by_name("sine_range_relays", regs[0]);
	reg_write_by_name("sine_range_relays_en", 0);
	reg_write_by_name("sine_range_relays_en", 1);
	reg_write_by_name("sine_range_relays_en", 0);

	reg_write_by_name("sine_relays_addr", 1);
	reg_write_by_name("sine_range_relays", regs[1]);
	reg_write_by_name("sine_range_relays_en", 0);
	reg_write_by_name("sine_range_relays_en", 1);
	reg_write_by_name("sine_range_relays_en", 0);

	reg_write_by_name("sinemux_relays_addr", 0);
	reg_write_by_name("sinemux_range_relays", regs[2]);
	reg_write_by_name("sinemux_range_relays_en", 0);
	reg_write_by_name("sinemux_range_relays_en", 1);
	reg_write_by_name("sinemux_range_relays_en", 0);

	set_so_calibration(param1, param2 + 2);//+2 is ID, RANGE ID FROM 2 START

	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_signal_scale(scpi_t *context)
{
	int32_t param1, range;
	double  param2,temp;
	uint64_t codevalue;
	ParamValue pv;
	ParamValue getpv;
	int model = get_device_model();

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	//scale and range is coupling, so when range changed, scale need to be updated, otherwise, the output amplitude will be different with expectation, this is a BUG we found in the early stage, but we don't know why, so we put the scale update before range update, and when range update, we update the scale again, this is a workaround solution to avoid this BUG.
	if(param1 == CH1) {
		get_param_by_name(PARAM_CURRENT,"so_range_ch1", 1, &getpv);
		range = getpv.value.i32;
		if(model == DEVICE_MODEL_L500K) {
			temp = param2 * sooutput_l500k_get_sine_quant(CH1, range);
		} else {
			temp = param2 * g_so_sineout_verify[range];
		}
		//printf("huangwei2 : %f\r\n", temp);
		codevalue = temp * (1ULL << 32); //float value * full scale(2^32) to get the code value for FPGA
		reg_write_by_name("signalout1_scaling2_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout1_scaling2_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_signal_scale_ch1", 1, pv);
	}else if(param1 == CH2) {
		get_param_by_name(PARAM_CURRENT,"so_range_ch2", 1, &getpv);
		range = getpv.value.i32;
		if(model == DEVICE_MODEL_L500K) {
			temp = param2 * sooutput_l500k_get_sine_quant(CH2, range);
		} else {
			temp = param2 * g_so_sineout_verify[range];
		}
		codevalue = temp * (1ULL << 32); //float value * full scale(2^32) to get the code value for FPGA
		reg_write_by_name("signalout2_scaling2_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout2_scaling2_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_signal_scale_ch2", 1, pv);
	}else {
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_offset(scpi_t *context)
{
	uint32_t temp;
	int32_t param1, rng;
	int model = get_device_model();
	double param2, actual_value;
	ParamValue pv, getpv;


	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;


	if(param1 == CH1) {
		get_param_by_name(PARAM_CURRENT,"so_range_ch1", 1, &getpv);
		rng = getpv.value.i32;
		actual_value = param2;
		if (model == DEVICE_MODEL_L500K) {
			actual_value = sooffset_l500k_calibrate_value(CH1, rng, param2);
			temp = (uint32_t)((actual_value + 5.0) / 10.0 * (double)((1 << 20) - 1));
		} else {
			temp = (uint32_t)((actual_value + 10.0) / 20.0 * (double)((1 << 20) - 1));
		}
		reg_write_by_name("singalout1_offset", temp);
		reg_write_by_name("singalout1_offset_en", 1);
		reg_write_by_name("singalout1_offset_en", 0);
		set_param_by_name(PARAM_CURRENT,"so_offset_ch1", 1, pv);
	}else if(param1 == CH2) {
		get_param_by_name(PARAM_CURRENT,"so_range_ch2", 1, &getpv);
		rng = getpv.value.i32;
		actual_value = param2;
		if (model == DEVICE_MODEL_L500K) {
			actual_value = sooffset_l500k_calibrate_value(CH2, rng, param2);
			temp = (uint32_t)((actual_value + 5.0) / 10.0 * (double)((1 << 20) - 1));
		} else {
			temp = (uint32_t)((actual_value + 10.0) / 20.0 * (double)((1 << 20) - 1));
		}
		reg_write_by_name("singalout2_offset", temp);
		reg_write_by_name("singalout2_offset_en", 1);
		reg_write_by_name("singalout2_offset_en", 0);
		set_param_by_name(PARAM_CURRENT,"so_offset_ch2", 1, pv);
	}else {

	}

	printf("temp: %d=0x%08x\r\n", temp, temp);
	return SCPI_RES_OK;
}
static scpi_result_t zwdx_set_so_switch(scpi_t *context)
{
	int32_t state;
	int32_t param1, param2;
	//const char * param2;
	//size_t param_len;
	ParamValue pv;


	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

    //SCPI_ParamCharacters(context, &param2, &param_len, TRUE);

	//printf("P: %.*s\n", param_len, param2);

    //if (strncasecmp(param2, "ON", param_len) == 0 ||
    //    strncasecmp(param2, "TRUE", param_len) == 0 ||
    //    strncasecmp(param2, "1", param_len) == 0) {
    //    state = 1;
    //} else if (strncasecmp(param2, "OFF", param_len) == 0 ||
    //           strncasecmp(param2, "FALSE", param_len) == 0 ||
    //           strncasecmp(param2, "0", param_len) == 0) {
    //    state = 0;
    //} else {
    //    return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE); //
    //}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	state = param2;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_osc_enable", state);
		reg_write_by_name("signalout1_enable", state);
		set_param_by_name(PARAM_CURRENT,"so_switch_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_osc_enable", state);
		reg_write_by_name("signalout2_enable", state);
		set_param_by_name(PARAM_CURRENT,"so_switch_ch2", 1, pv);
	}

	reg_write_by_name("osc_work_all", 0);
	usleep(100);
	reg_write_by_name("osc_work_all", 1);
	usleep(100);

	if(g_param.g_dev_trig_source == INT) {
		//printf("1.soft trigger\r\n");
		send_soft_trig();
	}else if(g_param.g_dev_trig_source == EXT) {
		/*extern thread handle*/
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_source(scpi_t *context)
{

	uint32_t source;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	source = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_source2", source);
		set_param_by_name(PARAM_CURRENT,"so_source_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_source2", source);
		set_param_by_name(PARAM_CURRENT,"so_source_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_wavemode(scpi_t *context)
{
	uint8_t regs[3];
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	if(param1 == CH1) {
		switch(param2)
		{
			case 0:
				g_param.g_so_range_wave_format.ch1_wave_format = 0x0A;
				set_so_calibration(param1, ID_FASTX_SIN);break;
			case 1:
				g_param.g_so_range_wave_format.ch1_wave_format = 0x05;
				set_so_calibration(param1, ID_FASTX_TTL);break;
			default:
				break;
		}
		set_param_by_name(PARAM_CURRENT,"so_wave_mode_ch1", 1, pv);
	}else if(param1 == CH2) {
		switch(param2)
		{
			case 0:
				g_param.g_so_range_wave_format.ch2_wave_format = 0x0A;
				set_so_calibration(param2, ID_FASTX_SIN);break;
			case 1:
				g_param.g_so_range_wave_format.ch2_wave_format = 0x05;
				set_so_calibration(param2, ID_FASTX_TTL);break;
			default:
				break;
		}
		set_param_by_name(PARAM_CURRENT,"so_wave_mode_ch2", 1, pv);
	}else {

	}
	pack_registers(&g_param.g_so_range_wave_format, regs);

	reg_write_by_name("sine_relays_addr", 0);
	reg_write_by_name("sine_range_relays", regs[0]);
	reg_write_by_name("sine_range_relays_en", 0);
	reg_write_by_name("sine_range_relays_en", 1);
	reg_write_by_name("sine_range_relays_en", 0);

	reg_write_by_name("sine_relays_addr", 1);
	reg_write_by_name("sine_range_relays", regs[1]);
	reg_write_by_name("sine_range_relays_en", 0);
	reg_write_by_name("sine_range_relays_en", 1);
	reg_write_by_name("sine_range_relays_en", 0);

	reg_write_by_name("sinemux_relays_addr", 0);
	reg_write_by_name("sinemux_range_relays", regs[2]);
	reg_write_by_name("sinemux_range_relays_en", 0);
	reg_write_by_name("sinemux_range_relays_en", 1);
	reg_write_by_name("sinemux_range_relays_en", 0);


	return SCPI_RES_OK;

}

static scpi_result_t zwdx_set_so_fast_scale(scpi_t *context)
{
	int32_t param1;
	double  param2,temp;
	uint64_t codevalue;
	ParamValue pv;
	int model = get_device_model();

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	if(model == DEVICE_MODEL_L500K) {
		temp = param2 * sooutput_l500k_get_fast_quant(param1);
	} else {
		temp = param2 * g_so_fastx_verify;
	}
	codevalue = temp * (1ULL << 32);
	if(param1 == CH1) {
		reg_write_by_name("signalout1_scaling1_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout1_scaling1_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_fast_scale_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_scaling1_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout2_scaling1_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"so_fast_scale_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_oscmode(scpi_t *context)
{
	int32_t param1, param2;
	uint32_t oscmode;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	oscmode = param2;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_osc_mode", oscmode);
		set_param_by_name(PARAM_CURRENT,"so_osc_mode_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_osc_mode", oscmode);
		set_param_by_name(PARAM_CURRENT,"so_osc_mode_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_oscrst(scpi_t *context)
{
	int32_t param1, param2;
	uint32_t oscrst;
	AxiDevice *pdev = NULL;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	get_plreg_intance(&pdev);

	oscrst = param2;
	if(param1 == 1) {		//ch1
		//pdev->reg_base[0x0C04] = oscmode;
		reg_write_by_name("signalout1_osc_rst", oscrst);

	}else if(param1 == 2) { //ch2
		//pdev->reg_base[0x0D04] = oscmode;
		reg_write_by_name("signalout2_osc_rst", oscrst);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_fastx_source(scpi_t *context)
{
	int32_t param1, param2;
	uint32_t fastx_source;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	fastx_source = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_source1", fastx_source);
		set_param_by_name(PARAM_CURRENT,"so_fastx_source_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_source1", fastx_source);
		set_param_by_name(PARAM_CURRENT,"so_fastx_source_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_inverted_enable(scpi_t *context)
{
	int32_t param1, param2;
	int32_t inverted;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	inverted = param2;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_inverted_enable", inverted);
		set_param_by_name(PARAM_CURRENT,"so_inverted_en_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout1_inverted_enable", inverted);
		set_param_by_name(PARAM_CURRENT,"so_inverted_en_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_subtract(scpi_t *context)
{
	int32_t param1;
	double	param2;
	uint32_t inverted;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	inverted = param2 / 3.0 * ((1 << 15) - 1);

	if(param1 == 1) {
		reg_write_by_name("signalout1_subtract", inverted);
		set_param_by_name(PARAM_CURRENT,"so_subtract_ch1", 1, pv);
	}else if(param1 == 2) {
		reg_write_by_name("signalout1_subtract", inverted);
		set_param_by_name(PARAM_CURRENT,"so_subtract_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_so_add_enable(scpi_t *context)
{
	int32_t param1,param2;
	uint32_t adden;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	adden = param2;

	if(param1 == CH1) {
		reg_write_by_name("signalout1_add_en", adden);
		set_param_by_name(PARAM_CURRENT,"so_add_en_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout2_add_en", adden);
		set_param_by_name(PARAM_CURRENT,"so_add_en_ch2", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscpmode(scpi_t *context)
{
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (zwdx_write_demo_reg(param1, "oscillators%d_mode", (uint32_t)param2) != 0) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	zwdx_save_demo_param(param1, "demo_osc_phasemode_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscrst(scpi_t *context)
{
	int32_t param1;
	double  param2;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
        return SCPI_RES_ERR;
    }

	if (zwdx_write_demo_reg(param1, "oscillators%d_rst", (uint32_t)param2) != 0) {
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscfreq(scpi_t *context)
{
	double temp;
	uint64_t freq;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!zwdx_valid_demo_channel(param1)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	temp = param2 * pow(2, 48) / zwdx_oscillator_ref_clock_hz();
	freq = (uint64_t)temp;

	if (zwdx_write_demo_reg(param1, "oscillators%d_freq_word_l", (uint32_t)(freq & 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "oscillators%d_freq_word_h", (uint32_t)((freq >> 32)& 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "oscillators%d_en", 1) != 0) {
		return SCPI_RES_ERR;
	}

	g_param.g_demod_param[param1 - 1].osc_freq = param2;
	zwdx_save_demo_param(param1, "demo_osc_freq_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscphase(scpi_t *context)
{
	uint64_t phase_code;
	int32_t param1;
	double  param2, phase_deg;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	if(param2 < 0) {
		phase_deg = param2 + 360;
	} else {
		phase_deg = param2;
	}
	phase_code = ((uint64_t)phase_deg * ((uint64_t)1 << 48)) / 360;

	if (zwdx_write_demo_reg(param1, "oscillators%d_phase_l", (uint32_t)(phase_code & 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "oscillators%d_phase_h", (uint32_t)((phase_code >> 32)& 0xFFFFFFFF)) != 0) {
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscharm(scpi_t *context)
{
	double harm;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!zwdx_valid_demo_channel(param1)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	harm = param2;
	g_param.g_demod_param[param1 - 1].osc_harm = harm;
	zwdx_save_demo_param(param1, "demo_osc_harm_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_enable(scpi_t *context)
{
	uint32_t enable;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	enable = param2;
	if (zwdx_write_demo_reg(param1, "demod%d_enable", enable) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_state_ch%d", pv);

	reg_write_by_name("osc_work_all", 0);
	usleep(100);
	reg_write_by_name("osc_work_all", 1);
	usleep(100);

	if(g_param.g_dev_trig_source == INT) {
		reg_write_by_name("soft_trig", 0);
		usleep(100);
		reg_write_by_name("soft_trig", 1);
		usleep(100);
	    reg_write_by_name("soft_trig", 0);
	}else if(g_param.g_dev_trig_source == EXT) {
		/* external trigger handled separately */
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_source(scpi_t *context)
{
	uint32_t source;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!zwdx_valid_demo_channel(param1)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	source = param2;
	if (zwdx_write_demo_reg(param1, "demod%d_source", source) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_source_ch%d", pv);
	g_param.g_demod_param[param1 - 1].source = param2 ? DEMO_INPUT_CH2 : DEMO_INPUT_CH1;

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_fir_order(scpi_t *context)
{
	uint32_t order;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	order = param2;
	if (zwdx_write_demo_reg(param1, "demod%d_fir_order", order) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_fir_order_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_fir_tc(scpi_t *context)
{
	double tc, param2;
	int32_t param1;
	uint64_t alpha_high, alpha_low;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	tc = param2;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	get_alpha_to_reg(&alpha_high, &alpha_low, tc);

	if (zwdx_write_demo_reg(param1, "demod%d_alpha_l", (uint32_t)(alpha_low & 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_alpha_m", (uint32_t)((alpha_low >> 32) & 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_alpha_h", (uint32_t)(alpha_high & 0xFFFFFFFF)) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_fir_tc_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_sinc_enable(scpi_t *context)
{
	uint32_t enable;
	uint32_t freq_l, freq_h;
	uint64_t freq_word = 0;
	uint32_t freq_in;
	int32_t param1, param2;
	uint32_t m1, m2, nm;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!zwdx_valid_demo_channel(param1)) {
		return SCPI_RES_ERR;
	}

	enable = param2;
	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	if(g_param.g_demod_param[param1 - 1].osc_source == EXT) {
		reg_read_by_name("pll_freq_l", &freq_l);
		reg_read_by_name("pll_freq_h", &freq_h);
		freq_word = ((uint64_t)freq_h << 32) | ((uint64_t)freq_l);
		freq_in = (uint32_t)(freq_word / pow(2, 48) * zwdx_oscillator_ref_clock_hz());
	} else {
		freq_in = (uint32_t)g_param.g_demod_param[param1 - 1].osc_freq;
	}

	sync_filter_params(freq_in, &m1, &m2, &nm);
	printf("freq_word: %llu, freq_in: %u, %u, %u, %u\r\n", freq_word, freq_in, m1, m2, nm);

	if (zwdx_write_demo_reg(param1, "demod%d_sinc_en", enable) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_sinc_dec_ratio_m1", m1) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_sinc_aver_ratio", nm) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_sinc_dec_ratio_m2", m2) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_sinc_dec_ratio_en", 1) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_sinc_dec_ratio_en", 0) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_sinc_state_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_scale(scpi_t *context)
{
	uint64_t code;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	code = (uint64_t)(param2 * pow(2, 32));
	if (zwdx_write_demo_reg(param1, "demod%d_psd_scaling_l", (uint32_t)(code & 0xFFFFFFFF)) != 0 ||
		zwdx_write_demo_reg(param1, "demod%d_psd_scaling_h", (uint32_t)((code >> 32) & 0xFFFFFFFF)) != 0) {
		return SCPI_RES_ERR;
	}
	zwdx_save_demo_param(param1, "demo_scale_ch%d", pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ai_impedance(scpi_t *context)
{
	uint32_t imp, temp_imp;
	int32_t param1;
	int32_t	param2;
	ParamValue pv;
	static char default_imp[] = {0x02, 0x02, 0x02, 0x02};

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	imp = param2;

	if(param1 ==CH1) {
		if(imp == 0) { 	/*50R*/
			default_imp[0] = 0x01;
		}else {			/*1MR*/
			default_imp[0] = 0x02;
		}
		set_param_by_name(PARAM_CURRENT,"ai_impedance_ch1", 1, pv);
	}else if(param1 == CH2) {
		if(imp == 0) { 	/*50R*/
			default_imp[1] = 0x01;
		}else {			/*1MR*/
			default_imp[1] = 0x02;
		}
		set_param_by_name(PARAM_CURRENT,"ai_impedance_ch2", 1, pv);
	}else if(param1 == CH3) {
		if(imp == 0) { 	/*50R*/
			default_imp[2] = 0x01;
		}else {			/*1MR*/
			default_imp[2] = 0x02;
		}
		set_param_by_name(PARAM_CURRENT,"ai_impedance_ch3", 1, pv);
	}else if(param1 == CH4) {
		if(imp == 0) { 	/*50R*/
			default_imp[3] = 0x01;
		}else {			/*1MR*/
			default_imp[3] = 0x02;
		}
		set_param_by_name(PARAM_CURRENT,"ai_impedance_ch4", 1, pv);
	}else {

	}

	temp_imp = default_imp[0] | (default_imp[1] << 2) | (default_imp[2] << 4) | (default_imp[3] << 6);

	reg_write_by_name("auxin_relays_addr", 0);
	reg_write_by_name("auxin_range_relays", temp_imp);
	reg_write_by_name("auxin_range_relays_en", 0);
	reg_write_by_name("auxin_range_relays_en", 1);
	reg_write_by_name("auxin_range_relays_en", 0);

	//trig input impedance, app no gui control, set to 1M by default
	reg_write_by_name("trigin_relays_addr", 0);
	reg_write_by_name("trigin_range_relays", temp_imp);
	reg_write_by_name("trigin_range_relays_en", 0);
	reg_write_by_name("trigin_range_relays_en", 1);
	reg_write_by_name("trigin_range_relays_en", 0);

	return SCPI_RES_OK;
}


static double l500k_si_voltage_range_to_volt(uint8_t range)
{
	switch(range) {
	case L500K_SINGLEINPUT_VRANGE_1MV:
		return 0.001;
	case L500K_SINGLEINPUT_VRANGE_5MV:
		return 0.005;
	case L500K_SINGLEINPUT_VRANGE_10MV:
		return 0.010;
	case L500K_SINGLEINPUT_VRANGE_50MV:
		return 0.050;
	case L500K_SINGLEINPUT_VRANGE_0V1:
		return 0.100;
	case L500K_SINGLEINPUT_VRANGE_0V5:
		return 0.500;
	case L500K_SINGLEINPUT_VRANGE_1V:
		return 1.000;
	case L500K_SINGLEINPUT_VRANGE_5V:
		return 5.000;
	default:
		return 0.0;
	}
}

static const char *zwdx_ao_source_param_name(int32_t channel)
{
	switch(channel) {
	case CH1:
		return "ao_source_ch1";
	case CH2:
		return "ao_source_ch2";
	case CH3:
		return "ao_source_ch3";
	case CH4:
		return "ao_source_ch4";
	default:
		return NULL;
	}
}

static const char *zwdx_ao_scale_param_name(int32_t channel)
{
	switch(channel) {
	case CH1:
		return "ao_scale_ch1";
	case CH2:
		return "ao_scale_ch2";
	case CH3:
		return "ao_scale_ch3";
	case CH4:
		return "ao_scale_ch4";
	default:
		return NULL;
	}
}

static int l500k_get_chout_demod_xyr_range_v(int ao_source, double *range_v)
{
	int demod_index;
	int component;
	int input_ch;

	if(!range_v || ao_source < 0 || ao_source >= DEVICE_DEMOD_NUM_L500K * 4) {
		return -1;
	}

	demod_index = ao_source / 4;
	component = ao_source % 4;
	if(component > 2) {
		return -1;
	}

	if(g_param.g_demod_param[demod_index].source == DEMO_INPUT_CH1) {
		input_ch = 0;
	} else if(g_param.g_demod_param[demod_index].source == DEMO_INPUT_CH2) {
		input_ch = 1;
	} else {
		return -1;
	}

	if(g_param.g_l500k_si_config[input_ch].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
		*range_v = l500k_si_voltage_range_to_volt(g_param.g_l500k_si_config[input_ch].v_range);
		return (*range_v > 0.0) ? 0 : -1;
	} else if(g_param.g_l500k_si_config[input_ch].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
		*range_v = 5.0;
		return 0;
	}

	return -1;
}

static int l500k_chout_source_is_auxin(int ao_source)
{
	const int auxin_start = DEVICE_DEMOD_NUM_L500K * 4;

	return (ao_source >= auxin_start && ao_source < auxin_start + 4);
}

static double zwdx_l500k_ao_scale_to_reg_value(int32_t channel, double scale_value)
{
	const char *source_param_name;
	ParamValue source_value;
	double range_v;

	if(get_device_model() != DEVICE_MODEL_L500K) {
		return scale_value;
	}

	source_param_name = zwdx_ao_source_param_name(channel);
	if(!source_param_name ||
	   get_param_by_name(PARAM_CURRENT, source_param_name, 1, &source_value) != 0) {
		return chout_l500k_apply_k2((unsigned)(channel - 1), scale_value);
	}

	if(l500k_chout_source_is_auxin(source_value.value.i32)) {
		return chout_l500k_apply_k2((unsigned)(channel - 1), scale_value * (3.0 / 2.86));//have bug
	}

	if(l500k_get_chout_demod_xyr_range_v(source_value.value.i32, &range_v) != 0) {
		return chout_l500k_apply_k2((unsigned)(channel - 1), scale_value);
	}

	return chout_l500k_apply_range_k2((unsigned)(channel - 1), scale_value, range_v);
}

static void zwdx_write_ao_scale_regs(int32_t channel, uint64_t scale)
{
	if(channel == CH1) {
		reg_write_by_name("auxout1_scaling_l", (uint32_t)(scale & 0xFFFFFFFF));
		reg_write_by_name("auxout1_scaling_h", (uint32_t)((scale >> 32) & 0xFFFFFFFF));
	} else if(channel == CH2) {
		reg_write_by_name("auxout2_scaling_l", (uint32_t)(scale & 0xFFFFFFFF));
		reg_write_by_name("auxout2_scaling_h", (uint32_t)((scale >> 32) & 0xFFFFFFFF));
	} else if(channel == CH3) {
		reg_write_by_name("auxout3_scaling_l", (uint32_t)(scale & 0xFFFFFFFF));
		reg_write_by_name("auxout3_scaling_h", (uint32_t)((scale >> 32) & 0xFFFFFFFF));
	} else if(channel == CH4) {
		reg_write_by_name("auxout4_scaling_l", (uint32_t)(scale & 0xFFFFFFFF));
		reg_write_by_name("auxout4_scaling_h", (uint32_t)((scale >> 32) & 0xFFFFFFFF));
	}
}

static void zwdx_refresh_ao_scale_from_param(int32_t channel)
{
	const char *scale_param_name = zwdx_ao_scale_param_name(channel);
	ParamValue scale_value;
	double reg_value;
	uint64_t scale;

	if(!scale_param_name ||
	   get_param_by_name(PARAM_CURRENT, scale_param_name, 1, &scale_value) != 0) {
		return;
	}

	reg_value = zwdx_l500k_ao_scale_to_reg_value(channel, scale_value.value.d);
	scale = (uint64_t)(reg_value * (1ULL << 32));
	zwdx_write_ao_scale_regs(channel, scale);
}

static scpi_result_t zwdx_set_ao_source(scpi_t *context)
{
	uint32_t source;
	int32_t param1, param2;
	ParamValue pv;
	const char *param_name;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	source = param2 + 1;
	param_name = zwdx_ao_source_param_name(param1);

	if(param1 == CH1) {
		reg_write_by_name("auxout1_source", source);
	}else if(param1 == CH2) {
		reg_write_by_name("auxout2_source", source);
	}else if(param1 == CH3) {
		reg_write_by_name("auxout3_source", source);
	}else if(param1 == CH4) {
		reg_write_by_name("auxout4_source", source);
	}else {

	}
	if(param_name) {
		set_param_by_name(PARAM_CURRENT, param_name, 1, pv);
		zwdx_refresh_ao_scale_from_param(param1);
	}
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_dec(scpi_t *context)
{
	uint32_t dec;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	dec = param2 * 8;

	if(param1 == CH1) {
		reg_write_by_name("auxout1_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"ao_dec_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("auxout2_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"ao_dec_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("auxout3_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"ao_dec_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("auxout4_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"ao_dec_ch4", 1, pv);
	}else {

	}
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_scale(scpi_t *context)
{
	uint64_t scale;
	int32_t param1;
	double param2,temp;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	temp = zwdx_l500k_ao_scale_to_reg_value(param1, param2);
	scale = (uint64_t)(temp * (1ULL << 32));

	if(param1 == CH1) {
		zwdx_write_ao_scale_regs(param1, scale);
		set_param_by_name(PARAM_CURRENT,"ao_scale_ch1", 1, pv);
	}else if(param1 == CH2) {
		zwdx_write_ao_scale_regs(param1, scale);
		set_param_by_name(PARAM_CURRENT,"ao_scale_ch2", 1, pv);
	}else if(param1 == CH3) {
		zwdx_write_ao_scale_regs(param1, scale);
		set_param_by_name(PARAM_CURRENT,"ao_scale_ch3", 1, pv);
	}else if(param1 == CH4) {
		zwdx_write_ao_scale_regs(param1, scale);
		set_param_by_name(PARAM_CURRENT,"ao_scale_ch4", 1, pv);
	}else {

	}
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_offset(scpi_t *context)
{
	int32_t offset;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	if(get_device_model() == DEVICE_MODEL_L500K) {
		param2 = chout_l500k_apply_b((unsigned)(param1 - 1), param2);
	}
	offset = param2 / 10.0 * ((1 << 19) - 1);

	if(param1 == CH1) {
		reg_write_by_name("auxout1_offset", offset);
		set_param_by_name(PARAM_CURRENT,"ao_offset_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("auxout2_offset", offset);
		set_param_by_name(PARAM_CURRENT,"ao_offset_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("auxout3_offset", offset);
		set_param_by_name(PARAM_CURRENT,"ao_offset_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("auxout4_offset", offset);
		set_param_by_name(PARAM_CURRENT,"ao_offset_ch4", 1, pv);
	}else {

	}
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_enable(scpi_t *context)
{
	uint32_t addenable;
	int32_t param1, param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	addenable = param2;

	if(param1 == CH1) {
		reg_write_by_name("auxout1_en", addenable);
		set_param_by_name(PARAM_CURRENT,"ao_state_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("auxout2_en", addenable);
		set_param_by_name(PARAM_CURRENT,"ao_state_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("auxout3_en", addenable);
		set_param_by_name(PARAM_CURRENT,"ao_state_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("auxout4_en", addenable);
		set_param_by_name(PARAM_CURRENT,"ao_state_ch4", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_upper(scpi_t *context)
{
	uint32_t sendvalue;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	sendvalue = (uint32_t)(param2 / 10.0 * ((1 << 19) - 1));
	//printf("xxx: %d, yyy: %u\r\n", temp, sendvalue);

	if(param1 == CH1) {
		reg_write_by_name("auxout1_upper_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_upper_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("auxout2_upper_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_upper_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("auxout3_upper_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_upper_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("auxout4_upper_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_upper_ch4", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_lower(scpi_t *context)
{
	int32_t sendvalue;
	int32_t param1;
	double  param2;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	sendvalue = param2 / 10.0 * ((1 << 19) - 1);
	//printf("yyy: %d->%.f\r\n", sendvalue, (param2 / 10.0 * ((1 << 19) - 1)));

	if(param1 == 1) {	   //ch1
		reg_write_by_name("auxout1_lower_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_lower_ch1", 1, pv);
	}else if(param1 == 2) {//ch2
		reg_write_by_name("auxout2_lower_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_lower_ch2", 1, pv);
	}else if(param1 == 3) {
		reg_write_by_name("auxout3_lower_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_lower_ch3", 1, pv);
	}else if(param1 == 4) {
		reg_write_by_name("auxout4_lower_limit", sendvalue);
		set_param_by_name(PARAM_CURRENT,"ao_lower_ch4", 1, pv);
	}else {

	}

	return SCPI_RES_OK;
}

static const char *zwdx_ao_dcvalue_param_name(int32_t channel)
{
	if(channel == CH1) {
		return "ao_dcvalue_ch1";
	} else if(channel == CH2) {
		return "ao_dcvalue_ch2";
	} else if(channel == CH3) {
		return "ao_dcvalue_ch3";
	} else if(channel == CH4) {
		return "ao_dcvalue_ch4";
	}
	return NULL;
}

static int zwdx_refresh_ao_dcvol_from_param(int32_t channel)
{
	ParamValue getpv;
	const char *param_name = zwdx_ao_dcvalue_param_name(channel);
	double dcvol;
	int32_t sendvalue;
	int model = get_device_model();

	if(!param_name || get_param_by_name(PARAM_CURRENT, param_name, 1, &getpv) != 0) {
		return -1;
	}

	if(getpv.type == PARAM_DOUBLE) {
		dcvol = getpv.value.d;
	} else if(getpv.type == PARAM_INT32) {
		dcvol = (double)getpv.value.i32;
	} else if(getpv.type == PARAM_UINT32) {
		dcvol = (double)getpv.value.u32;
	} else {
		return -1;
	}

	if(model == DEVICE_MODEL_L500K) {
		dcvol = auxout_l500k_calibrate_dcvol((unsigned)(channel - 1), dcvol);
	} else if((model == DEVICE_MODEL_M005M_VV) ||
			  (model == DEVICE_MODEL_M005M_VI) ||
			  (model == DEVICE_MODEL_M005M)) {
	}
	sendvalue = dcvol / 10.0 * ((1 << 19) - 1);

	if ((model == DEVICE_MODEL_M005M_VV) ||
		(model == DEVICE_MODEL_M005M_VI) ||
		(model == DEVICE_MODEL_M005M)) {
		if(channel == CH1) {
			reg_write_by_name("auxout1_con", sendvalue);
		}else if(channel == CH2) {
			reg_write_by_name("auxout2_con", sendvalue);
		}else if(channel == CH3) {
			reg_write_by_name("auxout3_con", sendvalue);
		}else if(channel == CH4) {
			reg_write_by_name("auxout4_con", sendvalue);
		}else {
			return -1;
		}
	} else if(model == DEVICE_MODEL_L500K) {
		if(channel == CH1) {
			reg_write_by_name("choutdc1_value", sendvalue);
			reg_write_by_name("chout1_en", 0);
			reg_write_by_name("chout1_en", 1);
			reg_write_by_name("chout1_en", 0);
		}else if(channel == CH2) {
			reg_write_by_name("choutdc2_value", sendvalue);
			reg_write_by_name("chout2_en", 0);
			reg_write_by_name("chout2_en", 1);
			reg_write_by_name("chout2_en", 0);
		}else if(channel == CH3) {
			reg_write_by_name("choutdc3_value", sendvalue);
			reg_write_by_name("chout3_en", 0);
			reg_write_by_name("chout3_en", 1);
			reg_write_by_name("chout3_en", 0);
		}else if(channel == CH4) {
			reg_write_by_name("choutdc4_value", sendvalue);
			reg_write_by_name("chout4_en", 0);
			reg_write_by_name("chout4_en", 1);
			reg_write_by_name("chout4_en", 0);
		}else {
			return -1;
		}
	} else {
		return -1;
	}

	return 0;
}

static scpi_result_t zwdx_set_ao_dcvol(scpi_t *context)
{
	int32_t sendvalue;
	int32_t param1;
	double  param2;
	ParamValue pv;
	int model = get_device_model();

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	if(model == DEVICE_MODEL_L500K) {
		param2 = auxout_l500k_calibrate_dcvol((unsigned)(param1 - 1), param2);
	} else if((model == DEVICE_MODEL_M005M_VV) ||
			  (model == DEVICE_MODEL_M005M_VI) ||
			  (model == DEVICE_MODEL_M005M)) {
	}
	sendvalue = param2 / 10.0 * ((1 << 19) - 1);
	printf("DC-VOL: %d\r\n", sendvalue);

	if ((model == DEVICE_MODEL_M005M_VV) ||
		(model == DEVICE_MODEL_M005M_VI) ||
		(model == DEVICE_MODEL_M005M)) {
		if(param1 == CH1) {
			reg_write_by_name("auxout1_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch1", 1, pv);
		}else if(param1 == CH2) {
			reg_write_by_name("auxout2_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch2", 1, pv);
		}else if(param1 == CH3) {
			reg_write_by_name("auxout3_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch3", 1, pv);
		}else if(param1 == CH4) {
			reg_write_by_name("auxout4_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch4", 1, pv);
		} else {

		}
	}else if(model == DEVICE_MODEL_L500K) {
		//BUG:这里配置的值不一样才会刷��?
		if(param1 == CH1) {
			reg_write_by_name("choutdc1_value", sendvalue);
			reg_write_by_name("chout1_en", 0); //refersh value
			reg_write_by_name("chout1_en", 1);
			reg_write_by_name("chout1_en", 0);
			//auxout temp output dc
			// reg_write_by_name("auxout1_dcen", 1);
			// reg_write_by_name("auxout1_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch1", 1, pv);
		}else if(param1 == CH2) {
			reg_write_by_name("choutdc2_value", sendvalue);
			reg_write_by_name("chout2_en", 0); //refersh value
			reg_write_by_name("chout2_en", 1);
			reg_write_by_name("chout2_en", 0);
			//auxout temp output dc
			// reg_write_by_name("auxout2_dcen", 1);
			// reg_write_by_name("auxout2_con", sendvalue);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch2", 1, pv);
		}else if(param1 == CH3) {
			reg_write_by_name("choutdc3_value", sendvalue);
			reg_write_by_name("chout3_en", 0); //refersh value
			reg_write_by_name("chout3_en", 1);
			reg_write_by_name("chout3_en", 0);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch3", 1, pv);
		}else if(param1 == CH4) {
			reg_write_by_name("choutdc4_value", sendvalue);
			reg_write_by_name("chout4_en", 0); //refersh value
			reg_write_by_name("chout4_en", 1);
			reg_write_by_name("chout4_en", 0);
			set_param_by_name(PARAM_CURRENT,"ao_dcvalue_ch4", 1, pv);
		} else {

		}
	} else {

	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ao_dcvol_switch(scpi_t *context)
{
	int32_t enable;
	int32_t param1, param2;
	ParamValue pv;
	int model = get_device_model();

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param2;
	enable = param2;

	if ((model == DEVICE_MODEL_M005M_VV) ||
		(model == DEVICE_MODEL_M005M_VI) ||
		(model == DEVICE_MODEL_M005M)) {

		if(param1 == CH1) {
			reg_write_by_name("auxout1_dcen", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch1", 1, pv);
		}else if(param1 == CH2) {
			reg_write_by_name("auxout2_dcen", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch2", 1, pv);
		}else if(param1 == CH3) {
			reg_write_by_name("auxout3_dcen", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch3", 1, pv);
		}else if(param1 == CH4) {
			reg_write_by_name("auxout4_dcen", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch4", 1, pv);
		}else {

		}
		if(enable && zwdx_refresh_ao_dcvol_from_param(param1) != 0) {
			return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		}
	}else if(model == DEVICE_MODEL_L500K) {
		if(param1 == CH1) {
			reg_write_by_name("choutdc1_switch", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch1", 1, pv);
		}else if(param1 == CH2) {
			reg_write_by_name("choutdc2_switch", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch2", 1, pv);
		}else if(param1 == CH3) {
			reg_write_by_name("choutdc3_switch", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch3", 1, pv);
		}else if(param1 == CH4) {
			reg_write_by_name("choutdc4_switch", enable);
			set_param_by_name(PARAM_CURRENT,"ao_dcswitch_ch4", 1, pv);
		}else {

		}
		if(enable && zwdx_refresh_ao_dcvol_from_param(param1) != 0) {
			return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
		}
	}else {

	}

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_daq_sample_rate(scpi_t *context)
{
	uint32_t sample_rate_index;
	uint32_t decimation_value;
	double daq_input_rate_hz;
	double target_sample_rate_hz;
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	sample_rate_index = param1;
	/* The host sends the combo-box index:
	 *   index 0 -> 2 MHz, index 1 -> 1 MHz, index 2 -> 500 kHz, ...
	 * M005M DAQ decimation starts from 40 MHz.
	 * L500K has already decimated 40 MHz to 2 MHz before this stage.
	 */
	daq_input_rate_hz = (get_device_model() == DEVICE_MODEL_L500K) ? 2000000.0 : 40000000.0;
	target_sample_rate_hz = 2000000.0 / (1U << sample_rate_index);
	decimation_value = (uint32_t)(daq_input_rate_hz / target_sample_rate_hz);

	reg_write_by_name("ch1_daq_decimation_value", decimation_value);
	reg_write_by_name("ch2_daq_decimation_value", decimation_value);
	set_param_by_name(PARAM_CURRENT,"daq_sample_rate", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_daq_data_source(scpi_t *context)
{
	uint64_t source;
	uint64_t param1;
	ParamValue pv,getpv;

	if (!SCPI_ParamUInt64(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	g_param.g_daq_scope_mode = 1;

	pv.type = PARAM_UINT64;
	pv.value.u64 = param1;

	printf("source :0x%016" PRIx64 "\r\n", param1);
	source = param1;

	//set source
	reg_write_by_name("ch1_daq_up_data_mux_l", source & 0xFFFFFFFF);//only use low 32bit
	reg_write_by_name("ch1_daq_up_data_mux_h",( source >> 32) & 0xFFFFFFFF);
	reg_write_by_name("ch2_daq_up_data_mux_l", source & 0xFFFFFFFF);//only use low 32bit
	reg_write_by_name("ch2_daq_up_data_mux_h",( source >> 32) & 0xFFFFFFFF);

	//update local param
	set_param_by_name(PARAM_CURRENT,"daq_source", 0, pv);

	return SCPI_RES_OK;
}

static uint32_t trig_source_from_compact_m005m(int32_t param)
{
	uint32_t source = (uint32_t)param + TS_M005M_1_VOL;

	if(source == TS_M005M_37_SIGNALOUT1_PHASE) {
		return TS_M005M_40_TRIGIN1;
	}
	if(source == TS_M005M_38_SIGNALOUT2_PHASE) {
		return TS_M005M_41_TRIGIN2;
	}

	return source;
}

static uint32_t trig_source_from_compact_l500k(int32_t param)
{
	uint32_t source = (uint32_t)param + TS_L500K_1_VOL;

	if(source >= TS_L500K_49_SIGNALOUT1_PHASE &&
	   source <= TS_L500K_52_TRIGIN1) {
		return source + 3;
	}

	return source;
}

static uint32_t trig_source_from_compact_by_model(int32_t param)
{
	if(get_device_model() == DEVICE_MODEL_L500K) {
		return trig_source_from_compact_l500k(param);
	}

	return trig_source_from_compact_m005m(param);
}

static scpi_result_t zwdx_set_daq_run_en(scpi_t *context)
{

	int32_t param1;
	g_param.g_daq_scope_mode = 1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"daq_run", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_daq_signal_en(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"daq_signal", 0, pv);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_daq_adata_query(scpi_t *context)
{
	DemoDaqData *pdemodata = NULL;
	int32_t inputsrc; //0: DAQ 1: SCOPE
	ParamValue status, rtnstatus;
	status.type = PARAM_INT32;

	if (!SCPI_ParamInt32(context, &inputsrc, TRUE)) {
		return SCPI_RES_ERR;
	}

	if(!inputsrc) { //DAQ
		status.value.i32 = 1;
		set_param_by_name(PARAM_CURRENT,"daq_signal", 0, status);
		usleep(10000);
		for(;;) {
			send_soft_trig();
			get_param_by_name(PARAM_CURRENT,"daq_signal", 0, &rtnstatus);
			if(!rtnstatus.value.i32) {
				//printf("daq loop exit...\r\n");
				break;
			}
			usleep(1000);
		}
	} else { //SCOPE
		status.value.i32 = 1;
		set_param_by_name(PARAM_CURRENT,"scope_signal", 0, status);
		usleep(10000);
		for(;;) {
			send_soft_trig();
			get_param_by_name(PARAM_CURRENT,"scope_signal", 0, &rtnstatus);
			if(!rtnstatus.value.i32) {
				//printf("scope loop exit...\r\n");
				break;
			}
			usleep(1000);
		}
	}

	get_demo_adata_instance(&pdemodata);

	SCPI_ResultArrayDouble(context, (double*)pdemodata, sizeof(DemoDaqData)/sizeof(double), SCPI_FORMAT_ASCII);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_daq_rdata_query(scpi_t *context)
{
	DemoDaqData *pdemordata = NULL;

	get_demo_rdata_instance(&pdemordata);

	read_demo_rdata(pdemordata);

	SCPI_ResultArrayDouble(context, (double*)pdemordata, sizeof(DemoDaqData)/sizeof(double), SCPI_FORMAT_ASCII);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_frame_data_query(scpi_t *context)
{
	int32_t channel_id;
	uint8_t *frame_data = NULL;
	size_t frame_len = 0;
	uint32_t frame_id = 0;
	FrameRingBuffer *frb = NULL;
	bool has_frame;

	if (!SCPI_ParamInt32(context, &channel_id, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel_id < 0 || channel_id >= CH_MAX) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	frame_data = (uint8_t *)malloc(FRAME_SIZE);
	if(!frame_data) {
		return SCPI_Error(context, SCPI_ERROR_EXECUTION_ERROR);
	}

	get_rb_signle_instance(&frb, (DataCh)channel_id);
	has_frame = frame_buffer_read(frb, frame_data, &frame_len, &frame_id);
	if(!has_frame) {
		P_LOG_DEBUG("SCPI frame data query: request_ch=%d, no frame\r\n", channel_id);
		SCPI_ResultArbitraryBlock(context, "", 0);
		free(frame_data);
		return SCPI_RES_OK;
	}

	P_LOG_DEBUG("SCPI frame data query: request_ch=%d, frame_id=%u, len=%u\r\n",
		channel_id, frame_id, (unsigned int)frame_len);
	SCPI_ResultArbitraryBlock(context, frame_data, frame_len);
	free(frame_data);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_scope_sample_rate(scpi_t *context)
{
	uint32_t sample_rate_index;
	uint32_t decimation_value;
	int32_t channel, value;

	if (!SCPI_ParamInt32(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = value;

	sample_rate_index = value;
	decimation_value = (1U << sample_rate_index);

	if(channel == CH1) {
		reg_write_by_name("ch1_scope_decimation_value", decimation_value);
		set_param_by_name(PARAM_CURRENT,"scope_sample_ch1", 1, pv);
	}else if(channel == CH2) {
		reg_write_by_name("ch2_scope_decimation_value", decimation_value);
		set_param_by_name(PARAM_CURRENT,"scope_sample_ch2", 1, pv);
	}

	reg_write_by_name("ch1_scpoe_up_en", 1);
	reg_write_by_name("ch1_scpoe_up_en", 0);

	reg_write_by_name("ch2_scpoe_up_en", 1);
	reg_write_by_name("ch2_scpoe_up_en", 0);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_scope_data_source(scpi_t *context)
{
	uint32_t source;
	int32_t param1;
	int32_t param2;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}
	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	source = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("ch1_scope_up_data_mux", source);
		set_param_by_name(PARAM_CURRENT,"scope_source_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("ch2_scope_up_data_mux", source);
		set_param_by_name(PARAM_CURRENT,"scope_source_ch2", 1, pv);
	}
	g_param.g_scope_source = param2;

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_scope_ch_switch(scpi_t *context)
{
	int32_t param1, param2;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param2;

	if(param1 == CH1) {
		set_param_by_name(PARAM_CURRENT,"scope_ch_switch_ch1", 1, pv);
	}else if(param1 == CH2) {
		set_param_by_name(PARAM_CURRENT,"scope_ch_switch_ch2", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_scope_run_en(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"scope_run", 0, pv);

	return SCPI_RES_OK;

}

static scpi_result_t zwdx_set_scope_signal_en(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"scope_signal", 0, pv);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_scope_average_en(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	reg_write_by_name("ch1_scpoe_average_en", param1);
	reg_write_by_name("ch2_scpoe_average_en", param1);
	set_param_by_name(PARAM_CURRENT,"scope_average_en", 0, pv);

	return SCPI_RES_OK;

}


void set_scope_sample_number()
{
	AxiDevice *pdev = NULL;
	get_plreg_intance(&pdev);

	if(g_param.g_scope_chopenst == 1) {
		pdev->reg_base[0x1A0C] = (g_param.g_sample_number / (32 / 4)) / 2;
		pdev->reg_base[0x1A0D] = (g_param.g_sample_number / (32 / 4)) / 2;
	}else if(g_param.g_scope_chopenst == 2) {
		pdev->reg_base[0x1A0C] = (g_param.g_sample_number / (32 / 4)) / 2;
		pdev->reg_base[0x1A0D] = (g_param.g_sample_number / (32 / 4)) / 2;
	}else if(g_param.g_scope_chopenst == 3) {
		pdev->reg_base[0x1A0C] = ((g_param.g_sample_number * 2) / (32 / 4)) / 2;
		pdev->reg_base[0x1A0D] = ((g_param.g_sample_number * 2) / (32 / 4)) / 2;
	}else {
		printf("no action...\r\n");//��Singel��ʱ����
	}
}

static scpi_result_t zwdx_set_pwa_signal_en(scpi_t *context)
{
	int32_t param1;
	uint32_t so_phase_trig_source;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	if(g_device_model == DEVICE_MODEL_M005M
		|| g_device_model == DEVICE_MODEL_M005M_VV
		|| g_device_model == DEVICE_MODEL_M005M_VI) {
		so_phase_trig_source = TS_M005M_37_SIGNALOUT1_PHASE;
	}else if(g_device_model == DEVICE_MODEL_L500K) {
		so_phase_trig_source = TS_L500K_49_SIGNALOUT1_PHASE;
	}

	reg_write_by_name("trig_source", so_phase_trig_source);

	reg_write_by_name("hysteresis_range_high_l", 0);
	reg_write_by_name("hysteresis_range_high_h", 0x3FFFFFFF);
	reg_write_by_name("hysteresis_range_low_l", 0);
	reg_write_by_name("hysteresis_range_low_h", 0x3BFFFFFF);

	set_param_by_name(PARAM_CURRENT,"pwa_signal", 0, pv);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_trig_sample_len(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	set_param_by_name(PARAM_CURRENT,"trig_smplen", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_times(scpi_t *context)
{
	uint32_t trigcnt;
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	trigcnt = param1;

	reg_write_by_name("ch1_storag_cnt", trigcnt);
	reg_write_by_name("ch2_storag_cnt", trigcnt);

	set_param_by_name(PARAM_CURRENT,"trig_times", 0, pv);


	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_source(scpi_t *context)
{
	uint32_t trigsource;
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if(param1 < 0) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	trigsource = trig_source_from_compact_by_model(param1);

	reg_write_by_name("trig_source", trigsource);
	set_param_by_name(PARAM_CURRENT,"trig_source", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_type(scpi_t *context)
{
	uint32_t trigtype;
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	trigtype = param1;

	reg_write_by_name("trig_mode", trigtype);
	set_param_by_name(PARAM_CURRENT,"trig_type", 0, pv);

	return SCPI_RES_OK;
}

static int physical_value_to_raw_code_by_trig_source(int trig_source, double value, int64_t *raw_code);

static int write_trig_level_window(double level, double hyst)
{
	ParamValue getpv;
	int trig_source;
	int64_t max_raw, min_raw;
	uint64_t max_bits, min_bits;

	if(get_param_by_name(PARAM_CURRENT,"trig_source", 0, &getpv) != 0) {
		P_LOG_ERROR("Failed to get trig_source parameter\r\n");
		return -1;
	}

	trig_source = trig_source_from_compact_by_model(getpv.value.i32);
	if(physical_value_to_raw_code_by_trig_source(trig_source, level + hyst, &max_raw) != 0 ||
	   physical_value_to_raw_code_by_trig_source(trig_source, level - hyst, &min_raw) != 0) {
		P_LOG_DEBUG("Skip trigger level raw-code conversion for unsupported trig_source %d, level %.3f, hyst %.3f\r\n",
			trig_source, level, hyst);
		return 0;
	}
	P_LOG_DEBUG("trig_source: %d, level: %.3f, hyst: %.3f, max_raw: 0x%016llx, min_raw: 0x%016llx\r\n",
		trig_source, level, hyst, (unsigned long long)max_raw, (unsigned long long)min_raw);
	max_bits = (uint64_t)max_raw;
	min_bits = (uint64_t)min_raw;
	reg_write_by_name("hysteresis_range_high_l", max_bits & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_high_h", (max_bits >> 32) & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_low_l", min_bits & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_low_h", (min_bits >> 32) & 0xFFFFFFFF);
	return 0;
}

static scpi_result_t zwdx_set_trig_level(scpi_t *context)
{
	double param1;
	double trig_hys;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue setpv, getpv;
	setpv.type = PARAM_DOUBLE;
	setpv.value.d = param1;

	get_param_by_name(PARAM_CURRENT,"trig_hysteresis", 0, &getpv);
	trig_hys = getpv.value.d;
	if(write_trig_level_window(param1, trig_hys) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	set_param_by_name(PARAM_CURRENT,"trig_level", 0, setpv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_hyst(scpi_t *context)
{
	double param1, trig_level;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue setpv, getpv;
	setpv.type = PARAM_DOUBLE;
	setpv.value.d = param1;

	get_param_by_name(PARAM_CURRENT,"trig_level", 0, &getpv);
	trig_level = getpv.value.d;
	if(write_trig_level_window(trig_level, param1) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	set_param_by_name(PARAM_CURRENT,"trig_hysteresis", 0, setpv);
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_holdoff(scpi_t *context)
{
	uint32_t hold;
	double param1;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;
	hold = param1;

	reg_write_by_name("trig_release_restrain", hold);
	set_param_by_name(PARAM_CURRENT,"trig_hold_off", 0, pv);


	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_trig_delay(scpi_t *context)
{
	uint32_t trigdelay;
	double param1;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;
	trigdelay = (param1 * 1e9) / 25;

	reg_write_by_name("trig_delay", trigdelay);
	set_param_by_name(PARAM_CURRENT,"trig_delay", 0, pv);

	printf("trig delay: %d\r\n", trigdelay);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_pw(scpi_t *context)
{
	uint32_t pluse_width;
	int32_t param1;
	AxiDevice *pdev = NULL;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	pluse_width = param1;

	reg_write_by_name("trig_plus_width", pluse_width);
	set_param_by_name(PARAM_CURRENT,"trig_pluse_width", 0, pv);

	return SCPI_RES_OK;
}



static scpi_result_t zwdx_set_trig_force(scpi_t *context)
{
	int32_t param1;
	//printf("scope software trig...\r\n");

	SCPI_ParamInt32(context, &param1, FALSE);

	send_soft_trig();

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_p(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint64_t pid_p;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	pid_p = (uint64_t)(param2 * (double)(1ULL << 48));

	if(param1 == CH1) {
		reg_write_by_name("pid1_Kp_l", (uint32_t)(pid_p & 0xFFFFFFFF));
		reg_write_by_name("pid1_Kp_h", (uint32_t)((pid_p >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_p_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_Kp_l", (uint32_t)(pid_p & 0xFFFFFFFF));
		reg_write_by_name("pid2_Kp_h", (uint32_t)((pid_p >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_p_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_Kp_l", (uint32_t)(pid_p & 0xFFFFFFFF));
		reg_write_by_name("pid3_Kp_h", (uint32_t)((pid_p >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_p_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_Kp_l", (uint32_t)(pid_p & 0xFFFFFFFF));
		reg_write_by_name("pid4_Kp_h", (uint32_t)((pid_p >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_p_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_i(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint64_t pid_i;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	pid_i = (uint64_t)(param2 * (double)(1ULL << 48));

	if(param1 == CH1) {
		reg_write_by_name("pid1_Ki_l", (uint32_t)(pid_i & 0xFFFFFFFF));
		reg_write_by_name("pid1_Ki_h", (uint32_t)((pid_i >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_i_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_Ki_l", (uint32_t)(pid_i & 0xFFFFFFFF));
		reg_write_by_name("pid2_Ki_h", (uint32_t)((pid_i >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_i_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_Ki_l", (uint32_t)(pid_i & 0xFFFFFFFF));
		reg_write_by_name("pid3_Ki_h", (uint32_t)((pid_i >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_i_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_Ki_l", (uint32_t)(pid_i & 0xFFFFFFFF));
		reg_write_by_name("pid4_Ki_h", (uint32_t)((pid_i >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_i_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_d(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint64_t pid_d;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	pid_d = (uint64_t)(param2 * (double)(1ULL << 48));

	if(param1 == CH1) {
		reg_write_by_name("pid1_Kd_l", (uint32_t)(pid_d & 0xFFFFFFFF));
		reg_write_by_name("pid1_Kd_h", (uint32_t)((pid_d >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_d_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_Kd_l", (uint32_t)(pid_d & 0xFFFFFFFF));
		reg_write_by_name("pid2_Kd_h", (uint32_t)((pid_d >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_d_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_Kd_l", (uint32_t)(pid_d & 0xFFFFFFFF));
		reg_write_by_name("pid3_Kd_h", (uint32_t)((pid_d >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_d_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_Kd_l", (uint32_t)(pid_d & 0xFFFFFFFF));
		reg_write_by_name("pid4_Kd_h", (uint32_t)((pid_d >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_d_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_ki_en(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint32_t kid_en;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	kid_en = (uint32_t)param2;

	if(param1 == CH1) {
		reg_write_by_name("pid1_ki_enable", kid_en);
		set_param_by_name(PARAM_CURRENT,"pid_ki_en_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_ki_enable", kid_en);
		set_param_by_name(PARAM_CURRENT,"pid_ki_en_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_ki_enable", kid_en);
		set_param_by_name(PARAM_CURRENT,"pid_ki_en_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_ki_enable", kid_en);
		set_param_by_name(PARAM_CURRENT,"pid_ki_en_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_filter(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint32_t filter;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	filter = (uint32_t)param2;

	if(param1 == CH1) {
		reg_write_by_name("pid1_moving_aver_ratio", filter);
		set_param_by_name(PARAM_CURRENT,"pid_filter_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_moving_aver_ratio", filter);
		set_param_by_name(PARAM_CURRENT,"pid_filter_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_moving_aver_ratio", filter);
		set_param_by_name(PARAM_CURRENT,"pid_filter_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_moving_aver_ratio", filter);
		set_param_by_name(PARAM_CURRENT,"pid_filter_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static int64_t round_to_i64(double value)
{
	return (value >= 0.0) ? (int64_t)(value + 0.5) : (int64_t)(value - 0.5);
}

static int raw_code_from_aux_offset_20bit(double value, int64_t *raw_code)
{
	double code;

	if(!raw_code) {
		return -1;
	}

	/* Reverse of datatask.c/scope_aux_like_to_voltage() and load_double_swap32()
	 * AUXIN/AUXOUT decoding: lower 20-bit unsigned code maps to -10 V..+10 V.
	 */
	code = (value + 10.0) * (double)((1ULL << 20) - 1) / 20.0;
	if(code < 0.0) {
		code = 0.0;
	} else if(code > (double)((1ULL << 20) - 1)) {
		code = (double)((1ULL << 20) - 1);
	}
	*raw_code = round_to_i64(code);
	return 0;
}

static int raw_code_from_signed16_bipolar(double value, double full_scale, int64_t *raw_code)
{
	double code;

	if(!raw_code || full_scale <= 0.0) {
		return -1;
	}

	/* Reverse of datatask.c/load_double() for sign-extended 16-bit sources:
	 * value = signed_raw * full_scale / ((1 << 15) - 1).
	 */
	code = value * (double)((1ULL << 15) - 1) / full_scale;
	if(code < -32768.0) {
		code = -32768.0;
	} else if(code > 32767.0) {
		code = 32767.0;
	}
	*raw_code = round_to_i64(code);
	return 0;
}

static int get_signal_input_map_item(int input_ch, const MapItem **map_item)
{
	gLobalParam *glp = NULL;
	MappingTable *cvmt = NULL;
	int range_index;

	if(!map_item || input_ch < 0 || input_ch > 1) {
		return -1;
	}
	*map_item = NULL;

	get_global_param(&glp);
	get_cv_maptable(&cvmt);
	if(!glp || !cvmt) {
		return -1;
	}

	if(g_device_model == DEVICE_MODEL_M005M) {
		if(glp->g_si_config[input_ch].range < 1) {
			return -1;
		}
		range_index = glp->g_si_config[input_ch].range - 1;
		if(glp->g_si_config[input_ch].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
			if((input_ch == 0 && range_index >= cvmt->voltage1_count) ||
			   (input_ch == 1 && range_index >= cvmt->voltage2_count)) {
				return -1;
			}
			*map_item = (input_ch == 0) ? &cvmt->voltage1[range_index] : &cvmt->voltage2[range_index];
		} else if(glp->g_si_config[input_ch].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
			if(range_index >= cvmt->current_count) {
				return -1;
			}
			*map_item = &cvmt->current[range_index];
		}
	} else if(g_device_model == DEVICE_MODEL_L500K) {
		if(glp->g_l500k_si_config[input_ch].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
			if(glp->g_l500k_si_config[input_ch].v_range < 1) {
				return -1;
			}
			range_index = glp->g_l500k_si_config[input_ch].v_range - 1;
			if((input_ch == 0 && range_index >= cvmt->voltage1_count) ||
			   (input_ch == 1 && range_index >= cvmt->voltage2_count)) {
				return -1;
			}
			*map_item = (input_ch == 0) ? &cvmt->voltage1[range_index] : &cvmt->voltage2[range_index];
		} else if(glp->g_l500k_si_config[input_ch].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
			if(glp->g_l500k_si_config[input_ch].i_range < 1) {
				return -1;
			}
			range_index = glp->g_l500k_si_config[input_ch].i_range - 1;
			if(range_index >= cvmt->current_count) {
				return -1;
			}
			*map_item = &cvmt->current[range_index];
		}
	} else {
		return -1;
	}

	return (*map_item != NULL) ? 0 : -1;
}

static int raw_code_from_signal_input(int input_ch, double value, uint64_t scale, int64_t *raw_code)
{
	const MapItem *map_item = NULL;

	if(!raw_code || scale == 0) {
		return -1;
	}
	if(get_signal_input_map_item(input_ch, &map_item) != 0 || !map_item || map_item->range == 0.0) {
		return -1;
	}

	*raw_code = round_to_i64(value * (double)scale / map_item->range);
	return 0;
}

static int trig_source_to_demod_component(int trig_source, int *demod_index, int *component)
{
	if(!demod_index || !component) {
		return -1;
	}

	if(g_device_model == DEVICE_MODEL_L500K) {
		if(trig_source >= TS_L500K_9_DEMODULATOR1X && trig_source <= TS_L500K_16_DEMODULATOR8X) {
			*demod_index = trig_source - TS_L500K_9_DEMODULATOR1X;
			*component = 0;
			return 0;
		}
		if(trig_source >= TS_L500K_17_DEMODULATOR1Y && trig_source <= TS_L500K_24_DEMODULATOR8Y) {
			*demod_index = trig_source - TS_L500K_17_DEMODULATOR1Y;
			*component = 1;
			return 0;
		}
		if(trig_source >= TS_L500K_25_DEMODULATOR1R && trig_source <= TS_L500K_32_DEMODULATOR8R) {
			*demod_index = trig_source - TS_L500K_25_DEMODULATOR1R;
			*component = 2;
			return 0;
		}
		if(trig_source >= TS_L500K_33_DEMODULATOR1Q && trig_source <= TS_L500K_40_DEMODULATOR8Q) {
			*demod_index = trig_source - TS_L500K_33_DEMODULATOR1Q;
			*component = 3;
			return 0;
		}
	} else if(g_device_model == DEVICE_MODEL_M005M) {
		if(trig_source >= TS_M005M_7_DEMODULATOR1X && trig_source <= TS_M005M_12_DEMODULATOR6X) {
			*demod_index = trig_source - TS_M005M_7_DEMODULATOR1X;
			*component = 0;
			return 0;
		}
		if(trig_source >= TS_M005M_13_DEMODULATOR1Y && trig_source <= TS_M005M_18_DEMODULATOR6Y) {
			*demod_index = trig_source - TS_M005M_13_DEMODULATOR1Y;
			*component = 1;
			return 0;
		}
		if(trig_source >= TS_M005M_19_DEMODULATOR1R && trig_source <= TS_M005M_24_DEMODULATOR6R) {
			*demod_index = trig_source - TS_M005M_19_DEMODULATOR1R;
			*component = 2;
			return 0;
		}
		if(trig_source >= TS_M005M_25_DEMODULATOR1Q && trig_source <= TS_M005M_30_DEMODULATOR6Q) {
			*demod_index = trig_source - TS_M005M_25_DEMODULATOR1Q;
			*component = 3;
			return 0;
		}
	}

	return -1;
}

static int raw_code_from_demod_source(int trig_source, double value, int64_t *raw_code)
{
	gLobalParam *glp = NULL;
	const MapItem *map_item = NULL;
	uint64_t scale;
	int demod_count;
	int demod_index;
	int component;
	int input_ch;

	if(!raw_code) {
		return -1;
	}
	if(trig_source_to_demod_component(trig_source, &demod_index, &component) != 0) {
		return -1;
	}

	demod_count = (g_device_model == DEVICE_MODEL_L500K) ? DEVICE_DEMOD_NUM_L500K : DEVICE_DEMOD_NUM_M005M;
	if(demod_index < 0 || demod_index >= demod_count) {
		return -1;
	}

	if(component == 3) {
		*raw_code = round_to_i64(value * (double)(1LL << 45) / 180.0);
		return 0;
	}

	get_global_param(&glp);
	if(!glp) {
		return -1;
	}
	if(glp->g_demod_param[demod_index].source == DEMO_INPUT_CH1) {
		input_ch = 0;
	} else if(glp->g_demod_param[demod_index].source == DEMO_INPUT_CH2) {
		input_ch = 1;
	} else {
		return -1;
	}

	if(get_signal_input_map_item(input_ch, &map_item) != 0 || !map_item || map_item->range == 0.0) {
		return -1;
	}

	if(component == 0) {
		scale = map_item->X;
	} else if(component == 1) {
		scale = map_item->Y;
	} else {
		scale = map_item->R;
	}
	if(scale == 0) {
		return -1;
	}

	*raw_code = round_to_i64(value * (double)scale / map_item->range);
	return 0;
}

/* Reverse the physical-value decoding used by datatask.c/load_double() and
 * load_double_swap32(). The input source is the complete trigger-source enum.
 * Unsupported digital or not-yet-defined sources return -1 and are left as reserved.
 */
static int physical_value_to_raw_code_by_trig_source(int trig_source, double value, int64_t *raw_code)
{
	const MapItem *map_item = NULL;

	if(!raw_code) {
		return -1;
	}

	if(g_device_model == DEVICE_MODEL_L500K) {
		if(trig_source == TS_L500K_1_VOL || trig_source == TS_L500K_2_VOLCUR) {
			if(get_signal_input_map_item(trig_source - TS_L500K_1_VOL, &map_item) != 0 || !map_item) {
				return -1;
			}
			return raw_code_from_signal_input(trig_source - TS_L500K_1_VOL, value, map_item->R1, raw_code);
		}
		if(trig_source >= TS_L500K_3_AUXIN1 && trig_source <= TS_L500K_6_AUXIN4) {
			return raw_code_from_signed16_bipolar(value, 10.0, raw_code);
		}
		if(trig_source >= TS_L500K_7_REFIN1 && trig_source <= TS_L500K_8_REFIN2) {
			return raw_code_from_signed16_bipolar(value, 2.5, raw_code);
		}
		if(raw_code_from_demod_source(trig_source, value, raw_code) == 0) {
			return 0;
		}
	} else if(g_device_model == DEVICE_MODEL_M005M) {
		if(trig_source == TS_M005M_1_VOL || trig_source == TS_M005M_2_VOLCUR) {
			if(get_signal_input_map_item(trig_source - TS_M005M_1_VOL, &map_item) != 0 || !map_item) {
				return -1;
			}
			return raw_code_from_signal_input(trig_source - TS_M005M_1_VOL, value, map_item->R1, raw_code);
		}
		if(trig_source >= TS_M005M_3_AUXIN1 && trig_source <= TS_M005M_6_AUXIN4) {
			return raw_code_from_aux_offset_20bit(value, raw_code);
		}
		if(raw_code_from_demod_source(trig_source, value, raw_code) == 0) {
			return 0;
		}
	}

	return -1;
}

static int pid_source_to_trig_source(int pid_source, int *trig_source)
{
	int source;
	int demod_index;
	int component;
	int demod_count;

	if(!trig_source || pid_source < 0) {
		return -1;
	}

	if(g_device_model == DEVICE_MODEL_L500K) {
		if(pid_source <= 3) {
			*trig_source = TS_L500K_3_AUXIN1 + pid_source;
			return 0;
		}
		source = pid_source - 4;
		demod_count = DEVICE_DEMOD_NUM_L500K;
		demod_index = source / 4;
		component = source % 4;
		if(demod_index < 0 || demod_index >= demod_count) {
			return -1;
		}
		if(component == 0) {
			*trig_source = TS_L500K_9_DEMODULATOR1X + demod_index;
		} else if(component == 1) {
			*trig_source = TS_L500K_17_DEMODULATOR1Y + demod_index;
		} else if(component == 2) {
			*trig_source = TS_L500K_25_DEMODULATOR1R + demod_index;
		} else {
			*trig_source = TS_L500K_33_DEMODULATOR1Q + demod_index;
		}
		return 0;
	} else if(g_device_model == DEVICE_MODEL_M005M) {
		if(pid_source <= 3) {
			*trig_source = TS_M005M_3_AUXIN1 + pid_source;
			return 0;
		}
		source = pid_source - 4;
		demod_count = DEVICE_DEMOD_NUM_M005M;
		demod_index = source / 4;
		component = source % 4;
		if(demod_index < 0 || demod_index >= demod_count) {
			return -1;
		}
		if(component == 0) {
			*trig_source = TS_M005M_7_DEMODULATOR1X + demod_index;
		} else if(component == 1) {
			*trig_source = TS_M005M_13_DEMODULATOR1Y + demod_index;
		} else if(component == 2) {
			*trig_source = TS_M005M_19_DEMODULATOR1R + demod_index;
		} else {
			*trig_source = TS_M005M_25_DEMODULATOR1Q + demod_index;
		}
		return 0;
	}

	return -1;
}

/* PID source is a compact subset of the full trigger-source enum:
 * AuxIn1..4, then DEMO1-X/Y/R/Q, DEMO2-X/Y/R/Q ...
 */
static int pid_setpoint_to_raw_code(int pid_channel, double setpoint, int64_t *raw_code)
{
	static const char *pid_source_param_names[] = {
		NULL,
		"pid_source_ch1",
		"pid_source_ch2",
		"pid_source_ch3",
		"pid_source_ch4",
	};
	ParamValue source_pv;
	int source;
	int trig_source;

	if(!raw_code || pid_channel < CH1 || pid_channel > CH4) {
		return -1;
	}

	if(get_param_by_name(PARAM_CURRENT, pid_source_param_names[pid_channel], 1, &source_pv) != 0) {
		return -1;
	}

	if(source_pv.type == PARAM_DOUBLE) {
		source = (int)source_pv.value.d;
	} else if(source_pv.type == PARAM_INT32) {
		source = source_pv.value.i32;
	} else if(source_pv.type == PARAM_UINT32) {
		source = (int)source_pv.value.u32;
	} else {
		return -1;
	}

	if(pid_source_to_trig_source(source, &trig_source) != 0) {
		return -1;
	}

	return physical_value_to_raw_code_by_trig_source(trig_source, setpoint, raw_code);
}

static int pid_normalization_by_source(int pid_channel, uint64_t *normalization)
{
	static const char *pid_source_param_names[] = {
		NULL,
		"pid_source_ch1",
		"pid_source_ch2",
		"pid_source_ch3",
		"pid_source_ch4",
	};
	ParamValue source_pv;
	int source;
	int trig_source;

	if(!normalization || pid_channel < CH1 || pid_channel > CH4) {
		return -1;
	}

	if(get_param_by_name(PARAM_CURRENT, pid_source_param_names[pid_channel], 1, &source_pv) != 0) {
		return -1;
	}

	if(source_pv.type == PARAM_DOUBLE) {
		source = (int)source_pv.value.d;
	} else if(source_pv.type == PARAM_INT32) {
		source = source_pv.value.i32;
	} else if(source_pv.type == PARAM_UINT32) {
		source = (int)source_pv.value.u32;
	} else {
		return -1;
	}

	if(pid_source_to_trig_source(source, &trig_source) != 0) {
		return -1;
	}

	/* Normalization follows the compact PID input source configured by
	 * zwdx_set_pid_source(): AuxIn uses 48-bit, Demod R/Q uses 18-bit, others use 1.
	 */
	*normalization = 1;
	if((trig_source >= TS_L500K_3_AUXIN1 && trig_source <= TS_L500K_6_AUXIN4) ||
	   (trig_source >= TS_M005M_3_AUXIN1 && trig_source <= TS_M005M_6_AUXIN4)) {
		*normalization = (1ULL << 48) - 1;
	} else if((trig_source >= TS_L500K_25_DEMODULATOR1R && trig_source <= TS_L500K_40_DEMODULATOR8Q) ||
	          (trig_source >= TS_M005M_19_DEMODULATOR1R && trig_source <= TS_M005M_30_DEMODULATOR6Q)) {
		*normalization = (1ULL << 18) - 1;
	}

	return 0;
}

static scpi_result_t zwdx_set_pid_setpoint(scpi_t *context)
{
	int32_t param1;
	double  param2;
	int64_t setpoint_raw;
	uint64_t setpoint;
	uint64_t normaliztion;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	if(pid_setpoint_to_raw_code(param1, param2, &setpoint_raw) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}
	setpoint = (uint64_t)setpoint_raw;

	//PID SOURCE: AUXIN 1-4:1<<48, DEMOD R/Q: 1<<18, OTHERS:1
	if(pid_normalization_by_source(param1, &normaliztion) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	if(param1 == CH1) {
		reg_write_by_name("pid1_set_point_l", (uint32_t)(setpoint & 0xFFFFFFFF));
		reg_write_by_name("pid1_set_point_h", (uint32_t)((setpoint >> 32) & 0xFFFFFFFF));
		reg_write_by_name("pid1_normaliztion_v_l", (uint32_t)(normaliztion & 0xFFFFFFFF));
		reg_write_by_name("pid1_normaliztion_v_h", (uint32_t)((normaliztion >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_setpoint_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_set_point_l", (uint32_t)(setpoint & 0xFFFFFFFF));
		reg_write_by_name("pid2_set_point_h", (uint32_t)((setpoint >> 32) & 0xFFFFFFFF));
		reg_write_by_name("pid2_normaliztion_v_l", (uint32_t)(normaliztion & 0xFFFFFFFF));
		reg_write_by_name("pid2_normaliztion_v_h", (uint32_t)((normaliztion >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_setpoint_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_set_point_l", (uint32_t)(setpoint & 0xFFFFFFFF));
		reg_write_by_name("pid3_set_point_h", (uint32_t)((setpoint >> 32) & 0xFFFFFFFF));
		reg_write_by_name("pid3_normaliztion_v_l", (uint32_t)(normaliztion & 0xFFFFFFFF));
		reg_write_by_name("pid3_normaliztion_v_h", (uint32_t)((normaliztion >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_setpoint_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_set_point_l", (uint32_t)(setpoint & 0xFFFFFFFF));
		reg_write_by_name("pid4_set_point_h", (uint32_t)((setpoint >> 32) & 0xFFFFFFFF));
		reg_write_by_name("pid4_normaliztion_v_l", (uint32_t)(normaliztion & 0xFFFFFFFF));
		reg_write_by_name("pid4_normaliztion_v_h", (uint32_t)((normaliztion >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pid_setpoint_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_dec(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint32_t dec;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	dec = (uint32_t)param2;

	if(param1 == CH1) {
		reg_write_by_name("pid1_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"pid_dec_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"pid_dec_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"pid_dec_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_decimation", dec);
		set_param_by_name(PARAM_CURRENT,"pid_dec_ch3", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_source(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint32_t source;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	source = (uint32_t)param2;

	if(param1 == CH1) {
		reg_write_by_name("pid1_source", source);
		set_param_by_name(PARAM_CURRENT,"pid_source_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_source", source);
		set_param_by_name(PARAM_CURRENT,"pid_source_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_source", source);
		set_param_by_name(PARAM_CURRENT,"pid_source_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_source", source);
		set_param_by_name(PARAM_CURRENT,"pid_source_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_enable(scpi_t *context)
{
	int32_t param1;
	double  param2;
	uint32_t enable;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	enable = (uint32_t)param2;

	if(param1 == CH1) {
		reg_write_by_name("pid1_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pid_enable_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pid2_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pid_enable_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pid3_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pid_enable_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pid4_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pid_enable_ch4", 1, pv);
	}else {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_param(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_param", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_start(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_start", 0, pv);
	scan_update_config();

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_stop(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_stop", 0, pv);
	scan_update_config();

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_points(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_points", 0, pv);
	scan_update_config();

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_mode(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_mode", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_appmode(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_appmode", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_precision(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_precision", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_single_en(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_single", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_sweep_run_en(scpi_t *context)
{
	double  param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_run", 0, pv);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_sweep_module_start(scpi_t *context)
{
	double	param1;
	ParamValue pv;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	set_param_by_name(PARAM_CURRENT,"sweep_module_start", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_en(scpi_t *context)
{
	int32_t param1;
	double param2;
	ParamValue pv;
	uint32_t enable;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	enable     = param2;

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pcnt_en_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pcnt_en_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pcnt_en_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_enable", enable);
		set_param_by_name(PARAM_CURRENT,"pcnt_en_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_mode(scpi_t *context)
{
	int32_t param1;
	double param2;

	ParamValue pv;
	uint32_t mode;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}


	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	mode	   = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_mode", mode);
		set_param_by_name(PARAM_CURRENT,"pcnt_mode_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_mode", mode);
		set_param_by_name(PARAM_CURRENT,"pcnt_mode_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_mode", mode);
		set_param_by_name(PARAM_CURRENT,"pcnt_mode_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_mode", mode);
		set_param_by_name(PARAM_CURRENT,"pcnt_mode_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_period(scpi_t *context)
{
	int32_t param1;
	double param2;
	ParamValue pv;
	uint64_t period;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	period = (uint64_t)(param2 * pow(10,9) / 25); /*s ->ns / 25ns*/

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_period_l", (uint32_t)(period & 0xFFFFFFFF));
		reg_write_by_name("pulsecounter1_period_h", (uint32_t)((period >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pcnt_period_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_period_l", (uint32_t)(period & 0xFFFFFFFF));
		reg_write_by_name("pulsecounter2_period_h", (uint32_t)((period >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pcnt_period_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_period_l", (uint32_t)(period & 0xFFFFFFFF));
		reg_write_by_name("pulsecounter3_period_h", (uint32_t)((period >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pcnt_period_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_period_l", (uint32_t)(period & 0xFFFFFFFF));
		reg_write_by_name("pulsecounter4_period_h", (uint32_t)((period >> 32) & 0xFFFFFFFF));
		set_param_by_name(PARAM_CURRENT,"pcnt_period_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_input(scpi_t *context)
{
	int32_t param1;
	double param2;
	ParamValue pv;
	uint32_t source;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	source	   = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_input_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_input_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_input_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_input_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_gate_input(scpi_t *context)
{
	int32_t param1;
	double param2;
	ParamValue pv;
	uint32_t source;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	source	   = param2 + 1;

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_gate_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_gate_input_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_gate_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_gate_input_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_gate_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_gate_input_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_gate_source", source);
		set_param_by_name(PARAM_CURRENT,"pcnt_gate_input_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pcnt_edge(scpi_t *context)
{
	int32_t param1;
	double param2;
	ParamValue pv;
	uint32_t edge;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	edge	   = param2;

	if(param1 == CH1) {
		reg_write_by_name("pulsecounter1_edge_mode", edge);
		set_param_by_name(PARAM_CURRENT,"pcnt_edge_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("pulsecounter2_edge_mode", edge);
		set_param_by_name(PARAM_CURRENT,"pcnt_edge_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("pulsecounter3_edge_mode", edge);
		set_param_by_name(PARAM_CURRENT,"pcnt_edge_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("pulsecounter4_edge_mode", edge);
		set_param_by_name(PARAM_CURRENT,"pcnt_edge_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_awg_data(scpi_t *context)
{
	const char *data;
	size_t len;
	static uint32_t total_data_len = 0;
	uint32_t frame_offset;
	int model;
	const uint8_t *u8;
	uint16_t total_frames;
	uint16_t current_frame;
	uint16_t data_len;
	uint16_t chnum;
	uint16_t id;

	if (!SCPI_ParamArbitraryBlock(context, &data, &len, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (len < 10) {
		printf("Invalid data length: %zu\n", len);
		return SCPI_RES_ERR;
	}

	u8 = (const uint8_t *)data;
	total_frames  = ((uint16_t)u8[1] << 8) | u8[0];
	current_frame = (((uint16_t)u8[3] << 8) | u8[2]) + 1;
	data_len      = ((uint16_t)u8[5] << 8) | u8[4];
	chnum         = ((uint16_t)u8[7] << 8) | u8[6];
	id            = ((uint16_t)u8[9] << 8) | u8[8];

	P_LOG_DEBUG("AWG data frame: ch=%u id=%u frame=%u/%u payload=%u block_len=%zu\r\n",
		chnum, id, current_frame, total_frames, data_len, len);

	if ((chnum >= AWG_CH_MAX) || (id >= AWG_QUEUE_MAX_STEPS) || ((size_t)data_len + 10U > len)) {
		P_LOG_DEBUG("Invalid AWG data header, ch=%u, id=%u, data_len=%u, len=%zu\r\n",
			chnum, id, data_len, len);
		total_data_len = 0;
		return SCPI_RES_ERR;
	}

	if(current_frame == 1) {
		P_LOG_DEBUG("AWG data segment start: ch=%u id=%u\r\n", chnum, id);
		total_data_len = 0;
	}

	model = get_device_model();
	if(model == DEVICE_MODEL_L500K) {
		AxiDevice *awg_ddr = NULL;
		uint32_t awg_base_addr;
		uint32_t write_offset;

		frame_offset = total_data_len;
		total_data_len += data_len;

		if(chnum == AWG_CH1) {
			get_awg_ddr_ch1_intance(&awg_ddr);
			awg_base_addr = AWG_L500K_DDR_CH1_BASE;
		}else if(chnum == AWG_CH2) {
			get_awg_ddr_ch2_intance(&awg_ddr);
			awg_base_addr = AWG_L500K_DDR_CH2_BASE;
		}else {
			total_data_len = 0;
			return SCPI_RES_ERR;
		}

		g_awg_ctx.cfg[chnum].queue_count = id + 1;
		if(id == 0) {
			g_awg_ctx.cfg[chnum].queue[id].start_addr = awg_base_addr;
		}else if(frame_offset == 0) {
			g_awg_ctx.cfg[chnum].queue[id].start_addr =
				g_awg_ctx.cfg[chnum].queue[id - 1].start_addr + g_awg_ctx.cfg[chnum].queue[id - 1].length;
		}

		write_offset = g_awg_ctx.cfg[chnum].queue[id].start_addr - awg_base_addr + frame_offset;
		P_LOG_DEBUG("L500K AWG DDR write: ch=%u id=%u base=0x%08x start=0x%08x offset=%u len=%u total_before=%u\r\n",
			chnum, id, awg_base_addr, g_awg_ctx.cfg[chnum].queue[id].start_addr,
			write_offset, data_len, total_data_len);
		if((awg_ddr == NULL) || (awg_ddr->reg_base == NULL) ||
			((write_offset + data_len) > awg_ddr->map_size)) {
			P_LOG_DEBUG("Invalid L500K AWG DDR write, ch=%u, offset=%u, len=%u, map_size=%u\r\n",
				chnum, write_offset, data_len, (awg_ddr != NULL) ? awg_ddr->map_size : 0);
			total_data_len = 0;
			return SCPI_RES_ERR;
		}

		memcpy((void *)((uint8_t *)awg_ddr->reg_base + write_offset), data + 10, data_len);

		if(total_frames == current_frame) {
			g_awg_ctx.cfg[chnum].queue[id].length = total_data_len;
			P_LOG_DEBUG("L500K AWG segment done: ch=%u id=%u start=0x%08x length=%u queue_count=%u\r\n",
				chnum, id, g_awg_ctx.cfg[chnum].queue[id].start_addr,
				g_awg_ctx.cfg[chnum].queue[id].length, g_awg_ctx.cfg[chnum].queue_count);
			total_data_len = 0;
		}
	}else {
		total_data_len += data_len;
		if(total_frames == current_frame) {
			g_awg_ctx.cfg[chnum].queue_count = id + 1;
			if(id == 0) {
				g_awg_ctx.cfg[chnum].queue[id].start_addr = 0;
			}else {
				g_awg_ctx.cfg[chnum].queue[id].start_addr =
					g_awg_ctx.cfg[chnum].queue[id - 1].start_addr + g_awg_ctx.cfg[chnum].queue[id - 1].length;
			}

			g_awg_ctx.cfg[chnum].queue[id].length = total_data_len / 16;
			P_LOG_DEBUG("M005M AWG segment done: ch=%u id=%u start=%u length=%u queue_count=%u\r\n",
				chnum, id, g_awg_ctx.cfg[chnum].queue[id].start_addr,
				g_awg_ctx.cfg[chnum].queue[id].length, g_awg_ctx.cfg[chnum].queue_count);
			total_data_len = 0;
		}

		P_LOG_DEBUG("M005M AWG DMA send: ch=%u id=%u len=%u\r\n", chnum, id, data_len);
		awg_data_send(chnum, (uint8_t *)(data + 10), data_len);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_data_upload(scpi_t *context)
{
	int32_t	param1;
	int model;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	model = get_device_model();
	if(model == DEVICE_MODEL_L500K) {
		reg_write_by_name("awg1_play_en", 0);
		reg_write_by_name("awg2_play_en", 0);
		reg_write_by_name("awg1_start_read", 0);
		reg_write_by_name("awg2_start_read", 0);
		reg_write_by_name("awg1_ch_en", 0);
		reg_write_by_name("awg2_ch_en", 0);
		reg_write_by_name("awg1_param_addr_en", 0);
		reg_write_by_name("awg2_param_addr_en", 0);
		reg_write_by_name("awg1_seq_param_addr_en", 0);
		reg_write_by_name("awg2_seq_param_addr_en", 0);
		reg_write_by_name("awg1_play_seq_cnt", 0);
		reg_write_by_name("awg2_play_seq_cnt", 0);

		/* Default AWG sample clock divider for the current host waveform path:
		 * 40 MHz / 8 MHz = 5. Add a SCPI parameter later if the GUI exposes it.
		 */
		reg_write_by_name("awg1_sample_time", 2);
		reg_write_by_name("awg2_sample_time", 2);
	}else {
		reg_write_by_name("awg1_addr", 0);
		reg_write_by_name("awg2_addr", 0);
		usleep(100);

		reg_write_by_name("awg1_addr_en", 0);
		reg_write_by_name("awg2_addr_en", 0);
		usleep(100);
		reg_write_by_name("awg1_addr_en", 1);
		reg_write_by_name("awg2_addr_en", 1);
		usleep(100);
		reg_write_by_name("awg1_addr_en", 0);
		reg_write_by_name("awg2_addr_en", 0);
	}

	set_param_by_name(PARAM_CURRENT,"awg_data_upload", 0, pv);

	return SCPI_RES_OK;
}


static scpi_result_t zwdx_set_awg_play_en(scpi_t *context)
{
	int32_t	ch, seqnum, id;
	double	param2;
	uint32_t play_enable;
	uint32_t play_id = 0;
	int model;
	int play_seq_cnt;

	if (!SCPI_ParamInt32(context, &ch, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	play_enable = param2;

	if((ch != CH1) && (ch != CH2)) {
		return SCPI_RES_ERR;
	}

	model = get_device_model();
	if(model == DEVICE_MODEL_L500K) {
		const char *ch_en_name;
		const char *start_read_name;
		const char *play_en_name;
		const char *play_seq_cnt_name;
		const char *play_start_addr_name;
		const char *play_length_name;
		const char *play_cnt_name;
		const char *play_waittime_name;
		const char *param_addr_name;
		const char *param_addr_en_name;
		const char *seq_param_addr_name;
		const char *seq_param_name;
		const char *seq_param_addr_en_name;
		const char *param_name;

		if(ch == CH1) {
			ch_en_name = "awg1_ch_en";
			start_read_name = "awg1_start_read";
			play_en_name = "awg1_play_en";
			play_seq_cnt_name = "awg1_play_seq_cnt";
			play_start_addr_name = "awg1_play_start_addr";
			play_length_name = "awg1_play_length";
			play_cnt_name = "awg1_play_cnt";
			play_waittime_name = "awg1_play_waittime";
			param_addr_name = "awg1_param_addr";
			param_addr_en_name = "awg1_param_addr_en";
			seq_param_addr_name = "awg1_seq_param_addr";
			seq_param_name = "awg1_seq_param";
			seq_param_addr_en_name = "awg1_seq_param_addr_en";
			param_name = "awg_play_en_ch1";
		}else {
			ch_en_name = "awg2_ch_en";
			start_read_name = "awg2_start_read";
			play_en_name = "awg2_play_en";
			play_seq_cnt_name = "awg2_play_seq_cnt";
			play_start_addr_name = "awg2_play_start_addr";
			play_length_name = "awg2_play_length";
			play_cnt_name = "awg2_play_cnt";
			play_waittime_name = "awg2_play_waittime";
			param_addr_name = "awg2_param_addr";
			param_addr_en_name = "awg2_param_addr_en";
			seq_param_addr_name = "awg2_seq_param_addr";
			seq_param_name = "awg2_seq_param";
			seq_param_addr_en_name = "awg2_seq_param_addr_en";
			param_name = "awg_play_en_ch2";
		}

		if(play_enable) {
			reg_write_by_name(play_en_name, 0);
			reg_write_by_name(start_read_name, 0);
			reg_write_by_name(ch_en_name, 0);

			for(seqnum = 0, play_seq_cnt = 0; seqnum < g_awg_ctx.cfg[ch-1].queue_count; seqnum++) {
				reg_write_by_name(play_start_addr_name, g_awg_ctx.cfg[ch-1].queue[seqnum].start_addr);
				reg_write_by_name(play_length_name, g_awg_ctx.cfg[ch-1].queue[seqnum].length);
				reg_write_by_name(play_cnt_name, g_awg_ctx.cfg[ch-1].queue[seqnum].repeat);
				reg_write_by_name(play_waittime_name, g_awg_ctx.cfg[ch-1].queue[seqnum].wait_us);

				/* L500K AWG parameter RAM is 0-based. */
				reg_write_by_name(param_addr_name, seqnum);
				reg_write_by_name(param_addr_en_name, 1);
				reg_write_by_name(param_addr_en_name, 0);

				if(g_awg_ctx.cfg[ch-1].queue[seqnum].play_num) {
					if(play_seq_cnt < g_awg_ctx.cfg[ch-1].queue[seqnum].play_num) {
						play_seq_cnt = g_awg_ctx.cfg[ch-1].queue[seqnum].play_num;
					}
				}
				P_LOG_DEBUG("L500K AWG ch=%d queue[%d]: start_addr=0x%08x length=%d repeat=%d wait_us=%d play_num=%d\r\n",
					ch, seqnum, g_awg_ctx.cfg[ch-1].queue[seqnum].start_addr,
					g_awg_ctx.cfg[ch-1].queue[seqnum].length,
					g_awg_ctx.cfg[ch-1].queue[seqnum].repeat,
					g_awg_ctx.cfg[ch-1].queue[seqnum].wait_us,
					g_awg_ctx.cfg[ch-1].queue[seqnum].play_num);
			}

			for(seqnum = 0; seqnum < play_seq_cnt; seqnum++) {
				for(id = 0; id < g_awg_ctx.cfg[ch-1].queue_count; id++) {
					if(g_awg_ctx.cfg[ch-1].queue[id].play_num == (uint32_t)(seqnum + 1)) {
						reg_write_by_name(seq_param_addr_name, seqnum);
						reg_write_by_name(seq_param_name, id);
						reg_write_by_name(seq_param_addr_en_name, 1);
						reg_write_by_name(seq_param_addr_en_name, 0);
						break;
					}
				}
			}

			reg_write_by_name(play_seq_cnt_name, play_seq_cnt);
			reg_write_by_name(ch_en_name, 1);
			sleep(1);
			reg_write_by_name(start_read_name, 1);
			sleep(1);
			reg_write_by_name(play_en_name, 1);

			if(g_awg_ctx.cfg[ch-1].mode == AWG_PLAY_SINGLE) {
				P_LOG_DEBUG("AWG CH1 single play mode, sending soft trigger\r\n");
				send_soft_trig();
			}
		}else {
			reg_write_by_name(play_en_name, 0);
			reg_write_by_name(start_read_name, 0);
			reg_write_by_name(ch_en_name, 0);
		}

		set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

		printf("L500K AWG ch=%d play_en=%d, play mode: %d, queue count: %d, start addr: 0x%08x, length: %d, repeat: %d, wait: %d, play_num: %d\r\n",
			ch, play_enable, g_awg_ctx.cfg[ch-1].mode, g_awg_ctx.cfg[ch-1].queue_count,
			g_awg_ctx.cfg[ch-1].queue[0].start_addr, g_awg_ctx.cfg[ch-1].queue[0].length,
			g_awg_ctx.cfg[ch-1].queue[0].repeat, g_awg_ctx.cfg[ch-1].queue[0].wait_us,
			g_awg_ctx.cfg[ch-1].queue[0].play_num);

	}else if((model == DEVICE_MODEL_M005M_VV) ||
			(model == DEVICE_MODEL_M005M_VI) ||
			(model == DEVICE_MODEL_M005M)) {
		if(ch == CH1) {

			for(seqnum = 0; seqnum < g_awg_ctx.cfg[ch-1].queue_count; seqnum++) {

				reg_write_by_name("awg1_play_start_addr", g_awg_ctx.cfg[ch-1].queue[seqnum].start_addr);
				reg_write_by_name("awg1_play_length", g_awg_ctx.cfg[ch-1].queue[seqnum].length);
				reg_write_by_name("awg1_play_cnt", g_awg_ctx.cfg[ch-1].queue[seqnum].repeat);
				reg_write_by_name("awg1_play_waittime", g_awg_ctx.cfg[ch-1].queue[seqnum].wait_us);

				reg_write_by_name("awg1_param_addr", seqnum + 1);
				reg_write_by_name("awg1_param_addr_en", 1);
				reg_write_by_name("awg1_param_addr_en", 0);

				if(g_awg_ctx.cfg[ch-1].queue[seqnum].play_num) {
					reg_write_by_name("awg1_seq_param_addr", g_awg_ctx.cfg[ch-1].queue[seqnum].play_num - 1);
					reg_write_by_name("awg1_seq_param", seqnum + 1);
					reg_write_by_name("awg1_seq_param_addr_en", 1);
					reg_write_by_name("awg1_seq_param_addr_en", 0);
					play_id += 1;
				}

			}
			reg_write_by_name("awg1_play_seq_cnt", play_id);

			reg_write_by_name("awg1_play_en", play_enable);
			set_param_by_name(PARAM_CURRENT,"awg_play_en_ch1", 1, pv);


			if(g_awg_ctx.cfg[ch-1].mode == AWG_PLAY_SINGLE) {
				send_soft_trig();
			}

		}else if(ch == CH2) {

			for(seqnum = 0; seqnum < g_awg_ctx.cfg[ch-1].queue_count; seqnum++) {

				reg_write_by_name("awg2_play_start_addr", g_awg_ctx.cfg[ch-1].queue[seqnum].start_addr);
				reg_write_by_name("awg2_play_length", g_awg_ctx.cfg[ch-1].queue[seqnum].length);
				reg_write_by_name("awg2_play_cnt", g_awg_ctx.cfg[ch-1].queue[seqnum].repeat);
				reg_write_by_name("awg2_play_waittime", g_awg_ctx.cfg[ch-1].queue[seqnum].wait_us);


				reg_write_by_name("awg2_param_addr", seqnum + 1);
				reg_write_by_name("awg2_param_addr_en", 1);
				reg_write_by_name("awg2_param_addr_en", 0);


				if(g_awg_ctx.cfg[ch-1].queue[seqnum].play_num) {
					reg_write_by_name("awg2_seq_param_addr", g_awg_ctx.cfg[ch-1].queue[seqnum].play_num - 1);
					reg_write_by_name("awg2_seq_param", seqnum + 1);
					reg_write_by_name("awg2_seq_param_addr_en", 1);
					reg_write_by_name("awg2_seq_param_addr_en", 0);
					play_id += 1;
				}
			}
			reg_write_by_name("awg2_play_seq_cnt", g_awg_ctx.cfg[ch-1].queue_count);

			reg_write_by_name("awg2_play_en", play_enable);
			set_param_by_name(PARAM_CURRENT,"awg_play_en_ch2", 1, pv);

			if(g_awg_ctx.cfg[ch-1].mode == AWG_PLAY_SINGLE) {
				send_soft_trig();
			}
		}

		printf("play mode: %d, quene count: %d, start addr: %d, length: %d, repetat: %d, wait: %d, play_num: %d\r\n", g_awg_ctx.cfg[ch-1].mode, g_awg_ctx.cfg[ch-1].queue_count,
			g_awg_ctx.cfg[ch-1].queue[0].start_addr, g_awg_ctx.cfg[ch-1].queue[0].length, g_awg_ctx.cfg[ch-1].queue[0].repeat,
			g_awg_ctx.cfg[ch-1].queue[0].wait_us, g_awg_ctx.cfg[ch-1].queue[0].play_num);
	}
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_play_mode(scpi_t *context)
{
	int32_t	param1;
	double	param2;
	uint32_t play_mode;
	uint32_t reg_play_mode;
	int model;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	play_mode = param2 + 1;
	reg_play_mode = play_mode;
	model = get_device_model();
	if(model == DEVICE_MODEL_L500K) {
		if(param2 == 0) {
			reg_play_mode = 2;
		}else if(param2 == 1) {
			reg_play_mode = 4;
		}else {
			return SCPI_RES_ERR;
		}
	}

	if(param1 == CH1) {
		g_awg_ctx.cfg[0].mode = play_mode;
		reg_write_by_name("awg1_play_mode", reg_play_mode);

		set_param_by_name(PARAM_CURRENT,"awg_play_mode_ch1", 1, pv);
	}else if(param1 == CH2) {
		g_awg_ctx.cfg[1].mode = play_mode;
		reg_write_by_name("awg2_play_mode", reg_play_mode);

		set_param_by_name(PARAM_CURRENT,"awg_play_mode_ch2", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_play_param(scpi_t *context)
{
	int32_t	ch, id;
	double	repeat, delay, playnum;
	uint32_t trig_in_value;

	if (!SCPI_ParamInt32(context, &ch, TRUE)) { /*CH*/
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &id, TRUE)) {/*ID*/
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &repeat, TRUE)) {/*repeat*/
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &delay, TRUE)) {/*delay*/
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &playnum, TRUE)) {/*play number*/
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;

	if(ch == CH1) {
		g_awg_ctx.cfg[0].queue[id].repeat  = repeat;
		g_awg_ctx.cfg[0].queue[id].wait_us = delay;
		g_awg_ctx.cfg[0].queue[id].play_num = playnum;

		pv.value.d = repeat;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch1", 2, pv);
		pv.value.d = delay;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch1", 3, pv);
		pv.value.d = playnum;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch1", 4, pv);
	}else if(ch == CH2) {
		g_awg_ctx.cfg[1].queue[id].repeat  = repeat;
		g_awg_ctx.cfg[1].queue[id].wait_us = delay;
		g_awg_ctx.cfg[1].queue[id].play_num = playnum;

		pv.value.d = repeat;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch2", 2, pv);
		pv.value.d = delay;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch2", 3, pv);
		pv.value.d = playnum;
		set_param_by_name(PARAM_CURRENT,"awg_play_param_ch2", 4, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_amfm_en(scpi_t *context)
{
	int32_t	param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	reg_write_by_name("am_fm_enable", param1);

	set_param_by_name(PARAM_CURRENT,"amfm_en", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_amfm_mode(scpi_t *context)
{
	int32_t	param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	if(param1 == 0)
		param1 = 4;
	if(param1 == 1)
		param1 = 0;

	reg_write_by_name("am_fm_mode", 3);
	reg_write_by_name("modulation_multi_1", param1);//�������ģ�?

	set_param_by_name(PARAM_CURRENT,"amfm_mode", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_amfm_amparam(scpi_t *context)
{
	double	ac, m1, m2;
	uint32_t n1, n2, n3;

	if (!SCPI_ParamDouble(context, &ac, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &m1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &m2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;

	n1 = ac / 10.0 * ((1ULL << 32) - 1); //10��ʾSO������?
	n2 = m1 / 2.0 / 10.0 * ((1ULL << 32) - 1);
	n3 = m2 / 2.0 / 10.0 * ((1ULL << 32) - 1);

	reg_write_by_name("am_n1", n1);
	reg_write_by_name("am_n2", n2);
	reg_write_by_name("am_n3", n3);

	pv.value.d = ac;
	set_param_by_name(PARAM_CURRENT,"amfm_param", 0, pv);
	pv.value.d = m1;
	set_param_by_name(PARAM_CURRENT,"amfm_param", 1, pv);
	pv.value.d = m2;
	set_param_by_name(PARAM_CURRENT,"amfm_param", 2, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_amfm_fmparam(scpi_t *context)
{
	double actemp, ac, mf;
	uint32_t n1, n2, n3;
	Fm3Coeffs fm3coeffs;

	if (!SCPI_ParamDouble(context, &ac, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &mf, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;

	actemp = ac / 10.0; //10��ʾSO������?

	fm3_compute_coeffs_libm(actemp, mf, &fm3coeffs);

	n1 = fm3coeffs.A_carrier * ((1ULL << 32) - 1);
	n2 = fm3coeffs.A_usb1 * ((1ULL << 32) - 1);
	n3 = fm3coeffs.A_usb2 * ((1ULL << 32) - 1);

	reg_write_by_name("am_n1", n1);
	reg_write_by_name("am_n2", n2);
	reg_write_by_name("am_n3", n3);

	pv.value.d = ac;
	set_param_by_name(PARAM_CURRENT,"amfm_fmparam", 0, pv);
	pv.value.d = mf;
	set_param_by_name(PARAM_CURRENT,"amfm_fmparam", 1, pv);


	return SCPI_RES_OK;
}



static scpi_result_t zwdx_set_verify_trig_offset(scpi_t *context)
{
	double  param1;
	double	param2;
	uint32_t trig_in_value;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	trig_in_value = round(param2 / 2.5 * (pow(2, 16) - 1));

	if(param1 == CH1) {
		reg_write_by_name("trigin_trim_ch", 0);
		reg_write_by_name("trigin_trim", trig_in_value);
		reg_write_by_name("trigin_trim_en", 0);
		reg_write_by_name("trigin_trim_en", 1);
		reg_write_by_name("trigin_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"trig_level_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("trigin_trim_ch", 1);
		reg_write_by_name("trigin_trim", trig_in_value);
		reg_write_by_name("trigin_trim_en", 0);
		reg_write_by_name("trigin_trim_en", 1);
		reg_write_by_name("trigin_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"trig_level_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("trigin_trim_ch", 2);
		reg_write_by_name("trigin_trim", trig_in_value);
		reg_write_by_name("trigin_trim_en", 0);
		reg_write_by_name("trigin_trim_en", 1);
		reg_write_by_name("trigin_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"trig_level_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("trigin_trim_ch", 3);
		reg_write_by_name("trigin_trim", trig_in_value);
		reg_write_by_name("trigin_trim_en", 0);
		reg_write_by_name("trigin_trim_en", 1);
		reg_write_by_name("trigin_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"trig_level_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_verify_vone_offset(scpi_t *context)
{
	double  param1;
	double	param2;
	uint32_t v1_value;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	v1_value = round(param2 / 2.5 * (pow(2, 16) - 1));

	if(param1 == CH1) {
		reg_write_by_name("v_trim_ch", 0);
		reg_write_by_name("v_trim", v1_value);
		reg_write_by_name("v_trim_en", 0);
		reg_write_by_name("v_trim_en", 1);
		reg_write_by_name("v_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v1_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("v_trim_ch", 1);
		reg_write_by_name("v_trim", v1_value);
		reg_write_by_name("v_trim_en", 0);
		reg_write_by_name("v_trim_en", 1);
		reg_write_by_name("v_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v1_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("v_trim_ch", 2);
		reg_write_by_name("v_trim", v1_value);
		reg_write_by_name("v_trim_en", 0);
		reg_write_by_name("v_trim_en", 1);
		reg_write_by_name("v_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v1_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("v_trim_ch", 3);
		reg_write_by_name("v_trim", v1_value);
		reg_write_by_name("v_trim_en", 0);
		reg_write_by_name("v_trim_en", 1);
		reg_write_by_name("v_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v1_ch4", 1, pv);
	}


	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_verify_vtwo_offset(scpi_t *context)
{
	double  param1;
	double	param2;
	uint32_t v2_value;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}


	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	v2_value = round(param2 / 2.5 * (pow(2, 16) - 1));

	if(param1 == CH1) {
		reg_write_by_name("i_trim_ch", 0);
		reg_write_by_name("i_trim", v2_value);
		reg_write_by_name("i_trim_en", 0);
		reg_write_by_name("i_trim_en", 1);
		reg_write_by_name("i_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v2_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("i_trim_ch", 1);
		reg_write_by_name("i_trim", v2_value);
		reg_write_by_name("i_trim_en", 0);
		reg_write_by_name("i_trim_en", 1);
		reg_write_by_name("i_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v2_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("i_trim_ch", 2);
		reg_write_by_name("i_trim", v2_value);
		reg_write_by_name("i_trim_en", 0);
		reg_write_by_name("i_trim_en", 1);
		reg_write_by_name("i_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v2_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("i_trim_ch", 3);
		reg_write_by_name("i_trim", v2_value);
		reg_write_by_name("i_trim_en", 0);
		reg_write_by_name("i_trim_en", 1);
		reg_write_by_name("i_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"v2_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_verify_so_offset(scpi_t *context)
{
	double  param1;
	double	param2;
	uint32_t so_value;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}


	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;
	so_value = round(param2 / 2.5 * (pow(2, 16) - 1));

	if(param1 == CH1) {
		reg_write_by_name("signalout_trim_ch", 0);
		reg_write_by_name("signalout_trim", so_value);
		reg_write_by_name("signalout_trim_en", 0);
		reg_write_by_name("signalout_trim_en", 1);
		reg_write_by_name("signalout_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"so_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("signalout_trim_ch", 1);
		reg_write_by_name("signalout_trim", so_value);
		reg_write_by_name("signalout_trim_en", 0);
		reg_write_by_name("signalout_trim_en", 1);
		reg_write_by_name("signalout_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"so_ch2", 1, pv);
	}else if(param1 == CH3) {
		reg_write_by_name("signalout_trim_ch", 2);
		reg_write_by_name("signalout_trim", so_value);
		reg_write_by_name("signalout_trim_en", 0);
		reg_write_by_name("signalout_trim_en", 1);
		reg_write_by_name("signalout_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"so_ch3", 1, pv);
	}else if(param1 == CH4) {
		reg_write_by_name("signalout_trim_ch", 3);
		reg_write_by_name("signalout_trim", so_value);
		reg_write_by_name("signalout_trim_en", 0);
		reg_write_by_name("signalout_trim_en", 1);
		reg_write_by_name("signalout_trim_en", 0);

		set_param_by_name(PARAM_CURRENT,"so_ch4", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t read_two_int_params(scpi_t *context, int32_t *channel, int32_t *value)
{
	if (!SCPI_ParamInt32(context, channel, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, value, TRUE)) {
		return SCPI_RES_ERR;
	}

	return SCPI_RES_OK;
}

static int trigout_reg_name(int32_t channel, const char *field, char *name, size_t name_size)
{
	if (channel < CH1 || channel > CH4 || !field || !name || name_size == 0) {
		return -1;
	}

	snprintf(name, name_size, "trigout%d_%s", channel, field);
	return 0;
}

static scpi_result_t zwdx_set_trigout_mode(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	char reg_name[32];
	char param_name[32];
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (channel < CH1 || channel > CH4 || value < 0 || value > 6) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}
	if (trigout_reg_name(channel, "mode", reg_name, sizeof(reg_name)) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	reg_write_by_name(reg_name, (uint32_t)(value + 1));
	snprintf(param_name, sizeof(param_name), "trigout_mode_ch%d", channel);
	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trigout_u32(scpi_t *context, const char *field, const char *param_prefix)
{
	int32_t channel;
	int32_t value;
	char reg_name[32];
	char param_name[32];
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (channel < CH1 || channel > CH4 || value < 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}
	if (trigout_reg_name(channel, field, reg_name, sizeof(reg_name)) != 0) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	reg_write_by_name(reg_name, (uint32_t)value);
	snprintf(param_name, sizeof(param_name), "%s_ch%d", param_prefix, channel);
	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trigout_period(scpi_t *context)
{
	return zwdx_set_trigout_u32(context, "period", "trigout_period");
}

static scpi_result_t zwdx_set_trigout_width(scpi_t *context)
{
	return zwdx_set_trigout_u32(context, "width", "trigout_width");
}

static scpi_result_t zwdx_set_trigout_loop(scpi_t *context)
{
	return zwdx_set_trigout_u32(context, "loop", "trigout_loop");
}

static scpi_result_t zwdx_set_trigout_enable(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	char param_name[32];
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (channel < CH1 || channel > CH4 || (value != 0 && value != 1)) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	snprintf(param_name, sizeof(param_name), "trigout_enable_ch%d", channel);
	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

	if (value == 1) {
		send_soft_trig();
	}

	return SCPI_RES_OK;
}

#define SINGLEINPUT_BUSY_GPIO_CH1        (43)
#define SINGLEINPUT_BUSY_GPIO_CH2        (44)
#define SINGLEINPUT_STATUS_IDLE          (0)
#define SINGLEINPUT_STATUS_BUSY          (1)
#define SINGLEINPUT_BUSY_WAIT_US         (1000)
#define SINGLEINPUT_BUSY_WAIT_RETRY_MAX  (10000)

static int zwdxm005m_wait_singleinput_idle(int32_t channel)
{
	int busy_gpio;
	int busy;
	int retry;

	if (channel == CH1) {
		busy_gpio = SINGLEINPUT_BUSY_GPIO_CH1;
	} else if (channel == CH2) {
		busy_gpio = SINGLEINPUT_BUSY_GPIO_CH2;
	} else {
		return -1;
	}

	for (retry = 0; retry < SINGLEINPUT_BUSY_WAIT_RETRY_MAX; retry++) {
		busy = gpio_get_value(busy_gpio);

		if (busy == SINGLEINPUT_STATUS_IDLE) {
			P_LOG_DEBUG("Single input channel %d is idle after %d retries\r\n", channel, retry);
			return 0;
		}
		if (busy != SINGLEINPUT_STATUS_BUSY) {
			P_LOG_DEBUG("Single input channel %d is in unknown state after %d retries\r\n", channel, retry);
			return -1;
		}
		usleep(SINGLEINPUT_BUSY_WAIT_US);
	}

	return -1;
}

static void zwdxm005m_readback_singleinput(int32_t channel)
{
	uint32_t read_l = 0;
	uint32_t read_h = 0;

	usleep(40000);
	if(channel == CH1) {
		reg_write_by_name("test1", 1);
		usleep(1000);
		reg_write_by_name("test1", 0);
		usleep(40000);

		reg_read_by_name("test3", &read_l);
		reg_read_by_name("test4", &read_h);
	}else {
		reg_write_by_name("test2", 1);
		usleep(1000);
		reg_write_by_name("test2", 0);
		usleep(40000);

		reg_read_by_name("test5", &read_l);
		reg_read_by_name("test6", &read_h);
	}

	P_LOG_DEBUG("Read back for verify switch channel %d: reg_low=0x%08x, reg_high=0x%08x\r\n",
		channel, read_l, read_h);
}

static int zwdxm005m_apply_singleinput_regs(int32_t channel)
{
	singleinput_regs_t out;
	int ret;

	if ((channel != CH1) && (channel != CH2)) {
		return -1;
	}

	ret = singleinput_calc_regs(&g_param.g_si_config[channel - 1], &out);
	if (ret != 0) {
		return ret;
	}

	ret = zwdxm005m_wait_singleinput_idle(channel);
	if (ret != 0) {
		return ret;
	}

	if (channel == CH1) {
		reg_write_by_name("v1_ctrl_low", out.reg_low);
		reg_write_by_name("v1_ctrl_high", out.reg_high);
		reg_write_by_name("v1_ctrl_en", 0);
		reg_write_by_name("v1_ctrl_en", 1);
		reg_write_by_name("v1_ctrl_en", 0);
		zwdxm005m_readback_singleinput(channel);
	} else {
		reg_write_by_name("v2_ctrl_low", out.reg_low);
		reg_write_by_name("v2_ctrl_high", out.reg_high);
		reg_write_by_name("v2_ctrl_en", 0);
		reg_write_by_name("v2_ctrl_en", 1);
		reg_write_by_name("v2_ctrl_en", 0);
		zwdxm005m_readback_singleinput(channel);
	}

	return 0;
}

static int zwdxm005m_apply_singleinput_verify_switch(int32_t channel, int32_t standard_ground)
{
	singleinput_regs_t out;
	int ret;

	if ((channel != CH1) && (channel != CH2)) {
		return -1;
	}

	ret = singleinput_calc_regs_with_standard_ground(&g_param.g_si_config[channel - 1], &out,
		standard_ground != 0);
	if (ret != 0) {
		return ret;
	}

	ret = zwdxm005m_wait_singleinput_idle(channel);
	if (ret != 0) {
		return ret;
	}

	if (channel == CH1) {
		reg_write_by_name("v1_ctrl_low", out.reg_low);
		reg_write_by_name("v1_ctrl_high", out.reg_high);
		reg_write_by_name("v1_ctrl_en", 0);
		reg_write_by_name("v1_ctrl_en", 1);
		reg_write_by_name("v1_ctrl_en", 0);
	} else {
		reg_write_by_name("v2_ctrl_low", out.reg_low);
		reg_write_by_name("v2_ctrl_high", out.reg_high);
		reg_write_by_name("v2_ctrl_en", 0);
		reg_write_by_name("v2_ctrl_en", 1);
		reg_write_by_name("v2_ctrl_en", 0);
	}

	return 0;
}
static void l500k_write_singleinput_reg(const char *addr_name, const char *value_name,
	const char *enable_name, uint32_t addr, uint32_t value)
{
	reg_write_by_name(addr_name, addr);
	reg_write_by_name(value_name, value);
	reg_write_by_name(enable_name, 0);
	reg_write_by_name(enable_name, 1);
	reg_write_by_name(enable_name, 0);
}
static int l500k_apply_singleinput_regs(int32_t channel)
{
	l500k_singleinput_regs_t out;
	l500k_singleinput_config_t *cfg;
	const char *addr_name;
	const char *value_name;
	const char *enable_name;
	if ((channel != CH1) && (channel != CH2)) {
		return -1;
	}
	cfg = &g_param.g_l500k_si_config[channel - 1];
	if (l500k_singleinput_calc_regs(cfg, &out) != 0) {
		return -1;
	}
	if (channel == CH1) {
		addr_name = "sinput1_reg_addr";
		value_name = "sinput1_reg_value";
		enable_name = "sinput1_reg_enable";
	} else {
		addr_name = "sinput2_reg_addr";
		value_name = "sinput2_reg_value";
		enable_name = "sinput2_reg_enable";
	}
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 0, out.reg1);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 0, 0);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 1, out.reg2);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 1, 0);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 2, out.reg3);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 2, 0);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 3, out.reg4);
	usleep(50000); //wait 50ms
	l500k_write_singleinput_reg(addr_name, value_name, enable_name, 3, (out.reg4 & 0xF0));
	usleep(50000); //wait 50ms
	// l500k_write_singleinput_reg(addr_name, value_name, enable_name, 0, 0);
	// l500k_write_singleinput_reg(addr_name, value_name, enable_name, 1, 0);
	// l500k_write_singleinput_reg(addr_name, value_name, enable_name, 2, 0);
	// l500k_write_singleinput_reg(addr_name, value_name, enable_name, 3, 0);
	P_LOG_DEBUG("l500k_apply_singleinput_regs: channel=%d reg1=0x%02x reg2=0x%02x reg3=0x%02x reg4=0x%02x\r\n",
		channel, out.reg1, out.reg2, out.reg3, out.reg4);
	return 0;
}
static scpi_result_t zwdx_set_verify_si_switch(scpi_t *context)
{
	int32_t param1, param2;
	int model = get_device_model();
	SiVolCtrolRegs regs;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (!SCPI_ParamInt32(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		if (zwdxm005m_apply_singleinput_verify_switch(param1, param2) != 0) {
			return SCPI_RES_ERR;
		}
		return SCPI_RES_OK;
	}
	if (model == DEVICE_MODEL_L500K) {
		if ((param1 != CH1) && (param1 != CH2)) {
			return SCPI_RES_ERR;
		}
		g_param.g_l500k_si_config[param1 - 1].iv_in_gnd =
			param2 ? L500K_SINGLEINPUT_BACKEND_GND : L500K_SINGLEINPUT_BACKEND_INPUT;
		if (l500k_apply_singleinput_regs(param1) != 0) {
			return SCPI_RES_ERR;
		}
		return SCPI_RES_OK;
	}
	if ((model != DEVICE_MODEL_M005M_VV) && (model != DEVICE_MODEL_M005M_VI)) {
		return SCPI_RES_ERR;
	}

	si_pack_regs(&g_param.g_si_vol_ch1_ctrol, &regs);

	/* 1: switch relay to the opposite side of 30mV, 0: restore */
	if (param2 == 1) {
		regs.reg3 &= 0xF0;
		regs.reg3 |= 0x0A;
	} else {
		regs.reg3 &= 0xF0;
		regs.reg3 |= 0x05;
	}

	config_si_vv_regs(regs, param1);

	printf("REG1:%08x\r\n", regs.reg1);
	printf("REG2:%08x\r\n", regs.reg2);
	printf("REG3:%08x\r\n", regs.reg3);

	return SCPI_RES_OK;
}
static scpi_result_t scpi_set_si_mmode(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].measure_mode = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_mmode_ch1" : "si_mmode_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_mmode: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].measure_mode =
			(value == 0) ? L500K_SINGLEINPUT_MEASURE_VOLTAGE : L500K_SINGLEINPUT_MEASURE_CURRENT;
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_mmode_ch1" : "si_mmode_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_mmode: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_smode(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].single_mode = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_smode_ch1" : "si_vol_smode_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_smode: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].v_single =
			(value == 0) ? L500K_SINGLEINPUT_VMODE_SINGLE : L500K_SINGLEINPUT_VMODE_DIFF;
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_smode_ch1" : "si_vol_smode_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_smode: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_gnd(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].grounding_mode = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_gnd_ch1" : "si_vol_gnd_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_gnd: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].v_gnd =
			(value == 0) ? L500K_SINGLEINPUT_VGND_FLOAT : L500K_SINGLEINPUT_VGND_END;
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_gnd_ch1" : "si_vol_gnd_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_gnd: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_couping(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].coupling_voltage = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_couping_ch1" : "si_vol_couping_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_couping: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].v_coupling =
			(value == 0) ? L500K_SINGLEINPUT_VCOUPLING_AC : L500K_SINGLEINPUT_VCOUPLING_DC;
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_couping_ch1" : "si_vol_couping_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_couping: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_impedance(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].impedance = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_impedance_ch1" : "si_vol_impedance_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_impedance: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].v_impedance = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_impedance_ch1" : "si_vol_impedance_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_impedance: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_cur_couping(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].coupling_current = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_couping_ch1" : "si_cur_couping_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_couping: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].i_coupling = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_couping_ch1" : "si_cur_couping_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_couping: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_filter(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].filter = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_filter_ch1" : "si_vol_filter_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_filter: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].iv_fir = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_filter_ch1" : "si_vol_filter_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_filter: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_cur_filter(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].filter = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_filter_ch1" : "si_cur_filter_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_filter: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].iv_fir = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_filter_ch1" : "si_cur_filter_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_filter: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_vol_range(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].range = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_range_ch1" : "si_vol_range_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_range: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].v_range = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_range_ch1" : "si_vol_range_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_vol_range: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static scpi_result_t scpi_set_si_cur_range(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	int model = get_device_model();

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if ((channel != CH1) && (channel != CH2)) {
		return SCPI_RES_ERR;
	}

	if (model == DEVICE_MODEL_M005M) {
		g_param.g_si_config[channel - 1].range = (uint8_t)(value + 1);
		if (zwdxm005m_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_m005m_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_range_ch1" : "si_cur_range_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_range: channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	} else if (model == DEVICE_MODEL_L500K) {
		g_param.g_l500k_si_config[channel - 1].i_range = (uint8_t)(value + 1);
		if (l500k_apply_singleinput_regs(channel) != 0) {
			return SCPI_RES_ERR;
		}
		set_si_l500k_calibration(channel);
		set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_cur_range_ch1" : "si_cur_range_ch2",
			 1, (ParamValue){.type = PARAM_INT32, .value.i32 = value});
		P_LOG_DEBUG("scpi_set_si_cur_range: L500K channel=%d value=%d\r\n", channel, value);
		return SCPI_RES_OK;
	}

	return SCPI_RES_ERR;
}

static int l500k_apply_refinput_wave_mode(int32_t channel, int32_t wave_mode)
{
	(void)channel;
	(void)wave_mode;

	/* TODO: Write the L500K reference-input SINE/TTL selector registers here. */
	return 0;
}

static const char *l500k_refinput_wave_mode_param_name(int32_t channel)
{
	switch (channel) {
		case CH1: return "refinput_wave_mode_ch1";
		case CH2: return "refinput_wave_mode_ch2";
		case CH3: return "refinput_wave_mode_ch3";
		case CH4: return "refinput_wave_mode_ch4";
		default:  return NULL;
	}
}

static scpi_result_t scpi_set_refinput_wavemode(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	const char *param_name;
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}

	if ((get_device_model() != DEVICE_MODEL_L500K) ||
		(channel < CH1) || (channel > CH4) ||
		((value != 0) && (value != 1))) {
		return SCPI_RES_ERR;
	}

	if (l500k_apply_refinput_wave_mode(channel, value) != 0) {
		return SCPI_RES_ERR;
	}

	param_name = l500k_refinput_wave_mode_param_name(channel);
	if (param_name == NULL) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

	P_LOG_DEBUG("scpi_set_refinput_wavemode: L500K channel=%d value=%d\r\n", channel, value);

	return SCPI_RES_OK;
}

/**
 * Reimplement IEEE488.2 *TST?
 *
 * Result should be 0 if everything is ok
 * Result should be 1 if something goes wrong
 *
 * Return SCPI_RES_OK
 */
static scpi_result_t My_CoreTstQ(scpi_t * context) {

    SCPI_ResultInt32(context, 0);

    return SCPI_RES_OK;
}

scpi_command_t scpi_commands[SCPI_COMMANDS_MAX];

void init_scpi_commands(void)
{
	int i = 0;
	int model = get_device_model();
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*CLS", .callback = SCPI_CoreCls,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*ESE", .callback = SCPI_CoreEse,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*ESE?", .callback = SCPI_CoreEseQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*OPC", .callback = SCPI_CoreOpc,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*RST", .callback = SCPI_CoreRst,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*SRE", .callback = SCPI_CoreSre,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*SRE?", .callback = SCPI_CoreSreQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*STB?", .callback = SCPI_CoreStbQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*TST?", .callback = My_CoreTstQ,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "*WAI", .callback = SCPI_CoreWai,};

	/*ZW-LIA-L500K*/
    scpi_commands[i++] = (scpi_command_t){ .pattern = "TRCA?", .callback = zwdx_trca_query,};
    scpi_commands[i++] = (scpi_command_t){ .pattern = "TRCA", .callback = zwdx_trca_write,};//TRCA 10,20
    scpi_commands[i++] = (scpi_command_t){ .pattern = "SOURce1:A1:B1:SWITCH", .callback = zwdx_test_string,};//SOURce1:A1:B1:SWITCH ON/TRUE/1
    scpi_commands[i++] = (scpi_command_t){ .pattern = "DATA:ARB", .callback = zwdx_test_chardata,};//DATA:ARB #41024ABCDEFG
    scpi_commands[i++] = (scpi_command_t){ .pattern = "SOURce1:A1", .callback = zwdx_test_param_unit,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:PL:REG", .callback = zwdx_set_pl_reg,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:PL:REG?",.callback = zwdx_get_pl_reg,};
	/*SYSTEM*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":SYStem:ACTIVEPRE", .callback = zwdx_set_activepre,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":SYStem:DEFAULTPRE", .callback = zwdx_set_defaultpre,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":SYStem:NETWORK:STATIC:IP", .callback = zwdx_set_static_ip};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":SYStem:NETWORK:DHCP:ENABLE", .callback = zwdx_set_dhcp_en};
	/*ZW-LIA-DEVICE*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:CLK:SOURCE", .callback = zwdx_set_dev_clk_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:CLK:EXTFREQ", .callback = zwdx_set_dev_clk_extfreq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:TRIG:SOURCE", .callback = zwdx_set_dev_trig_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:CLK:LOCK?", .callback = zwdx_get_dev_clk_state,};
	/*ZW-LIA-REFENCE*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:FREQuency?", .callback = zwdx_get_ref_freq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:SOURce", .callback = zwdx_set_ref_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:HYS:HIGH", .callback = zwdx_set_ref_sys_high,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:HYS:LOW", .callback = zwdx_set_ref_sys_low};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:FILTER:WINDOWS", .callback = zwdx_set_ref_filter_win,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:CALC:TIME", .callback = zwdx_set_ref_calc_time,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:INTFILTER:WINDOWS", .callback = zwdx_set_ref_intfilter_win,};
	/*ZW-LIA-SignalInput*/
	if( (model == DEVICE_MODEL_M005M_VV) || (model == DEVICE_MODEL_M005M_VI) ) {
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:MODE", .callback = zwdx_set_si_volch_mode,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:GND", .callback = zwdx_set_si_volch_gnd,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:COUPING", .callback = zwdx_set_si_volch_couping,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:IMPEDANCE", .callback = zwdx_set_si_volch_impedance,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:FILTER", .callback = zwdx_set_si_volch_filter,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOLCH:RANGE", .callback = zwdx_set_si_volch_range,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CURCH:GND", .callback = zwdx_set_si_curch_gnd,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CURCH:RANGE", .callback = zwdx_set_si_curch_range,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CURCH:COUPING", .callback = zwdx_set_si_curch_couping,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNAin:WORK:TEST", .callback = zwdx_set_si_work_test,};
	} else if((model == DEVICE_MODEL_M005M) || (model == DEVICE_MODEL_L500K)) {
		//Keep the protocol synchronized with the host GUI.
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:MMODE", .callback = scpi_set_si_mmode,};//Measure mode
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:SMODE", .callback = scpi_set_si_vol_smode,};//Single mode
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:GND", .callback = scpi_set_si_vol_gnd,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:COUPING", .callback = scpi_set_si_vol_couping,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:IMPEDANCE", .callback = scpi_set_si_vol_impedance,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CUR:COUPING", .callback = scpi_set_si_cur_couping,};
		//common parameters
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:FILTER", .callback = scpi_set_si_vol_filter,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CUR:FILTER", .callback = scpi_set_si_cur_filter,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:RANGE", .callback = scpi_set_si_vol_range,};
		scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:CUR:RANGE", .callback = scpi_set_si_cur_range,};
		if (model == DEVICE_MODEL_L500K) {
			scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REFin:WAVEMODE", .callback = scpi_set_refinput_wavemode,};
		}
	}
	/*SignalOutput*/
	//{.pattern = ":ZWLia:REFerence:PLL:SOURce", .callback = zwdx_set_refsource,},
	//{.pattern = ":ZWLia:SIGNALout:OSC:SOURce", .callback = zwdx_set_so_oscsource,},
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OSC:FREQ", .callback = zwdx_set_so_oscfreq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OSC:PHASE", .callback = zwdx_set_so_oscphase,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OSC:HARM", .callback = zwdx_set_so_oscharm,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:RANGE", .callback = zwdx_set_so_range,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SIGNAL:SCALE", .callback = zwdx_set_so_signal_scale,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OFFSET", .callback = zwdx_set_so_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SWITCH", .callback = zwdx_set_so_switch,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SOURce", .callback = zwdx_set_so_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:WAVEMODE", .callback = zwdx_set_so_wavemode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:FAST:SCALE", .callback = zwdx_set_so_fast_scale,};//new add
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OSC:MODE", .callback = zwdx_set_so_oscmode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OSC:RST", .callback = zwdx_set_so_oscrst,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:FASTX:SOURce", .callback = zwdx_set_so_fastx_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:INVERTED:ENABLE", .callback = zwdx_set_so_inverted_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SUBTRACT", .callback = zwdx_set_so_subtract,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:ADD:ENABLE", .callback = zwdx_set_so_add_enable,};
	/*Demod*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:REF:MODE", .callback = zwdx_set_demo_ref_mode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:PHASEMODE", .callback = zwdx_set_demo_oscpmode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:RST", .callback = zwdx_set_demo_oscrst,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:FREQ", .callback = zwdx_set_demo_oscfreq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:PHASE", .callback = zwdx_set_demo_oscphase,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:HARM", .callback = zwdx_set_demo_oscharm,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:ENABLE", .callback = zwdx_set_demo_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:SOURce", .callback = zwdx_set_demo_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:FIR:ORDER", .callback = zwdx_set_demo_fir_order,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:FIR:TC", .callback = zwdx_set_demo_fir_tc,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:SINC:ENABLE", .callback = zwdx_set_demo_sinc_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:SCALE", .callback = zwdx_set_demo_scale,};
	/*AUX-INPUT*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXINPUT:IMPedance", .callback = zwdx_set_ai_impedance,};
	/*AUXOUT*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:SOURce", .callback = zwdx_set_ao_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:DEC", .callback = zwdx_set_ao_dec,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:SCALE", .callback = zwdx_set_ao_scale,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:OFFSET", .callback = zwdx_set_ao_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:ENABLE", .callback = zwdx_set_ao_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:UPPER", .callback = zwdx_set_ao_upper,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:LOWER", .callback = zwdx_set_ao_lower,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:DCVOL", .callback = zwdx_set_ao_dcvol,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AUXOUT:DCVOL:ENABLE", .callback = zwdx_set_ao_dcvol_switch,};
	/*DAQ*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:RUN:ENABLE", .callback = zwdx_set_daq_run_en,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:SIGNAL:ENABLE", .callback = zwdx_set_daq_signal_en,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:DATA:SR", .callback = zwdx_set_daq_sample_rate,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:DATA:SOURCE", .callback = zwdx_set_daq_data_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:ADATA:QUERY?", .callback = zwdx_set_daq_adata_query,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DAQ:RDATA:QUERY?", .callback = zwdx_set_daq_rdata_query,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:FRAME:DATA?", .callback = zwdx_set_frame_data_query,};
	/*SCOPE*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:RUN:ENABLE", .callback = zwdx_set_scope_run_en,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:SIGNAL:ENABLE", .callback = zwdx_set_scope_signal_en,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:DATA:SR", .callback = zwdx_set_scope_sample_rate,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:DATA:SOURCE", .callback = zwdx_set_scope_data_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:CH:SWITCH", .callback = zwdx_set_scope_ch_switch,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SCOPE:AVERAGE:ENABLE", .callback = zwdx_set_scope_average_en,};
	/*PWA-Periodic Waveform Analyzer*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PWA:SIGNAL:ENABLE", .callback = zwdx_set_pwa_signal_en,};
	/*TRIG*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:SMPLEN", .callback = zwdx_set_trig_sample_len,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:TIMES", .callback = zwdx_set_trig_times,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:SOURce", .callback = zwdx_set_trig_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:TYPE", .callback = zwdx_set_trig_type,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:LEVEL", .callback = zwdx_set_trig_level,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:HYSteresis", .callback = zwdx_set_trig_hyst,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:HOLDOFF", .callback = zwdx_set_trig_holdoff,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:DELAY", .callback = zwdx_set_trig_delay,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:PLUSE:WIDTH", .callback = zwdx_set_trig_pw,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:FORCE", .callback = zwdx_set_trig_force,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIGOUT:MODE", .callback = zwdx_set_trigout_mode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIGOUT:PERIOD", .callback = zwdx_set_trigout_period,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIGOUT:WIDTH", .callback = zwdx_set_trigout_width,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIGOUT:LOOP", .callback = zwdx_set_trigout_loop,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIGOUT:ENABLE", .callback = zwdx_set_trigout_enable,};
	/*PID*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:P", .callback = zwdx_set_pid_p,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:I", .callback = zwdx_set_pid_i,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:D", .callback = zwdx_set_pid_d,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:KI:ENABLE", .callback = zwdx_set_pid_ki_en,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:FILTER", .callback = zwdx_set_pid_filter,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:SETPOINT", .callback = zwdx_set_pid_setpoint,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:DEC", .callback = zwdx_set_pid_dec,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:SOURCE", .callback = zwdx_set_pid_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:ENABLE", .callback = zwdx_set_pid_enable,};
	/*SWEEP*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:PARAM",  			.callback = zwdx_set_sweep_param,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:START",         		.callback = zwdx_set_sweep_start,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:STOP",          		.callback = zwdx_set_sweep_stop,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:POINTS",        		.callback = zwdx_set_sweep_points,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:MODE",             	.callback = zwdx_set_sweep_mode,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:APPMODE",          	.callback = zwdx_set_sweep_appmode,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:PRECISION",        	.callback = zwdx_set_sweep_precision,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:SINGLE:ENABLE",    	.callback = zwdx_set_sweep_single_en,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:RUN:ENABLE",  	  	.callback = zwdx_set_sweep_run_en,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SWEEP:MODULE:START",      	.callback = zwdx_set_sweep_module_start,};
	/*PCNT*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:ENABLE",           .callback = zwdx_set_pcnt_en,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:MODE",             .callback = zwdx_set_pcnt_mode,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:PERIOD",           .callback = zwdx_set_pcnt_period,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:INPUT",            .callback = zwdx_set_pcnt_input,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:GATE:INPUT",       .callback = zwdx_set_pcnt_gate_input,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PCNT:EDGE",             .callback = zwdx_set_pcnt_edge,};
	/*AWG*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:DATA", 				.callback = zwdx_set_awg_data,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:DATA:UPLOAD", 		.callback = zwdx_set_awg_data_upload,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:PLAY:ENABLE",       .callback = zwdx_set_awg_play_en,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:PLAY:MODE",         .callback = zwdx_set_awg_play_mode,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:PLAY:PARAM",        .callback = zwdx_set_awg_play_param,};
	/*AF/FM*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AMFM:ENABLE",           .callback = zwdx_set_amfm_en,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AMFM:MODE",             .callback = zwdx_set_amfm_mode,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AMFM:AM:PARAM",         .callback = zwdx_set_amfm_amparam,};
    scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AMFM:FM:PARAM",         .callback = zwdx_set_amfm_fmparam,};
	/*verify*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:VERIFY:TRIG:OFFSET", .callback = zwdx_set_verify_trig_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:VERIFY:VONE:OFFSET", .callback = zwdx_set_verify_vone_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:VERIFY:VTWO:OFFSET", .callback = zwdx_set_verify_vtwo_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:VERIFY:SO:OFFSET", .callback = zwdx_set_verify_so_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:VERIFY:SI:SWITCH", .callback = zwdx_set_verify_si_switch,};
	/* End of command list */
	scpi_commands[i++] = (scpi_command_t)SCPI_CMD_LIST_END;
}

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;
