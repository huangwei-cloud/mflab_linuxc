#ifndef SOOFFSETCALIB_MFLLAB_H
#define SOOFFSETCALIB_MFLLAB_H

#define SOOFFSET_CALIB_MFILAB_PATH "/home/root/app/param/mfilab/sooffsetcalib.csv"

int init_sooffsetcalib_mfilab_param(void);
double sooffset_mfilab_calibrate_value(int channel, double value);

#endif
