typedef unsigned __int128 uint128_t;

#define UINT128_MAX (~(uint128_t)0)

uint pow3(size_t n)
{
	uint r = 1;
	uint b = 3;

	while (n) {
		if (n & 1) {
			r *= b;
		}
		b *= b;
		n >>= 1;
	}

	return r;
}

#define LUT_SIZE32 21

#define SIEVE_LOGSIZE 16
#define SIEVE_SIZE ((1UL << SIEVE_LOGSIZE) / 8)
#define SIEVE_MASK ((1UL << SIEVE_LOGSIZE) - 1)
#define IS_LIVE(n) ( ( local_sieve[ (n)>>3 ] >> ((n)&7) ) & 1 )

__kernel void worker(
	__global ulong *checksum_alpha,
	__global ulong *lbegin,
	__global ulong *hbegin,
	__global ulong *lsup,
	__global ulong *hsup,
	__global uchar *sieve,
	__global ulong *mxoffset
)
{
	ulong private_checksum_alpha = 0;
	size_t id = get_global_id(0);

	__local uint lut[LUT_SIZE32];

	__local uchar local_sieve[SIEVE_SIZE];

	if (get_local_id(0) == 0) {
		for (size_t i = 0; i < LUT_SIZE32; ++i) {
			lut[i] = pow3(i);
		}

		for (size_t i = 0; i < SIEVE_SIZE; ++i) {
			local_sieve[i] = sieve[i];
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	uint128_t n_begin = ((uint128_t)hbegin[id] << 64) + (uint128_t)lbegin[id];
	uint128_t n_supremum = ((uint128_t)hsup[id] << 64) + (uint128_t)lsup[id];

	uint128_t max_n = 0;
	uint128_t max_n0;

	for (uint128_t n0 = n_begin; n0 < n_supremum; n0 += 4) {
		uint128_t n = n0;

		if (!IS_LIVE(n0 & SIEVE_MASK)) {
			continue;
		}

		do {
			n++;

			size_t alpha = min((size_t)ctz((uint)n), (size_t)LUT_SIZE32 - 1);

			private_checksum_alpha += alpha;
			n >>= alpha;

			if (n > UINT128_MAX >> 2*alpha) {
				private_checksum_alpha = 0;
				goto end;
			}

			n *= lut[alpha];

			n--;

			if (n > max_n) {
				max_n = n;
				max_n0 = n0;
			}

			n >>= ctz((uint)n);
		} while (n >= n0);
	}

end:
	checksum_alpha[id] = private_checksum_alpha;
	mxoffset[id] = (ulong)(max_n0 - n_begin);
}
