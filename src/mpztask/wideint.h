/**
 * Support for 128-bit integers.
 */

#ifndef WIDEINT_WIDEINT_H_
#define WIDEINT_WIDEINT_H_

/* all GNU compilers */
#ifdef __GNUC__

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-pedantic"
typedef unsigned __int128 uint128_t;
#pragma GCC diagnostic pop

#define UINT128_C(n) ( (uint128_t)n )
#define UINT128_MAX ( ~UINT128_C(0) )

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-pedantic"
typedef __int128 int128_t;
#pragma GCC diagnostic pop

#define INT128_C(n) ( (int128_t)n )
#define INT128_MAX ( ((INT128_C(1)<<(128-2))-1)*2+1 )
#define INT128_MIN ( -INT128_MAX - 1 )

#include <limits.h>

/* count trailing zeros */
__attribute__ ((unused))
static int __builtin_ctzx(uint128_t n)
{
	if ((unsigned long)n == 0)
		return (sizeof(unsigned long) * CHAR_BIT) + __builtin_ctzl((unsigned long)(n >> (sizeof(unsigned long) * CHAR_BIT)));
	else
		return __builtin_ctzl((unsigned long)n);
}

#else
#	error "Unsupported compiler"
#endif

#endif /* WIDEINT_WIDEINT_H_ */