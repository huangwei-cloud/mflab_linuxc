#include <fcntl.h>
#include <poll.h>
#include <inttypes.h>
#include <math.h>
#include "datatask.h"
#include "../axigpio/gpio.h"
#include "../clk/hmc7043.h"
#include "../clk/reg.h"
#include "../kfifo/kfifo.h"
#include "../axidma/libaxidma.h"
#include "../utils/utils.h"
#include "../utils/scpi-def.h"
#include "../pm/param.h"
#include "../sweep/sweep.h"
#include "../clk/hmc7043.h"
#include "../log/log.h"
#include "../clk/siregctrl.h"
#include "../calib/auxincalib_l500k.h"

#define CH_DAQ   1
#define CH_SCOPE 2
#define SOURCE_MAX 8
#define BRAM_READ_SIZE (8*1024)
#define DMA_PACK_SIZE_DAQ 	(16384 * 8)
#define DMA_PACK_SIZE_SCOPE (16384 * 4)
#define SCOPE_FRAME_BYTE_MAX (16384 * 4)
#define DATATASK_DMA_POLL_TIMEOUT_MS 100


#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define DATATASK_DEBUG

#ifdef DATATASK_DEBUG
//#define datatask_printf(fmt, ...) printf(fmt "\n", ##__VA_ARGS__)
#define datatask_printf(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
#define datatask_printf(fmt, ...) ((void)0)
#endif

extern ScanState  g_scan;
extern int g_device_model;
int g_running = 0;

static DataCh daq_source_to_data_ch(int source);

static int datatask_high_dma_channel_index(void)
{
	return (g_device_model == DEVICE_MODEL_L500K) ? 0 : 1;
}

