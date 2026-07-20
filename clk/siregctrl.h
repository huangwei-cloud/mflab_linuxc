#ifndef SIREGCTRL_H
#define SIREGCTRL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* measure_mode */
#define SINGLEINPUT_MEASURE_MODE_VOLTAGE   1U
#define SINGLEINPUT_MEASURE_MODE_CURRENT   2U

/* filter */
#define SINGLEINPUT_FILTER_BYPASS          1U
#define SINGLEINPUT_FILTER_50HZ            2U
#define SINGLEINPUT_FILTER_100HZ           3U
#define SINGLEINPUT_FILTER_BOTH            4U

/* range */
#define SINGLEINPUT_RANGE_1MV_1NA          1U
#define SINGLEINPUT_RANGE_3MV_10NA         2U
#define SINGLEINPUT_RANGE_10MV_100NA       3U
#define SINGLEINPUT_RANGE_30MV_1UA         4U
#define SINGLEINPUT_RANGE_0V1_10UA         5U
#define SINGLEINPUT_RANGE_0V3_100UA        6U
#define SINGLEINPUT_RANGE_1V_1MA           7U
#define SINGLEINPUT_RANGE_3V_10MA          8U

/* impedance */
#define SINGLEINPUT_IMPEDANCE_50R          1U
#define SINGLEINPUT_IMPEDANCE_10MR         2U

/* coupling */
#define SINGLEINPUT_COUPLING_AC            1U
#define SINGLEINPUT_COUPLING_DC            2U

/* single_mode */
#define SINGLEINPUT_MODE_SINGLE            1U
#define SINGLEINPUT_MODE_DIFF              2U

/* grounding_mode */
#define SINGLEINPUT_GROUNDING_GND          1U
#define SINGLEINPUT_GROUNDING_EGND         2U

typedef struct {
    uint8_t measure_mode;
    uint8_t filter;
    uint8_t range;
    uint8_t impedance;
    uint8_t coupling_voltage;
    uint8_t coupling_current;
    uint8_t single_mode;
    uint8_t grounding_mode;
} singleinput_config_t;

typedef struct {
    uint32_t reg_low;
    uint32_t reg_high;
} singleinput_regs_t;

int singleinput_calc_regs(const singleinput_config_t *cfg, singleinput_regs_t *out);
int singleinput_calc_regs_with_standard_ground(const singleinput_config_t *cfg,
                                               singleinput_regs_t *out,
                                               int standard_ground);

#ifdef __cplusplus
}
#endif

#endif /* SIREGCTRL_H */
