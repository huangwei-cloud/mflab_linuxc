#ifndef _SICALIB_M005M_H_
#define _SICALIB_M005M_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SI_CALIB_M005M_VOLCH1_PATH "/home/root/app/param/liam005m/sicalibvolch1.csv"
#define SI_CALIB_M005M_VOLCH2_PATH "/home/root/app/param/liam005m/sicalibvolch2.csv"
#define SI_CALIB_M005M_CURCH1_PATH "/home/root/app/param/liam005m/sicalibcurch1.csv"
#define SI_CALIB_M005M_CURCH2_PATH "/home/root/app/param/liam005m/sicalibcurch2.csv"

int init_sicalib_m005m_param(void);
void set_si_m005m_calibration(int ch);

#ifdef __cplusplus
}
#endif

#endif /* _SICALIB_M005M_H_ */
