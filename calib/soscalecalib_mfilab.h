#ifndef SOSCALECALIB_MFILAB_H
#define SOSCALECALIB_MFILAB_H

#define SOSCALE_CALIB_MFILAB_PATH "/home/root/app/param/mfilab/soscalecalib.csv"

int init_soscalecalib_mfilab_param(void);
double soscale_mfilab_calibrate_value(int channel, double value);

#endif
