#ifndef _SIGINPUTCTRL_H_
#define _SIGINPUTCTRL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Channel index follows the original ZW_CS1000.ipynb v1_range_set() logic:
 *   0: write relay group 1/2
 *   1: write relay group 3/4
 */
#define SIGINPUT_CH0                 0U
#define SIGINPUT_CH1                 1U

/* Voltage coupling, same values as the notebook:
 *   1: DC
 *   2: AC
 */
#define SIGINPUT_V_COUPLING_DC       1U
#define SIGINPUT_V_COUPLING_AC       2U

/* Voltage range, same values as the notebook:
 *   1: 1 V
 *   2: 10 V
 */
#define SIGINPUT_V_RANGE_1V          1U
#define SIGINPUT_V_RANGE_10V         2U

/* Voltage input impedance, same values as the notebook:
 *   1: 50 ohm
 *   2: 10 Mohm
 */
#define SIGINPUT_V_IMPEDANCE_50R     1U
#define SIGINPUT_V_IMPEDANCE_10MR    2U

typedef struct {
    uint8_t ch;
    uint8_t coupling;
    uint8_t range;
    uint8_t impedance;
} siginput_config_t;

int siginput_config_to_regs(const siginput_config_t *cfg);

#ifdef __cplusplus
}
#endif

#endif
