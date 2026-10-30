#include "stdio.h"
#include "sweep.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include "../pm/param.h"
#include "../clk/reg.h"
#include "../ring/ringbuffer.h"


ScanState  g_scan;
ScanConfig g_cfg;

extern double g_so_sineout_verify[5];

#define SWEEP_PARAM_SO1_FREQ   0
#define SWEEP_PARAM_SO1_SCALE  1
#define SWEEP_PARAM_SO2_FREQ   2
#define SWEEP_PARAM_SO2_SCALE  3
#define SWEEP_DAQ_ARM_DELAY_US 10000

static double signalout_range_voltage(int32_t range)
{
	static const double range_m005m[] = {0.01, 0.1, 1.0, 2.0, 10.0};
	static const double range_l500k[] = {0.01, 0.1, 1.0, 3.0, 10.0};

	if (range < 0) range = 0;
	if (range > 4) range = 4;

	if (get_device_model() == DEVICE_MODEL_L500K) {
		return range_l500k[range];
	}

	return range_m005m[range];
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

void output_value(double value)
{
	ParamValue getpv;
	ParamValue setpv;
	int output_signal;
	double temp;
	uint64_t freq;
	uint64_t codevalue;
	int32_t range;
	double range_voltage;
	double scale;

	get_param_by_name(PARAM_CURRENT,"sweep_param", 0, &getpv);
	output_signal = (int)(getpv.value.d + 0.5);

	setpv.type = PARAM_DOUBLE;
	setpv.value.d = value;

	switch(output_signal) {
	case SWEEP_PARAM_SO1_FREQ:
		if (value < 0.0) {
			value = 0.0;
			setpv.value.d = value;
		}
		temp = value * pow(2, 48) / 500000000.0;
		freq = (uint64_t)temp;
		reg_write_by_name("oscillators2_freq_word_l", (uint32_t)(freq & 0xFFFFFFFF));
		reg_write_by_name("oscillators2_freq_word_h", (uint32_t)((freq >> 32) & 0xFFFFFFFF));
		reg_write_by_name("oscillators2_en", 1);
		set_param_by_name(PARAM_CURRENT,"osc1_freq", 1, setpv);
		break;
	case SWEEP_PARAM_SO1_SCALE:
		if (value < 0.0) {
			value = 0.0;
		}
		get_param_by_name(PARAM_CURRENT,"so_range_ch1", 1, &getpv);
		range = getpv.value.i32;
		if (range < 0) range = 0;
		if (range > 4) range = 4;
		range_voltage = signalout_range_voltage(range);
		scale = value / (g_so_sineout_verify[range] * range_voltage);
		setpv.value.d = scale;
		temp = scale * g_so_sineout_verify[range];
		codevalue = temp * (1ULL << 32);
		reg_write_by_name("signalout1_scaling1_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout1_scaling1_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		reg_write_by_name("signalout1_bigwidth_write_en", 1);
		reg_write_by_name("signalout1_bigwidth_write_en", 0);
		set_param_by_name(PARAM_CURRENT,"so_scaling1_ch1", 1, setpv);
		break;
	case SWEEP_PARAM_SO2_FREQ:
		if (value < 0.0) {
			value = 0.0;
			setpv.value.d = value;
		}
		temp = value * pow(2, 48) / 500000000.0;
		freq = (uint64_t)temp;
		reg_write_by_name("oscillators3_freq_word_l", (uint32_t)(freq & 0xFFFFFFFF));
		reg_write_by_name("oscillators3_freq_word_h", (uint32_t)((freq >> 32) & 0xFFFFFFFF));
		reg_write_by_name("oscillators3_en", 1);
		set_param_by_name(PARAM_CURRENT,"osc2_freq", 1, setpv);
		break;
	case SWEEP_PARAM_SO2_SCALE:
		if (value < 0.0) {
			value = 0.0;
		}
		get_param_by_name(PARAM_CURRENT,"so_range_ch2", 1, &getpv);
		range = getpv.value.i32;
		if (range < 0) range = 0;
		if (range > 4) range = 4;
		range_voltage = signalout_range_voltage(range);
		scale = value / (g_so_sineout_verify[range] * range_voltage);
		setpv.value.d = scale;
		temp = scale * g_so_sineout_verify[range];
		codevalue = temp * (1ULL << 32);
		reg_write_by_name("signalout2_scaling1_l", (uint32_t)(codevalue & 0xFFFFFFFF));
		reg_write_by_name("signalout2_scaling1_h", (uint32_t)((codevalue >> 32) & 0xFFFFFFFF));
		reg_write_by_name("signalout2_bigwidth_write_en", 1);
		reg_write_by_name("signalout2_bigwidth_write_en", 0);
		set_param_by_name(PARAM_CURRENT,"so_scaling1_ch2", 1, setpv);
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




