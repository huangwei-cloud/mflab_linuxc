#include "calib_init.h"

#include "sicalib.h"
#include "socalib.h"
#include "auxoutcalib_l500k.h"
#include "auxincalib_l500k.h"
#include "choutcalib_l500k.h"
#include "soscalecalib_mfilab.h"
#include "sooffsetcalib_mfllab.h"

int init_calib_param(void)
{
	int ret = 0;

	// if(init_sicalib_param() != 0) {
	// 	ret = -1;
	// }
	// if(init_socalib_param() != 0) {
	// 	ret = -1;
	// }
	// if(init_auxoutcalib_l500k_param() != 0) {
	// 	ret = -1;
	// }
	// if(init_auxincalib_l500k_param() != 0) {
	// 	ret = -1;
	// }
	// if(init_choutcalib_l500k_param() != 0) {
	// 	ret = -1;
	// }
	if(init_soscalecalib_mfilab_param() != 0) {
		ret = -1;
	}
	if(init_sooffsetcalib_mfilab_param() != 0) {
		ret = -1;
	}

	return ret;
}
