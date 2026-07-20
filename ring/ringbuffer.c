#include "ringbuffer.h"
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../task/datatask.h"


#define DAQ_RING_BUFFER_MAX (CH_MAX)

FrameRingBuffer g_ring_buffer[DAQ_RING_BUFFER_MAX];
FrameRingBuffer g_ring_buffer_scope;
DemoDaqData g_demo_adata,g_demo_rdata;//aveg,real-time
extern int g_daq_start;//3:start 2:stop-->display 5:start 4:stop -->DAQ

void get_rb_signle_instance(FrameRingBuffer **frb, DataCh ch)
{
	assert(frb != NULL);
	assert(ch >= 0 && ch < DAQ_RING_BUFFER_MAX);

	*frb = &g_ring_buffer[ch];
}

void get_rb_multiple_instance(FrameRingBuffer **frb)
{
	assert(frb != NULL);

	*frb = &g_ring_buffer[0];
}


void get_rb_scope_instance(FrameRingBuffer **frb)
{
	assert(frb != NULL);

	*frb = &g_ring_buffer_scope;
}

void get_demo_adata_instance(DemoDaqData **demodata)
{
	assert(demodata != NULL);

	*demodata = &g_demo_adata;
}

void get_demo_rdata_instance(DemoDaqData **demodata)
{
	assert(demodata != NULL);

	*demodata = &g_demo_rdata;
}

FrameFilter* frame_filter_create(size_t frame_size)
{
    if (frame_size == 0 || frame_size > 30000000) {
        return NULL;
    }

    // 先分配结构体
    FrameFilter *ff = (FrameFilter*)malloc(sizeof(FrameFilter));
    if (!ff) return NULL;

    ff->cur_frame = NULL;
    ff->sum_frame = NULL;
    ff->filtered_frame = NULL;
    ff->frame_size = frame_size;
    ff->received = 0;
	ff->total_frames = 0;

    // 分配当前帧
    ff->cur_frame = (int32_t*)calloc(frame_size, sizeof(int32_t));
    if (!ff->cur_frame) {
		goto fail;
    }

    // 分配上一帧
    ff->sum_frame = (int64_t*)calloc(frame_size, sizeof(int64_t));
    if (!ff->sum_frame) {
		goto fail;
	}

    // 分配滤波结果帧
    ff->filtered_frame = (double*)calloc(frame_size, sizeof(double));
    if (!ff->filtered_frame) {
		printf("calloc failed...\r\n");
		goto fail;
	}

    return ff;

fail:
    if (ff->cur_frame) free(ff->cur_frame);
    if (ff->sum_frame) free(ff->sum_frame);
    if (ff->filtered_frame) free(ff->filtered_frame);
    free(ff);
    return NULL;
}


void frame_filter_destroy(FrameFilter *ff)
{
    if(!ff) return;
    free(ff->cur_frame);
    free(ff->sum_frame);
    free(ff->filtered_frame);
    free(ff);
}

int frame_filter_calloc_test()
{
    size_t frame_size = 30000000; // 用户实际帧大小
    FrameFilter *ff = frame_filter_create(frame_size);
    frame_filter_destroy(ff);
    return 0;
}

int frame_filter_accumulate(FrameFilter *ff, int32_t *data, size_t len)
{
    if(!ff || !data || len == 0) return -1;
    if(ff->received + len > ff->frame_size) return -2; // 超过单帧大小

    memcpy(ff->cur_frame + ff->received, data, len * sizeof(int32_t));
    ff->received += len;

    return 0;
}

void frame_filter_process(FrameFilter *ff, int last_frame)
{
	int32_t avg;

    if (!ff) return;
    if (ff->received == 0) return;

	//printhex(ff->cur_frame, 8, 8);
    // 先把当前帧累加到总和里
    for (size_t i = 0; i < ff->received; ++i) {
        ff->sum_frame[i] += (int64_t)ff->cur_frame[i];
    }

    // 累计帧数 +1
    ff->total_frames++;

    // 如果是最后一帧，则做平均，量化到浮点数
    if (last_frame) {
        for (size_t i = 0; i < ff->received; ++i) {
			avg = ff->sum_frame[i] / (int64_t)ff->total_frames;
            ff->filtered_frame[i] = load_double(&avg);
        }
    }

    // 清空当前帧，为下一帧接收做准备
    memset(ff->cur_frame, 0, ff->frame_size * sizeof(int32_t));
    ff->received = 0;
}