/*****************************************************************************
 Function    : get_channels
 Description : Get channel count. DAQ has 36 sources; Scope has 2 channels, one source per channel.
 Inputs      : int *channel_list
             int ch_type
             int *lowsrc //record whether low/high sources are selected
             int *highsrc
 Outputs     : none
 Return      : int
 Calls       :
 Called by   :

 History     :
  1.Date     : 2025-08-06
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
int get_channels(int *channel_list, int ch_type, int *lowsrc, int *hightsrc)
{
	int rtn;
    uint64_t number;
    int count = 0;
	int lowsrc_cnt = 0;
	int highsrc_cnt = 0;
	int ch1state,ch2state;
    ParamValue pv;
	gLobalParam *glp = NULL;

	get_global_param(&glp);

	if(ch_type == CH_DAQ) {
		rtn = get_param_by_name(PARAM_CURRENT,"daq_source", 0, &pv);
	    number = pv.value.u64;
		printf("number :0x%016" PRIx64 "\r\n", number); // hexadecimal, lowercase
	    for (int i = 0; i < 64; i++) {
	        if ((number >> i) & 0x1) {
				DataCh data_ch = daq_source_to_data_ch(i);
	            channel_list[count] = i;  // record channel index
	            if(data_ch > CH32_Q8) {
					highsrc_cnt++;
				}else {
					lowsrc_cnt++;
				}
	            count++;
	        }
	    }
	}else if(ch_type == CH_SCOPE) {
		get_param_by_name(PARAM_CURRENT,"scope_ch_switch_ch1", 1, &pv);
		ch1state = pv.value.i32;
		get_param_by_name(PARAM_CURRENT,"scope_ch_switch_ch2", 1, &pv);
		ch2state = pv.value.i32;

		if (ch1state == 0 && ch2state == 0) {
			count = 0;
			lowsrc_cnt = 0;
			highsrc_cnt = 0;
		} else if (ch1state == 0 && ch2state == 1) {
			channel_list[0] = CH58_SCOPE_CH2;
			count = 1;
			lowsrc_cnt = 0;
			highsrc_cnt = 1;
		} else if (ch1state == 1 && ch2state == 0) {
			channel_list[0] = CH57_SCOPE_CH1;
			count = 1;
			lowsrc_cnt = 1;//CH1 enabled
			highsrc_cnt = 0;//CH2 disabled
		} else if (ch1state == 1 && ch2state == 1) {
			channel_list[0] = CH57_SCOPE_CH1;
			channel_list[1] = CH58_SCOPE_CH2;
			count = 2;
			lowsrc_cnt = 1;
			highsrc_cnt = 1;
		}

	} else {

	}

	*lowsrc = lowsrc_cnt;
	*hightsrc = highsrc_cnt;
    return count;  // return total channel count
}


int gpio_fd_open(int emio_gpio)
{
    char buf[64];
    int fd;
    int sysfs_gpio;

    if (gpio_get_sysfs_num(emio_gpio, &sysfs_gpio) != 0) {
        return -1;
    }

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", sysfs_gpio);
    fd = open(buf, O_RDONLY | O_NONBLOCK);
    return fd;
}
void read_from_bram1_to_fifo(uint32_t size)
{
	//uint8_t reafbuff[BRAM_READ_SIZE] = {0};
	struct __kfifo *pfifo;
	AxiDevice *pdev;

	get_kfifo_instance(&pfifo);
	get_bram1_intance(&pdev);

	__kfifo_in(pfifo, (uint8_t *)pdev->reg_base, size);
}

void read_from_bram2_to_fifo(uint32_t size)
{
	//uint8_t reafbuff[BRAM_READ_SIZE] = {0};
	struct __kfifo *pfifo;
	AxiDevice *pdev;

	get_kfifo_instance(&pfifo);
	get_bram2_intance(&pdev);

	__kfifo_in(pfifo, (uint8_t *)pdev->reg_base, size);
}

void read_from_dma_to_fifo(uint32_t size)
{
	int rtn;
	dma_context_t *ctx;
	struct __kfifo *pfifo;

	get_dma_context(&ctx);
	get_kfifo_instance(&pfifo);

	rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, size, true);

	if(rtn !=0 ) {
		printf("dma read failed\r\n");
	}else {
		printf("dma read success: %d\r\n",rtn);
	}
	//printhex(ctx->rxbuff, size, 256);
	__kfifo_in(pfifo, ctx->channel[0].rxbuff, size);
}


#define MAX_SAMPLES_PER_CH      SAMPLE_POINT_MAX  // configured for the maximum possible value
#define DAQ_BYTES_PER_SAMPLE    8
#define SCOPE_BYTES_PER_SAMPLE  4
#define SAMPLES_PER_BLOCK       4                         // 4 samples per channel in each block
#define DAQ_BLOCK_BYTES        (SAMPLES_PER_BLOCK * DAQ_BYTES_PER_SAMPLE)
#define SCOPE_BLOCK_BYTES      (SAMPLES_PER_BLOCK * SCOPE_BYTES_PER_SAMPLE)

static double daq_buf[MAX_SAMPLES_PER_CH] = {0};
static double scope_buf[MAX_SAMPLES_PER_CH] = {0};

static inline uint64_t load_u64(const void *p)
{
    uint64_t v;
    memcpy(&v, p, sizeof(v));                       // avoid unaligned access; handle endian conversion here if needed
    return v;
}

static inline uint32_t load_u32(const void *p)
{
    uint32_t v;
    memcpy(&v, p, sizeof(v));                       // avoid unaligned access; handle endian conversion here if needed

    return v;
}

static inline int64_t load_i64(const void *p)
{
    int64_t v;
    memcpy(&v, p, sizeof(v));                       // avoid unaligned access; handle endian conversion here if needed
    return v;
}

// Swap the front/back 4-byte blocks: [b0..b3 | b4..b7] -> [b4..b7 | b0..b3], then read as int64_t
static inline int64_t load_i64_swap32(const void *p)
{
    uint64_t v;
    memcpy(&v, p, 8);   // read in original byte order

    // swap low/high 32-bit words
    uint64_t lo = (v & 0xFFFFFFFFULL);
    uint64_t hi = (v >> 32) & 0xFFFFFFFFULL;

    uint64_t swapped = (lo << 32) | hi;
    return (int64_t)swapped;
}

typedef enum {
	DAQ_SOURCE_KIND_X,
	DAQ_SOURCE_KIND_Y,
	DAQ_SOURCE_KIND_R,
	DAQ_SOURCE_KIND_Q,
	DAQ_SOURCE_KIND_FREQ,
	DAQ_SOURCE_KIND_AUXIN,
	DAQ_SOURCE_KIND_AUXOUT,
	DAQ_SOURCE_KIND_PID,
	DAQ_SOURCE_KIND_PCNT,
	DAQ_SOURCE_KIND_UNKNOWN,
} DaqSourceKind;

static int daq_source_demod_count_by_model(void)
{
	return (g_device_model == DEVICE_MODEL_L500K) ? DEVICE_DEMOD_NUM_L500K : DEVICE_DEMOD_NUM_M005M;
}

static DaqSourceKind daq_source_get_kind_index(int source, unsigned *index)
{
	int demod_count = daq_source_demod_count_by_model();
	int base;

	if (source >= 0 && source < demod_count * 4) {
		*index = (unsigned)(source / 4);
		switch(source % 4) {
		case 0: return DAQ_SOURCE_KIND_X;
		case 1: return DAQ_SOURCE_KIND_Y;
		case 2: return DAQ_SOURCE_KIND_R;
		case 3: return DAQ_SOURCE_KIND_Q;
		default: break;
		}
	}
	base = demod_count * 4;
	if (source >= base && source < base + demod_count) {
		*index = (unsigned)(source - base);
		return DAQ_SOURCE_KIND_FREQ;
	}
	base = demod_count * 5;
	if (source >= base && source < base + 4) {
		*index = (unsigned)(source - base);
		return DAQ_SOURCE_KIND_AUXIN;
	}
	base += 4;
	if (source >= base && source < base + 4) {
		*index = (unsigned)(source - base);
		return DAQ_SOURCE_KIND_AUXOUT;
	}
	base += 4;
	if (source >= base && source < base + 4) {
		*index = (unsigned)(source - base);
		return DAQ_SOURCE_KIND_PID;
	}
	base += 4;
	if (source >= base && source < base + 4) {
		*index = (unsigned)(source - base);
		return DAQ_SOURCE_KIND_PCNT;
	}

	*index = 0;
	return DAQ_SOURCE_KIND_UNKNOWN;
}

static DataCh daq_source_to_data_ch(int source)
{
	unsigned index;
	DaqSourceKind kind = daq_source_get_kind_index(source, &index);

	switch(kind) {
	case DAQ_SOURCE_KIND_X: return (DataCh)(CH1_X1 + index * 4);
	case DAQ_SOURCE_KIND_Y: return (DataCh)(CH2_Y1 + index * 4);
	case DAQ_SOURCE_KIND_R: return (DataCh)(CH3_R1 + index * 4);
	case DAQ_SOURCE_KIND_Q: return (DataCh)(CH4_Q1 + index * 4);
	case DAQ_SOURCE_KIND_FREQ: return (DataCh)(CH33_FREQ1 + index);
	case DAQ_SOURCE_KIND_AUXIN: return (DataCh)(CH41_AUXIN1 + index);
	case DAQ_SOURCE_KIND_AUXOUT: return (DataCh)(CH45_AUXOUT1 + index);
	case DAQ_SOURCE_KIND_PID: return (DataCh)(CH49_PID1 + index);
	case DAQ_SOURCE_KIND_PCNT: return (DataCh)(CH53_PCNT1 + index);
	default: return CH1_X1;
	}
}

static DemoInputSource get_demo_source_by_daq_source(const gLobalParam *glp, int source)
{
	unsigned demod_index;

	if(daq_source_get_kind_index(source, &demod_index) > DAQ_SOURCE_KIND_Q) {
		return DEMO_INPUT_CH1;
	}

	if(demod_index >= DEVICE_DEMOD_NUM_L500K) {
		return DEMO_INPUT_CH1;
	}

	return glp->g_demod_param[demod_index].source;
}

static double load_double_swap32(const void *p, int source)
{
	gLobalParam *glp = NULL;
	MappingTable *cvmt = NULL;
	DemoInputSource demo_source;
	DaqSourceKind source_kind;
	unsigned source_index;
	double value = 0.0;
    uint64_t v;
	int64_t swapped;
	int16_t aux_code;
	uint64_t swapped_u;
	uint64_t lo, hi;

	get_cv_maptable(&cvmt);
	get_global_param(&glp);
	demo_source = get_demo_source_by_daq_source(glp, source);
	source_kind = daq_source_get_kind_index(source, &source_index);
    memcpy(&v, p, 8);   // read in original byte order

    // swap low/high 32-bit words
    lo = (v & 0xFFFFFFFFULL);
    hi = (v >> 32) & 0xFFFFFFFFULL;
    swapped_u = (lo << 32) | hi;
    swapped = (int64_t)swapped_u;

	//P_LOG_DEBUG("demo_source: %d, kind:%d, code:0x%016" PRIx64 "\r\n", demo_source, source_kind, swapped_u);
	switch(source_kind) {
	case DAQ_SOURCE_KIND_X:
		if(demo_source == DEMO_INPUT_CH1) {
			if(g_device_model == DEVICE_MODEL_M005M_VV || g_device_model == DEVICE_MODEL_M005M_VI) {
				 value = swapped * cvmt->voltage1[glp->g_vol_range1].range / cvmt->voltage1[glp->g_vol_range1].X; //FPGA ALREADY X 2
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_si_config[0].range-1].range / cvmt->voltage1[glp->g_si_config[0].range-1].X;
				} else if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[0].range-1].range / cvmt->current[glp->g_si_config[0].range-1].X;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].range / cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].X;
				} else if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[0].i_range-1].range / cvmt->current[glp->g_l500k_si_config[0].i_range-1].X;
				}
				//printf("%" PRId64 " * %.5f / %lld = %.9f\r\n", swapped, cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].range, cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].X, value);
			}
		}else if(demo_source == DEMO_INPUT_CH2) {
			if(g_device_model == DEVICE_MODEL_M005M_VV) {
				value = swapped * cvmt->voltage2[glp->g_vol_range2].range / cvmt->voltage2[glp->g_vol_range2].X;
			}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
				value = swapped * cvmt->current[glp->g_cur_range].range / cvmt->current[glp->g_cur_range].X;
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_si_config[1].range-1].range / cvmt->voltage2[glp->g_si_config[1].range-1].X;
				} else if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[1].range-1].range / cvmt->current[glp->g_si_config[1].range-1].X;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].range / cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].X;
				} else if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[1].i_range-1].range / cvmt->current[glp->g_l500k_si_config[1].i_range-1].X;
				}
			}
		}
		break;
	case DAQ_SOURCE_KIND_Y:
		if(demo_source == DEMO_INPUT_CH1) {
			if(g_device_model == DEVICE_MODEL_M005M_VV || g_device_model == DEVICE_MODEL_M005M_VI) {
				 value = swapped * cvmt->voltage1[glp->g_vol_range1].range / cvmt->voltage1[glp->g_vol_range1].Y;
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_si_config[0].range-1].range / cvmt->voltage1[glp->g_si_config[0].range-1].Y;
				} else if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[0].range-1].range / cvmt->current[glp->g_si_config[0].range-1].Y;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].range / cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].Y;
				} else if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[0].i_range-1].range / cvmt->current[glp->g_l500k_si_config[0].i_range-1].Y;
				}
			}
		}else if(demo_source == DEMO_INPUT_CH2) {
			if(g_device_model == DEVICE_MODEL_M005M_VV) {
				value = swapped * cvmt->voltage2[glp->g_vol_range2].range / cvmt->voltage2[glp->g_vol_range2].Y;
			}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
				value = swapped * cvmt->current[glp->g_cur_range].range / cvmt->current[glp->g_cur_range].Y;
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_si_config[1].range-1].range / cvmt->voltage2[glp->g_si_config[1].range-1].Y;
				} else if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[1].range-1].range / cvmt->current[glp->g_si_config[1].range-1].Y;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].range / cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].Y;
				} else if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[1].i_range-1].range / cvmt->current[glp->g_l500k_si_config[1].i_range-1].Y;
				}
			}
		}
		break;
	case DAQ_SOURCE_KIND_R:
		if(demo_source == DEMO_INPUT_CH1) {
			if(g_device_model == DEVICE_MODEL_M005M_VV || g_device_model == DEVICE_MODEL_M005M_VI) {
				value = swapped * cvmt->voltage1[glp->g_vol_range1].range / cvmt->voltage1[glp->g_vol_range1].R;
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_si_config[0].range-1].range / cvmt->voltage1[glp->g_si_config[0].range-1].R;
				} else if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[0].range-1].range / cvmt->current[glp->g_si_config[0].range-1].R;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				//P_LOG_DEBUG("swapped: 0x%016" PRIx64 "\r\n", (uint64_t)swapped);
				//P_LOG_DEBUG("L500K R demo source 1 measure mode: %d", glp->g_l500k_si_config[0].measure_mode);
				if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].range / cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].R;
					//P_LOG_DEBUG("value: %.9f\r\n", value);
				} else if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[0].i_range-1].range / cvmt->current[glp->g_l500k_si_config[0].i_range-1].R;
				}
			}
			//printf("%" PRId64 " * %.5f / %lld = %.9f\r\n", swapped, cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].range, cvmt->voltage1[glp->g_l500k_si_config[0].v_range-1].R, value);
		}else if(demo_source == DEMO_INPUT_CH2) {
			if(g_device_model == DEVICE_MODEL_M005M_VV) {
				value = swapped * cvmt->voltage2[glp->g_vol_range2].range / cvmt->voltage2[glp->g_vol_range2].R;
			}else if(g_device_model == DEVICE_MODEL_M005M_VI) {
				value = swapped * cvmt->current[glp->g_cur_range].range / cvmt->current[glp->g_cur_range].R;
			}else if(g_device_model == DEVICE_MODEL_M005M) {
				if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_si_config[1].range-1].range / cvmt->voltage2[glp->g_si_config[1].range-1].R;
				} else if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
					value = swapped * cvmt->current[glp->g_si_config[1].range-1].range / cvmt->current[glp->g_si_config[1].range-1].R;
				}
			}else if(g_device_model == DEVICE_MODEL_L500K) {
				if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
					value = swapped * cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].range / cvmt->voltage2[glp->g_l500k_si_config[1].v_range-1].R;
				} else if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
					value = swapped * cvmt->current[glp->g_l500k_si_config[1].i_range-1].range / cvmt->current[glp->g_l500k_si_config[1].i_range-1].R;
				}
			}
		}
		break;
	case DAQ_SOURCE_KIND_Q:
		value = swapped * (180.0 / (double)(1LL << 45)); //+/-180
		break;
	case DAQ_SOURCE_KIND_PCNT:
		value = swapped;
		break;
	case DAQ_SOURCE_KIND_AUXOUT:
		/* AUXOUT uses an unsigned 20-bit code: 0..0xFFFFF maps to -10 V..+10 V. */
		value = ((double)(swapped_u & 0xFFFFFULL) * 20.0) / ((double)((1ULL << 20) - 1));
		value -= 10;
		break;
	case DAQ_SOURCE_KIND_AUXIN:
		/* AUXIN uses the lower 16 bits as a signed code: bit15 is sign, +/-10 V full scale. */
		aux_code = (int16_t)(swapped_u & 0xFFFFULL);
		value = ((double)aux_code * 10.0) / ((double)((1ULL << 15) - 1));
		if(g_device_model == DEVICE_MODEL_L500K) {
			value = auxin_l500k_calibrate_value(source_index, value);
		}
		break;
	default:
		break;
	}

    return value;
}



