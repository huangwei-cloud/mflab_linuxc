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
#include <unistd.h>
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
#include "../calib/soscalecalib_mfilab.h"
#include "../calib/sooffsetcalib_mfllab.h"
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

    regs[0] = ((fmt->ch1_wave_format & 0x0F) << 4) |
              (fmt->ch1_range & 0x0F);

    
    regs[1] = ((fmt->ch2_wave_format & 0x0F) << 4) |
              (fmt->ch2_range & 0x0F);

    //bit[0:1]=CH1_RANGE2, bit[2:3]=CH2_RANGE2
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
		return SCPI_RES_ERR;
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

	addr = 0x80030000 + (offset*4);

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
	//g_restart_config = 1;

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

            if (strncmp(line, "    address", 11) == 0) {
                char cur_ip[64];
                if (sscanf(line, "    address %63s", cur_ip) == 1) {
                    if (strcmp(cur_ip, ip) == 0) {
                        need_update = 0;   
                    }
                }
                break;
            }
        }
        fclose(fp);
    }

    fp = fopen(config_file, "w");
    if (!fp) {
        perror("fopen");
        return;
    }

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

    system("ifdown eth0 && ifup eth0");

    printf("static ip set success\n");
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

	system("killall udhcpc 2>/dev/null");
	system("ifdown eth0 2>/dev/null");
	system("ifup eth0 2>/dev/null");

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

	}else {
		psc->dhcpen = param1; //1:enable 0:disable
	}

	save_system_config(SYSTEM_PATH, psc, 1);

	if(psc->dhcpen) {
		if(isfirst_start) {
			start_dhcp_ip(psc->mac);
		}
	} else {
		start_static_ip(psc->ip, psc->gw, psc->mac);
	}

	isfirst_start = 0;

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_dev_clk_source(scpi_t *context)
{
	int32_t param1;
	ParamValue pv;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(param1 < INTERNAL_100M || param1 > EXTERN_10M) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	/* Clock switching is handled by clk_thread through g_dev_clk_source:
		*   0: internal 100 MHz, 1: external 100 MHz, 2: external 10 MHz
		*/
	g_param.g_dev_lock_state = 0; //lock state reset
	g_param.g_dev_clk_source = param1;
	set_param_by_name(PARAM_CURRENT,"dev_clk_source", 0, pv);

	printf("CLK SOURCE : %d\r\n", param1);

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
	uint32_t freq_l, freq_h;
	double freq[9];
	uint64_t freq_code;
	char reg_l_name[64], reg_h_name[64];

	/* Return PLL, OSC0..OSC7, in Hz. Read the system result registers. */
	for (int i = 0; i < 9; i++) {
		if (i == 0) {
			sprintf(reg_l_name, "pll_freq_word_l");
			sprintf(reg_h_name, "pll_freq_word_h");
		} else {
			sprintf(reg_l_name, "osc%d_freq_word_l", i);
			sprintf(reg_h_name, "osc%d_freq_word_h", i);
		}
		if (reg_read_by_name(reg_l_name, &freq_l) != 0 ||
			reg_read_by_name(reg_h_name, &freq_h) != 0) {
			return SCPI_RES_ERR;
		}
		freq_code = ((uint64_t)(freq_h & 0xFFFF) << 32) | freq_l;
		freq[i] = (double)freq_code / (double)((1ULL << 48) - 1)
			* (i == 0 ? PLL_SAMPLE_RATE : OSC_SAMPLE_RATE);
	}
	SCPI_ResultArrayDouble(context, freq, 9, SCPI_FORMAT_ASCII);

	return SCPI_RES_OK;
}

