#include <limits.h>
#include <stdio.h>

#include "beanstalk.h"

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <queue> <message>\n", argv[0]);
		return 1;
	}

	int socket = bs_connect("127.0.0.1", 11300);
	if (socket == BS_STATUS_FAIL) {
		fputs("Unable to connect to beanstalk\n", stderr);
		return 2;
	}

	if (bs_use(socket, argv[1]) == BS_STATUS_FAIL) {
		bs_disconnect(socket);
		fprintf(stderr, "Unable to use beanstalk queue %s\n", argv[1]);
		return 3;
	}

	// socket, priority (0=max), delay (seconds), time-to-run (seconds), data, sizeof(data)
	int64_t id = bs_put(socket, 0, UINT32_MAX, 60, argv[2], strlen(argv[2]));
	if (id == 0) {
		bs_disconnect(socket);
		fprintf(stderr, "Unable to put message into queue %s\n", argv[1]);
		return 4;
	}

	printf("put job id: %ld\n", id);

	bs_disconnect(socket);
	
	return 0;
}