static inline int32_t load_i32(const void *p)
{
    int32_t v;
    memcpy(&v, p, sizeof(v));                       // avoid unaligned access; handle endian conversion here if needed
    return v;
}

static double scope_aux_like_to_voltage(int32_t temp)
{
	uint32_t raw = (uint32_t)temp & 0xFFFFFU;

	return ((double)raw * 20.0) / ((double)((1U << 20) - 1)) - 10.0;
}

static double scope_logic_level(int32_t temp)
{
	return temp > 0 ? 1.0 : 0.0;
}

static double scope_signal_input_to_value(int32_t temp, int channel, gLobalParam *glp, MappingTable *cvmt)
{
	double v = 0.0;

	if(channel == 0) {
		if(g_device_model == DEVICE_MODEL_M005M_VV || g_device_model == DEVICE_MODEL_M005M_VI) {
			v = temp * cvmt->voltage1[glp->g_vol_range1].range / cvmt->voltage1[glp->g_vol_range1].R1;
		} else if(g_device_model == DEVICE_MODEL_M005M) {
			if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
				v = temp * cvmt->voltage1[glp->g_si_config[0].range - 1].range / cvmt->voltage1[glp->g_si_config[0].range - 1].R1;
			} else if(glp->g_si_config[0].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
				v = temp * cvmt->current[glp->g_si_config[0].range - 1].range / cvmt->current[glp->g_si_config[0].range - 1].R1;
			} else {
				P_LOG_ERROR("load_double: unknown M005M ch1 measure mode %d", glp->g_si_config[0].measure_mode);
			}
		} else if(g_device_model == DEVICE_MODEL_L500K) {
			if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
				v = temp * cvmt->voltage1[glp->g_l500k_si_config[0].v_range - 1].range / cvmt->voltage1[glp->g_l500k_si_config[0].v_range - 1].R1;
			} else if(glp->g_l500k_si_config[0].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
				v = temp * cvmt->current[glp->g_l500k_si_config[0].i_range - 1].range / cvmt->current[glp->g_l500k_si_config[0].i_range - 1].R1;
			} else {
				P_LOG_ERROR("load_double: unknown L500K ch1 measure mode %d", glp->g_l500k_si_config[0].measure_mode);
			}
		}
	} else if(channel == 1) {
		if(g_device_model == DEVICE_MODEL_M005M_VV) {
			v = temp * cvmt->voltage2[glp->g_vol_range2].range / cvmt->voltage2[glp->g_vol_range2].R1;
		} else if(g_device_model == DEVICE_MODEL_M005M_VI) {
			v = temp * cvmt->current[glp->g_cur_range].range / cvmt->current[glp->g_cur_range].R1;
		} else if(g_device_model == DEVICE_MODEL_M005M) {
			if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_VOLTAGE) {
				v = temp * cvmt->voltage1[glp->g_si_config[1].range - 1].range / cvmt->voltage1[glp->g_si_config[1].range - 1].R1;
			} else if(glp->g_si_config[1].measure_mode == SINGLEINPUT_MEASURE_MODE_CURRENT) {
				v = temp * cvmt->current[glp->g_si_config[1].range - 1].range / cvmt->current[glp->g_si_config[1].range - 1].R1;
			} else {
				P_LOG_ERROR("load_double: unknown M005M ch2 measure mode %d", glp->g_si_config[1].measure_mode);
			}
		} else if(g_device_model == DEVICE_MODEL_L500K) {
			if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_VOLTAGE) {
				v = temp * cvmt->voltage1[glp->g_l500k_si_config[1].v_range - 1].range / cvmt->voltage1[glp->g_l500k_si_config[1].v_range - 1].R1;
			} else if(glp->g_l500k_si_config[1].measure_mode == L500K_SINGLEINPUT_MEASURE_CURRENT) {
				v = temp * cvmt->current[glp->g_l500k_si_config[1].i_range - 1].range / cvmt->current[glp->g_l500k_si_config[1].i_range - 1].R1;
			} else {
				P_LOG_ERROR("load_double: unknown L500K ch2 measure mode %d", glp->g_l500k_si_config[1].measure_mode);
			}
		}
	}

	return v;
}

double load_double(const void *p)
{
	int32_t temp;
	double v = 0.0;
	gLobalParam *glp = NULL;
	MappingTable *cvmt = NULL;
	get_cv_maptable(&cvmt);
	get_global_param(&glp);

	memcpy(&temp, p, sizeof(temp)); /* avoid unaligned access */

	if(g_device_model == DEVICE_MODEL_L500K) {
		switch(glp->g_scope_source) {
		case SCOPE_SOURCE_SIGNALIN1:
			v = scope_signal_input_to_value(temp, 0, glp, cvmt);
			break;
		case SCOPE_SOURCE_SIGNALIN2:
			v = scope_signal_input_to_value(temp, 1, glp, cvmt);
			break;
		case SCOPE_SOURCE_AUXIN1:
		case SCOPE_SOURCE_AUXIN2:
		case SCOPE_SOURCE_AUXIN3:
		case SCOPE_SOURCE_AUXIN4:
			/* AUXIN uses a sign-extended 16-bit code: -32768..32767 maps to about -10 V..+10 V. */
			v = (double)(temp * 10.0) / ((double)((1ULL << 15) - 1));
			v = auxin_l500k_calibrate_value((unsigned)(glp->g_scope_source - SCOPE_SOURCE_AUXIN1), v);
			break;
		case SCOPE_SOURCE_L500K_REFIN1:
		case SCOPE_SOURCE_L500K_REFIN2:
			/* REFIN uses a sign-extended 16-bit code: -32768..32767 maps to about -2.5 V..+2.5 V. */
			v = (double)(temp * 2.5) / ((double)((1ULL << 15) - 1));
			break;
		case SCOPE_SOURCE_L500K_TRIG1:
		case SCOPE_SOURCE_L500K_TRIG2:
		case SCOPE_SOURCE_L500K_TRIG3:
		case SCOPE_SOURCE_L500K_TRIG4:
			v = scope_logic_level(temp);
			break;
		default:
			P_LOG_ERROR("load_double: unknown L500K scope source %d", glp->g_scope_source);
			break;
		}
	} else if(g_device_model == DEVICE_MODEL_M005M_VV
			  || g_device_model == DEVICE_MODEL_M005M_VI
			  || g_device_model == DEVICE_MODEL_M005M) {
		switch(glp->g_scope_source) {
		case SCOPE_SOURCE_SIGNALIN1:
			v = scope_signal_input_to_value(temp, 0, glp, cvmt);
			break;
		case SCOPE_SOURCE_SIGNALIN2:
			v = scope_signal_input_to_value(temp, 1, glp, cvmt);
			break;
		case SCOPE_SOURCE_AUXIN1:
		case SCOPE_SOURCE_AUXIN2:
		case SCOPE_SOURCE_AUXIN3:
		case SCOPE_SOURCE_AUXIN4:
			v = scope_aux_like_to_voltage(temp);
			break;
		case SCOPE_SOURCE_TRIG1:
		case SCOPE_SOURCE_TRIG2:
			v = scope_logic_level(temp);
			break;
		default:
			P_LOG_ERROR("load_double: unknown M005M scope source %d", glp->g_scope_source);
			break;
		}
	} else {
		P_LOG_ERROR("load_double: unsupported device model %d", g_device_model);
	}

	return v;
}
double mean_double_array(const double *data, int len)
{
    if (!data || len == 0) {
        return 0.0;  /* avoid divide by zero */
    }

    double sum = 0.0;
    for (int i = 0; i < len; ++i) {
        sum += data[i];
    }

    return sum / (double)len;
}


void trig_source_signal_reset()
{
	int source;

	ParamValue getpv;
	get_param_by_name(PARAM_CURRENT,"trig_source", 0, &getpv);/*read previous trigger source*/
	source = getpv.value.i32 + 1;

	if(g_device_model == DEVICE_MODEL_L500K) {
		if(source >= TS_L500K_49_SIGNALOUT1_PHASE
		   && source <= TS_L500K_52_TRIGIN1) {
			source += 3;
		}
	}else {
		if(source == TS_M005M_37_SIGNALOUT1_PHASE) {
			source = TS_M005M_40_TRIGIN1;
		}else if(source == TS_M005M_38_SIGNALOUT2_PHASE) {
			source = TS_M005M_41_TRIGIN2;
		}
	}

	reg_write_by_name("trig_source", source);
}