//demodulator reference source setting, 0:OSC0(INT) 1:IN1(EXT) 2:IN2(EXT)
static scpi_result_t zwdx_set_demo_ref_source(scpi_t *context)
{
	double ch, source;
	uint32_t code;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &source, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* User: 0=OSC, 1=IN1, 2=IN2, 3=constant; FPGA swaps IN1/IN2. */
	code = source == 1 ? 2 : (source == 2 ? 1 : (uint32_t)source);
	reg_write_by_name("ref_source", code);
	g_param.g_demod_param[0].osc_source = (int32_t)source;
	set_param_by_name(PARAM_CURRENT, "demo_ref_source_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = source});
	return SCPI_RES_OK;
}

//PLL reference source setting, 0:IN1 1:IN2
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

	pv.type = PARAM_INT32;
	pv.value.i32 = param1;
	set_param_by_name(PARAM_CURRENT,"ref_source", 0, pv);

	reg_write_by_name("pll_source", (uint32_t)param1);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_sys_high(scpi_t *context)
{
	double param1;
	double range;
	int32_t srccode;
	ParamValue pv;
	ParamValue getpv;
	int32_t ref_source;
	int32_t input_range;
	const char *input_range_param_name;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	/* MFILAB PLL reference source:
	 *   ref_source 0:IN1, 1:IN2
	 *   si_vol_range 0:+/-0.5 V, 1:+/-5 V
	 */
	ref_source = 0;
	if(get_param_by_name(PARAM_CURRENT, "ref_source", 0, &getpv) == 0) {
		ref_source = getpv.value.i32;
	}

	input_range_param_name = (ref_source == 0) ? "si_vol_range_ch1" : "si_vol_range_ch2";
	input_range = 1;
	if(get_param_by_name(PARAM_CURRENT, input_range_param_name, 1, &getpv) == 0) {
		input_range = getpv.value.i32;
	}

	range = (input_range == 0) ? 0.5 : 5.0;
	srccode = (int32_t)(param1 * (pow(2, 15) - 1) / range);

	reg_write_by_name("pll_ref_hysteresis_high", srccode);

	set_param_by_name(PARAM_CURRENT,"ref_hys_high", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_sys_low(scpi_t *context)
{
	double param1;
	double range;
	int32_t srccode;
	ParamValue pv;
	ParamValue getpv;
	int32_t ref_source;
	int32_t input_range;
	const char *input_range_param_name;

	if (!SCPI_ParamDouble(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_DOUBLE;
	pv.value.d = param1;

	/* MFILAB PLL reference source:
	 *   ref_source 0:IN1, 1:IN2
	 *   si_vol_range 0:+/-0.5 V, 1:+/-5 V
	 */
	ref_source = 0;
	if(get_param_by_name(PARAM_CURRENT, "ref_source", 0, &getpv) == 0) {
		ref_source = getpv.value.i32;
	}

	input_range_param_name = (ref_source == 0) ? "si_vol_range_ch1" : "si_vol_range_ch2";
	input_range = 1;
	if(get_param_by_name(PARAM_CURRENT, input_range_param_name, 1, &getpv) == 0) {
		input_range = getpv.value.i32;
	}

	range = (input_range == 0) ? 0.5 : 5.0;
	srccode = (int32_t)(param1 * (pow(2, 15) - 1) / range);

	reg_write_by_name("pll_ref_hysteresis_low", srccode);

	set_param_by_name(PARAM_CURRENT,"ref_hys_low", 0, pv);

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

static scpi_result_t zwdx_set_ref_stat_period(scpi_t *context)
{
	double value;
	uint64_t calc_time;

	if (!SCPI_ParamDouble(context, &value, TRUE) || !isfinite(value) || value <= 0.0) {
		return SCPI_RES_ERR;
	}

	/* Same Hz interface as L500K; MFILAB uses 500 MHz (2 ns ticks). */
	if (500e6 / value >= (double)UINT64_MAX) return SCPI_RES_ERR;
	calc_time = (uint64_t)(500e6 / value);
	reg_write_by_name("pll_calc_time_l", (uint32_t)calc_time);
	reg_write_by_name("pll_calc_time_h", (uint32_t)(calc_time >> 32));
	reg_write_by_name("pll_bigwidth_write_en", 1);
	reg_write_by_name("pll_bigwidth_write_en", 0);
	set_param_by_name(PARAM_CURRENT, "ref_stat_period", 0, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_ref_timeout(scpi_t *context)
{
	double value;
	uint64_t timeout;

	if (!SCPI_ParamDouble(context, &value, TRUE) || !isfinite(value) || value < 0.0) {
		return SCPI_RES_ERR;
	}

	/* Timeout in seconds, converted to 500 MHz ticks. */
	if (value * 500e6 >= (double)UINT64_MAX) return SCPI_RES_ERR;
	timeout = (uint64_t)(value * 500e6);
	reg_write_by_name("pll_calc_stopdog_l", (uint32_t)timeout);
	reg_write_by_name("pll_calc_stopdog_h", (uint32_t)(timeout >> 32));
	reg_write_by_name("pll_bigwidth_write_en", 1);
	reg_write_by_name("pll_bigwidth_write_en", 0);
	set_param_by_name(PARAM_CURRENT, "ref_timeout", 0, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
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

		}
	}
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

/* MFILAB OSC config
 * OSC is a global resource now:
 *   osc index 0..7 -> oscillators1..8
 *   OSC0 is used as the demod internal reference, but still belongs to this global OSC block.
 */
static scpi_result_t scpi_set_osc_freq(scpi_t *context)
{
	double osc, value;
	uint64_t freq_word;
	char reg_l_name[64], reg_h_name[64], en_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &osc, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE) ||
		!isfinite(osc) || osc < 0 || osc > 7 || osc != (int)osc ||
		!isfinite(value) || value < 0 || value >= OSC_SAMPLE_RATE) {
		return SCPI_RES_ERR;
	}

	sprintf(reg_l_name, "oscillators%d_freq_word_l", (int)osc + 1);
	sprintf(reg_h_name, "oscillators%d_freq_word_h", (int)osc + 1);
	sprintf(en_name, "oscillators%d_freq_en", (int)osc + 1);
	freq_word = (uint64_t)(value * (double)((1ULL << 48) - 1) / OSC_SAMPLE_RATE);

	reg_write_by_name(reg_l_name, (uint32_t)(freq_word & 0xFFFFFFFF));
	reg_write_by_name(reg_h_name, (uint32_t)((freq_word >> 32) & 0xFFFF));
	reg_write_by_name(en_name, 1);
	reg_write_by_name(en_name, 0);

	/* Same apply sequence as L500K. */
	reg_write_by_name("soft_trig", 0);
	reg_write_by_name("soft_trig", 1);
	reg_write_by_name("soft_trig", 0);
	sprintf(param_name, "osc%d_freq", (int)osc);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	
	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_osc_sync(scpi_t *context)
{
	(void)context;
	/* Synchronize all oscillators with one rising edge, as in L500K. */
	reg_write_by_name("osc_work_all", 0);
	reg_write_by_name("osc_work_all", 1);
	reg_write_by_name("osc_work_all", 0);

	/*synchronize start*/
	reg_write_by_name("soft_trig", 0);
	reg_write_by_name("soft_trig", 1);
	reg_write_by_name("soft_trig", 0);

	return SCPI_RES_OK;
}

/* MFILAB SignalOut config
 * Register table: SignalOut config.
 * The low/high register pairs are handled as one SCPI parameter here:
 *   scaling1 -> signalout_scaling1_l / signalout_scaling1_h
 */
static int scpi_save_so_param(int32_t channel, const char *name_ch1, const char *name_ch2, double value)
{
	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = value;

	if (channel == CH1) {
		return set_param_by_name(PARAM_CURRENT, name_ch1, 1, pv);
	}
	if (channel == CH2) {
		return set_param_by_name(PARAM_CURRENT, name_ch2, 1, pv);
	}

	return -1;
}

static scpi_result_t scpi_set_so_enable(scpi_t *context)
{
	double channel;
	double value;
	uint32_t reg_value;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_value = (uint32_t)value;
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_enable", reg_value);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_enable", reg_value);
	} else {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("osc_work_all", 0);
	usleep(100);
	reg_write_by_name("osc_work_all", 1);
	usleep(100);

	if(g_param.g_dev_trig_source == INTTRIG) {
		P_LOG_DEBUG("intern trig source send soft trigger\r\n");
		send_soft_trig();
	}else if(g_param.g_dev_trig_source == EXTTRIG) {
		/*extern thread handle*/
	}

	scpi_save_so_param((int32_t)channel, "so_enable_ch1", "so_enable_ch2", value);
	P_LOG_DEBUG("so_enable_ch%d: %f\r\n", (int32_t)channel, value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_source1(scpi_t *context)
{
	double channel;
	double value;
	uint32_t reg_value;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* UI option is zero-based. Register 0x00-0x02 are not used by the UI,
	 * so host value 0 maps to 0x03:X, 1 maps to 0x04:Y, and so on.
	 */
	reg_value = (uint32_t)value + 3;
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_source1", reg_value);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_source1", reg_value);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_source1_ch1", "so_source1_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_scaling1(scpi_t *context)
{
	double channel;
	double value;
	uint64_t scaling;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* Calibrate the hardware gain, but keep the requested Scale in the parameter table. */
	scaling = (uint64_t)(soscale_mfilab_calibrate_value((int32_t)channel, value) * (1ULL << 32) - 1);
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_scaling1_l", (uint32_t)(scaling & 0xFFFFFFFF));
		reg_write_by_name("signalout1_scaling1_h", (uint32_t)((scaling >> 32) & 0xFFFFFFFF));
		/* Commit both 32-bit words together. */
		reg_write_by_name("signalout1_bigwidth_write_en", 1);
		reg_write_by_name("signalout1_bigwidth_write_en", 0);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_scaling1_l", (uint32_t)(scaling & 0xFFFFFFFF));
		reg_write_by_name("signalout2_scaling1_h", (uint32_t)((scaling >> 32) & 0xFFFFFFFF));
		reg_write_by_name("signalout2_bigwidth_write_en", 1);
		reg_write_by_name("signalout2_bigwidth_write_en", 0);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_scaling1_ch1", "so_scaling1_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_offset_enable(scpi_t *context)
{
	double channel;
	double value;
	uint32_t reg_value;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_value = (uint32_t)value;
	if ((int32_t)channel == CH1) {
		reg_write_by_name("singalout1_offset_en", reg_value);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("singalout2_offset_en", reg_value);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_offset_enable_ch1", "so_offset_enable_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_offset(scpi_t *context)
{
	double channel;
	double offset_value;
	int32_t code;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &offset_value, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (int32_t)(sooffset_mfilab_calibrate_value((int32_t)channel, offset_value) /
		2.0 * (double)((1 << 15) - 1)); // Fixed +/-2 V; save the requested value below.
	if ((int32_t)channel == CH1) {
		reg_write_by_name("singalout1_offset", (uint32_t)code);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("singalout2_offset", (uint32_t)code);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_offset_ch1", "so_offset_ch2", offset_value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_limit_enable(scpi_t *context)
{
	double channel;
	double value;
	uint32_t reg_value;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_value = (uint32_t)value;
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_limit_en", reg_value);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_limit_en", reg_value);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_limit_enable_ch1", "so_limit_enable_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_upper_limit(scpi_t *context)
{
	double channel;
	double value;
	int32_t code;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (int32_t)(value / 2.0 * (double)((1 << 15) - 1) + (value >= 0.0 ? 0.5 : -0.5));
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_upper_limit", (uint32_t)code);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_upper_limit", (uint32_t)code);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_upper_limit_ch1", "so_upper_limit_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_lower_limit(scpi_t *context)
{
	double channel;
	double value;
	int32_t code;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (int32_t)(value / 2.0 * (double)((1 << 15) - 1) + (value >= 0.0 ? 0.5 : -0.5));
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_lower_limit", (uint32_t)code);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_lower_limit", (uint32_t)code);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_lower_limit_ch1", "so_lower_limit_ch2", value);

	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_so_constant1(scpi_t *context)
{
	double channel;
	double value;
	int32_t code;

	if (!SCPI_ParamDouble(context, &channel, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (int32_t)(value / 2.0 * (double)((1 << 15) - 1) + (value >= 0.0 ? 0.5 : -0.5));
	if ((int32_t)channel == CH1) {
		reg_write_by_name("signalout1_constant1", (uint32_t)code);
	} else if ((int32_t)channel == CH2) {
		reg_write_by_name("signalout2_constant1", (uint32_t)code);
	} else {
		return SCPI_RES_ERR;
	}

	scpi_save_so_param((int32_t)channel, "so_constant1_ch1", "so_constant1_ch2", value);

	return SCPI_RES_OK;
}

/* L500K-style ch1..ch8 selects lookup table 1..8; osc_index selects OSC0..OSC7.
 * Example: ch=2, out_ch=1 -> oscillators2_sinout1_en.
 */
static scpi_result_t zwdx_set_demo_ref_mode(scpi_t *context)
{
	double ch, mode;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &mode, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* 0=manual, 1=PLL reference, 2=manual/PLL difference. */
	sprintf(reg_name, "oscillators%d_mode", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)mode + 1);
	g_param.g_demod_param[(int)ch - 1].ref_mode = (int32_t)mode;
	sprintf(param_name, "demo_ref_mode_ch%d", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = mode});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscnumber(scpi_t *context)
{
	double ch, osc_index;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &osc_index, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* Select the phase accumulator for the selected lookup table, not its waveform type. */
	sprintf(reg_name, "oscillators%d_source", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)osc_index + 1);
	g_param.g_demod_param[(int)ch - 1].osc_index = (int32_t)osc_index;
	sprintf(param_name, "demo_osc_number_ch%d", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = osc_index});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscphase(scpi_t *context)
{
	double ch, phase;
	char reg_name[64], param_name[64];
	double phase_deg;
	uint64_t code;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &phase, TRUE)) {
		return SCPI_RES_ERR;
	}

	phase_deg = fmod(phase, 360.0);
	if (phase_deg < 0) phase_deg += 360.0;
	code = (uint64_t)(phase_deg * (double)(1ULL << 48) / 360.0);
	sprintf(reg_name, "oscillators%d_phase_l", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)code);
	sprintf(reg_name, "oscillators%d_phase_h", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)((code >> 32) & 0xFFFF));
	sprintf(reg_name, "oscillators%d_freq_en", (int)ch);
	reg_write_by_name(reg_name, 1);
	sprintf(reg_name, "oscillators%d_freq_en", (int)ch);
	reg_write_by_name(reg_name, 0);
	sprintf(param_name, "demo_osc_phase_ch%d", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = phase});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscharm(scpi_t *context)
{
	double ch, harm;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &harm, TRUE)) {
		return SCPI_RES_ERR;
	}

	sprintf(reg_name, "oscillators%d_harm", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)harm);
	g_param.g_demod_param[(int)ch - 1].osc_harm = harm;
	sprintf(param_name, "demo_osc_harm_ch%d", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = harm});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscmode(scpi_t *context)
{
	double ch, out_ch, mode;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &out_ch, TRUE) ||
		!SCPI_ParamDouble(context, &mode, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* Both outputs share the selected lookup table's waveform type, as in L500K. */
	sprintf(reg_name, "oscillators%d_osc_type", (int)ch);
	reg_write_by_name(reg_name, (uint32_t)mode + 1);
	sprintf(param_name, "demo_osc_mode_ch%d_out1", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = mode});
	sprintf(param_name, "demo_osc_mode_ch%d_out2", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = mode});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_osc_pulse_duty(scpi_t *context)
{
	double ch, out_ch, duty;
	char reg_name[64], param_name[64];
	uint32_t code;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &out_ch, TRUE) ||
		!SCPI_ParamDouble(context, &duty, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (uint32_t)((1.0 - duty / 100.0) * (double)((1UL << 16) - 1));
	sprintf(reg_name, "oscillators%d_osc_pluse_duty", (int)ch);
	reg_write_by_name(reg_name, code);
	sprintf(param_name, "demo_osc_duty_ch%d_out1", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = duty});
	sprintf(param_name, "demo_osc_duty_ch%d_out2", (int)ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = duty});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscamp(scpi_t *context)
{
	double ch, out_ch, amp;
	uint32_t code;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &out_ch, TRUE) ||
		!SCPI_ParamDouble(context, &amp, TRUE)) {
		return SCPI_RES_ERR;
	}

	/* MFILAB amplitude is VP: output VP = scale * scale(1) * 2 VP. ±2VP*/
	code = (uint32_t)(amp / 2.0 * (double)((1UL << 16) - 1));
	sprintf(reg_name, "oscillators%d_sinout%d_scaling1", (int)ch, (int)out_ch);
	reg_write_by_name(reg_name, code);
	sprintf(param_name, "demo_osc_amp_ch%d_out%d", (int)ch, (int)out_ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = amp});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_oscampen(scpi_t *context)
{
	double ch, out_ch, enable;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &out_ch, TRUE) ||
		!SCPI_ParamDouble(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	sprintf(reg_name, "oscillators%d_sinout%d_en", (int)ch, (int)out_ch);
	reg_write_by_name(reg_name, (uint32_t)enable);
	sprintf(param_name, "demo_osc_amp_en_ch%d_out%d", (int)ch, (int)out_ch);
	set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = enable});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_enable(scpi_t *context)
{
	double ch, enable;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("demod1_enable", (uint32_t)enable);
	set_param_by_name(PARAM_CURRENT, "demo_state_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = enable});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_source(scpi_t *context)
{
	double ch, source;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &source, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("demod1_source", (uint32_t)source);
	g_param.g_demod_param[0].source = source == 1 ? DEMO_INPUT_CH2 : DEMO_INPUT_CH1;
	set_param_by_name(PARAM_CURRENT, "demo_source_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = source});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_fir_order(scpi_t *context)
{
	double ch, order;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &order, TRUE)) {
		return SCPI_RES_ERR;
	}

	reg_write_by_name("demod1_fir_order", (uint32_t)order);
	set_param_by_name(PARAM_CURRENT, "demo_fir_order_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = order});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_fir_tc(scpi_t *context)
{
	double ch, tc;
	uint64_t alpha_high, alpha_low;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &tc, TRUE)) {
		return SCPI_RES_ERR;
	}

	get_alpha_to_reg(&alpha_high, &alpha_low, tc);
	reg_write_by_name("demod1_alpha_l", (uint32_t)alpha_low);
	reg_write_by_name("demod1_alpha_m", (uint32_t)(alpha_low >> 32));
	reg_write_by_name("demod1_alpha_h", (uint32_t)alpha_high);
	reg_write_by_name("demod_bigwidth_write_en", 1);
	reg_write_by_name("demod_bigwidth_write_en", 0);
	set_param_by_name(PARAM_CURRENT, "demo_fir_tc_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = tc});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_sinc_enable(scpi_t *context)
{
	double ch, enable;
	uint32_t lo, hi, m1, m2, nm;
	uint64_t word;
	double osc_hz, pll_hz = 0, freq_hz;
	char reg_name[64];

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &enable, TRUE)) {
		return SCPI_RES_ERR;
	}

	if (enable == 0) {
		reg_write_by_name("demod1_sinc_en", 0);
	set_param_by_name(PARAM_CURRENT, "demo_sinc_state_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = enable});
		return SCPI_RES_OK;
	}
	sprintf(reg_name, "osc%d_freq_word_l", g_param.g_demod_param[0].osc_index + 1);
	if (reg_read_by_name(reg_name, &lo) != 0) return SCPI_RES_ERR;
	sprintf(reg_name, "osc%d_freq_word_h", g_param.g_demod_param[0].osc_index + 1);
	if (reg_read_by_name(reg_name, &hi) != 0) return SCPI_RES_ERR;
	word = ((uint64_t)(hi & 0xFFFF) << 32) | lo;
	osc_hz = (double)word / (double)((1ULL << 48) - 1) * OSC_SAMPLE_RATE;
	if (g_param.g_demod_param[0].ref_mode != 0) {
		if (reg_read_by_name("pll_freq_word_l", &lo) != 0 ||
			reg_read_by_name("pll_freq_word_h", &hi) != 0) return SCPI_RES_ERR;
		word = ((uint64_t)(hi & 0xFFFF) << 32) | lo;
		pll_hz = (double)word / (double)((1ULL << 48) - 1) * PLL_SAMPLE_RATE;
	}
	freq_hz = g_param.g_demod_param[0].ref_mode == 0 ? osc_hz :
		(g_param.g_demod_param[0].ref_mode == 1 ? pll_hz : fabs(pll_hz - osc_hz));
	freq_hz *= g_param.g_demod_param[0].osc_harm;
	if (freq_hz < 1 || freq_hz > UINT32_MAX ||
		sync_filter_params((uint32_t)freq_hz, &m1, &m2, &nm) != 0) return SCPI_RES_ERR;
	reg_write_by_name("demod1_sinc_dec_ratio_m1", m1);
	reg_write_by_name("demod1_sinc_aver_ratio", nm);
	reg_write_by_name("demod1_sinc_dec_ratio_m2", m2);
	reg_write_by_name("demod1_sinc_dec_ratio_en", 1);
	reg_write_by_name("demod1_sinc_dec_ratio_en", 0);
	reg_write_by_name("demod1_sinc_en", 1);
	set_param_by_name(PARAM_CURRENT, "demo_sinc_state_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = enable});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_demo_scale(scpi_t *context)
{
	double ch, scale;
	uint64_t code;

	if (!SCPI_ParamDouble(context, &ch, TRUE) ||
		!SCPI_ParamDouble(context, &scale, TRUE)) {
		return SCPI_RES_ERR;
	}

	code = (uint64_t)(scale * (double)(1ULL << 32));
	reg_write_by_name("demod1_psd_scaling_l", (uint32_t)code);
	reg_write_by_name("demod1_psd_scaling_h", (uint32_t)(code >> 32));
	reg_write_by_name("demod_bigwidth_write_en", 1);
	reg_write_by_name("demod_bigwidth_write_en", 0);
	set_param_by_name(PARAM_CURRENT, "demo_scale_ch1", 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = scale});
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
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	sample_rate_index = param1;
	if(sample_rate_index >= 31) {
		return SCPI_RES_ERR;
	}

	/* Host sends the sample-rate combo-box index.
	 * DAQ is divided from 125 MHz, and FPGA decimation value is 2^index:
	 *   index 0 -> 125 MHz, index 1 -> 62.5 MHz, index 2 -> 31.25 MHz, ...
	 */
	decimation_value = 1U << sample_rate_index;

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
	if(sample_rate_index >= 31) {
		return SCPI_RES_ERR;
	}

	/* Scope sample-rate index:
	 *   index 0 -> 500 MHz raw path, write 0 directly.
	 *   index 1 -> 125 MHz / 2, index 2 -> 125 MHz / 4,
	 *   index 3 -> 125 MHz / 8, ...
	 */
	decimation_value = (sample_rate_index == 0) ? 0 : (1U << sample_rate_index);

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
		g_param.g_scope_source[0] = (ScopeDataSource)param2;
		set_param_by_name(PARAM_CURRENT,"scope_source_ch1", 1, pv);
	}else if(param1 == CH2) {
		reg_write_by_name("ch2_scope_up_data_mux", source);
		g_param.g_scope_source[1] = (ScopeDataSource)param2;
		set_param_by_name(PARAM_CURRENT,"scope_source_ch2", 1, pv);
	}

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

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	reg_write_by_name("trig_source", TS_SCOPE_PHASE);

	reg_write_by_name("hysteresis_range_high_l", 0);
	reg_write_by_name("hysteresis_range_high_h", 0x3FFFFFFF);
	reg_write_by_name("hysteresis_range_low_l", 0);
	reg_write_by_name("hysteresis_range_low_h", 0x3BFFFFFF);
	reg_write_by_name("trig_bigwidth_write_en", 1);
	reg_write_by_name("trig_bigwidth_write_en", 0);

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

static scpi_result_t zwdx_set_trig_holdoff_count(scpi_t *context)
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

	set_param_by_name(PARAM_CURRENT,"trig_holdoff_count", 0, pv);


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

	trigsource = (uint32_t)param1;

	reg_write_by_name("trig_source", trigsource);
	set_param_by_name(PARAM_CURRENT,"trig_source", 0, pv);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_trig_mode(scpi_t *context)
{
	double mode;

	if (!SCPI_ParamDouble(context, &mode, TRUE) || (mode != 0 && mode != 1)) {
		return SCPI_RES_ERR;
	}
	//0：NORMAL 1：AUTO
	/* Software acquisition policy, not the FPGA trig_mode edge/pulse register. */
	set_param_by_name(PARAM_CURRENT, "trig_mode", 0, (ParamValue){.type = PARAM_DOUBLE, .value.d = mode});
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

static int write_trig_level_window(double level, double hyst)
{
	ParamValue getpv;
	int trig_source;
	double range;
	uint64_t scale;
	int64_t max_raw, min_raw;
	uint64_t max_bits, min_bits;
	gLobalParam *glp = NULL;
	MappingTable *cvmt = NULL;

	if(get_param_by_name(PARAM_CURRENT,"trig_source", 0, &getpv) != 0) {
		P_LOG_ERROR("Failed to get trig_source parameter\r\n");
		return -1;
	}

	trig_source = getpv.value.i32;
	get_global_param(&glp);
	get_cv_maptable(&cvmt);
	if(glp == NULL || cvmt == NULL) {
		return -1;
	}

	if(trig_source == TS_SCOPE_IN1) {
		range = cvmt->voltage1[glp->g_siginput_config[0].range - 1].range;
		scale = cvmt->voltage1[glp->g_siginput_config[0].range - 1].R1;
	} else if(trig_source == TS_SCOPE_IN2) {
		range = cvmt->voltage1[glp->g_siginput_config[1].range - 1].range;
		scale = cvmt->voltage1[glp->g_siginput_config[1].range - 1].R1;
	} else {
		P_LOG_DEBUG("Skip trigger level raw-code conversion for unsupported trig_source %d, level %.3f, hyst %.3f\r\n",
			trig_source, level, hyst);
		return 0;
	}

	if(range == 0.0 || scale == 0) {
		return -1;
	}

	max_raw = (int64_t)(((level + hyst) * (double)scale / range) + (((level + hyst) >= 0.0) ? 0.5 : -0.5));
	min_raw = (int64_t)(((level - hyst) * (double)scale / range) + (((level - hyst) >= 0.0) ? 0.5 : -0.5));
	P_LOG_DEBUG("trig_source: %d, level: %.3f, hyst: %.3f, max_raw: 0x%016llx, min_raw: 0x%016llx\r\n",
		trig_source, level, hyst, (unsigned long long)max_raw, (unsigned long long)min_raw);
	max_bits = (uint64_t)max_raw;
	min_bits = (uint64_t)min_raw;
	reg_write_by_name("hysteresis_range_high_l", max_bits & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_high_h", (max_bits >> 32) & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_low_l", min_bits & 0xFFFFFFFF);
	reg_write_by_name("hysteresis_range_low_h", (min_bits >> 32) & 0xFFFFFFFF);
	/* Commit the upper/lower 64-bit thresholds together. */
	reg_write_by_name("trig_bigwidth_write_en", 1);
	reg_write_by_name("trig_bigwidth_write_en", 0);
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

static scpi_result_t zwdx_set_trig_holdoff_time(scpi_t *context)
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
	set_param_by_name(PARAM_CURRENT,"trig_holdoff_time", 0, pv);


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
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];
	uint64_t code;

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	code = (uint64_t)(value * (double)(1ULL << 48));
	P_LOG_DEBUG("zwdx_set_pid_p: channel=%d, value=%.6f, code=0x%016llx\r\n", channel, value, (unsigned long long)code);
	sprintf(reg_name, "pid%d_Kp_l", channel);
	reg_write_by_name(reg_name, (uint32_t)code);
	sprintf(reg_name, "pid%d_Kp_h", channel);
	reg_write_by_name(reg_name, (uint32_t)(code >> 32));
	sprintf(reg_name, "pid%d_bigwidth_write_en", channel);
	reg_write_by_name(reg_name, 1);
	reg_write_by_name(reg_name, 0);
	sprintf(param_name, "pid_p_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_i(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];
	uint64_t code;

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	code = (uint64_t)(value * (double)(1ULL << 48));
	P_LOG_DEBUG("zwdx_set_pid_i: channel=%d, value=%.6f, code=0x%016llx\r\n", channel, value, (unsigned long long)code);
	sprintf(reg_name, "pid%d_Ki_l", channel);
	reg_write_by_name(reg_name, (uint32_t)code);
	sprintf(reg_name, "pid%d_Ki_h", channel);
	reg_write_by_name(reg_name, (uint32_t)(code >> 32));
	sprintf(reg_name, "pid%d_bigwidth_write_en", channel);
	reg_write_by_name(reg_name, 1);
	reg_write_by_name(reg_name, 0);
	sprintf(param_name, "pid_i_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_d(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];
	uint64_t code;

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	code = (uint64_t)(value * (double)(1ULL << 48));
	P_LOG_DEBUG("zwdx_set_pid_d: channel=%d, value=%.6f, code=0x%016llx\r\n", channel, value, (unsigned long long)code);
	sprintf(reg_name, "pid%d_Kd_l", channel);
	reg_write_by_name(reg_name, (uint32_t)code);
	sprintf(reg_name, "pid%d_Kd_h", channel);
	reg_write_by_name(reg_name, (uint32_t)(code >> 32));
	sprintf(reg_name, "pid%d_bigwidth_write_en", channel);
	reg_write_by_name(reg_name, 1);
	reg_write_by_name(reg_name, 0);
	sprintf(param_name, "pid_d_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_ki_en(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_ki_enable", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_ki_en_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_filter(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_moving_aver_ratio", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_filter_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
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
			// if(range_index >= cvmt->current_count) {
			// 	return -1;
			// }
			// *map_item = &cvmt->current[range_index];
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

static scpi_result_t zwdx_set_pid_setpoint(scpi_t *context)
{
	int32_t channel;
	double value;
	char param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	/* The dependency sync thread converts and writes the target. */
	sprintf(param_name, "pid_setpoint_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_dec(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_decimation", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_dec_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_source(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_source", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_source_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_enable(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_enable", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_enable_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_pid_invert(scpi_t *context)
{
	int32_t channel;
	double value;
	char reg_name[64], param_name[64];

	if (!SCPI_ParamInt32(context, &channel, TRUE) ||
		!SCPI_ParamDouble(context, &value, TRUE)) {
		return SCPI_RES_ERR;
	}
	if(channel < CH1 || channel > CH4) {
		return SCPI_Error(context, SCPI_ERROR_ILLEGAL_PARAMETER_VALUE);
	}

	sprintf(reg_name, "pid%d_invert", channel);
	reg_write_by_name(reg_name, (uint32_t)value);
	sprintf(param_name, "pid_invert_ch%d", channel);
	set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
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
		total_data_len = 0;
	}

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
		P_LOG_DEBUG("MFILAB AWG segment done: ch=%u id=%u start=%u length=%u queue_count=%u\r\n",
			chnum, id, g_awg_ctx.cfg[chnum].queue[id].start_addr,
			g_awg_ctx.cfg[chnum].queue[id].length, g_awg_ctx.cfg[chnum].queue_count);
		total_data_len = 0;
	}

	awg_data_send(chnum, (uint8_t *)(data + 10), data_len);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_data_upload(scpi_t *context)
{
	int32_t param1;

	if (!SCPI_ParamInt32(context, &param1, TRUE)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_INT32;
	pv.value.i32 = param1;

	reg_write_by_name("awg1_play_en", 0);
	reg_write_by_name("awg2_play_en", 0);
	reg_write_by_name("awg1_play_seq_cnt", 0);
	reg_write_by_name("awg2_play_seq_cnt", 0);
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

	set_param_by_name(PARAM_CURRENT,"awg_data_upload", 0, pv);

	return SCPI_RES_OK;
}

/*
 * AWG uses the old M005M FPGA table format.
 *
 * Queue parameter table:
 *
 *   FPGA param_addr | host queue_id | start_addr | length | repeat | wait_us
 *   ----------------+---------------+------------+--------+--------+--------
 *          1        |       0       |     ...    |  ...   |  ...   |   ...
 *          2        |       1       |     ...    |  ...   |  ...   |   ...
 *         ...       |      ...      |     ...    |  ...   |  ...   |   ...
 *
 * Playback sequence table:
 *
 *   FPGA seq_addr | host play_num | FPGA seq_param
 *   --------------+---------------+---------------
 *        0        |       0       | host queue_id + 1
 *        1        |       1       | host queue_id + 1
 *       ...       |      ...      | ...
 *
 * Host queue_id/play_num are zero-based. FPGA queue ids are one-based.
 * play_seq_cnt is the number of valid rows in the playback sequence table.
 */
static scpi_result_t zwdx_set_awg_play_en(scpi_t *context)
{
	int32_t ch, seqnum, id;
	double param2;
	uint32_t play_enable;
	uint32_t play_seq_cnt = 0;
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

	if(ch == CH1) {
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
	}else if(ch == CH2) {
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
	}else {
		return SCPI_RES_ERR;
	}

	if(play_enable) {
		reg_write_by_name(play_en_name, 0);
		reg_write_by_name(play_seq_cnt_name, 0);

		for(seqnum = 0; seqnum < g_awg_ctx.cfg[ch-1].queue_count; seqnum++) {
			reg_write_by_name(play_start_addr_name, g_awg_ctx.cfg[ch-1].queue[seqnum].start_addr);
			reg_write_by_name(play_length_name, g_awg_ctx.cfg[ch-1].queue[seqnum].length);
			reg_write_by_name(play_cnt_name, g_awg_ctx.cfg[ch-1].queue[seqnum].repeat);
			reg_write_by_name(play_waittime_name, g_awg_ctx.cfg[ch-1].queue[seqnum].wait_us);

			/* FPGA param_addr is one-based: host queue 0 -> FPGA row 1. */
			reg_write_by_name(param_addr_name, seqnum + 1);
			reg_write_by_name(param_addr_en_name, 1);
			reg_write_by_name(param_addr_en_name, 0);

			/* host play_num is zero-based and selects the sequence row. */
			if(g_awg_ctx.cfg[ch-1].queue[seqnum].play_num < AWG_QUEUE_MAX_STEPS) {
				if(play_seq_cnt < g_awg_ctx.cfg[ch-1].queue[seqnum].play_num + 1) {
					play_seq_cnt = g_awg_ctx.cfg[ch-1].queue[seqnum].play_num + 1;
				}
			}
		}

		/* play_num is zero-based: sequence 0 plays the queue whose play_num is 0. */
		for(seqnum = 0; seqnum < play_seq_cnt; seqnum++) {
			for(id = 0; id < g_awg_ctx.cfg[ch-1].queue_count; id++) {
				if(g_awg_ctx.cfg[ch-1].queue[id].play_num == (uint32_t)seqnum) {
					reg_write_by_name(seq_param_addr_name, seqnum);
					reg_write_by_name(seq_param_name, id + 1);
					reg_write_by_name(seq_param_addr_en_name, 1);
					reg_write_by_name(seq_param_addr_en_name, 0);
					break;
				}
			}
		}

		reg_write_by_name(play_seq_cnt_name, play_seq_cnt);
		reg_write_by_name(play_en_name, 1);

		/* mode 1: continuous play, use software trigger to start automatic playback. */
		if(g_awg_ctx.cfg[ch-1].mode == 1) {
			send_soft_trig();
		}
	}else {
		reg_write_by_name(play_en_name, 0);
	}

	set_param_by_name(PARAM_CURRENT, param_name, 1, pv);

	P_LOG_DEBUG("MFILAB AWG ch=%d play_en=%d, play_mode=%d, queue_count=%d\r\n",
		ch, play_enable, g_awg_ctx.cfg[ch-1].mode, g_awg_ctx.cfg[ch-1].queue_count);

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_play_mode(scpi_t *context)
{
	int32_t ch;
	double param2;
	uint32_t play_mode;

	if (!SCPI_ParamInt32(context, &ch, TRUE)) {
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &param2, TRUE)) {
		return SCPI_RES_ERR;
	}

	if((ch != CH1) && (ch != CH2)) {
		return SCPI_RES_ERR;
	}
	if((param2 < 0) || (param2 > 3) || (param2 != (double)((uint32_t)param2))) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;
	pv.value.d = param2;

	/* Host sends 0-based mode values; FPGA register uses 1-based values:
	 * 0 -> 1: continuous play
	 * 1 -> 2: one trigger plays all queue rows
	 * 2 -> 3: one trigger plays one item in one queue
	 * 3 -> 4: one trigger plays once in configured order
	 */
	play_mode = (uint32_t)param2 + 1;

	if(ch == CH1) {
		g_awg_ctx.cfg[0].mode = play_mode;
		reg_write_by_name("awg1_play_mode", play_mode);
		set_param_by_name(PARAM_CURRENT,"awg_play_mode_ch1", 1, pv);
	}else {
		g_awg_ctx.cfg[1].mode = play_mode;
		reg_write_by_name("awg2_play_mode", play_mode);
		set_param_by_name(PARAM_CURRENT,"awg_play_mode_ch2", 1, pv);
	}

	return SCPI_RES_OK;
}

static scpi_result_t zwdx_set_awg_play_param(scpi_t *context)
{
	int32_t ch, id;
	double repeat, delay, playnum;

	if (!SCPI_ParamInt32(context, &ch, TRUE)) { /* CH: 1/2 */
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamInt32(context, &id, TRUE)) { /* host queue id: starts from 0 */
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &repeat, TRUE)) { /* repeat */
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &delay, TRUE)) { /* delay_us */
		return SCPI_RES_ERR;
	}
	if (!SCPI_ParamDouble(context, &playnum, TRUE)) { /* host play order id: starts from 0 */
		return SCPI_RES_ERR;
	}

	if(((ch != CH1) && (ch != CH2)) || (id < 0) || (id >= AWG_QUEUE_MAX_STEPS) ||
		(playnum < 0) || (playnum >= AWG_QUEUE_MAX_STEPS)) {
		return SCPI_RES_ERR;
	}

	ParamValue pv;
	pv.type = PARAM_DOUBLE;

	g_awg_ctx.cfg[ch-1].queue[id].repeat = repeat;
	g_awg_ctx.cfg[ch-1].queue[id].wait_us = delay;
	g_awg_ctx.cfg[ch-1].queue[id].play_num = playnum;

	pv.value.d = repeat;
	set_param_by_name(PARAM_CURRENT, (ch == CH1) ? "awg_play_param_ch1" : "awg_play_param_ch2", 2, pv);
	pv.value.d = delay;
	set_param_by_name(PARAM_CURRENT, (ch == CH1) ? "awg_play_param_ch1" : "awg_play_param_ch2", 3, pv);
	pv.value.d = playnum;
	set_param_by_name(PARAM_CURRENT, (ch == CH1) ? "awg_play_param_ch1" : "awg_play_param_ch2", 4, pv);

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
static scpi_result_t scpi_set_si_vol_couping(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	siginput_config_t *cfg;
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (((channel != CH1) && (channel != CH2)) || ((value != 0) && (value != 1))) {
		return SCPI_RES_ERR;
	}

	cfg = &g_param.g_siginput_config[channel - 1];
	cfg->ch = (uint8_t)(channel - 1);
	cfg->coupling = (uint8_t)(value + 1);

	if (siginput_config_to_regs(cfg) != 0) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_couping_ch1" : "si_vol_couping_ch2", 1, pv);

	P_LOG_DEBUG("scpi_set_si_vol_couping: channel=%d value=%d", channel, value);
	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_si_vol_impedance(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	siginput_config_t *cfg;
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (((channel != CH1) && (channel != CH2)) || ((value != 0) && (value != 1))) {
		return SCPI_RES_ERR;
	}

	cfg = &g_param.g_siginput_config[channel - 1];
	cfg->ch = (uint8_t)(channel - 1);
	cfg->impedance = (uint8_t)(value + 1);

	if (siginput_config_to_regs(cfg) != 0) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_impedance_ch1" : "si_vol_impedance_ch2", 1, pv);

	P_LOG_DEBUG("scpi_set_si_vol_impedance: channel=%d value=%d", channel, value);
	return SCPI_RES_OK;
}

static scpi_result_t scpi_set_si_vol_range(scpi_t *context)
{
	int32_t channel;
	int32_t value;
	siginput_config_t *cfg;
	ParamValue pv;

	if (read_two_int_params(context, &channel, &value) != SCPI_RES_OK) {
		return SCPI_RES_ERR;
	}
	if (((channel != CH1) && (channel != CH2)) || ((value != 0) && (value != 1))) {
		return SCPI_RES_ERR;
	}

	cfg = &g_param.g_siginput_config[channel - 1];
	cfg->ch = (uint8_t)(channel - 1);
	cfg->range = (uint8_t)(value + 1);

	if (siginput_config_to_regs(cfg) != 0) {
		return SCPI_RES_ERR;
	}

	pv.type = PARAM_INT32;
	pv.value.i32 = value;
	set_param_by_name(PARAM_CURRENT, channel == CH1 ? "si_vol_range_ch1" : "si_vol_range_ch2", 1, pv);

	P_LOG_DEBUG("scpi_set_si_vol_range: channel=%d value=%d", channel, value);
	return SCPI_RES_OK;
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
	/*MFILAB*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:CLK:SOURCE", .callback = zwdx_set_dev_clk_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:TRIG:SOURCE", .callback = zwdx_set_dev_trig_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":DEVice:CLK:LOCK?", .callback = zwdx_get_dev_clk_state,};
	/*ZW-LIA-REFENCE*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:FREQuency?", .callback = zwdx_get_ref_freq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:SOURce", .callback = zwdx_set_ref_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:HYS:HIGH", .callback = zwdx_set_ref_sys_high,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:HYS:LOW", .callback = zwdx_set_ref_sys_low};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:FILTER:WINDOWS", .callback = zwdx_set_ref_filter_win,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:STAT:PERIOD", .callback = zwdx_set_ref_stat_period,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:REF:TIMEOUT", .callback = zwdx_set_ref_timeout,};
	/*MFILAB SignalInput: voltage input only. User values are zero-based. */
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:COUPING", .callback = scpi_set_si_vol_couping,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:IMPEDANCE", .callback = scpi_set_si_vol_impedance,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALin:VOL:RANGE", .callback = scpi_set_si_vol_range,};

	/*MFILAB OSC: global OSC0-OSC7, independent from SignalOut. */
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:OSC:FREQ", .callback = scpi_set_osc_freq,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:OSC:SYNC", .callback = scpi_set_osc_sync,};

	/*MFILAB SignalOutput: follows the SignalOut config register table. */
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:ENABLE", .callback = scpi_set_so_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SOURce1", .callback = scpi_set_so_source1,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:SCALing1", .callback = scpi_set_so_scaling1,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OFFSet:ENABLE", .callback = scpi_set_so_offset_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:OFFSet", .callback = scpi_set_so_offset,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:LIMit:ENABLE", .callback = scpi_set_so_limit_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:LIMit:UPPer", .callback = scpi_set_so_upper_limit,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:LIMit:LOWer", .callback = scpi_set_so_lower_limit,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:SIGNALout:CONStant1", .callback = scpi_set_so_constant1,};

	/*MFILAB Demod*/
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:REF:SOURce", .callback = zwdx_set_demo_ref_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:REF:MODE", .callback = zwdx_set_demo_ref_mode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:NUMBER", .callback = zwdx_set_demo_oscnumber,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:PHASE", .callback = zwdx_set_demo_oscphase,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:HARM", .callback = zwdx_set_demo_oscharm,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:MODE", .callback = zwdx_set_demo_oscmode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:PULSE:DUTY", .callback = zwdx_set_demo_osc_pulse_duty,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:AMP", .callback = zwdx_set_demo_oscamp,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:OSC:AMP:ENABLE", .callback = zwdx_set_demo_oscampen,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:ENABLE", .callback = zwdx_set_demo_enable,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:DEMO:DATA:SOURce", .callback = zwdx_set_demo_source,};
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
	/*SCOPE-MFILAB*/
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
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:HOLDOFF:COUNT", .callback = zwdx_set_trig_holdoff_count,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:SOURce", .callback = zwdx_set_trig_source,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:MODE", .callback = zwdx_set_trig_mode,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:TYPE", .callback = zwdx_set_trig_type,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:LEVEL", .callback = zwdx_set_trig_level,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:HYSteresis", .callback = zwdx_set_trig_hyst,};
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:TRIG:HOLDOFF:TIME", .callback = zwdx_set_trig_holdoff_time,};
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
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:PID:INVERT", .callback = zwdx_set_pid_invert,};
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
	scpi_commands[i++] = (scpi_command_t){.pattern = ":ZWLia:AWG:DATA", 			.callback = zwdx_set_awg_data,};
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
