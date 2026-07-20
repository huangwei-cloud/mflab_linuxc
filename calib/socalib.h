#ifndef _SOCALIB_H_
#define _SOCALIB_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SO_CALIB_VVVI_CH1_PATH  "/home/root/app/param/liam005mvvvi/socalibch1.csv"
#define SO_CALIB_VVVI_CH2_PATH  "/home/root/app/param/liam005mvvvi/socalibch2.csv"

#define SO_CALIB_M005M_CH1_PATH "/home/root/app/param/liam005m/socalibch1.csv"
#define SO_CALIB_M005M_CH2_PATH "/home/root/app/param/liam005m/socalibch2.csv"

#define SO_CALIB_L500K_CH1_PATH "/home/root/app/param/lial500k/socalibch1.csv"
#define SO_CALIB_L500K_CH2_PATH "/home/root/app/param/lial500k/socalibch2.csv"



#define CH_PARAM_COUNT   4
#define CH_TABLE_SIZE    8

typedef struct {
    double vouta, voutb, voutc, voutd;  /* DAC VOUTA-D calibration values */
    int    valid;
} ch_row_t;

typedef struct {
    ch_row_t table[CH_TABLE_SIZE];
} socaldb_t;

enum {
    ID_FASTX_SIN   = 0,
    ID_FASTX_TTL   = 1,
    ID_SINE_10mV   = 2,
    ID_SINE_100mV  = 3,
    ID_SINE_1V     = 4,
    ID_SINE_2V     = 5,
    ID_SINE_10V    = 6
};

void get_socaldb1_instance(socaldb_t **sdb);
void get_socaldb2_instance(socaldb_t **sdb);

int socaldb_load_csv(socaldb_t *db, const char *csv_path);
int socaldb_get_by_id(const socaldb_t *db, unsigned id, ch_row_t *out);

void set_so_calibration(int ch, int id);
int init_socalib_param();

#ifdef __cplusplus
}
#endif
#endif /* CALIB_CH_H */

