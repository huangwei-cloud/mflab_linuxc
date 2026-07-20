#ifndef L500K_SIREGCTRL_H
#define L500K_SIREGCTRL_H

#include <stdint.h>
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif

#define L500K_SINGLEINPUT_CHANNEL_1      1U
#define L500K_SINGLEINPUT_CHANNEL_2      2U

/* measure_mode */
#define L500K_SINGLEINPUT_MEASURE_CURRENT       1U
#define L500K_SINGLEINPUT_MEASURE_VOLTAGE       2U

/* v_single */
#define L500K_SINGLEINPUT_VMODE_DIFF            1U
#define L500K_SINGLEINPUT_VMODE_SINGLE          2U

/* v_coupling */
#define L500K_SINGLEINPUT_VCOUPLING_DC          1U
#define L500K_SINGLEINPUT_VCOUPLING_AC          2U

/* v_gnd */
#define L500K_SINGLEINPUT_VGND_END              1U
#define L500K_SINGLEINPUT_VGND_FLOAT            2U

/* voltage range */
#define L500K_SINGLEINPUT_VRANGE_1MV            1U
#define L500K_SINGLEINPUT_VRANGE_5MV            2U
#define L500K_SINGLEINPUT_VRANGE_10MV           3U
#define L500K_SINGLEINPUT_VRANGE_50MV           4U
#define L500K_SINGLEINPUT_VRANGE_0V1            5U
#define L500K_SINGLEINPUT_VRANGE_0V5            6U
#define L500K_SINGLEINPUT_VRANGE_1V             7U
#define L500K_SINGLEINPUT_VRANGE_5V             8U

/* iv_in_gnd */
#define L500K_SINGLEINPUT_BACKEND_GND           1U
#define L500K_SINGLEINPUT_BACKEND_INPUT         2U

/* iv_fir */
#define L500K_SINGLEINPUT_FILTER_BYPASS         1U
#define L500K_SINGLEINPUT_FILTER_50HZ           2U
#define L500K_SINGLEINPUT_FILTER_100HZ          3U
#define L500K_SINGLEINPUT_FILTER_BOTH           4U

/* v_impedance */
#define L500K_SINGLEINPUT_IMPEDANCE_50R         1U
#define L500K_SINGLEINPUT_IMPEDANCE_10MR        2U

/* current range */
#define L500K_SINGLEINPUT_IRANGE_1NA            1U
#define L500K_SINGLEINPUT_IRANGE_10NA           2U
#define L500K_SINGLEINPUT_IRANGE_100NA          3U
#define L500K_SINGLEINPUT_IRANGE_1UA            4U
#define L500K_SINGLEINPUT_IRANGE_10UA           5U
#define L500K_SINGLEINPUT_IRANGE_100UA          6U
#define L500K_SINGLEINPUT_IRANGE_1MA            7U
#define L500K_SINGLEINPUT_IRANGE_10MA           8U
#define L500K_SINGLEINPUT_IRANGE_100MA          9U

/* i_coupling */
#define L500K_SINGLEINPUT_ICOUPLING_AC          1U
#define L500K_SINGLEINPUT_ICOUPLING_DC          2U

typedef struct {
    uint8_t channel;
    uint8_t measure_mode;
    uint8_t v_single;
    uint8_t v_coupling;
    uint8_t v_gnd;
    uint8_t v_range;
    uint8_t iv_in_gnd;
    uint8_t iv_fir;
    uint8_t v_impedance;
    uint8_t i_range;
    uint8_t i_coupling;
} l500k_singleinput_config_t;

typedef struct {
    uint8_t reg1;
    uint8_t reg2;
    uint8_t reg3;
    uint8_t reg4;
} l500k_singleinput_regs_t;

int l500k_singleinput_calc_regs(const l500k_singleinput_config_t *cfg,
                                l500k_singleinput_regs_t *out);

#ifdef __cplusplus
}
#endif

#endif /* L500K_SIREGCTRL_H */
