#ifndef _SWEEP_H_
#define _SWEEP_H_

typedef struct {
    double start;
    double stop;
    double step;
    double current;
    int    initialized;  // 是否已根据当前配置初始化过扫描
    int    finished;     // 是否扫完
	int    running; 	// 连续扫描：0=停止, 1=正在扫

	int    total_points;   // 总点数（>=1）
    int    index;          // 当前点序号：0..total_points-1
} ScanState;

typedef struct {
    double start;
    double stop;
    double step;
	double points;
    int    dirty;        // 1 = 配置已更新，需要重新 init
} ScanConfig;

void send_soft_trig();
void scan_update_config();
void *scan_thread(void *arg);


#endif

