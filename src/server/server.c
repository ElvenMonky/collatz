#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <assert.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <stdarg.h>

const uint16_t serverport = 5006;

static volatile int quit = 0;

void sigint_handler(int i)
{
	(void)i;

	quit = 1;
}

#define ERR "ERROR: "
#define WARN "WARNING: "
#define DBG "DEBUG: "
#define INFO "INFO: "

int message(const char *format, ...)
{
	va_list ap;
	time_t now = time(NULL);
	char buf[26];
	int n;

	ctime_r(&now, buf);

	buf[strlen(buf)-1] = 0;

	n = printf("[%s] ", buf);

	va_start(ap, format);
	n += vprintf(format, ap);
	va_end(ap);

	fflush(stdout);

	return n;
}

int init_sockaddr(struct sockaddr_in *name, uint16_t port)
{
	assert(name);

	bzero(name, sizeof(struct sockaddr_in));

	name->sin_family = AF_INET;
	name->sin_addr.s_addr = INADDR_ANY;
	name->sin_port = htons(port);

	return 0;
}

ssize_t write_(int fd, const char *buf, size_t count)
{
	ssize_t written = 0;

	while ((size_t)written < count) {
		ssize_t t = write(fd, buf+written, count - written);

		if (t < 0) {
			/* errno is set appropriately */
			return -1;
		}
		if (t == 0 && (size_t)(written + t) != count) {
			/* zero indicates nothing was written */
			return -1;
		}

		written += t;
	}

	return written;
}

ssize_t read_(int fd, char *buf, size_t count)
{
	ssize_t readen = 0; /* read was already taken */

	while ((size_t)readen < count) {
		ssize_t t = read(fd, buf + readen, count - readen);

		if (t < 0) {
			/*  errno is set appropriately */
			return -1;
		}
		if (t == 0 && (size_t)(readen + t) != count) {
			/* zero indicates end of file */
			return -1;
		}

		readen += t;
	}

	return readen;
}

int write_assignment_no(int fd, uint64_t n)
{
	uint32_t nh, nl;

	nh = (uint32_t)(n >> 32);
	nl = (uint32_t)(n);

	nh = htonl(nh);
	nl = ntohl(nl);

	if (write_(fd, (void *)&nh, 4) < 0) {
		return -1;
	}

	if (write_(fd, (void *)&nl, 4) < 0) {
		return -1;
	}

	return 0;
}

int read_assignment_no(int fd, uint64_t *n)
{
	uint32_t nh, nl;

	if (read_(fd, (void *)&nh, 4) < 0) {
		return -1;
	}

	if (read_(fd, (void *)&nl, 4) < 0) {
		return -1;
	}

	nh = ntohl(nh);
	nl = ntohl(nl);

	assert( n != NULL );

	*n = ((uint64_t)nh << 32) + nl;

	return 0;
}

/* 2^32 assignments */
#define ASSIGNMENTS_NO (1UL<<32)

#define MAP_SIZE (ASSIGNMENTS_NO>>3)

#define IS_ASSIGNED(n) ( ( g_map_assigned[ (n)>>3 ] >> ((n)&7) ) & 1 )
#define IS_COMPLETE(n) ( ( g_map_complete[ (n)>>3 ] >> ((n)&7) ) & 1 )

#define SET_ASSIGNED(n) ( g_map_assigned[(n)>>3] |= (1<<((n)&7)) )
#define SET_COMPLETE(n) ( g_map_complete[(n)>>3] |= (1<<((n)&7)) )

size_t g_lowest_unassigned = 0; /* bit index, not byte */
size_t g_lowest_incomplete = 0;

unsigned char *g_map_assigned;
unsigned char *g_map_complete;

void set_complete(unsigned long n)
{
	if (IS_COMPLETE(n)) {
		message(INFO "assignment %lu was already complete (duplicate result)\n", n);
	}

	if (!IS_ASSIGNED(n)) {
		message(WARN "assignment %lu was not assigned\n", n);
	}

	SET_COMPLETE(n);

	/* advance g_lowest_incomplete pointer */
	if (n == g_lowest_incomplete) {
		for (; IS_COMPLETE(g_lowest_incomplete); ++g_lowest_incomplete)
			;
	}
}

