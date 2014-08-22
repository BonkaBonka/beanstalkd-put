#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "beanstalk.h"

#define MAX_JOB_SIZE_KEY "max-job-size: "

char *tube_name = "default";
char *server_host = "localhost";
int server_port = 11300;

void display_help(char *prog)
{
	fprintf(stderr, "Usage:\n");
	fprintf(stderr, "  %s [options] [job-count]\n\n", basename(prog));
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "  -t, --tube      <tube>      beantsalk tube to put the job into (%s)\n", tube_name);
	fprintf(stderr, "  -s, --server    <server>    beanstalk server hostname (%s)\n", server_host);
	fprintf(stderr, "  -p, --port      <port>      beanstalk server port number (%u)\n", server_port);
	fprintf(stderr, "  -h, --help                  display this help\n\n");
}

int process_args(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "tube",     required_argument, 0, 't' },
		{ "server",   required_argument, 0, 's' },
		{ "port",     required_argument, 0, 'p' },
		{ "help",     no_argument,       0, 'h' },
		{ NULL,       0,                 0, 0   }
	};

	int c;

	while ((c = getopt_long(argc, argv, "t:s:p:h", long_options, NULL)) != -1) {
		switch (c) {
			case 't':
				tube_name = optarg;
				break;
			case 's':
				server_host = optarg;
				break;
			case 'p':
				server_port = atoi(optarg);
				break;
			default:
				display_help(argv[0]);
				return -1;
		}
	}

	if (optind + 1 < argc) {
		display_help(argv[0]);
		return -1;
	}

	return optind;
}

int main(int argc, char **argv) {
	int c = process_args(argc, argv);
	if (c < 1) {
		return 1;
	}

	int64_t count = 1;
	if (c < argc) {
		count = atoi(argv[c]);
	}

	int socket = bs_connect(server_host, server_port);
	if (socket == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to connect to beanstalkd on %s:%d\n", server_host, server_port);
		return 2;
	}

	if (bs_use(socket, tube_name) == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to use beanstalkd tube %s\n", tube_name);
		bs_disconnect(socket);
		return 2;
	}

	int rc = bs_kick(socket, count);
	if (rc == BS_STATUS_FAIL) {
		fprintf(stderr, "Error kicking job(s)\n");
		bs_disconnect(socket);
		return 2;
	}

	bs_disconnect(socket);

	return 0;
}
