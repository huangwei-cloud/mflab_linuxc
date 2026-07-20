#include <math.h>
#include <limits.h>
#include "utils.h"
#include "../pm/param.h"

#define FLOAT_EPS_REL   (1e-9)

static long double get_filter_sample_rate_hz(void)
{
	if (get_device_model() == DEVICE_MODEL_L500K) {
		return 2e6L;  // L500K: fs_base = 2 MHz, dn = 1.
	}

	return 40e6L / 10.0L;  // M005M: fs_base = 40 MHz, dn = 10.
}


/**
 * Compute FM synthesis coefficients up to the second order (J0/J1/J2); amplitudes include signs.
 * A: nominal total carrier amplitude.
 * mf: modulation index.
 * out: output coefficient structure.
 * Return 0 on success, non-zero on failure.
 */
int fm3_compute_coeffs_libm(double A, double mf, Fm3Coeffs* out)
{
    if (!out || !isfinite(A) || !isfinite(mf)) return -1;

    // glibc/libm provides j0/j1/jn, available on most Linux distributions.
    double J0 = j0(mf);
    double J1 = j1(mf);
    double J2 = jn(2, mf);

    out->A_carrier = A * J0;
    out->A_lsb1    = -A * J1; // Odd order: upper/lower sidebands are inverted.
    out->A_usb1    = +A * J1;
    out->A_lsb2    = +A * J2; // Even order: same phase.
    out->A_usb2    = +A * J2;

    return 0;
}


void printhex(void *buffer, int size, int linecnt)
{
	int i;
	unsigned char *p = (unsigned char *)buffer;	
	for(i = 0; i < size; i++) {
		if((i%linecnt == 0) && (i > 0)) {
			printf("\n");
		}
		printf("0x%02x ",p[i]);
	}
	printf("\n");
	
}

int feq(double a, double b)
{
    double diff = fabs(a - b);

    /* 1) Absolute error, used when a and b are very small. */
    if (diff <= FLOAT_EPS_REL)
        return 1;

    return 0;
}


/* Generate the shortest round-trippable decimal string, similar to Python str(double).
 * Strategy: try %.Ng from 17 significant digits downward, accepting shorter strings that round-trip with strtod.
 */
static void double_to_shortest(char out[64], double x) {
    snprintf(out, 64, "%.17g", x);
    char cand[64];
    for (int n = 16; n >= 1; --n) {
        snprintf(cand, sizeof(cand), "%.*g", n, x);
        char *endp = NULL;
        double back = strtod(cand, &endp);
        if (endp && *endp == '\0' && back == x) {
            strcpy(out, cand);
        } else {
            break;
        }
    }
}

#if defined(__SIZEOF_INT128__)

/* ===== Original 64-bit implementation: unchanged. ===== */

static void parse_decimal_fraction(const char *s, unsigned __int128 *unum, unsigned __int128 *uden) 
{
    while (*s==' '||*s=='\t') ++s;
    if (*s=='+') ++s;
    char buf[128]; strncpy(buf, s, sizeof(buf)-1); buf[sizeof(buf)-1]=0;

    char *e = strchr(buf,'e'); if(!e) e = strchr(buf,'E');
    int exp10 = 0;
    if (e) { exp10 = atoi(e+1); *e = 0; }

    char digits[64]; int di=0; int frac_len=0;
    for (const char *p=buf; *p; ++p) {
        if (*p=='.') { for (const char *q=p+1; *q; ++q) if (*q>='0'&&*q<='9') ++frac_len; continue; }
        if (*p>='0' && *p<='9') digits[di++]=*p;
    }
    if (di==0) digits[di++]='0';
    digits[di]=0;

    long k = (long)frac_len - (long)exp10; // den = 10^k
    unsigned __int128 num=0;
    for (int i=0;i<di;i++) num = num*10 + (unsigned)(digits[i]-'0');

    unsigned __int128 den=1;
    if (k>=0) { for (long i=0;i<k;i++) den *= 10; }
    else      { for (long i=0;i<-k;i++) num *= 10; }

    *unum = num; *uden = den;
}

