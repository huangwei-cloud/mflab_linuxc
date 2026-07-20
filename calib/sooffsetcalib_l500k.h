#ifndef SOOFFSETCALIB_L500K_H
#define SOOFFSETCALIB_L500K_H

#define SOOFFSET_CALIB_L500K_CH1_PATH "/home/root/app/param/lial500k/sooffsetcalibch1.csv"
#define SOOFFSET_CALIB_L500K_CH2_PATH "/home/root/app/param/lial500k/sooffsetcalibch2.csv"
#define SOOFFSET_CALIB_L500K_RANGE_COUNT 5

int init_sooffsetcalib_l500k_param(void);
double sooffset_l500k_calibrate_value(int channel, int range, double value);

#endif
