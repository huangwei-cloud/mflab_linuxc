#ifndef _SICALIB_L500K_H_
#define _SICALIB_L500K_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SI_CALIB_L500K_VOLCH1_PATH "/home/root/app/param/lial500k/sicalibvolch1.csv"
#define SI_CALIB_L500K_VOLCH2_PATH "/home/root/app/param/lial500k/sicalibvolch2.csv"
#define SI_CALIB_L500K_CURCH1_PATH "/home/root/app/param/lial500k/sicalibcurch1.csv"
#define SI_CALIB_L500K_CURCH2_PATH "/home/root/app/param/lial500k/sicalibcurch2.csv"

int init_sicalib_l500k_param(void);
void set_si_l500k_calibration(int ch);

#ifdef __cplusplus
}
#endif

#endif /* _SICALIB_L500K_H_ */