/* Compute Q = int(Decimal(str(alpha)) * 2**96). */
static unsigned __int128 decimal_times_2p96_to_u128(const char *str_alpha)
{
    unsigned __int128 num, den;
    parse_decimal_fraction(str_alpha, &num, &den);

    unsigned __int128 ip  = num / den;
    unsigned __int128 rem = num % den;
    unsigned __int128 Q   = 0;   /* Start from 0 to accumulate the 96 fractional bits. */

    /* Generate the 96-bit fractional part floor((rem/den) * 2^96). */
    for (int i = 0; i < 96; ++i) {
        rem <<= 1;
        unsigned bit = 0;
        if (rem >= den) {
            rem -= den;
            bit = 1;
        }
        Q = (Q << 1) | bit;
    }

    /* Add the integer part: ip * 2^96. */
    Q |= (ip << 96);

    return Q;
}

#else

/* ===== 32-bit fallback: no __int128, output hi/lo directly. ===== */
static int parse_digits_u64(const char *digits, int len, uint64_t *out)
{
    uint64_t value = 0;

    for (int i = 0; i < len; ++i) {
        unsigned d = (unsigned)(digits[i] - '0');
        if (d > 9) return -1;
        if (value > (UINT64_MAX - d) / 10) return -1;
        value = value * 10 + d;
    }

    *out = value;
    return 0;
}

static int decimal_times_2p96_to_hi_lo_u64(const char *str_alpha, uint64_t *hi, uint64_t *lo)
{
    const char *s = str_alpha;
    char digits[80];
    int di = 0;
    int frac_len = 0;
    int seen_dot = 0;
    int exp10 = 0;

    while (*s == ' ' || *s == '\t') ++s;
    if (*s == '+') ++s;
    if (*s == '-') return -1;

    for (; *s && *s != 'e' && *s != 'E'; ++s) {
        if (*s == '.') {
            seen_dot = 1;
            continue;
        }
        if (*s < '0' || *s > '9') return -1;
        if (di >= (int)sizeof(digits) - 1) return -1;
        digits[di++] = *s;
        if (seen_dot) ++frac_len;
    }
    if (di == 0) digits[di++] = '0';
    digits[di] = '\0';

    if (*s == 'e' || *s == 'E') {
        char *endp = NULL;
        long exp = strtol(s + 1, &endp, 10);
        if (endp == s + 1 || *endp != '\0') return -1;
        if (exp > INT_MAX || exp < INT_MIN) return -1;
        exp10 = (int)exp;
    } else if (*s != '\0') {
        return -1;
    }

    int scale = frac_len - exp10;
    uint64_t ip = 0;
    char *rem = NULL;
    int rem_len = 0;

    if (scale <= 0) {
        if (parse_digits_u64(digits, di, &ip) != 0) return -1;
        for (int i = 0; i < -scale; ++i) {
            if (ip > UINT64_MAX / 10) return -1;
            ip *= 10;
        }
    } else {
        int int_len = di - scale;
        if (int_len > 0) {
            if (parse_digits_u64(digits, int_len, &ip) != 0) return -1;
        }

        if (ip > 0xFFFFFFFFULL) return -1;

        rem_len = scale;
        rem = (char *)malloc((size_t)rem_len + 1);
        if (!rem) return -1;
        memset(rem, '0', (size_t)rem_len);
        rem[rem_len] = '\0';

        if (int_len > 0) {
            memcpy(rem, digits + int_len, (size_t)(di - int_len));
        } else {
            memcpy(rem + (scale - di), digits, (size_t)di);
        }
    }

    uint64_t q_hi = 0;
    uint64_t q_lo = 0;

    for (int bit_idx = 0; bit_idx < 96; ++bit_idx) {
        unsigned bit = 0;

        if (rem_len > 0) {
            int carry = 0;
            for (int i = rem_len - 1; i >= 0; --i) {
                int v = (rem[i] - '0') * 2 + carry;
                rem[i] = (char)('0' + (v % 10));
                carry = v / 10;
            }
            bit = (unsigned)carry;
        }

        q_hi = (q_hi << 1) | (q_lo >> 63);
        q_lo = (q_lo << 1) | (uint64_t)bit;
    }

    free(rem);

    if (ip > 0xFFFFFFFFULL) return -1;
    q_hi |= (ip << 32);

    *hi = q_hi;
    *lo = q_lo;
    return 0;
}