/*****************************************************************************
 Function    : demux_one_frame
 Description : Demux DAQ multi-source data. Scope channels do not need source demux because each channel comes from an independent DMA path.
 Inputs      : void *rxbuff
             int total_bytes
             int *ch_list
             int num_channels
             int ch_type
 Outputs     : none
 Return      : int
 Calls       :
 Called by   :

 History     :
  1.Date     : 2025-08-16
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
int demux_one_frame(void *rxbuff, int total_bytes, int *ch_list, int num_channels, int ch_type)
{
	int set_bytes,dst, s;
	DataCh data_ch;
	FrameRingBuffer *frb = NULL;
	int num_sets;
	int samples_per_ch;
	double average;
	uint8_t *base = (uint8_t *)rxbuff;
	DemoDaqData *pavgdemodata = NULL;

	get_rb_multiple_instance(&frb);
	get_demo_adata_instance(&pavgdemodata);

	if (num_channels <= 0) return -1;

	if(ch_type == CH_DAQ) {

		set_bytes = (size_t)num_channels * DAQ_BLOCK_BYTES; 	// bytes per group; each channel has 4 samples
		if (set_bytes == 0 || (total_bytes % set_bytes) != 0) {
			fprintf(stderr, "demux: length and channels, block size not aligned\n");
			return -1;
		}

		num_sets = total_bytes / set_bytes; 				// number of rows
		samples_per_ch = num_sets * SAMPLES_PER_BLOCK;		// total samples per channel

		// Aggregate by channel: each group stores one block for this channel, block size is 4*8=32 bytes
		for (int ch = 0; ch < num_channels; ++ch) {
			dst = 0;
			for (s = 0; s < num_sets; ++s) {
				const uint8_t *blk = base + s * set_bytes + (size_t)ch * DAQ_BLOCK_BYTES;
				// copy the 4 samples for this channel in the group
				for (int b = 0; b < SAMPLES_PER_BLOCK; ++b) {
					//daq_buf[dst++] = load_i64_swap32(blk + b * DAQ_BYTES_PER_SAMPLE);
					daq_buf[dst++] = load_double_swap32(blk + b * DAQ_BYTES_PER_SAMPLE, ch_list[ch]);
				}
			}
			//save the final average for VISA demod-data query
			average = mean_double_array(daq_buf, dst);
			data_ch = daq_source_to_data_ch(ch_list[ch]);
			save_demo_data(pavgdemodata, data_ch, average);
#if 0
			// save DMA data to file
	        FILE *fp = fopen("dma_dump.bin", "wb");
	        if (!fp) {
	            perror("Failed to open dump file");
	        } else {
	            size_t written = fwrite(daq_buf, 1, total_bytes, fp);
	            if (written != total_bytes) {
	                fprintf(stderr, "Warning: wrote %zu of %d bytes\n", written, total_bytes);
	            }
	            fclose(fp);
	            printf("DMA buffer saved to dma_dump.bin (%zu bytes)\n", written);
	        }
#endif
			//printhex(daq_buf, total_bytes/num_channels, 24);

			//printf("IN QUENUE CH: %d Size:%d\r\n", ch_list[ch], samples_per_ch * DAQ_BYTES_PER_SAMPLE);

			// write all samples of this channel at once
			if( (g_scan.initialized == 1) && (g_scan.finished == 0) ) { //scan initialized and not finished
				frame_buffer_write(frb+data_ch, data_ch, (uint8_t *)&average, 8);//write average value to the scan channel
			}else {
				frame_buffer_write(frb+data_ch, data_ch, (uint8_t *)daq_buf, samples_per_ch * DAQ_BYTES_PER_SAMPLE);//write frame data to the corresponding channel
			}
		}
	}else if(ch_type == CH_SCOPE) {
		set_bytes = (size_t)num_channels * SCOPE_BLOCK_BYTES; 	// bytes per group; each channel has 4 samples
		if (set_bytes == 0 || (total_bytes % set_bytes) != 0) {
			fprintf(stderr, "demux: length and channels, block size not aligned\n");
			return -1;
		}

		num_sets = total_bytes / set_bytes; 				// number of groups
		samples_per_ch = num_sets * SAMPLES_PER_BLOCK;		// total samples per channel

		// Aggregate by channel: each group stores one block for this channel, block size is 4*8=32 bytes
		for (int ch = 0; ch < num_channels; ++ch) {
			dst = 0;
			for (s = 0; s < num_sets; ++s) {
				const uint8_t *blk = base + s * set_bytes + (size_t)ch * SCOPE_BLOCK_BYTES;
				// copy the 4 samples for this channel in the group
				for (int b = 0; b < SAMPLES_PER_BLOCK; ++b) {
					//scope_buf[dst++] = load_u32(blk + b * SCOPE_BYTES_PER_SAMPLE);
					scope_buf[dst++] = load_double(blk + b * SCOPE_BYTES_PER_SAMPLE);

				}
			}
#if 0
			// save DMA data to file
	        FILE *fp = fopen("dma_dump.bin", "wb");
	        if (!fp) {
	            perror("Failed to open dump file");
	        } else {
	            size_t written = fwrite(daq_buf, 1, total_bytes, fp);
	            if (written != total_bytes) {
	                fprintf(stderr, "Warning: wrote %zu of %d bytes\n", written, total_bytes);
	            }
	            fclose(fp);
	            printf("DMA buffer saved to dma_dump.bin (%zu bytes)\n", written);
	        }
#endif

			//save the final average for VISA demod-data query
			average = mean_double_array(scope_buf, dst);

			save_demo_data(pavgdemodata, ch_list[ch], average);

			// write all samples of this channel at once
			// SCOPE_BYTES_PER_SAMPLE is the raw PL sample width. scope_buf has been converted to double.
			{
				size_t scope_frame_len = samples_per_ch * sizeof(double);
				bool write_ok = frame_buffer_write(&frb[ch_list[ch]], ch_list[ch], (uint8_t *)scope_buf, scope_frame_len);
				P_LOG_DEBUG("Scope frame write: ch=%d samples=%d len=%u ok=%d\r\n",
					ch_list[ch], samples_per_ch, (unsigned int)scope_frame_len, write_ok ? 1 : 0);
			}
		}
	}

	return 0;
}

/*****************************************************************************
 Function    : demux_frame_part_to_ff
 Description : Parse partial frame data into the frame-filter structure
 Inputs      : void *rxbuff
             int total_bytes
             int *ch_list
             int num_channels
             int ch_type
             FrameFilter *ff
 Outputs     : none
 Return      : int
 Calls       :
 Called by   :

 History     :
  1.Date     : 2026-03-09
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
int demux_frame_part_to_ff(void *rxbuff, int total_bytes, int *ch_list, int num_channels, int ch_type, FrameFilter *ff)
{
	int set_bytes,dst, s;
	int num_sets;
	int samples_per_ch;
	double average;
	uint8_t *base = (uint8_t *)rxbuff;

	if (num_channels <= 0) return -1;

	if(ch_type == CH_SCOPE) {
		set_bytes = (size_t)num_channels * SCOPE_BLOCK_BYTES; 	// bytes per group; each channel has 4 samples
		if (set_bytes == 0 || (total_bytes % set_bytes) != 0) {
			fprintf(stderr, "demux: length and channels, block size not aligned\n");
			return -1;
		}

		num_sets = total_bytes / set_bytes; 				// number of groups
		samples_per_ch = num_sets * SAMPLES_PER_BLOCK;		// total samples per channel

		// Aggregate by channel: each group stores one block for this channel, block size is 4*8=32 bytes
		for (int ch = 0; ch < num_channels; ++ch) {
			dst = 0;
			for (s = 0; s < num_sets; ++s) {
				const uint8_t *blk = base + s * set_bytes + (size_t)ch * SCOPE_BLOCK_BYTES;
				// copy the 4 samples for this channel in the group
				for (int b = 0; b < SAMPLES_PER_BLOCK; ++b) {
					//scope_buf[dst++] = load_u32(blk + b * SCOPE_BYTES_PER_SAMPLE);
					scope_buf[dst++] = load_double(blk + b * SCOPE_BYTES_PER_SAMPLE);
				}
			}

			// write partial samples
			frame_filter_accumulate(ff, scope_buf, dst);
			//printhex(ff->cur_frame, 8, 8);
			//printf("\r\n");
		}
	}

	return 0;
}

/*****************************************************************************
 Function    : read_from_dma_to_ringbuff
 Description : Read data from PL DDR to PS DDR
 Inputs      : uint32_t size in bytes; this is the size of a single source because the function separates two DMA paths internally
             int32_t ch_type
 Outputs     : none
 Return      : void
 Calls       :
 Called by   :

 History     :
  1.Date     : 2025-08-06
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
void read_from_dma_to_ringbuff(uint32_t size, int32_t ch_type)
{
	int i, rtn, chcnt;
	int total_bytes;
	int source_lowcnt;
	int source_highcnt;
	int channel_list[SOURCE_MAX];
	int channel_list_l[SOURCE_MAX], channel_list_h[SOURCE_MAX];
	dma_context_t *ctx = NULL;
	FrameRingBuffer *frb = NULL;

	get_dma_context(&ctx);
	get_rb_multiple_instance(&frb);

	chcnt = get_channels(channel_list, ch_type, &source_lowcnt, &source_highcnt);
	memcpy(channel_list_l, channel_list, sizeof(int) * source_lowcnt);
	memcpy(channel_list_h, channel_list + source_lowcnt, sizeof(int) * source_highcnt);

	if(source_lowcnt) {
		//[7] ch1_up_data_mode set upload mode
		datatask_printf("[%d]\r\n", 7);
		reg_write_by_name("ch1_up_data_mode", 2);

		//[8] read_block select which block to read
		datatask_printf("[%d]\r\n", 8);
		reg_write_by_name("ch1_read_block", 1);

		//[9] dma_up_en enable DMA
		datatask_printf("[%d]\r\n", 9);
		reg_write_by_name("ch1_dma_up_en", 1);

		//[10] dma_rd_length set DMA read length
		datatask_printf("[%d]\r\n", 10);
		reg_write_by_name("ch1_dma_rd_length", (size / 16 * source_lowcnt) );
	}

	if(source_highcnt) {
		//[7] ch1_up_data_mode set upload mode
		datatask_printf("[%d]\r\n", 7);
		reg_write_by_name("ch2_up_data_mode", 2);

		//[8] read_block select which block to read
		datatask_printf("[%d]\r\n", 8);
		reg_write_by_name("ch2_read_block", 1);

		//[9] dma_up_en enable DMA
		datatask_printf("[%d]\r\n", 9);
		reg_write_by_name("ch2_dma_up_en", 1);

		//[10] dma_rd_length set DMA read length
		datatask_printf("[%d]\r\n", 10);
		reg_write_by_name("ch2_dma_rd_length", (size / 16 * source_highcnt));
	}

	//[11] soft trig issue soft trigger after applying parameters [7] and [8]
	datatask_printf("[%d]\r\n", 11);
	reg_write_by_name("ch1_updata_soft_trig", 0);
	reg_write_by_name("ch1_updata_soft_trig", 1);
	reg_write_by_name("ch1_updata_soft_trig", 0);


	reg_write_by_name("ch2_updata_soft_trig", 0);
	reg_write_by_name("ch2_updata_soft_trig", 1);
	reg_write_by_name("ch2_updata_soft_trig", 0);

	if(ch_type == CH_DAQ) {
		if(source_lowcnt != 0) {
			datatask_printf("read low source: %d\r\n", source_lowcnt);
			total_bytes = size*source_lowcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}
#if 0
			// save DMA data to file
	        FILE *fp = fopen("dma_dump.bin", "wb");
	        if (!fp) {
	            perror("Failed to open dump file");
	        } else {
	            size_t written = fwrite(daq_buf, 1, total_bytes, fp);
	            if (written != total_bytes) {
	                fprintf(stderr, "Warning: wrote %zu of %d bytes\n", written, total_bytes);
	            }
	            fclose(fp);
	            printf("DMA buffer saved to dma_dump.bin (%zu bytes)\n", written);
	        }
#endif
			demux_one_frame(ctx->channel[0].rxbuff, total_bytes, channel_list_l, source_lowcnt, CH_DAQ);

		}

		if(source_highcnt !=0) {
			int high_dma_ch = datatask_high_dma_channel_index();
			datatask_printf("read high source: %d\r\n", source_highcnt);
			total_bytes = size*source_highcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[high_dma_ch].rx_channel, ctx->channel[high_dma_ch].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			demux_one_frame(ctx->channel[high_dma_ch].rxbuff, total_bytes, channel_list_h, source_highcnt, CH_DAQ);
		}

	}else if(ch_type == CH_SCOPE) {
		if(source_lowcnt != 0) { /*CH1*/
			datatask_printf("read low source: %d\r\n", source_lowcnt);
			total_bytes = size * source_lowcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			channel_list_l[0] = CH57_SCOPE_CH1;
			demux_one_frame(ctx->channel[0].rxbuff, total_bytes, channel_list_l, source_lowcnt, CH_SCOPE);

		}

		if(source_highcnt != 0) { /*CH2*/
			int high_dma_ch = datatask_high_dma_channel_index();
			datatask_printf("read high source: %d\r\n", source_highcnt);
			total_bytes = size*source_highcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[high_dma_ch].rx_channel, ctx->channel[high_dma_ch].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			channel_list_h[0] = CH58_SCOPE_CH2;
			demux_one_frame(ctx->channel[high_dma_ch].rxbuff, total_bytes, channel_list_h, source_highcnt, CH_SCOPE);

		}
	}else {

	}
}
/*****************************************************************************
 Function    : read_bd_from_dma_to_ringbuff
 Description : Read large data from PL DDR to PS DDR for processing and upload; DAQ large-data path
 Inputs      : uint32_t size
             int32_t ch_type
 Outputs     : none
 Return      : void
 Calls       :
 Called by   :

 History     :
  1.Date     : 2026-02-27
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
void read_bd_from_dma_to_ringbuff(uint32_t size, int32_t ch_type)
{
	int i, rtn, chcnt,packs_int,packs_remain;
	int total_bytes,dmapack_size;
	int source_lowcnt;
	int source_highcnt;
	int channel_list[SOURCE_MAX];
	int channel_list_l[SOURCE_MAX], channel_list_h[SOURCE_MAX];
	dma_context_t *ctx = NULL;
	FrameRingBuffer *frb = NULL;

	get_dma_context(&ctx);
	get_rb_multiple_instance(&frb);

	chcnt = get_channels(channel_list, ch_type, &source_lowcnt, &source_highcnt);
	memcpy(channel_list_l, channel_list, sizeof(int) * source_lowcnt);
	memcpy(channel_list_h, channel_list + source_lowcnt, sizeof(int) * source_highcnt);

	if(source_lowcnt) {
		//[7] ch1_up_data_mode set upload mode
		datatask_printf("[%d]\r\n", 7);
		reg_write_by_name("ch1_up_data_mode", 2);

		//[8] read_block select which block to read
		datatask_printf("[%d]\r\n", 8);
		reg_write_by_name("ch1_read_block", 1);
	}

	if(source_highcnt) {
		//[7] ch1_up_data_mode set upload mode
		datatask_printf("[%d]\r\n", 7);
		reg_write_by_name("ch2_up_data_mode", 2);

		//[8] read_block select which block to read
		datatask_printf("[%d]\r\n", 8);
		reg_write_by_name("ch2_read_block", 1);
	}

	//[11] soft trig issue soft trigger after applying parameters [7] and [8]
	datatask_printf("[%d]\r\n", 11);
	reg_write_by_name("ch1_updata_soft_trig", 0);
	reg_write_by_name("ch1_updata_soft_trig", 1);
	reg_write_by_name("ch1_updata_soft_trig", 0);

	reg_write_by_name("ch2_updata_soft_trig", 0);
	reg_write_by_name("ch2_updata_soft_trig", 1);
	reg_write_by_name("ch2_updata_soft_trig", 0);

	//[12] read large data in batches
	if(ch_type == CH_DAQ) {
		if(source_lowcnt != 0) {

			total_bytes = size * source_lowcnt;//total byte count

			//[8][8][8][8] | [8][8][8][8] | [8][8][8][8]...
			dmapack_size = (DMA_PACK_SIZE_DAQ / (4 * 8 * source_lowcnt)) * (4 * 8 * source_lowcnt); //packet size must be an integer multiple of one data row
			dmapack_size = MIN(dmapack_size, total_bytes);
			packs_int = total_bytes / dmapack_size;
			packs_remain = total_bytes % dmapack_size;

			printf("dmapacksize: %d, packs: %d = %d / %d, remain: %d\r\n", dmapack_size, packs_int, total_bytes, dmapack_size, packs_remain);
			for(i = 0; i < packs_int; i++) {
				reg_write_by_name("ch1_dma_up_en", 1);
				reg_write_by_name("ch1_dma_up_data_clr", 0);
				reg_write_by_name("ch1_dma_rd_length", (dmapack_size / 16) );
				axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, dmapack_size, true);
				demux_one_frame(ctx->channel[0].rxbuff, dmapack_size, channel_list_l, source_lowcnt, CH_DAQ);
			}
			if(packs_remain > 0) {
				reg_write_by_name("ch1_dma_up_en", 1);
				reg_write_by_name("ch1_dma_up_data_clr", 0);
				reg_write_by_name("ch1_dma_rd_length", (packs_remain / 16) );
				axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, packs_remain, true);
				demux_one_frame(ctx->channel[0].rxbuff, packs_remain, channel_list_l, source_lowcnt, CH_DAQ);
			}
		}

		if(source_highcnt !=0) {
			int high_dma_ch = datatask_high_dma_channel_index();
			datatask_printf("read high source: %d\r\n", source_highcnt);
			total_bytes = size*source_highcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[high_dma_ch].rx_channel, ctx->channel[high_dma_ch].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			demux_one_frame(ctx->channel[high_dma_ch].rxbuff, total_bytes, channel_list_h, source_highcnt, CH_DAQ);
		}

	}else if(ch_type == CH_SCOPE) {
		if(source_lowcnt != 0) { /*CH1*/
			datatask_printf("read low source: %d\r\n", source_lowcnt);
			total_bytes = size * source_lowcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			channel_list_l[0] = CH57_SCOPE_CH1;
			demux_one_frame(ctx->channel[0].rxbuff, total_bytes, channel_list_l, source_lowcnt, CH_SCOPE);

		}

		if(source_highcnt != 0) { /*CH2*/
			int high_dma_ch = datatask_high_dma_channel_index();
			datatask_printf("read high source: %d\r\n", source_highcnt);
			total_bytes = size*source_highcnt;
			rtn = axidma_oneway_transfer(ctx->dev, ctx->channel[high_dma_ch].rx_channel, ctx->channel[high_dma_ch].rxbuff, total_bytes, true);
			if(rtn !=0 ) {
				printf("dma read failed\r\n");
			}

			channel_list_h[0] = CH58_SCOPE_CH2;
			demux_one_frame(ctx->channel[high_dma_ch].rxbuff, total_bytes, channel_list_h, source_highcnt, CH_SCOPE);

		}
	}else {

	}

}

