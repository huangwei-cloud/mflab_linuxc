#ifndef SOOUTPUTQUANT_L500K_H
#define SOOUTPUTQUANT_L500K_H

#define SOOUTPUT_QUANT_L500K_CH1_PATH "/home/root/app/param/lial500k/sooutputquantch1.csv"
#define SOOUTPUT_QUANT_L500K_CH2_PATH "/home/root/app/param/lial500k/sooutputquantch2.csv"

#define SOOUTPUT_QUANT_L500K_RANGE_COUNT 5

int init_sooutputquant_l500k_param(void);
double sooutput_l500k_get_sine_quant(int channel, int range);
double sooutput_l500k_get_fast_quant(int channel);

#endif
