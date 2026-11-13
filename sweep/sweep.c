#include "stdio.h"
#include "sweep.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "../pm/param.h"
#include "../clk/reg.h"
#include "../ring/ringbuffer.h"
#include "../utils/scpi-def.h"


ScanState  g_scan;
ScanConfig g_cfg;

#define SWEEP_PARAM_OSC_FREQ    0
#define SWEEP_PARAM_OSC_AMP     1
#define SWEEP_PARAM_DEMOD_PHASE 2
#define SWEEP_DAQ_ARM_DELAY_US  20000

static uint32_t sweep_get_settle_delay_us(void)
{
	ParamValue pv;
	if(get_param_by_name(PARAM_CURRENT, "sweep_interval", 0, &pv) != 0 ||
	   !(pv.value.d >= 0.001 && pv.value.d <= 3600.0)) {
		return 1000;
	}
	return (uint32_t)(pv.value.d * 1000000.0);
}

void send_soft_trig()
{
	ParamValue getpv;
    int32_t trig_source;;

	reg_write_by_name("trig_source", TS_SOFTWARE);
	usleep(100);
	reg_write_by_name("soft_trig", 0);
	usleep(200);
	reg_write_by_name("soft_trig", 1);
	usleep(200);
    reg_write_by_name("soft_trig", 0);

	get_param_by_name(PARAM_CURRENT, "trig_source", 0, &getpv);
    trig_source = getpv.value.i32;

	reg_write_by_name("trig_source", trig_source);
}

int calc_step(double start, double stop, int points, double *step_out)
{
    if (!step_out) return -1;

    if (points <= 1) {
        *step_out = 0.0;
        return -2;
    }

    *step_out = (stop - start) / (double)(points - 1);

    return 0;
}

void scan_update_config(void)
{
    double step_out = 0.0;
    ParamValue startpv, stoppv, pointspv;

    get_param_by_name(PARAM_CURRENT, "sweep_start",  0, &startpv);
    get_param_by_name(PARAM_CURRENT, "sweep_stop",   0, &stoppv);
    get_param_by_name(PARAM_CURRENT, "sweep_points", 0, &pointspv);

    int points = (int)(pointspv.value.d + 0.5);
    if (points < 1) {
        points = 1;
    }

    int ret = calc_step(startpv.value.d, stoppv.value.d, points, &step_out);

    g_cfg.start  = startpv.value.d;
    g_cfg.stop   = stoppv.value.d;
    g_cfg.step   = step_out;
    g_cfg.points = points;
    g_cfg.dirty  = 1;

    g_scan.initialized = 0;
    g_scan.finished    = 0;
    g_scan.running     = 0;

}



static void scan_init_from_cfg(void)
{
    // 配置到扫描状态
    g_scan.start        = g_cfg.start;
    g_scan.stop         = g_cfg.stop;
    g_scan.total_points = (g_cfg.points < 1) ? 1 : g_cfg.points;  // 至少 1 点
    g_scan.index        = 0;                                      // 从第 0 点开始

    if (g_scan.total_points == 1) {
        // 单点扫描：没有步进，只扫 start 这个点
        g_scan.step    = 0.0;
        g_scan.current = g_scan.start;
    } else {
        // 多点扫描：正常步进
        if (g_cfg.step == 0.0) {
            // 理论上不会出现，保险起见重新算一次
            g_scan.step = (g_scan.stop - g_scan.start)
                          / (double)(g_scan.total_points - 1);
        } else {
            g_scan.step = g_cfg.step;
        }
        g_scan.current = g_scan.start;
    }

    g_scan.initialized = 1;
    g_scan.finished    = 0;

    // 连续扫描开关仍然由外部控制
    // g_scan.running = 0;  // 如有需要可以在这里清零

    // 初始化时切换到软触发源
    /* 源配置成软触发源 */
    reg_write_by_name("trig_source", TS_SOFTWARE);
}

/* type 0: OSC frequency (Hz), target 0-7 -> OSC0-OSC7.
 * type 1: OSC amplitude (Vp), target 0-15 -> OSC0/OUT1, OSC0/OUT2, ... OSC7/OUT2.
 * type 2: oscillator phase (degrees), target 0-7 -> demo_osc_phase_ch1-ch8.
 */
