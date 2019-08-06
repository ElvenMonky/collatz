/**
 * @file
 * @brief Simple program that check convergence of the Collatz problem.
 *
 * @author David Barina <ibarina@fit.vutbr.cz>
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "wideint.h"

#define LUT_SIZE 41
#define LUT_SIZE128 81

unsigned long g_lut[LUT_SIZE];

uint128_t g_lut128[LUT_SIZE128];

/* 3^n, in runtime */
unsigned long lut_rt(unsigned long n)
{
	unsigned long r = 1;

	for (; n > 0; --n) {
		assert( r <= ULONG_MAX / 3 );

		r *= 3;
	}

	return r;
}

uint128_t lut_rt128(uint128_t n)
{
	uint128_t r = 1;

	for (; n > 0; --n) {
		assert( r <= UINT128_MAX / 3 );

		r *= 3;
	}

	return r;
}

void init_lut()
{
	unsigned long a;

	for (a = 0; a < LUT_SIZE; ++a) {
		g_lut[a] = lut_rt(a);
	}

	for (a = 0; a < LUT_SIZE128; ++a) {
		g_lut128[a] = lut_rt128(a);
	}
}

/* 3^n */
static unsigned long lut(unsigned long n)
{
	assert( n < LUT_SIZE );

	return g_lut[n];
}

static uint128_t lut128(uint128_t n)
{
	assert( n < LUT_SIZE128 );

	return g_lut128[n];
}

/* check convergence */
void check(unsigned long n)
{
	unsigned long n0 = n/2;

	do {
		unsigned long e;

		n >>= __builtin_ctzl(n);

		/* 6n+2 < n0 (all numbers below n0 have already been checked) */
		if (n < n0 && n%3 == 1)
			return;

		n++;

		e = __builtin_ctzl(n);
		n >>= e;
		assert( n <= ULONG_MAX >> 2*e );
		n *= lut(e);

		n--;
	} while (1);
}

static uint128_t ctz128(uint128_t n)
{
	switch (sizeof(uint128_t)) {
		case 2*sizeof(unsigned long): {
			if ((unsigned long)n == 0)
				return (sizeof(unsigned long) * CHAR_BIT) + __builtin_ctzl((unsigned long)(n >> (sizeof(unsigned long) * CHAR_BIT)));
			else
				return __builtin_ctzl((unsigned long)n);
		}
		default:
			abort();
	}
}

void prescreen128(uint128_t n, uint128_t n_sup, uint128_t e)
{
	uint128_t n0 = n;
	uint128_t e0 = e;

goto entry;
	do {
		n >>= ctz128(n);

		if (n == 1)
			return;

		n++;

		e = ctz128(n);
		n >>= e;

		/* now we have (n,e) pair */

		/* all (n,e) with n < n_sup and e < e0 have already been checked */
		if ( (n < n_sup && e < e0) || (n < n0 && e == e0) )
			return;
entry:

		assert( n <= UINT128_MAX >> 2*e );
		n *= lut128(e);

		n--;
	} while (1);
}

void prescreen(unsigned long n, unsigned long n_sup, unsigned long e)
{
	unsigned long n0 = n;
	unsigned long e0 = e;

goto entry;
	do {
		n >>= __builtin_ctzl(n);

		if (n == 1)
			return;

		n++;

		e = __builtin_ctzl(n);
		n >>= e;

		/* now we have (n,e) pair */

		/* all (n,e) with n < n_sup and e < e0 have already been checked */
		if ( (n < n_sup && e < e0) || (n < n0 && e == e0) )
			return;
entry:
		if ( n > ULONG_MAX >> 2*e) {
			prescreen128(n0, n_sup, e0);
			return;
		}
		assert( n <= ULONG_MAX >> 2*e );
		if ( e >= LUT_SIZE ) {
			prescreen128(n0, n_sup, e0);
			return;
		}
		n *= lut(e);

		n--;
	} while (1);
}

int main(int argc, char *argv[])
{
	unsigned long e;
	unsigned long n;
	unsigned long n_sup = (argc > 1) ? (unsigned long)atol(argv[1]) : (1UL<<32);

	assert( ctz128( INT128_C(1)<<0 ) == 0 );
	assert( ctz128( INT128_C(1)<<1 ) == 1 );
	assert( ctz128( INT128_C(1)<<15 ) == 15 );
	assert( ctz128( INT128_C(1)<<35 ) == 35 );
	assert( ctz128( INT128_C(1)<<65 ) == 65 );
	assert( ctz128( INT128_C(1)<<127 ) == 127 );

	init_lut();
#if 0
	/* trajectory of all numbers of the form 2l+0 passes (in finitely many steps) through some 2m+1 < 2l+0 */
	/* trajectory of all numbers of the form 2m+1 passes (in finitely many steps) through some 6n+2 > 2m+1 */
	/* therefore, check only the numbers the form 6n+2, in order from the smallest one to the largest one */
	for (n = 6+2; n < n_max; n += 6) {
		check(n);
	}
#endif

	/* for each e */
	for (e = 1; ; ++e) {
		printf("e = %lu...\n", e);
		/* for each odd n */
		for (n = 1; n < n_sup; n += 2) {
			/* check */
			prescreen(n, n_sup, e);
		}
	}

	return 0;
}