void frame_filter_reset(FrameFilter *ff)
{
    if (!ff) return;

    if (ff->cur_frame) memset(ff->cur_frame, 0, ff->frame_size * sizeof(int32_t));
    if (ff->sum_frame) memset(ff->sum_frame, 0, ff->frame_size * sizeof(int64_t));
    if (ff->filtered_frame) memset(ff->filtered_frame, 0, ff->frame_size * sizeof(double));

    ff->received = 0;
    ff->total_frames = 0;
}

int save_demo_data(DemoDaqData *pddata, DataCh ch, double data)
{
	switch(ch) {
	case CH1_X1: pddata->X1 = data; break;
	case CH2_Y1: pddata->Y1 = data; break;
	case CH3_R1: pddata->R1 = data; break;
	case CH4_Q1: pddata->Q1 = data; break;
	case CH5_X2: pddata->X2 = data; break;
	case CH6_Y2: pddata->Y2 = data; break;
	case CH7_R2: pddata->R2 = data; break;
	case CH8_Q2: pddata->Q2 = data; break;
	case CH9_X3: pddata->X3 = data; break;
	case CH10_Y3: pddata->Y3 = data; break;
	case CH11_R3: pddata->R3 = data; break;
	case CH12_Q3: pddata->Q3 = data; break;
	case CH13_X4: pddata->X4 = data; break;
	case CH14_Y4: pddata->Y4 = data; break;
	case CH15_R4: pddata->R4 = data; break;
	case CH16_Q4: pddata->Q4 = data; break;
	case CH17_X5: pddata->X5 = data; break;
	case CH18_Y5: pddata->Y5 = data; break;
	case CH19_R5: pddata->R5 = data; break;
	case CH20_Q5: pddata->Q5 = data; break;
	case CH21_X6: pddata->X6 = data; break;
	case CH22_Y6: pddata->Y6 = data; break;
	case CH23_R6: pddata->R6 = data; break;
	case CH24_Q6: pddata->Q6 = data; break;
	case CH25_X7: pddata->X7 = data; break;
	case CH26_Y7: pddata->Y7 = data; break;
	case CH27_R7: pddata->R7 = data; break;
	case CH28_Q7: pddata->Q7 = data; break;
	case CH29_X8: pddata->X8 = data; break;
	case CH30_Y8: pddata->Y8 = data; break;
	case CH31_R8: pddata->R8 = data; break;
	case CH32_Q8: pddata->Q8 = data; break;
	case CH33_FREQ1: pddata->FREQ1 = data; break;
	case CH34_FREQ2: pddata->FREQ2 = data; break;
	case CH35_FREQ3: pddata->FREQ3 = data; break;
	case CH36_FREQ4: pddata->FREQ4 = data; break;
	case CH37_FREQ5: pddata->FREQ5 = data; break;
	case CH38_FREQ6: pddata->FREQ6 = data; break;
	case CH39_FREQ7: pddata->FREQ7 = data; break;
	case CH40_FREQ8: pddata->FREQ8 = data; break;
	case CH41_AUXIN1: pddata->AUXIN1 = data; break;
	case CH42_AUXIN2: pddata->AUXIN2 = data; break;
	case CH43_AUXIN3: pddata->AUXIN3 = data; break;
	case CH44_AUXIN4: pddata->AUXIN4 = data; break;
	case CH45_AUXOUT1: pddata->AUXOUT1 = data; break;
	case CH46_AUXOUT2: pddata->AUXOUT2 = data; break;
	case CH47_AUXOUT3: pddata->AUXOUT3 = data; break;
	case CH48_AUXOUT4: pddata->AUXOUT4 = data; break;
	case CH49_PID1: pddata->PID1 = data; break;
	case CH50_PID2: pddata->PID2 = data; break;
	case CH51_PID3: pddata->PID3 = data; break;
	case CH52_PID4: pddata->PID4 = data; break;
	case CH53_PCNT1: pddata->PCNT1 = data; break;
	case CH54_PCNT2: pddata->PCNT2 = data; break;
	case CH55_PCNT3: pddata->PCNT3 = data; break;
	case CH56_PCNT4: pddata->PCNT4 = data; break;
	case CH57_SCOPE_CH1: pddata->MUX_SCH1_X = data; break;
	case CH58_SCOPE_CH2: pddata->MUX_SCH2_X = data; break;
	default:
		break;
	}

	return 0;
}