void output_value(double value)
{
	ParamValue pv;
	int type, target, osc, out_ch;
	uint64_t code;
	char reg_name[64], param_name[64];

	if(get_param_by_name(PARAM_CURRENT, "sweep_param", 0, &pv) != 0) return;
	type = (int)pv.value.d;
	if(get_param_by_name(PARAM_CURRENT, "sweep_param", 1, &pv) != 0) return;
	target = (int)pv.value.d;
	if(!isfinite(value)) return;

	switch(type) {
	case SWEEP_PARAM_OSC_FREQ:
		if(target < 0 || target > 7) return;
		if(value < 0.0) value = 0.0;
		if(value >= OSC_SAMPLE_RATE) return;
		osc = target + 1;
		code = (uint64_t)(value * (double)((1ULL << 48) - 1) / OSC_SAMPLE_RATE);
		sprintf(reg_name, "oscillators%d_freq_word_l", osc);
		reg_write_by_name(reg_name, (uint32_t)code);
		sprintf(reg_name, "oscillators%d_freq_word_h", osc);
		reg_write_by_name(reg_name, (uint32_t)((code >> 32) & 0xFFFF));
		sprintf(reg_name, "oscillators%d_freq_en", osc);
		reg_write_by_name(reg_name, 1);
		reg_write_by_name(reg_name, 0);
		reg_write_by_name("soft_trig", 0);
		reg_write_by_name("soft_trig", 1);
		reg_write_by_name("soft_trig", 0);
		sprintf(param_name, "osc%d_freq", target);
		set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
		break;

	case SWEEP_PARAM_OSC_AMP:
		if(target < 0 || target > 15) return;
		if(value < 0.0) value = 0.0;
		if(value > 2.0) return;
		osc = target / 2 + 1;
		out_ch = target % 2 + 1;
		/* Same Vp conversion as zwdx_set_demo_oscamp(), fixed +/-2 V output. */
		code = (uint32_t)(value / 2.0 * (double)((1UL << 16) - 1));
		sprintf(reg_name, "oscillators%d_sinout%d_scaling1", osc, out_ch);
		reg_write_by_name(reg_name, (uint32_t)code);
		sprintf(param_name, "demo_osc_amp_ch%d_out%d", osc, out_ch);
		set_param_by_name(PARAM_CURRENT, param_name, 2, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
		break;

	case SWEEP_PARAM_DEMOD_PHASE:
		if(target < 0 || target > 7) return;
		osc = target + 1;
		/* Wrap the hardware phase, but save the original requested angle. */
		double phase_deg = fmod(value, 360.0);
		if(phase_deg < 0.0) phase_deg += 360.0;
		code = (uint64_t)(phase_deg * (double)(1ULL << 48) / 360.0);
		sprintf(reg_name, "oscillators%d_phase_l", osc);
		reg_write_by_name(reg_name, (uint32_t)code);
		sprintf(reg_name, "oscillators%d_phase_h", osc);
		reg_write_by_name(reg_name, (uint32_t)((code >> 32) & 0xFFFF));
		sprintf(reg_name, "oscillators%d_freq_en", osc);
		reg_write_by_name(reg_name, 1);
		reg_write_by_name(reg_name, 0);
		sprintf(param_name, "demo_osc_phase_ch%d", osc);
		set_param_by_name(PARAM_CURRENT, param_name, 1, (ParamValue){.type = PARAM_DOUBLE, .value.d = value});
		break;
	default:
		return;
	}
}

int scan_step_once(void)
{
    ParamValue daq_signal_state;

    if (!g_scan.initialized || g_scan.finished) {
        return -1; // 未初始化或已经扫完
    }

    // 判断是否到达总点数（用 index 控制，而不是 current vs stop）
    if (g_scan.index >= g_scan.total_points) {
        g_scan.finished = 1;
        return -1;
    }

    // 计算本次 setpoint：最后一个点强制钳到 stop，避免浮点误差
    double setpoint;
    if (g_scan.total_points == 1) {
        setpoint = g_scan.start;
    } else if (g_scan.index == g_scan.total_points - 1) {
        setpoint = g_scan.stop;
    } else {
        setpoint = g_scan.start + g_scan.index * g_scan.step;
    }

    g_scan.current = setpoint;

	//设置参数
	output_value(g_scan.current);

	/* Let the output settle before arming the acquisition. */
	usleep(sweep_get_settle_delay_us());

    // 下面是原来的 DAQ SINGLE 采集逻辑
    daq_signal_state.type = PARAM_INT32;
    daq_signal_state.value.i32 = 1;
    set_param_by_name(PARAM_CURRENT, "daq_signal", 0, daq_signal_state);

	usleep(SWEEP_DAQ_ARM_DELAY_US);
	send_soft_trig();

    for (;;) {
        get_param_by_name(PARAM_CURRENT, "daq_signal", 0, &daq_signal_state);
        if (!daq_signal_state.value.i32) {
            break;
        }
		usleep(1000);
    }

    //printf("xxxxxxxxxxxxxxxxxxxxxxx:%.3f\r\n", setpoint);

    // 递增 index，下一次 step_once 就会走到下一个点
    g_scan.index++;

    if (g_scan.index >= g_scan.total_points) {
        g_scan.finished = 1;
    }

	//printf("index: %d total: %d\r\n", g_scan.index, g_scan.total_points);

    return 0;
}


// 全局标志（例子）：
volatile int g_btn_single = 0;  // UI 按下单次时置 1
volatile int g_btn_run    = 0;  // UI 勾选/按下连续扫描：1=连续，0=停止


// 扫描线程（设备端）
void *scan_thread(void *arg)
{
    (void)arg;

	ParamValue sweep_run_state, sweep_single_state;

    for (;;) {

		get_param_by_name(PARAM_CURRENT,"sweep_run", 0, &sweep_run_state);
		get_param_by_name(PARAM_CURRENT,"sweep_single", 0, &sweep_single_state);

		g_btn_single = (int)sweep_single_state.value.d;
		g_btn_run    = (int)sweep_run_state.value.d;

        // 1) 单次扫描优先处理（比如希望点“单次”时可以打断连续）
        if (g_btn_single) {

			g_btn_single = 0;   // 立即清零，防止多次处理同一次点击

			// UI 参数归零（sweep_single 拉回 0）
            sweep_single_state.value.d = 0;
            set_param_by_name(PARAM_CURRENT, "sweep_single", 0, sweep_single_state);

            // Single 模式：允许在 finished == 1 时重新从 Start 开始
            if (g_cfg.dirty || !g_scan.initialized || g_scan.finished) {
                scan_init_from_cfg();   // 从 g_cfg 重新装载 start/stop/points/step
                g_cfg.dirty = 0;
            }

            if (!g_scan.finished) {

                if (scan_step_once() == 0) {
                    // TODO: 把本次结果上报给上位机或放进缓冲区
                }
            }

            // 单次模式就是只走一步，不再 loop 内 while，多余的步由下一次按钮触发
        }
        // 2) 连续扫描
        else if (g_btn_run) {

			// 连续模式：只在“未初始化”或“配置改变”时 init
            // 注意：这里特意不在 finished==1 时重新 init，防止从头再扫
            if (g_cfg.dirty || !g_scan.initialized) {
                scan_init_from_cfg();
                g_cfg.dirty = 0;
            }

            if (!g_scan.finished) {
                scan_step_once();
            } else {
                // 扫完了：自动退出连续模式（只扫一遍）
                g_btn_run = 0;

                // UI 参数归零，通知上位机“扫描结束”
                sweep_run_state.value.d = 0;
                set_param_by_name(PARAM_CURRENT, "sweep_run", 0, sweep_run_state);

				// 下一次还能 RUN
				g_scan.initialized = 0;
            }

            // 这里是“每圈一步”策略：让每一轮循环只扫一个点，
            // 再由 time constant + wait_settle 控制速度。
            // 如果还想更慢一点，可以在下面再加个延时。
        }
        else {
            // 什么都没做：既不是单次请求，也不是连续模式
            // 可以适当 sleep，降低 CPU 占用
        }

        usleep(1000);
    }

    return NULL;
}




