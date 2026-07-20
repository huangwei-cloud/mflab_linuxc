#ifndef CHOUTCALIB_L500K_H
#define CHOUTCALIB_L500K_H

#define CHOUT_CALIB_L500K_PATH "/home/root/app/param/lial500k/choutcalib.csv"

int init_choutcalib_l500k_param(void);
double chout_l500k_calibrate_value(unsigned channel_index, double value);
double chout_l500k_apply_k2(unsigned channel_index, double value);
double chout_l500k_apply_b(unsigned channel_index, double value);
double chout_l500k_apply_range_k2(unsigned channel_index, double value, double range_v);

#endif