/*****************************************************************************
 Function    : read_nbd_from_dma_to_ringbuff
 Description : PWA reads multiple blocks from PL DDR, averages them, and uploads the result; same idea as Scope
 Inputs      : uint32_t single_freame_size in bytes
             int32_t ch_type
 Outputs     : none
 Return      : void
 Calls       :
 Called by   :

 History     :
  1.Date     : 2026-03-09
    Author   : huangwei
    Changes  : Initial version

*****************************************************************************/
void read_nbd_from_dma_to_ringbuff(uint32_t single_freame_size, int32_t ch_type)
{
	int i,j,rtn, chcnt,packs_int,packs_remain;
	int total_bytes,dmapack_size,nbd;
	int source_lowcnt,source_highcnt;
	int channel_list[SOURCE_MAX];
	int channel_list_l[SOURCE_MAX], channel_list_h[SOURCE_MAX];
	dma_context_t *ctx = NULL;
	FrameRingBuffer *frb = NULL;
	ParamValue pv_trig_times;
	FrameFilter *pff = NULL;
	int frame_int,frame_remain,single_freame_spoints;

	get_dma_context(&ctx);
	get_rb_multiple_instance(&frb);
	get_frame_filter_intance(&pff);

	chcnt = get_channels(channel_list, ch_type, &source_lowcnt, &source_highcnt);
	memcpy(channel_list_l, channel_list, sizeof(int) * source_lowcnt);
	memcpy(channel_list_h, channel_list + source_lowcnt, sizeof(int) * source_highcnt);

	get_param_by_name(PARAM_CURRENT,"trig_times", 0, &pv_trig_times);
	nbd = pv_trig_times.value.i32;

	//[12] read large data in batches
	if(ch_type == CH_SCOPE) {
		if(source_lowcnt != 0) { /*CH1*/

			total_bytes = single_freame_size * source_lowcnt;//total byte count

			//[4][4][4][4] | [4][4][4][4] | [4][4][4][4]...
			dmapack_size = (DMA_PACK_SIZE_SCOPE / (4 * 4 * source_lowcnt)) * (4 * 4 * source_lowcnt); //packet size must be an integer multiple of one data row
			dmapack_size = MIN(dmapack_size, total_bytes);
			packs_int = total_bytes / dmapack_size;
			packs_remain = total_bytes % dmapack_size;
			channel_list_l[0] = CH57_SCOPE_CH1;

			printf("dmapacksize: %d, packs: %d = %d / %d, remain: %d\r\n", dmapack_size, packs_int, total_bytes, dmapack_size, packs_remain);

			//[0]configure frame read
			reg_write_by_name("ch1_up_data_mode", 2);//set upload mode

			//printf("==============%d\r\n", nbd);

			for(i = 0; i < nbd; i++) {
				reg_write_by_name("ch1_read_block", i+1);//select which block to read, starting from 1

				reg_write_by_name("ch1_updata_soft_trig", 0);
				reg_write_by_name("ch1_updata_soft_trig", 1);
				reg_write_by_name("ch1_updata_soft_trig", 0);

				reg_write_by_name("ch2_updata_soft_trig", 0);
				reg_write_by_name("ch2_updata_soft_trig", 1);
				reg_write_by_name("ch2_updata_soft_trig", 0);

				//[1]receive frame data in segments
				for(j = 0; j < packs_int; j++) {
					reg_write_by_name("ch1_dma_up_en", 1);
					reg_write_by_name("ch1_dma_up_data_clr", 0);
					reg_write_by_name("ch1_dma_rd_length", (dmapack_size / 16) );
					axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, dmapack_size, true);
					//demux_frame_part_to_ff(ctx->channel[0].rxbuff, dmapack_size, channel_list_l, source_lowcnt, CH_SCOPE, pff);
					frame_filter_accumulate(pff, (int32_t *)ctx->channel[0].rxbuff, dmapack_size / 4);
				}
				if(packs_remain > 0) {
					reg_write_by_name("ch1_dma_up_en", 1);
					reg_write_by_name("ch1_dma_up_data_clr", 0);
					reg_write_by_name("ch1_dma_rd_length", (packs_remain / 16) );
					axidma_oneway_transfer(ctx->dev, ctx->channel[0].rx_channel, ctx->channel[0].rxbuff, packs_remain, true);
					//demux_frame_part_to_ff(ctx->channel[0].rxbuff, packs_remain, channel_list_l, source_lowcnt, CH_SCOPE, pff);
					frame_filter_accumulate(pff, (int32_t *)ctx->channel[0].rxbuff, packs_remain / 4);
				}

				//[2]one frame received; run accumulation filter
				if(i == (nbd - 1))
					frame_filter_process(pff, 1);
				else
					frame_filter_process(pff, 0);
			}

			printf("send data...\r\n");
			//[4]write the filtered result frame into RingBuffer for automatic sending; split into frames if needed
			single_freame_spoints = single_freame_size / 4; //total sample points in one acquisition frame
			frame_int = single_freame_spoints / SAMPLE_POINT_MAX;      //RingBuffer frame maximum is 16384 double samples; write the acquisition frame in chunks
			frame_remain = single_freame_spoints % SAMPLE_POINT_MAX;   //remaining sample count
			for(i = 0; i < frame_int; i++) {
				frame_buffer_write(frb+channel_list_l[0], channel_list_l[0], pff->filtered_frame + (i * SAMPLE_POINT_MAX), SAMPLE_POINT_MAX*8);
				if(single_freame_spoints > 1000000) {
					usleep(10000);
				}
			}
			if(frame_remain) {
				frame_buffer_write(frb+channel_list_l[0], channel_list_l[0], pff->filtered_frame + (i * SAMPLE_POINT_MAX), frame_remain*8);
			}

			frame_filter_reset(pff);
			printf("finish...\r\n");

		}

		if(source_highcnt != 0) { /*CH2*/
			int high_dma_ch = datatask_high_dma_channel_index();
			total_bytes = single_freame_size * source_highcnt;
			axidma_oneway_transfer(ctx->dev, ctx->channel[high_dma_ch].rx_channel, ctx->channel[high_dma_ch].rxbuff, total_bytes, true);
			channel_list_h[0] = CH58_SCOPE_CH2;
			demux_one_frame(ctx->channel[high_dma_ch].rxbuff, total_bytes, channel_list_h, source_highcnt, CH_SCOPE);
		}
	}
}