void frame_buffer_init(FrameRingBuffer *rb)
{
    rb->write_index = 0;
    rb->read_index = 0;
    rb->full_count = 0;
    pthread_mutex_init(&rb->mutex, NULL);
    for (int i = 0; i < FRAME_COUNT; ++i) {
        rb->frames[i].length = 0;
        rb->frames[i].frame_id = 0;
    }
}

bool frame_buffer_write(FrameRingBuffer *rb, DataCh ch, const uint8_t *data, size_t len)
{
    if (len > FRAME_SIZE) return false;

    pthread_mutex_lock(&rb->mutex);

    if (rb->full_count >= FRAME_COUNT) {
		printf("frame is full: %d\r\n", rb->full_count);
        pthread_mutex_unlock(&rb->mutex);
        return false;
    }

    Frame *frame = &rb->frames[rb->write_index];
    memcpy(frame->data, data, len);
    frame->length = len;
    frame->frame_id = ch;

    rb->write_index = (rb->write_index + 1) % FRAME_COUNT;
    rb->full_count++;

    pthread_mutex_unlock(&rb->mutex);
    return true;
}

bool frame_buffer_read(FrameRingBuffer *rb, uint8_t *out, size_t *out_len, uint32_t *frame_id)
{
    pthread_mutex_lock(&rb->mutex);

    if (rb->full_count == 0) {
        pthread_mutex_unlock(&rb->mutex);
        return false;
    }

    Frame *frame = &rb->frames[rb->read_index];
    memcpy(out, frame->data, frame->length);
    *out_len = frame->length;
    *frame_id = frame->frame_id;

    rb->read_index = (rb->read_index + 1) % FRAME_COUNT;
    rb->full_count--;

    pthread_mutex_unlock(&rb->mutex);
    return true;
}

// 模拟正弦波写入帧缓冲区（2048 点，每点 64 位）
uint64_t samples[2048];
static void generate_sine_frame(FrameRingBuffer *rb, DataCh ch, int amp, int point_count)
{
    const double freq = 100;
    const double sample_rate = 2048.0;

	static int first = 1;
	FILE *fp = NULL;

	if(first == 1) {
		fp = fopen("sine_output.csv", "w");
	    if (fp) {
	        fprintf(fp, "index,voltage\n");
	    }
	}

    for (int i = 0; i < point_count; ++i) {
        double t = i / sample_rate;
        double val = sin(2 * M_PI * freq * t) * amp;
        int64_t sample_val = (int64_t)(val * ((1LL << 16) - 1));  // 模拟 48 位有效位（有符号）
        int64_t noise = ((rand() % 1001) - 500) * ((1LL << 16) - 1) / 10000;
        samples[i] = (uint64_t)(sample_val + noise);
        //samples[i] = (uint64_t)(sample_val);

		if(first == 1) {
			if (fp) {
            	fprintf(fp, "%d,%lld\n", i, samples[i]);
        	}
		}
    }
	if(first == 1) {
		if (fp) {
        	fclose(fp);
    	}
	}

	first = 0;

    frame_buffer_write(rb, ch, (uint8_t *)samples, point_count * sizeof(uint64_t));
}


// 循环生成正弦波帧的线程函数（测试用）
void test_ring_buffer()
{
    while (1) {
		if(g_daq_start == 3) { /*display*/
			generate_sine_frame(&g_ring_buffer[CH1_X1], CH1_X1, 1, 128);
		}else if(g_daq_start == 5) { /*daq*/
			generate_sine_frame(&g_ring_buffer[CH1_X1], CH1_X1, 1,   2048);
			generate_sine_frame(&g_ring_buffer[CH2_Y1], CH2_Y1, 0.8, 2048);
		}else if(g_daq_start == 7) {
			generate_sine_frame(&g_ring_buffer[CH57_SCOPE_CH1], CH57_SCOPE_CH1, 1, 2048);
			generate_sine_frame(&g_ring_buffer[CH58_SCOPE_CH2], CH58_SCOPE_CH2, 0.8, 2048);
		}else {

		}
        usleep(10000); // 10ms 产生一帧（可调）
    }
}

void test_single_ring_buffer()
{
	generate_sine_frame(&g_ring_buffer[CH1_X1], CH1_X1, 1,   2048);
}

