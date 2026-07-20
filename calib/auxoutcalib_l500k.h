#ifndef AUXOUTCALIB_L500K_H
#define AUXOUTCALIB_L500K_H

#define AUXOUT_CALIB_L500K_PATH "/home/root/app/param/lial500k/auxoutcalib.csv"

int init_auxoutcalib_l500k_param(void);
double auxout_l500k_calibrate_dcvol(unsigned channel_index, double value);

#endif