int read_demo_rdata(DemoDaqData *prdata)
{
	uint32_t i;
	uint32_t readcnt = sizeof(DemoDaqData) / sizeof(double);
	uint32_t temp_h, temp_l;
	uint64_t temp;
	double   data;

	//这里应该考虑6解调器的读取情况，暂时未考虑，这里上传的结构是按照最大解调器设计的
	for(i = 0; i < readcnt; i++) {
		temp_l = reg_read(0x0200 + i*2);//real-time data base addr:0x0200
		temp_h = reg_read(0x0200 + i*2 + 1);

		temp = ((uint64_t)temp_l << 32) | (temp_h);
		//P_LOG_DEBUG("temp_h: 0x%08x, temp_l: 0x%08x, temp: 0x%016lx\r\n", temp_h, temp_l, temp);
		data = load_double_swap32(&temp, i);
		save_demo_data(prdata, i, data);
	}
	return 0;
}

typedef struct {
	struct pollfd *fdset;
	int nfds;
	int fd_ch1_status;
	int fd_ch2_status;
	char *rxbuff1;
	int rxbuff1_size;
	char *rxbuff2;
	int rxbuff2_size;
} dma_status_wait_t;

static void datatask_pulse_upload_trigger(void)
{
	/* Trigger the upload/readout path after capture registers are configured. */
	reg_write_by_name("ch1_updata_soft_trig", 0);
	reg_write_by_name("ch1_updata_soft_trig", 1);
	reg_write_by_name("ch1_updata_soft_trig", 0);

	reg_write_by_name("ch2_updata_soft_trig", 0);
	reg_write_by_name("ch2_updata_soft_trig", 1);
	reg_write_by_name("ch2_updata_soft_trig", 0);
}

static void datatask_clear_capture_paths(void)
{
	reg_write_by_name("ch1_daq_up_en", 0);
	reg_write_by_name("ch1_scpoe_up_en", 0);
	reg_write_by_name("ch2_daq_up_en", 0);
	reg_write_by_name("ch2_scpoe_up_en", 0);
	reg_write_by_name("ch1_up_en", 0);
	reg_write_by_name("ch2_up_en", 0);
	reg_write_by_name("ch1_dma_up_en", 0);
	reg_write_by_name("ch2_dma_up_en", 0);
	reg_write_by_name("trig_en", 0);
}

static void datatask_clear_pwa_paths(void)
{
	reg_write_by_name("ch1_daq_up_en", 0);
	reg_write_by_name("ch2_daq_up_en", 0);
	reg_write_by_name("ch1_up_en", 0);
	reg_write_by_name("ch2_up_en", 0);
	reg_write_by_name("ch1_dma_up_en", 0);
	reg_write_by_name("ch2_dma_up_en", 0);
	reg_write_by_name("trig_en", 0);
}

