#include <getopt.h>
#include <limits.h>
#include <stdio.h>

#include "beanstalk.h"

char *tube_name = "default";
char *server_host = "localhost";
int server_port = 11300;
// Priority 0 is max, UINT32_MAX is lowest
// See: https://github.com/kr/beanstalkd/blob/v1.3/doc/protocol.txt#l135
uint32_t job_priority = UINT32_MAX;
uint32_t job_delay = 0;
uint32_t job_ttr = 120;

void display_help()
{
	puts("Usage:");
	puts("  bsc [options] <message>");
	puts("");
	puts("Options:");
	puts("  -t, --tube      <tube>      beantsalk tube to put the message into");
	puts("  -s, --server    <server>    beanstalk server hostname");
	puts("  -p, --port      <port>      beanstalk server port number");
	puts("  -P, --priority  <priority>  message priority (0=max, UINT32_MAX=min)");
	puts("  -D, --delay     <delay>     how long before the message becomes available");
	puts("  -T, --ttr       <ttr>       how long to give the message processor");
	puts("  -h, --help                  display this help");
}

int process_args(int argc, char **argv)
{
	static struct option long_options[] = {
		{ "tube",     required_argument, 0, 't' },
		{ "server",   required_argument, 0, 's' },
		{ "port",     required_argument, 0, 'p' },
		{ "priority", required_argument, 0, 'P' },
		{ "delay",    required_argument, 0, 'D' },
		{ "ttr",      required_argument, 0, 'T' },
		{ "help",     no_argument,       0, 'h' },
		{ NULL,       0,                 0, 0   }
	};

	int c;

	while((c = getopt_long(argc, argv, "t:s:p:P:D:T:h", long_options, NULL)) != -1)
	{
		switch(c)
		{
			case 't':
				tube_name = optarg;
				break;
			case 's':
				server_host = optarg;
				break;
			case 'p':
				server_port = atoi(optarg);
				break;
			case 'P':
				job_priority = atoi(optarg);
				break;
			case 'D':
				job_delay = atoi(optarg);
				break;
			case 'T':
				job_ttr = atoi(optarg);
				break;
			default:
				display_help();
				return -1;
		}
	}

	if(optind >= argc || optind + 1 < argc)
	{
		display_help();
		return -1;
	}

	return optind;
}

int main(int argc, char **argv) {
	int c = process_args(argc, argv);
	if(c < 1)
	{
		return 1;
	}

	int socket = bs_connect(server_host, server_port);
	if (socket == BS_STATUS_FAIL) {
		fprintf(stderr, "Unable to connect to beanstalk %s:%d\n", server_host, server_port);
		return 2;
	}

	if (bs_use(socket, tube_name) == BS_STATUS_FAIL) {
		bs_disconnect(socket);
		fprintf(stderr, "Unable to use beanstalk tube %s\n", tube_name);
		return 3;
	}

	int64_t id = bs_put(socket, job_priority, job_delay, job_ttr, argv[c], strlen(argv[c]));
	if (id == 0) {
		bs_disconnect(socket);
		fprintf(stderr, "Unable to put message into tube %s\n", argv[c]);
		return 4;
	}

	printf("put job id: %ld\n", id);

	bs_disconnect(socket);
	
	return 0;
}