#endif


int alpha_to_hi_lo(double alpha, uint64_t *hi, uint64_t *lo)
{
    if (!hi || !lo) return -1;

    setlocale(LC_NUMERIC, "C");
    fesetround(FE_TONEAREST);

    char s[64];
    double_to_shortest(s, alpha);

#if defined(__SIZEOF_INT128__)
    unsigned __int128 Q = decimal_times_2p96_to_u128(s);
    *hi = (uint64_t)(Q >> 64);
    *lo = (uint64_t)Q;
    return 0;
#else
    return decimal_times_2p96_to_hi_lo_u64(s, hi, lo);
#endif

}


/**
 * @brief Compute the first-order low-pass filter coefficient alpha.
 * @param tau Time constant in seconds.
 * @return alpha coefficient.
 */
double calc_alpha(double tau)
{
	const long double f_s	  = get_filter_sample_rate_hz();
	const long double f_c	  = 1.0L / (2.0L * M_PI * (long double)tau);
	const long double gama	  = 2.0L * M_PI * f_c / f_s;

	long double alphaL;

	if (gama > 0.001L) {
		const long double gama_cos = cosl(gama);
		// Use the same expression as the Python reference.
		alphaL = gama_cos - 1.0L + sqrtl(gama_cos * gama_cos - 4.0L * gama_cos + 3.0L);
	} else {
		alphaL = gama / (1.0L + gama);
	}

	double alpha = (double)alphaL;

	// Print both formats for comparison with the Python reference.
	printf("alpha = %.17g (%.17f)\n", alpha, alpha);

	return alpha;
}


int get_alpha_to_reg(uint64_t *alpha_high, uint64_t *alpha_low, double tau) 
{
	double alpha;
	char buf[64];
	uint64_t alpha_h, alpha_l;

	alpha = calc_alpha(tau);

	if (alpha_to_hi_lo(alpha, &alpha_h, &alpha_l) == 0) { /* Compare with the Python reference result. */
        printf("alpha_hi = 0x%llX\n",        (unsigned long long)alpha_h);
        printf("alpha_lo = 0x%016llX\n",     (unsigned long long)alpha_l);
		// Print the full 128-bit value in hexadecimal.
        printf("alpha128 = 0x%llX%016llX\n", (unsigned long long)alpha_h, (unsigned long long)alpha_l);
    } else {
        fprintf(stderr, "alpha_to_hi_lo failed\n");
    }

	*alpha_high = alpha_h;
	*alpha_low  = alpha_l;
	
	return 0;
}

int sync_filter_params(uint32_t fin, uint32_t *M1, uint32_t *M2, uint32_t *Nm)
{

	// Initial value.
   uint32_t m1 = 1;
   double threshold = 1024.0;

   if(fin == 0) {
		printf("input freq is zero\r\n");
		return -1; 
   }
   	
   // Double M1 for each lower frequency range until fin >= threshold.
   while (fin < threshold) {
	   m1 <<= 1;		   // M1 *= 2
	   threshold /= 2.0;   // Next range threshold
   }

   // Calculate M2 and Nm using the formula.
   uint32_t m2 = 8 / m1;
   if (m2 < 1) m2 = 1;

   uint32_t nm = (uint32_t)((double)get_filter_sample_rate_hz() / ((double)fin * m1));

   // Output parameters.
   *M1 = m1;
   *M2 = m2;
   *Nm = nm;

	return 0;
}



