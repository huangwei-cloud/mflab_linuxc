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

#ifndef __SCPI_DEF_H_
#define __SCPI_DEF_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "scpi.h"
#include "../clk/regctrl.h"
#include "../clk/siregctrl.h"
#include "../clk/l500k_siregctrl.h"
#include "../siginput/siginputctrl.h"
#include "../ring/ringbuffer.h"
#include "../pm/param.h"

#define SCPI_INPUT_BUFFER_LENGTH (1024*12)
#define SCPI_ERROR_QUEUE_SIZE (17)
#define SCPI_IDN1 "ZWDX"
#define SCPI_IDN2 "LIA-L500K"
#define SCPI_IDN3 "010203"
#define SCPI_IDN4 "1.0"

#define CH1 (1)
#define CH2 (2)
#define CH3 (3)
#define CH4 (4)
#define CH5 (5)
#define CH6 (6)
#define CH7 (7)
#define CH8 (8)


#define INTTRIG    (0)
#define EXTTRIG    (1)
#define EXTIN1 (1)
#define EXTIN2 (2)

#define OSC_SAMPLE_RATE (1000000000.0) //1GSa/s
#define PLL_SAMPLE_RATE (500000000.0)  //500MSa/s

typedef struct _SO_RANGE_WAVEFORMAT{
	uint8_t ch1_range;
	uint8_t ch1_wave_format;

	uint8_t ch2_range;
	uint8_t ch2_wave_format;

	uint8_t ch1_range2;
	uint8_t ch2_range2;
}SoRangeWaveFormat;

typedef struct _DEMOD_PARAM {
	int32_t osc_source;
	int32_t ref_mode;  // 0:manual, 1:PLL reference, 2:difference
	int32_t osc_index; // Global OSC0..OSC7 selected by this channel's lookup table
	double osc_freq;
	double osc_harm;
	DemoInputSource source;
} DemodParam;


typedef struct _GLOBAL_PARAM {
	int32_t g_dev_trig_source;
	int32_t g_dev_clk_source;
	int32_t g_dev_lock_state;
	DemodParam g_demod_param[DEVICE_DEMOD_NUM_L500K];

	double  g_osc_int_freq;
	int64_t g_level;
	int64_t g_hyst;
	int32_t g_daq_scope_mode;
	int32_t g_daq_trig_num;
	int32_t g_daq_sample_num;
	int32_t g_trig_number;
	int32_t g_sample_number;
	int32_t g_scope_chopenst;
	VolRange g_vol_range1;
	VolRange g_vol_range2;
	CurRange g_cur_range;
	int32_t g_daq_source;
	ScopeDataSource g_scope_source[2]; /* [0]=Scope CH1, [1]=Scope CH2. */
	SoRangeWaveFormat g_so_range_wave_format;
	SiVolCtrol g_si_vol_ch1_ctrol;
	SiVolCtrol g_si_vol_ch2_ctrol;
	SiCurCtrol g_si_cur_ctrol;
	siginput_config_t g_siginput_config[2];//mflab singleinput config
	singleinput_config_t g_si_config[2];
	l500k_singleinput_config_t g_l500k_si_config[2];
}gLobalParam;

#define SCPI_COMMANDS_MAX 400

void get_global_param(gLobalParam ** glp);
double zwdx_oscillator_ref_clock_hz(void);

extern scpi_command_t scpi_commands[SCPI_COMMANDS_MAX];
extern scpi_interface_t scpi_interface;
extern char scpi_input_buffer[];
extern scpi_error_t scpi_error_queue_data[];
extern scpi_t scpi_context;
void init_scpi_commands(void);

size_t SCPI_Write(scpi_t * context, const char * data, size_t len);
int SCPI_Error(scpi_t * context, int_fast16_t err);
scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val);
scpi_result_t SCPI_Reset(scpi_t * context);
scpi_result_t SCPI_Flush(scpi_t * context);


scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context);

#ifdef __cplusplus
}
#endif
#endif /* __SCPI_DEF_H_ */
