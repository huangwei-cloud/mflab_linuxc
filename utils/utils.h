#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <locale.h>
#include <fenv.h>
#include <stddef.h>

typedef struct {
    double A_carrier;   // A * J0(mf)
    double A_lsb1;      // -A * J1(mf)  (1阶下边带：负号)
    double A_usb1;      // +A * J1(mf)  (1阶上边带：正号)
    double A_lsb2;      // +A * J2(mf)  (2阶下边带：正号)
    double A_usb2;      // +A * J2(mf)  (2阶上边带：正号)
} Fm3Coeffs;

int fm3_compute_coeffs_libm(double A, double mf, Fm3Coeffs* out);

void printhex(void *buffer, int size, int linecnt);
int feq(double a, double b);

int get_alpha_to_reg(uint64_t *alpha_high, uint64_t *alpha_low, double tau);

int sync_filter_params(uint32_t fin, uint32_t *M1, uint32_t *M2, uint32_t *Nm);

	
#endif
