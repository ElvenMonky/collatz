#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "wideint.h"

/* 2^32 assignments (tasks) */
#define ASSIGNMENTS_NO (UINT64_C(1) << 32)

#define CHECKSUMS_SIZE (ASSIGNMENTS_NO * 8)
#define USERTIMES_SIZE (ASSIGNMENTS_NO * 8)
#define OVERFLOWS_SIZE (ASSIGNMENTS_NO * 8)
#define CLIENTIDS_SIZE (ASSIGNMENTS_NO * 8)

const uint64_t *open_checksums()
{
	const char *path = "checksums.dat";
	int fd = open(path, O_RDONLY, 0600);
	const void *ptr;

	if (fd < 0) {
		perror("open");
		abort();
	}

	ptr = mmap(NULL, (size_t)CHECKSUMS_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	close(fd);

	return (const uint64_t *)ptr;
}

const uint64_t *open_usertimes()
{
	const char *path = "usertimes.dat";
	int fd = open(path, O_RDONLY, 0600);
	const void *ptr;

	if (fd < 0) {
		perror("open");
		abort();
	}

	ptr = mmap(NULL, (size_t)USERTIMES_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	close(fd);

	return (const uint64_t *)ptr;
}

const uint64_t *open_overflows()
{
	const char *path = "overflows.dat";
	int fd = open(path, O_RDONLY, 0600);
	const void *ptr;

	if (fd < 0) {
		perror("open");
		abort();
	}

	ptr = mmap(NULL, (size_t)OVERFLOWS_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	close(fd);

	return (const uint64_t *)ptr;
}

const uint64_t *open_clientids()
{
	const char *path = "clientids.dat";
	int fd = open(path, O_RDONLY, 0600);
	const void *ptr;

	if (fd < 0) {
		perror("open");
		abort();
	}

	ptr = mmap(NULL, (size_t)CLIENTIDS_SIZE, PROT_READ, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	close(fd);

	return (const uint64_t *)ptr;
}

const uint64_t *g_checksums = 0;
const uint64_t *g_usertimes = 0;
const uint64_t *g_overflows = 0;
const uint64_t *g_clientids = 0;

#define MIN(a, b) ( ((a) < (b)) ? (a): (b) )
#define MAX(a, b) ( ((a) > (b)) ? (a): (b) )

int main()
{
	uint64_t n;
	uint64_t min = UINT64_MAX, max = 0;
	uint128_t total_user_time = 0;
	uint64_t user_time_count = 0;
	int c = 0;
	int overflow_found = 0;
	uint64_t clientids_count = 0;

	g_checksums = open_checksums();
	g_usertimes = open_usertimes();
	g_overflows = open_overflows();
	g_clientids = open_clientids();

	for (n = 0; n < ASSIGNMENTS_NO; ++n) {
		uint64_t checksum = g_checksums[n];

		if (checksum != 0) {
			min = MIN(min, checksum);
			max = MAX(max, checksum);
		}
	}

	printf("min = %" PRIu64 " (0x%" PRIx64 "); min>>24 = %" PRIu64 " (0x%" PRIx64 "); min>>23 = %" PRIu64 " (0x%" PRIx64 ")\n",
		min, min, min>>24, min>>24, min>>23, min>>23);
	printf("max = %" PRIu64 " (0x%" PRIx64 "); max>>24 = %" PRIu64 " (0x%" PRIx64 "); max>>23 = %" PRIu64 " (0x%" PRIx64 ")\n",
		max, max, max>>24, max>>24, max>>23, max>>23);

	printf("\n");

	for (n = 91226112; n < ASSIGNMENTS_NO; ++n) {
		uint64_t checksum = g_checksums[n];

		if (checksum == 0) {
			printf("missing checksum on the assignment %" PRIu64 "\n", n);

			if (++c == 24)
				break;
		}
	}

	printf("\n");

	for (n = 0; n < ASSIGNMENTS_NO; ++n) {
		uint64_t user_time = g_usertimes[n];

		if (user_time != 0) {
			total_user_time += user_time;
			user_time_count++;
		}
	}

	if (user_time_count > 0) {
		uint64_t avg_user_time = (uint64_t) ((total_user_time+(user_time_count/2)) / user_time_count);

		printf("recorded user times: %" PRIu64 "\n", user_time_count);
		printf("total user time: %" PRIu64 " hours = %" PRIu64 " days = %" PRIu64 " years\n",
			(uint64_t)((total_user_time+1800)/3600) /* hours (60*60) */,
			(uint64_t)((total_user_time+43200)/86400), /* days (60*60*24) */
			(uint64_t)((total_user_time+15768000)/31536000) /* years (60*60*24*365) */
		);
		printf("average user time: %" PRIu64 ":%02" PRIu64 ":%02" PRIu64 " (h:m:s)\n", (uint64_t)(avg_user_time/60/60), (uint64_t)(avg_user_time/60%60), (uint64_t)(avg_user_time%60));
	}

	printf("\n");

	for (n = 0; n < ASSIGNMENTS_NO; ++n) {
		uint64_t overflow = g_overflows[n];

		if (overflow != 0) {
			overflow_found = 1;
		}
	}

	printf("overflow found: %s\n", overflow_found ? "yes" : "no");
	printf("\n");

	for (n = 0; n < ASSIGNMENTS_NO; ++n) {
		uint64_t clid = g_clientids[n];

		if (clid != 0) {
			clientids_count++;
		}
	}

	printf("found %" PRIu64 " active assignments (client IDs)\n", clientids_count);

	return 0;
}