unsigned long get_assignment()
{
	unsigned long n = g_lowest_unassigned++;

	SET_ASSIGNED(n);

	return n;
}

unsigned long get_missed_assignment()
{
	unsigned long n = g_lowest_incomplete;

	SET_ASSIGNED(n);

	return n;
}

void *open_map(const char *path)
{
	int fd = open(path, O_RDWR | O_CREAT, 0600);
	void *ptr;

	if (fd < 0) {
		perror("open");
		abort();
	}

	if (ftruncate(fd, MAP_SIZE) < 0) {
		perror("ftruncate");
		abort();
	}

	ptr = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (ptr == MAP_FAILED) {
		perror("mmap");
		abort();
	}

	close(fd);

	return ptr;
}

int read_message(int fd)
{
	char msg[4];

	if (read_(fd, msg, 4) < 0) {
		return -1;
	}

	/* unsupported protocol */
	if (msg[3] != 0) {
		return -1;
	}

	if (strcmp(msg, "REQ") == 0) {
		/* requested assignment */
		unsigned long n = get_assignment();

		message(INFO "assignment requested: %lu\n", n);

		assert( sizeof(uint64_t) == sizeof(unsigned long) );

		if (write_assignment_no(fd, (uint64_t)n) < 0) {
			return -1;
		}
	} else if (strcmp(msg, "RET") == 0) {
		unsigned long n;
		/* returning assignment */

		assert( sizeof(uint64_t) == sizeof(unsigned long) );

		if (read_assignment_no(fd, (uint64_t *)&n) < 0) {
			return -1;
		}

		message(INFO "assignment returned: %lu\n", n);

		set_complete(n);
	} else if (strcmp(msg, "req") == 0) {
		/* requested lowest incomplete assignment */
		unsigned long n = get_missed_assignment();

		message(INFO "assignment requested: %lu (lowest incomplete)\n", n);

		assert( sizeof(uint64_t) == sizeof(unsigned long) );

		if (write_assignment_no(fd, (uint64_t)n) < 0) {
			return -1;
		}
	} else {
		message(ERR "%s: unknown client message!\n", msg);
		return -1;
	}

	return 0;
}

int main(/*int argc, char *argv[]*/)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in server_addr;
	int reuse = 1;

	message(INFO "starting server...\n");

	g_map_assigned = open_map("assigned.map");
	g_map_complete = open_map("complete.map");

	for (g_lowest_unassigned = 0; IS_ASSIGNED(g_lowest_unassigned); ++g_lowest_unassigned)
		;

	for (g_lowest_incomplete = 0; IS_COMPLETE(g_lowest_incomplete); ++g_lowest_incomplete)
		;

	message(INFO "lowest unassigned = %lu\n", (unsigned long)g_lowest_unassigned);
	message(INFO "lowest incomplete = %lu\n", (unsigned long)g_lowest_incomplete);

	signal(SIGINT, sigint_handler);
	signal(SIGTERM, sigint_handler);

	if (fd < 0) {
		perror("socket");
		abort();
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse, (socklen_t)sizeof(reuse)) < 0) {
		perror("setsockopt");
		abort();
	}

	init_sockaddr(&server_addr, serverport);

	if (bind(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("bind");
		abort();
	}

	if (listen(fd, 10) < 0) {
		perror("listen");
		abort();
	}

	message(INFO "listening...\n");

	while (1) {
		int cl_fd = accept(fd, NULL, NULL);

		if (-1 == cl_fd) {
			if (errno == EINTR) {
				if (quit)
					break;
			}
		}

		message(INFO "client connected\n");

		if (read_message(cl_fd) < 0) {
			message(ERR "client <--> server communication failure!\n");
		}

		close(cl_fd);

		if (quit)
			break;
	}

	message(INFO "closing server socket...\n");

	close(fd);

	msync(g_map_assigned, MAP_SIZE, MS_SYNC);
	msync(g_map_complete, MAP_SIZE, MS_SYNC);

	munmap(g_map_assigned, MAP_SIZE);
	munmap(g_map_complete, MAP_SIZE);

	return 0;
}