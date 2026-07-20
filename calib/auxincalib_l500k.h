#ifndef AUXINCALIB_L500K_H
#define AUXINCALIB_L500K_H

#define AUXIN_CALIB_L500K_PATH "/home/root/app/param/lial500k/auxincalib.csv"

int init_auxincalib_l500k_param(void);
double auxin_l500k_calibrate_value(unsigned channel_index, double value);

#endif