static int datatask_is_param_enabled(const char *param_name)
{
	ParamValue pv;

	if(param_name == NULL) {
		return 1;
	}

	if(get_param_by_name(PARAM_CURRENT, param_name, 0, &pv) != 0) {
		return 0;
	}

	return pv.value.i32 != 0;
}

/*
 * Wait for one DMA status interrupt.
 * Return 0: DMA interrupt arrived normally, caller may read DMA data.
 * Return 1: wait was aborted by host stop/clear request, caller must skip DMA read and clean capture paths.
 */
static int datatask_wait_dma_status(dma_status_wait_t *wait_ctx, int dma_index, int mark_running, const char *abort_param_name)
{
	int fd;
	char *rxbuff;
	int rxbuff_size;

	if(dma_index == 0) {
		fd = wait_ctx->fd_ch1_status;
		rxbuff = wait_ctx->rxbuff1;
		rxbuff_size = wait_ctx->rxbuff1_size;
	}else {
		fd = wait_ctx->fd_ch2_status;
		rxbuff = wait_ctx->rxbuff2;
		rxbuff_size = wait_ctx->rxbuff2_size;
	}

	while(datatask_is_param_enabled(abort_param_name)) {
		int poll_ret;

		if(mark_running) {
			g_running = 1;
		}

		poll_ret = poll(wait_ctx->fdset, wait_ctx->nfds, DATATASK_DMA_POLL_TIMEOUT_MS);
		if(poll_ret < 0) {
			continue;
		}
		if(poll_ret == 0) {
			continue;
		}

		if(wait_ctx->fdset[dma_index].revents & POLLPRI) {
			lseek(fd, 0, SEEK_SET);
			read(fd, rxbuff, rxbuff_size);
			if(mark_running) {
				g_running = 0;
			}
			return 0;
		}
	}

	if(mark_running) {
		g_running = 0;
	}

	return 1;
}

/*
 * Wait for all selected DMA paths.
 * Return 0: all selected DMA paths completed normally.
 * Return 1: at least one path was aborted by host stop/clear request.
 */
static int datatask_wait_selected_dma(dma_status_wait_t *wait_ctx, int lowsrc_cnt, int highsrc_cnt, int mark_running, const char *abort_param_name)
{
	datatask_printf("[%d]\r\n", 6);
	if(lowsrc_cnt) {
		if(datatask_wait_dma_status(wait_ctx, 0, mark_running, abort_param_name)) {
			return 1;
		}
	}

	if(highsrc_cnt) {
		if(datatask_wait_dma_status(wait_ctx, 1, mark_running, abort_param_name)) {
			return 1;
		}
	}

	return 0;
}
static void datatask_clear_initial_dma_events(dma_status_wait_t *wait_ctx)
{
	poll(wait_ctx->fdset, wait_ctx->nfds, -1);
	if(wait_ctx->fdset[0].revents & POLLPRI) {
		lseek(wait_ctx->fd_ch1_status, 0, SEEK_SET);
		read(wait_ctx->fd_ch1_status, wait_ctx->rxbuff1, wait_ctx->rxbuff1_size);
		printf("DMA CH1 status interrupt!\n");
	}

	if(wait_ctx->fdset[1].revents & POLLPRI) {
		lseek(wait_ctx->fd_ch2_status, 0, SEEK_SET);
		read(wait_ctx->fd_ch2_status, wait_ctx->rxbuff2, wait_ctx->rxbuff2_size);
		printf("DMA CH2 status interrupt!\n");
	}
}

static void datatask_prepare_daq_first_frame(int lowsrc_cnt, int highsrc_cnt, int depth_l, int depth_h)
{
	if(lowsrc_cnt) {
		datatask_printf("[%d]\r\n", 0);
		reg_write_by_name("ch1_up_data_cache_length", depth_l/2);
		reg_write_by_name("ch1_up_data_write_length", depth_l/2);
		reg_write_by_name("ch1_block_size", depth_l);

		datatask_printf("[%d]\r\n", 1);
		reg_write_by_name("ch1_daq_up_en", 1);
		reg_write_by_name("ch1_scpoe_up_en", 0);

		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch1_up_data_mode", 1);

		datatask_printf("[%d]\r\n", 3);
		reg_write_by_name("ch1_up_en", 1);
	}

	if(highsrc_cnt) {
		datatask_printf("[%d]\r\n", 0);
		reg_write_by_name("ch2_up_data_cache_length", depth_h / 2);
		reg_write_by_name("ch2_up_data_write_length", depth_h / 2);
		reg_write_by_name("ch2_block_size", depth_h);

		datatask_printf("[%d]\r\n", 1);
		reg_write_by_name("ch1_daq_up_en", 1);
		reg_write_by_name("ch1_scpoe_up_en", 0);

		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch2_up_data_mode", 1);

		datatask_printf("[%d]\r\n", 3);
		reg_write_by_name("ch2_up_en", 1);
	}
}

static void datatask_prepare_daq_next_frame(int lowsrc_cnt, int highsrc_cnt)
{
	if(lowsrc_cnt) {
		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch1_up_data_mode", 1);
	}

	if(highsrc_cnt) {
		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch2_up_data_mode", 1);
	}
}

static void datatask_prepare_scope_first_frame(int lowsrc_cnt, int highsrc_cnt, int depth, int pwa_mode)
{
	int cache_length = pwa_mode ? 10 : (depth / 2);
	int write_length = pwa_mode ? (depth - 10) : (depth / 2);

	if(lowsrc_cnt) {
		datatask_printf("[%d]\r\n", 0);
		reg_write_by_name("ch1_up_data_cache_length", cache_length);
		reg_write_by_name("ch1_up_data_write_length", write_length);
		reg_write_by_name("ch1_block_size", depth);

		datatask_printf("[%d]\r\n", 1);
		reg_write_by_name("ch1_daq_up_en", 0);
		reg_write_by_name("ch1_scpoe_up_en", 1);

		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch1_up_data_mode", 1);

		datatask_printf("[%d]\r\n", 3);
		reg_write_by_name("ch1_up_en", 1);
	}

	if(highsrc_cnt) {
		datatask_printf("[%d]\r\n", 0);
		reg_write_by_name("ch2_up_data_cache_length", cache_length);
		reg_write_by_name("ch2_up_data_write_length", write_length);
		reg_write_by_name("ch2_block_size", depth);

		datatask_printf("[%d]\r\n", 1);
		reg_write_by_name("ch1_daq_up_en", 0);
		reg_write_by_name("ch1_scpoe_up_en", 1);

		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch2_up_data_mode", 1);

		datatask_printf("[%d]\r\n", 3);
		reg_write_by_name("ch2_up_en", 1);
	}
}

static void datatask_prepare_scope_next_frame(int lowsrc_cnt, int highsrc_cnt)
{
	if(lowsrc_cnt) {
		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch1_up_data_mode", 1);
	}

	if(highsrc_cnt) {
		datatask_printf("[%d]\r\n", 2);
		reg_write_by_name("ch2_up_data_mode", 1);
	}
}

static int datatask_handle_daq_signal(dma_status_wait_t *wait_ctx)
{
	ParamValue setpv, data;
	int channel_list[SOURCE_MAX];
	int chcnt, lowsrc_cnt, highsrc_cnt;
	int depth_l, depth_h;

	datatask_printf("Hello Signal\r\n");
	setpv.type = PARAM_INT32;
	setpv.value.i32 = 0;

	chcnt = get_channels(channel_list, CH_DAQ, &lowsrc_cnt, &highsrc_cnt);
	if(!chcnt) {
		printf("[%s][%d]: channel is null\r\n", __FUNCTION__, __LINE__);
		set_param_by_name(PARAM_CURRENT,"daq_signal", 0, setpv);
		return 1;
	}

	get_param_by_name(PARAM_CURRENT,"trig_smplen", 0, &data);
	depth_l = ((data.value.i32 * 8) / 16) * lowsrc_cnt;
	depth_h = ((data.value.i32 * 8) / 16) * highsrc_cnt;

	reg_write_by_name("trig_en", 1);
	datatask_prepare_daq_first_frame(lowsrc_cnt, highsrc_cnt, depth_l, depth_h);
	datatask_printf("[%d]\r\n", 5);
	datatask_pulse_upload_trigger();
	if(datatask_wait_selected_dma(wait_ctx, lowsrc_cnt, highsrc_cnt, 0, "daq_signal")) {
		datatask_printf("DAQ single aborted by host stop\r\n");
		datatask_clear_capture_paths();
		set_param_by_name(PARAM_CURRENT,"daq_signal", 0, setpv);
		return 1;
	}
	read_bd_from_dma_to_ringbuff((data.value.i32 * 8), CH_DAQ);
	datatask_clear_capture_paths();
	set_param_by_name(PARAM_CURRENT,"daq_signal", 0, setpv);
	return 0;
}

