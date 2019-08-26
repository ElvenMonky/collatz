#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#ifdef _OPENMP
	#include <omp.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>

int threads_get_thread_id()
{
#ifdef _OPENMP
	return omp_get_thread_num();
#else
	return 0;
#endif
}

const char *servername = "pcbarina2.fit.vutbr.cz";

void init_sockaddr(struct sockaddr_in *name, const char *hostname, uint16_t port)
{
	struct hostent *hostinfo;

	assert(name);

	bzero(name, sizeof(struct sockaddr_in));

	name->sin_family = AF_INET;
	name->sin_port = htons(port);

	hostinfo = gethostbyname(hostname);

	if (hostinfo == NULL) {
		fprintf(stderr, "gethostbyname failed: %s\n", servername);
		abort();
	}

	name->sin_addr = *(struct in_addr *) hostinfo->h_addr_list[0];
}

ssize_t write_(int fd, const void *buf, size_t count)
{
	ssize_t written = 0;

	while ((size_t)written < count) {
		ssize_t t = write(fd, buf, count);
		if (-1 == t) {
			/* errno is set appropriately */
			perror("write");
			abort();
		}
		written += t;
	}

	return written;
}

int main(int argc, char *argv[])
{
	int threads = (argc > 1) ? atoi(argv[1]) : 1;

	assert(threads > 0);

	printf("starting %u worker threads...\n", threads);

	#pragma omp parallel num_threads(threads)
	{
		printf("thread %i started\n", threads_get_thread_id());

		do {
			int fd = socket(AF_INET, SOCK_STREAM, 0);
			struct sockaddr_in server_addr;
			char buffer[4096];

			if (fd < 0) {
				/* errno is set appropriately */
				perror("socket");
				abort();
			}

			init_sockaddr(&server_addr, servername, 5006);

			if (connect(fd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
				/* errno is set appropriately */
				perror("connect");
				abort();
			}

			/* give me the assignment */
			strcpy(buffer, "ASSIGNMENT");
			write_(fd, buffer, strlen(buffer)+1);

			/* TODO get assignment from server */
			/* TODO spawn sub-process */
			/* TODO send the result back to server */

			close(fd);
		} while (1);
	}

	return 0;
}