static int datatask_handle_daq_run(dma_status_wait_t *wait_ctx, int *start_work_first)
{
	ParamValue data;
	int channel_list[SOURCE_MAX];
	int chcnt, lowsrc_cnt, highsrc_cnt;
	int depth_l, depth_h;

	datatask_printf("Hello Run...\r\n");
	chcnt = get_channels(channel_list, CH_DAQ, &lowsrc_cnt, &highsrc_cnt);
	(void)chcnt;
	get_param_by_name(PARAM_CURRENT,"trig_smplen", 0, &data);
	depth_l = ((data.value.i32 * 8) / 16) * lowsrc_cnt;
	depth_h = ((data.value.i32 * 8) / 16) * highsrc_cnt;

	if(*start_work_first) {
		*start_work_first = 0;
		reg_write_by_name("trig_en", 1);
		datatask_prepare_daq_first_frame(lowsrc_cnt, highsrc_cnt, depth_l, depth_h);
	}else {
		datatask_prepare_daq_next_frame(lowsrc_cnt, highsrc_cnt);
	}

	datatask_printf("[%d]\r\n", 5);
    datatask_pulse_upload_trigger();
    if(datatask_wait_selected_dma(wait_ctx, lowsrc_cnt, highsrc_cnt, 1, "daq_run")) {
        datatask_printf("DAQ run aborted by host stop\r\n");
        return 1;
    }
    read_bd_from_dma_to_ringbuff((data.value.i32 * 8), CH_DAQ);
    return 0;
}

static void datatask_handle_daq_stop(int *start_work_first)
{
	if(*start_work_first == 0) {
		datatask_printf("DAQ Running Stop\r\n");
		*start_work_first = 1;
		datatask_clear_capture_paths();
	}
}

static int datatask_handle_scope_signal(dma_status_wait_t *wait_ctx)
{
	ParamValue setpv, data;
	int channel_list[SOURCE_MAX];
	int chcnt, lowsrc_cnt, highsrc_cnt;
	int depth;

	datatask_printf("Hello Scope Signal\r\n");
	setpv.type = PARAM_INT32;
	setpv.value.i32 = 0;

	chcnt = get_channels(channel_list, CH_SCOPE, &lowsrc_cnt, &highsrc_cnt);
	if(!chcnt) {
		printf("[%s][%d]: channel is null\r\n", __FUNCTION__, __LINE__);
		set_param_by_name(PARAM_CURRENT,"scope_signal", 0, setpv);
		return 1;
	}

	get_param_by_name(PARAM_CURRENT,"trig_smplen", 0, &data);
	depth = ((data.value.i32 * 4) / 16);

	reg_write_by_name("trig_en", 1);
	datatask_prepare_scope_first_frame(lowsrc_cnt, highsrc_cnt, depth, 0);
	datatask_printf("[%d]\r\n", 5);
	datatask_pulse_upload_trigger();
	if(datatask_wait_selected_dma(wait_ctx, lowsrc_cnt, highsrc_cnt, 0, "scope_signal")) {
		datatask_printf("Scope single aborted by host stop\r\n");
		datatask_clear_capture_paths();
		set_param_by_name(PARAM_CURRENT,"scope_signal", 0, setpv);
		return 1;
	}
	read_from_dma_to_ringbuff((data.value.i32 * 4), CH_SCOPE);
	datatask_clear_capture_paths();
	set_param_by_name(PARAM_CURRENT,"scope_signal", 0, setpv);
	return 0;
}

static int datatask_handle_scope_run(dma_status_wait_t *wait_ctx, int *start_work_first)
{
	ParamValue data;
	int channel_list[SOURCE_MAX];
	int chcnt, lowsrc_cnt, highsrc_cnt;
	int depth;

	chcnt = get_channels(channel_list, CH_SCOPE, &lowsrc_cnt, &highsrc_cnt);
	if(!chcnt) {
		printf("[%s][%d]: channel is null\r\n", __FUNCTION__, __LINE__);
		return 1;
	}

	get_param_by_name(PARAM_CURRENT,"trig_smplen", 0, &data);
	depth = ((data.value.i32 * 4) / 16);

	if(*start_work_first) {
		datatask_printf("Scope Running...\r\n");
		*start_work_first = 0;
		reg_write_by_name("trig_en", 1);
		datatask_prepare_scope_first_frame(lowsrc_cnt, highsrc_cnt, depth, 0);
	}else {
		datatask_prepare_scope_next_frame(lowsrc_cnt, highsrc_cnt);
	}

	datatask_printf("[%d]\r\n", 5);
	datatask_pulse_upload_trigger();
	if(datatask_wait_selected_dma(wait_ctx, lowsrc_cnt, highsrc_cnt, 1, "scope_run")) {
		datatask_printf("Scope run aborted by host stop\r\n");
		return 1;
	}
	read_from_dma_to_ringbuff((data.value.i32 * 4), CH_SCOPE);
	return 0;
}

static void datatask_handle_scope_stop(int *start_work_first)
{
	if(*start_work_first == 0) {
		datatask_printf("Scope Running Stop\r\n");
		*start_work_first = 1;
		datatask_clear_capture_paths();
	}
}

static int datatask_handle_pwa_signal(dma_status_wait_t *wait_ctx)
{
	ParamValue setpv, data;
	int channel_list[SOURCE_MAX];
	int chcnt, lowsrc_cnt, highsrc_cnt;
	int depth;

	datatask_printf("PWA Single Start...\r\n");
	setpv.type = PARAM_INT32;
	setpv.value.i32 = 0;

	chcnt = get_channels(channel_list, CH_SCOPE, &lowsrc_cnt, &highsrc_cnt);
	if(!chcnt) {
		printf("[%s][%d]: channel is null\r\n", __FUNCTION__, __LINE__);
		set_param_by_name(PARAM_CURRENT,"pwa_signal", 0, setpv);
		return 1;
	}

	get_param_by_name(PARAM_CURRENT,"trig_smplen", 0, &data);
	depth = ((data.value.i32 * 4) / 16);

	reg_write_by_name("trig_en", 1);
	datatask_prepare_scope_first_frame(lowsrc_cnt, highsrc_cnt, depth, 1);
	datatask_printf("[%d]\r\n", 5);
	datatask_pulse_upload_trigger();
	if(datatask_wait_selected_dma(wait_ctx, lowsrc_cnt, highsrc_cnt, 0, "pwa_signal")) {
		datatask_printf("PWA single aborted by host stop\r\n");
		trig_source_signal_reset();
		datatask_clear_pwa_paths();
		set_param_by_name(PARAM_CURRENT,"pwa_signal", 0, setpv);
		return 1;
	}
	datatask_printf("[%d]\r\n", 7);
	read_nbd_from_dma_to_ringbuff((data.value.i32 * 4), CH_SCOPE);
	trig_source_signal_reset();
	datatask_clear_pwa_paths();
	set_param_by_name(PARAM_CURRENT,"pwa_signal", 0, setpv);
	return 0;
}

void *data_server_thread(void *arg)
{
	int daq_start_work_first = 1;
	int scope_start_work_first = 1;
	char rxbuff1[8], rxbuff2[8];
	struct pollfd fdset[2];
	ParamValue daq_run_state, daq_signal_state, scope_run_state, scope_signal_state, pwa_signal_state;
	int nfds = 2;
	int dma_status_emio_ch1 = 32; //dma1 status EMIO32
	int dma_status_emio_ch2 = 33; //dma2 status EMIO33
	dma_status_wait_t wait_ctx;

	int fd_dma_ch1_status = gpio_fd_open(dma_status_emio_ch1);
	int fd_dma_ch2_status = gpio_fd_open(dma_status_emio_ch2);

	(void)arg;

	if(fd_dma_ch1_status < 0 || fd_dma_ch2_status < 0) {
		fprintf(stderr, "Failed to open GPIOs.\n");
		return NULL;
	}

	memset((void*)fdset, 0, sizeof(fdset));

	fdset[0].fd = fd_dma_ch1_status;
	fdset[0].events = POLLPRI;

	fdset[1].fd = fd_dma_ch2_status;
	fdset[1].events = POLLPRI;

	wait_ctx.fdset = fdset;
	wait_ctx.nfds = nfds;
	wait_ctx.fd_ch1_status = fd_dma_ch1_status;
	wait_ctx.fd_ch2_status = fd_dma_ch2_status;
	wait_ctx.rxbuff1 = rxbuff1;
	wait_ctx.rxbuff1_size = sizeof(rxbuff1);
	wait_ctx.rxbuff2 = rxbuff2;
	wait_ctx.rxbuff2_size = sizeof(rxbuff2);

	printf("Listening for GPIO interrupts on DMA status EMIO32 and EMIO33...\n");
	datatask_clear_initial_dma_events(&wait_ctx);

	while(1) {
		get_param_by_name(PARAM_CURRENT,"daq_run", 0, &daq_run_state);
		get_param_by_name(PARAM_CURRENT,"daq_signal", 0, &daq_signal_state);
		get_param_by_name(PARAM_CURRENT,"scope_run", 0, &scope_run_state);
		get_param_by_name(PARAM_CURRENT,"scope_signal", 0, &scope_signal_state);
		get_param_by_name(PARAM_CURRENT,"pwa_signal", 0, &pwa_signal_state);

		if(daq_signal_state.value.i32) {
			if(datatask_handle_daq_signal(&wait_ctx)) {
				continue;
			}
		}

        if(daq_run_state.value.i32) {
            if(datatask_handle_daq_run(&wait_ctx, &daq_start_work_first)) {
                continue;
            }
        }else {
            datatask_handle_daq_stop(&daq_start_work_first);
        }

		if(scope_signal_state.value.i32) {
			if(datatask_handle_scope_signal(&wait_ctx)) {
				continue;
			}
		}

		if(scope_run_state.value.i32) {
			if(datatask_handle_scope_run(&wait_ctx, &scope_start_work_first)) {
				continue;
			}
		}else {
			datatask_handle_scope_stop(&scope_start_work_first);
		}

		if(pwa_signal_state.value.i32) {
			if(datatask_handle_pwa_signal(&wait_ctx)) {
				continue;
			}
		}
	}
}







